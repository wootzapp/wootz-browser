// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/ipc/service/image_transport_surface_overlay_mac.h"

#include <memory>
#include <sstream>

#include "base/command_line.h"
#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/metrics/histogram_macros.h"
#include "base/task/single_thread_task_runner.h"
#include "base/time/time.h"
#include "base/trace_event/trace_event.h"
#include "components/viz/common/features.h"
#include "gpu/command_buffer/common/swap_buffers_complete_params.h"
#include "gpu/ipc/service/gpu_channel_manager.h"
#include "gpu/ipc/service/gpu_channel_manager_delegate.h"
#include "gpu/ipc/service/image_transport_surface_delegate.h"
#include "ui/accelerated_widget_mac/ca_layer_tree_coordinator.h"
#include "ui/base/cocoa/remote_layer_api.h"
#include "ui/base/ui_base_switches.h"
#include "ui/gfx/geometry/rect_conversions.h"
#include "ui/gfx/gpu_fence.h"
#include "ui/gfx/overlay_plane_data.h"
#include "ui/gfx/video_types.h"
#include "ui/gl/ca_renderer_layer_params.h"
#include "ui/gl/gl_features.h"

#if BUILDFLAG(IS_MAC)
#include "ui/accelerated_widget_mac/io_surface_context.h"
#include "ui/gl/gl_context.h"
#endif

// From ANGLE's EGL/eglext_angle.h. This should be included instead of being
// redefined here.
#ifndef EGL_ANGLE_device_metal
#define EGL_ANGLE_device_metal 1
#define EGL_METAL_DEVICE_ANGLE 0x34A6
#endif /* EGL_ANGLE_device_metal */

namespace gpu {

namespace {
constexpr base::TimeDelta kHistogramMinTime = base::Microseconds(5);
constexpr base::TimeDelta kHistogramMaxTime = base::Milliseconds(16);
constexpr int kHistogramTimeBuckets = 50;

// Control use of AVFoundation to draw video content.
BASE_FEATURE(kAVFoundationOverlays,
             "avfoundation-overlays",
             base::FEATURE_ENABLED_BY_DEFAULT);

#if BUILDFLAG(IS_MAC)
// Whether the presentation should be delayed until the next CVDisplayLink
// callback when kCVDisplayLinkBeginFrameSource is enabled. This flag has no
// effect if kCVDisplayLinkBeginFrameSource is disabled.
BASE_FEATURE(kVSyncAlignedPresent,
             "VSyncAlignedPresent",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Whether the presentation for the first frame after VSync stops should be
// delayed when kVSyncAlignedPresent is enabled.
BASE_FEATURE(kNoDelayOnFirstFramePresent,
             "NoDelayOnFirstFramePresent",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Use CVDisplayLink timing for PresentationFeedback timestamps.
BASE_FEATURE(kNewPresentationFeedbackTimeStamps,
             "NewPresentationFeedbackTimeStamps",
             base::FEATURE_DISABLED_BY_DEFAULT);
#endif  // BUILDFLAG(IS_MAC)
}  // namespace

ImageTransportSurfaceOverlayMacEGL::ImageTransportSurfaceOverlayMacEGL()
    : use_remote_layer_api_(ui::RemoteLayerAPISupported()),
      scale_factor_(1),
      weak_ptr_factory_(this) {
  static bool av_disabled_at_command_line =
      !base::FeatureList::IsEnabled(kAVFoundationOverlays);

  ca_layer_tree_coordinator_ = std::make_unique<ui::CALayerTreeCoordinator>(
      use_remote_layer_api_, !av_disabled_at_command_line);

  // Create the CAContext to send this to the GPU process, and the layer for
  // the context.
  if (use_remote_layer_api_) {
#if BUILDFLAG(IS_MAC)
    CGSConnectionID connection_id = CGSMainConnectionID();
    ca_context_ = [CAContext contextWithCGSConnection:connection_id
                                              options:@{}];
#else
    // Use a very large display ID to ensure that the context is never put
    // on-screen without being explicitly parented.
    ca_context_ = [CAContext remoteContextWithOptions:@{
      kCAContextIgnoresHitTest : @YES,
      kCAContextDisplayId : @10000
    }];
#endif
    ca_context_.layer = ca_layer_tree_coordinator_->GetCALayerForDisplay();
  }
}

ImageTransportSurfaceOverlayMacEGL::~ImageTransportSurfaceOverlayMacEGL() {
  ca_layer_tree_coordinator_.reset();
}

void ImageTransportSurfaceOverlayMacEGL::ApplyBackpressure() {
  TRACE_EVENT0("gpu", "ImageTransportSurfaceOverlayMac::ApplyBackpressure");
  // Create the fence for the current frame before waiting on the previous
  // frame's fence (to maximize CPU and GPU execution overlap).
  gl::GLContext* current_context = gl::GLContext::GetCurrent();
  if (current_context) {
    uint64_t this_frame_fence = current_context->BackpressureFenceCreate();
    current_context->BackpressureFenceWait(previous_frame_fence_);
    previous_frame_fence_ = this_frame_fence;
  }
}

void ImageTransportSurfaceOverlayMacEGL::BufferPresented(
    PresentationCallback callback,
    const gfx::PresentationFeedback& feedback) {
  DCHECK(!callback.is_null());
  std::move(callback).Run(feedback);
}

void ImageTransportSurfaceOverlayMacEGL::Present(
    SwapCompletionCallback completion_callback,
    PresentationCallback presentation_callback,
    gfx::FrameData data) {
  TRACE_EVENT0("gpu", "ImageTransportSurfaceOverlayMac::Present");

  // Only one committed CALayer tree is permitted. Populate the previous frame
  // if there is already a committed CALayer tree waiting to be populated. At
  // the end of this function, another committed CALayer tree will be produced.
  if (num_committed_ca_layer_trees_ >= 1) {
    PopulateCALayerParameters();
  }
  DCHECK_EQ(num_committed_ca_layer_trees_, 0);

  // Query the underlying Metal device, if one exists. This is needed to ensure
  // synchronization between the display compositor and the HDRCopierLayer.
  // https://crbug.com/1372898
  if (gl::GLDisplayEGL* display =
          gl::GLDisplayEGL::GetDisplayForCurrentContext()) {
    EGLAttrib angle_device_attrib = 0;
    if (eglQueryDisplayAttribEXT(display->GetDisplay(), EGL_DEVICE_EXT,
                                 &angle_device_attrib)) {
      EGLDeviceEXT angle_device =
          reinterpret_cast<EGLDeviceEXT>(angle_device_attrib);
      EGLAttrib metal_device_attrib = 0;
      if (eglQueryDeviceAttribEXT(angle_device, EGL_METAL_DEVICE_ANGLE,
                                  &metal_device_attrib)) {
        id<MTLDevice> metal_device = (__bridge id)(void*)metal_device_attrib;
        ca_layer_tree_coordinator_->GetPendingCARendererLayerTree()
            ->SetMetalDevice(metal_device);
      }
    }
  }

#if BUILDFLAG(IS_MAC)
  // The GPU has finished all the drawing commands.
  ready_timestamp_ = base::TimeTicks::Now();
#endif

  completion_callback_ = std::move(completion_callback);
  presentation_callback_ = std::move(presentation_callback);
  num_committed_ca_layer_trees_++;

#if BUILDFLAG(IS_MAC)
  // With display_link_mac_, delay the presentation until next VSync if
  // 1) It's not the first frame or
  // 2) We allow delay on all frames including the first frame.
  bool delay_presenetation_until_next_vsync =
      !!vsync_callback_mac_ ||
      !base::FeatureList::IsEnabled(kNoDelayOnFirstFramePresent);
  if (display_link_mac_ && !vsync_callback_mac_) {
    vsync_callback_mac_ = display_link_mac_->RegisterCallback(
        base::BindRepeating(
            &ImageTransportSurfaceOverlayMacEGL::OnVSyncPresentation,
            weak_ptr_factory_.GetWeakPtr()),
        /*do_callback_on_register_thread=*/true);
  }

  // To avoid FID (First Input Delay), delay PopulateCALayerParameters only if
  // this is not the first frame after vsync stops.
  if (vsync_callback_mac_) {
    vsync_callback_mac_keep_alive_counter_ = kMaxKeepAliveCounter;
    if (delay_presenetation_until_next_vsync &&
        base::FeatureList::IsEnabled(kVSyncAlignedPresent)) {
      // PopulateCALayerParameters will be called in OnVSyncPresentation.
      return;
    }
  }
#endif

  PopulateCALayerParameters();
}

void ImageTransportSurfaceOverlayMacEGL::PopulateCALayerParameters() {
  // Do a GL fence for flush to apply back-pressure before drawing.
  {
    base::TimeTicks start_time = base::TimeTicks::Now();
    ApplyBackpressure();

    UMA_HISTOGRAM_CUSTOM_MICROSECONDS_TIMES(
        "Gpu.Mac.BackpressureUs", base::TimeTicks::Now() - start_time,
        kHistogramMinTime, kHistogramMaxTime, kHistogramTimeBuckets);
  }

  // Update the CALayer tree in the GPU process.
  {
    base::TimeTicks before_transaction_time = base::TimeTicks::Now();
    TRACE_EVENT0("gpu", "CommitPendingTreesToCA");
    ca_layer_tree_coordinator_->CommitPendingTreesToCA();

    base::TimeDelta transaction_time =
        base::TimeTicks::Now() - before_transaction_time;
    UMA_HISTOGRAM_CUSTOM_MICROSECONDS_TIMES(
        "GPU.IOSurface.CATransactionTimeUs", transaction_time,
        kHistogramMinTime, kHistogramMaxTime, kHistogramTimeBuckets);

#if BUILDFLAG(IS_MAC)
    latch_timestamp_ = base::TimeTicks::Now();
#endif
  }

  // Populate the CA layer parameters to send to the browser.
  gfx::CALayerParams params;
  {
    TRACE_EVENT_INSTANT2("test_gpu", "SwapBuffers", TRACE_EVENT_SCOPE_THREAD,
                         "GLImpl", static_cast<int>(gl::GetGLImplementation()),
                         "width", pixel_size_.width());
    if (use_remote_layer_api_) {
      params.ca_context_id = [ca_context_ contextId];
    } else {
      IOSurfaceRef io_surface =
          ca_layer_tree_coordinator_->GetIOSurfaceForDisplay();
      if (io_surface) {
        params.io_surface_mach_port.reset(IOSurfaceCreateMachPort(io_surface));
      }
    }
    params.pixel_size = pixel_size_;
    params.scale_factor = scale_factor_;
    params.is_empty = false;
  }

  // Send the swap parameters to the browser.
  if (completion_callback_) {
    base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE,
        base::BindOnce(std::move(completion_callback_),
                       gfx::SwapCompletionResult(
                           gfx::SwapResult::SWAP_ACK,
                           std::make_unique<gfx::CALayerParams>(params))));
  }

  gfx::PresentationFeedback feedback(base::TimeTicks::Now(), base::Hertz(60),
                                     /*flags=*/0);
  feedback.ca_layer_error_code = ca_layer_error_code_;

#if BUILDFLAG(IS_MAC)
  if (base::FeatureList::IsEnabled(kNewPresentationFeedbackTimeStamps)) {
    feedback.ready_timestamp = ready_timestamp_;
    feedback.latch_timestamp = latch_timestamp_;
    feedback.interval = frame_interval_;
    feedback.timestamp = GetDisplaytime(latch_timestamp_);

    // `update_vsync_params_callback` is not available in
    // SkiaOutputSurfaceImpl::BufferPresented(). Setting kVSync here will not
    // update vsync params.
    feedback.flags = gfx::PresentationFeedback::kHWCompletion |
                     gfx::PresentationFeedback::kVSync;
  }
#endif

  base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE,
      base::BindOnce(&ImageTransportSurfaceOverlayMacEGL::BufferPresented,
                     weak_ptr_factory_.GetWeakPtr(),
                     std::move(presentation_callback_), feedback));

  num_committed_ca_layer_trees_--;
}

bool ImageTransportSurfaceOverlayMacEGL::ScheduleOverlayPlane(
    gl::OverlayImage image,
    std::unique_ptr<gfx::GpuFence> gpu_fence,
    const gfx::OverlayPlaneData& overlay_plane_data) {
  if (absl::get<gfx::OverlayTransform>(overlay_plane_data.plane_transform) !=
      gfx::OVERLAY_TRANSFORM_NONE) {
    DLOG(ERROR) << "Invalid overlay plane transform.";
    return false;
  }
  if (overlay_plane_data.z_order) {
    DLOG(ERROR) << "Invalid non-zero Z order.";
    return false;
  }
  // TODO(1290313): the display_bounds might not need to be rounded to the
  // nearest rect as this eventually gets made into a CALayer. CALayers work in
  // floats.
  const ui::CARendererLayerParams overlay_as_calayer_params(
      /*is_clipped=*/false,
      /*clip_rect=*/gfx::Rect(),
      /*rounded_corner_bounds=*/gfx::RRectF(),
      /*sorting_context_id=*/0, gfx::Transform(), image,
      overlay_plane_data.color_space,
      /*contents_rect=*/overlay_plane_data.crop_rect,
      /*rect=*/gfx::ToNearestRect(overlay_plane_data.display_bounds),
      /*background_color=*/SkColors::kTransparent,
      /*edge_aa_mask=*/0,
      /*opacity=*/1.f,
      /*nearest_neighbor_filter=*/GL_LINEAR,
      /*hdr_metadata=*/gfx::HDRMetadata(),
      /*protected_video_type=*/gfx::ProtectedVideoType::kClear,
      /*is_render_pass_draw_quad=*/false);

  return ca_layer_tree_coordinator_->GetPendingCARendererLayerTree()
      ->ScheduleCALayer(overlay_as_calayer_params);
}

bool ImageTransportSurfaceOverlayMacEGL::ScheduleCALayer(
    const ui::CARendererLayerParams& params) {
  return ca_layer_tree_coordinator_->GetPendingCARendererLayerTree()
      ->ScheduleCALayer(params);
}

bool ImageTransportSurfaceOverlayMacEGL::Resize(
    const gfx::Size& pixel_size,
    float scale_factor,
    const gfx::ColorSpace& color_space,
    bool has_alpha) {
  pixel_size_ = pixel_size;
  scale_factor_ = scale_factor;
  ca_layer_tree_coordinator_->Resize(pixel_size, scale_factor);
  return true;
}

void ImageTransportSurfaceOverlayMacEGL::SetCALayerErrorCode(
    gfx::CALayerResult ca_layer_error_code) {
  ca_layer_error_code_ = ca_layer_error_code;
}

void ImageTransportSurfaceOverlayMacEGL::SetMaxPendingSwaps(
    int max_pending_swaps) {
  cap_max_pending_swaps_ = max_pending_swaps;
}

#if BUILDFLAG(IS_MAC)
void ImageTransportSurfaceOverlayMacEGL::SetVSyncDisplayID(int64_t display_id) {
  if (!(base::FeatureList::IsEnabled(
            features::kCVDisplayLinkBeginFrameSource) ||
        base::FeatureList::IsEnabled(kVSyncAlignedPresent) ||
        base::FeatureList::IsEnabled(kNewPresentationFeedbackTimeStamps))) {
    return;
  }

  if ((!display_link_mac_ || display_id != display_id_) &&
      display_id != display::kInvalidDisplayId) {
    // Call PopulateCALayerParameters if there is a pending frame.
    if (vsync_callback_mac_) {
      // Set the keep_alive_counter to the last one so vsync_callback_mac_ will
      // be destroyed.
      vsync_callback_mac_keep_alive_counter_ =
          num_committed_ca_layer_trees_ ? 0 : 1;
      OnVSyncPresentation(ui::VSyncParamsMac());
      DCHECK(!vsync_callback_mac_);
    }

    display_link_mac_ = ui::DisplayLinkMac::GetForDisplay(display_id);
  }
  display_id_ = display_id;
}

base::TimeTicks ImageTransportSurfaceOverlayMacEGL::GetDisplaytime(
    base::TimeTicks latch_time) {
  // From the CVDisplayLink params dump:
  // |next_display_time_| ~= |current_display_time_| + |frame_interval|.
  // params.display_time ~= params.callback_time + 1.5x |frame_interval|.

  // From the experiment, frames committed before (|current_display_time_| - 1.5
  // ms) will be displayed at the next display time. 1.5 ms is roughly the safe
  // zone for the latch deadline. The result is inconsistent in the experiment
  // if commit is too close to the display_time.
  constexpr base::TimeDelta kLatchBufferTime = base::Microseconds(1500);
  auto latch_deadline_for_next_display =
      current_display_time_ - kLatchBufferTime;
  if (latch_time < latch_deadline_for_next_display) {
    return next_display_time_;
  }

  // We just missed the |current_display_time|, the display will be at the next
  // one after |next_display_time_|.
  if (!frame_interval_.is_zero() && next_display_time_ != base::TimeTicks()) {
    base::TimeTicks present_time =
        latch_time.SnappedToNextTick(next_display_time_ - kLatchBufferTime,
                                     frame_interval_) +
        kLatchBufferTime + frame_interval_;
    return present_time;
  }

  // When there is no display_time info, just use the latch_time.
  // This only happens at the very first frame after the browser starts,
  return latch_time;
}

// The CVDisplayLink callback on the GPU thread.
void ImageTransportSurfaceOverlayMacEGL::OnVSyncPresentation(
    ui::VSyncParamsMac params) {
  // Documentation for the CVDisplayLink display_time
  // https://developer.apple.com/documentation/corevideo/cvdisplaylinkoutputcallback

  current_display_time_ = next_display_time_;

  if (params.display_times_valid) {
    next_display_time_ = params.display_timebase;
    frame_interval_ = params.display_interval;
  }

  if (num_committed_ca_layer_trees_) {
    PopulateCALayerParameters();
  } else {
    DCHECK(vsync_callback_mac_keep_alive_counter_ > 0);
    vsync_callback_mac_keep_alive_counter_ -= 1;
  }

  if (vsync_callback_mac_keep_alive_counter_ == 0) {
    vsync_callback_mac_ = nullptr;
  }
}
#endif

}  // namespace gpu
