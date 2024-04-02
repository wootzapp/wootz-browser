// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/capture_mode/camera_video_frame_handler.h"

#include "base/check.h"
#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/logging.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/shared_memory_mapping.h"
#include "base/memory/unsafe_shared_memory_region.h"
#include "base/system/sys_info.h"
#include "components/viz/common/gpu/context_lost_observer.h"
#include "components/viz/common/gpu/context_provider.h"
#include "gpu/command_buffer/client/client_shared_image.h"
#include "gpu/command_buffer/client/gpu_memory_buffer_manager.h"
#include "gpu/command_buffer/client/shared_image_interface.h"
#include "gpu/command_buffer/common/context_result.h"
#include "gpu/command_buffer/common/gpu_memory_buffer_support.h"
#include "gpu/command_buffer/common/shared_image_usage.h"
#include "gpu/ipc/common/gpu_memory_buffer_impl_shared_memory.h"
#include "gpu/ipc/common/gpu_memory_buffer_support.h"
#include "media/base/media_switches.h"
#include "media/base/video_frame.h"
#include "media/base/video_types.h"
#include "media/capture/video_capture_types.h"
#include "mojo/public/cpp/system/buffer.h"
#include "ui/compositor/compositor.h"
#include "ui/gfx/buffer_types.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/gpu_memory_buffer.h"

#if BUILDFLAG(IS_WIN)
#include "gpu/command_buffer/common/shared_image_capabilities.h"
#endif

#if BUILDFLAG(IS_MAC)
#include "media/capture/video/apple/video_capture_device_factory_apple.h"
#endif

namespace capture_mode {

namespace {

// The `kGpuMemoryBuffer` type is requested only when running on an actual
// device. This allows force-requesting them when testing in which case
// SharedMemory GMBs are used.
bool g_force_use_gpu_memory_buffer_for_test = false;

// A constant flag that describes which APIs the shared image mailboxes created
// for the video frame will be used with. They will be read via the raster
// interface (which will be going over GLES2 if OOP-R is not enabled), sent
// to the display compositor, and may be used as overlays.
constexpr uint32_t kSharedImageUsage =
    gpu::SHARED_IMAGE_USAGE_GLES2_READ | gpu::SHARED_IMAGE_USAGE_RASTER_READ |
    gpu::SHARED_IMAGE_USAGE_DISPLAY_READ | gpu::SHARED_IMAGE_USAGE_SCANOUT;

// The usage of the GpuMemoryBuffer that backs the video frames on an actual
// device (of type `NATIVE_PIXMAP`). The buffer is going to be presented on the
// screen for rendering, will be used as a texture, and can be read by CPU and
// potentially a video encode accelerator.
constexpr gfx::BufferUsage kGpuMemoryBufferUsage =
    gfx::BufferUsage::SCANOUT_VEA_CPU_READ;

// The usage of the GpuMemoryBuffer that backs the video frames in unittests,
// since the type of that buffer will be `SHARED_MEMORY_BUFFER` which doesn't
// support the above on-device usage.
constexpr gfx::BufferUsage kGpuMemoryBufferUsageForTest =
    gfx::BufferUsage::SCANOUT_CPU_READ_WRITE;

// The only supported video pixel format used on devices is `PIXEL_FORMAT_NV12`.
// This maps to a buffer format of `YUV_420_BIPLANAR`.
constexpr gfx::BufferFormat kGpuMemoryBufferFormat =
    gfx::BufferFormat::YUV_420_BIPLANAR;

// In unittests, the video pixel format used is `PIXEL_FORMAT_ARGB`, since the
// video frames are painted and verified manually using Skia. The buffer format
// used for this is `BGRA_8888`.
constexpr gfx::BufferFormat kGpuMemoryBufferFormatForTest =
    gfx::BufferFormat::BGRA_8888;

gfx::BufferUsage GetBufferUsage() {
  return g_force_use_gpu_memory_buffer_for_test ? kGpuMemoryBufferUsageForTest
                                                : kGpuMemoryBufferUsage;
}

gfx::BufferFormat GetBufferFormat() {
  return g_force_use_gpu_memory_buffer_for_test ? kGpuMemoryBufferFormatForTest
                                                : kGpuMemoryBufferFormat;
}

viz::SharedImageFormat GetSharedImageFormat() {
  return g_force_use_gpu_memory_buffer_for_test
             ? viz::SinglePlaneFormat::kBGRA_8888
             : viz::MultiPlaneFormat::kNV12;
}

#if BUILDFLAG(IS_CHROMEOS)
// Adjusts the requested video capture `params` depending on whether we're
// running on an actual device or the linux-chromeos build.
void AdjustParamsForCurrentConfig(media::VideoCaptureParams* params) {
  DCHECK(params);

  // The default params are good enough when running on linux-chromeos.
  if (!base::SysInfo::IsRunningOnChromeOS() &&
      !g_force_use_gpu_memory_buffer_for_test) {
    DCHECK_EQ(params->buffer_type,
              media::VideoCaptureBufferType::kSharedMemory);
    return;
  }

  // On an actual device, the camera HAL only supports NV12 pixel formats in a
  // GPU memory buffer.
  params->requested_format.pixel_format = media::PIXEL_FORMAT_NV12;
  params->buffer_type = media::VideoCaptureBufferType::kGpuMemoryBuffer;
}
#endif

// Whether to use the SharedImageInterface entrypoint taking a SharedImageFormat
// to create multiplanar SharedImages via viz::MultiPlaneFormat rather than
// going through the legacy entrypoint for SI creation that passes a GMB.
bool CreateNonLegacyMultiPlaneSharedImage() {
  return media::IsMultiPlaneFormatForHardwareVideoEnabled();
}

// Whether to use per-plane sampling rather than external sampling.
bool UsePerPlaneSampling() {
  return base::FeatureList::IsEnabled(
      media::kMultiPlaneVideoCaptureSharedImages);
}

// Creates and returns a list of the buffer planes for each we'll need to create
// a shared image and store it in `GpuMemoryBufferHandleHolder::shared_images_`.
std::vector<gfx::BufferPlane> CreateGpuBufferPlanes() {
  std::vector<gfx::BufferPlane> planes;
  if (UsePerPlaneSampling() && !CreateNonLegacyMultiPlaneSharedImage()) {
    planes.push_back(gfx::BufferPlane::Y);
    planes.push_back(gfx::BufferPlane::UV);
  } else {
    planes.push_back(gfx::BufferPlane::DEFAULT);
  }
  return planes;
}

bool IsFatalError(media::VideoCaptureError error) {
  switch (error) {
    case media::VideoCaptureError::kCrosHalV3FailedToStartDeviceThread:
    case media::VideoCaptureError::kCrosHalV3DeviceDelegateMojoConnectionError:
    case media::VideoCaptureError::
        kCrosHalV3DeviceDelegateFailedToOpenCameraDevice:
    case media::VideoCaptureError::
        kCrosHalV3DeviceDelegateFailedToInitializeCameraDevice:
    case media::VideoCaptureError::
        kCrosHalV3DeviceDelegateFailedToConfigureStreams:
    case media::VideoCaptureError::kCrosHalV3BufferManagerFatalDeviceError:
      return true;
    default:
      return false;
  }
}

#if BUILDFLAG(IS_MAC) || BUILDFLAG(IS_WIN)
bool IsGpuRasterizationSupported(ui::ContextFactory* context_factory) {
  DCHECK(context_factory);
  auto provider = context_factory->SharedMainThreadRasterContextProvider();
  return provider && provider->ContextCapabilities().gpu_rasterization;
}
#endif

#if BUILDFLAG(IS_WIN)
bool IsD3DSharedImageSupported(ui::ContextFactory* context_factory) {
  DCHECK(context_factory);
  if (auto provider =
          context_factory->SharedMainThreadRasterContextProvider()) {
    auto* shared_image_interface = provider->SharedImageInterface();
    return shared_image_interface &&
           shared_image_interface->GetCapabilities().shared_image_d3d;
  }
  return false;
}

void AdjustWinParamsForCurrentConfig(media::VideoCaptureParams* params,
                                     ui::ContextFactory* context_factory) {
  if (media::IsMediaFoundationD3D11VideoCaptureEnabled() &&
      params->requested_format.pixel_format == media::PIXEL_FORMAT_NV12) {
    if (IsGpuRasterizationSupported(context_factory)) {
      params->buffer_type = media::VideoCaptureBufferType::kGpuMemoryBuffer;
    } else {
      params->requested_format.pixel_format = media::PIXEL_FORMAT_I420;
    }
  }
}
#endif

#if BUILDFLAG(IS_MAC)
void AdjustMacParamsForCurrentConfig(media::VideoCaptureParams* params,
                                     const std::string& device_id) {
  if (media::ShouldEnableGpuMemoryBuffer(device_id)) {
    params->buffer_type = media::VideoCaptureBufferType::kGpuMemoryBuffer;
  }
}
#endif

// -----------------------------------------------------------------------------
// SharedMemoryBufferHandleHolder:

// Defines an implementation for a `BufferHandleHolder` that can extract a video
// frame that is backed by a `kSharedMemory` buffer type.
class SharedMemoryBufferHandleHolder : public BufferHandleHolder {
 public:
  explicit SharedMemoryBufferHandleHolder(base::UnsafeSharedMemoryRegion region)
      : region_(std::move(region)) {
    CHECK(region_.IsValid());
  }
  explicit SharedMemoryBufferHandleHolder(
      media::mojom::VideoBufferHandlePtr buffer_handle)
      : region_(std::move(buffer_handle->get_unsafe_shmem_region())) {
    DCHECK(buffer_handle->is_unsafe_shmem_region());
#if BUILDFLAG(IS_CHROMEOS)
    DCHECK(!base::SysInfo::IsRunningOnChromeOS());
#endif
  }
  SharedMemoryBufferHandleHolder(const SharedMemoryBufferHandleHolder&) =
      delete;
  SharedMemoryBufferHandleHolder& operator=(
      const SharedMemoryBufferHandleHolder&) = delete;
  ~SharedMemoryBufferHandleHolder() override = default;

  // BufferHandleHolder:
  scoped_refptr<media::VideoFrame> OnFrameReadyInBuffer(
      video_capture::mojom::ReadyFrameInBufferPtr buffer) override {
    const size_t mapping_size = media::VideoFrame::AllocationSize(
        buffer->frame_info->pixel_format, buffer->frame_info->coded_size);
    if (!MaybeUpdateMapping(mapping_size)) {
      return {};
    }

    auto& frame_info = buffer->frame_info;
    auto frame = media::VideoFrame::WrapExternalData(
        frame_info->pixel_format, frame_info->coded_size,
        frame_info->visible_rect, frame_info->visible_rect.size(),
        mapping_.GetMemoryAs<uint8_t>(), mapping_.size(),
        frame_info->timestamp);

    return frame;
  }

 private:
  // Maps a new region with a size `new_mapping_size` bytes if no `mapping_` is
  // available. Returns true if already mapped, or mapping is successful, false
  // otherwise.
  bool MaybeUpdateMapping(size_t new_mapping_size) {
    if (mapping_.IsValid()) {
      DCHECK_GE(mapping_.size(), new_mapping_size);
      return true;
    }

    mapping_ = region_.Map();
    DCHECK_GE(mapping_.size(), new_mapping_size);
    return mapping_.IsValid();
  }

  // The held shared memory region associated with this object.
  base::UnsafeSharedMemoryRegion region_;

  // Shared memory mapping associated with the held `region_`.
  base::WritableSharedMemoryMapping mapping_;
};

// -----------------------------------------------------------------------------
// GpuMemoryBufferHandleHolder:

// Defines an implementation for a `BufferHandleHolder` that can extract a video
// frame that is backed by a `kGpuMemoryBuffer` buffer type.
class GpuMemoryBufferHandleHolder : public BufferHandleHolder,
                                    public viz::ContextLostObserver {
 public:
  GpuMemoryBufferHandleHolder(media::mojom::VideoBufferHandlePtr buffer_handle,
                              ui::ContextFactory* context_factory)
      : gpu_memory_buffer_handle_(
            std::move(buffer_handle->get_gpu_memory_buffer_handle())),
        buffer_planes_(CreateGpuBufferPlanes()),
        context_factory_(context_factory),
        context_provider_(
            context_factory_->SharedMainThreadRasterContextProvider()) {
    DCHECK(buffer_handle->is_gpu_memory_buffer_handle());
    if (context_provider_) {
      context_provider_->AddObserver(this);
    }
  }

  GpuMemoryBufferHandleHolder(const GpuMemoryBufferHandleHolder&) = delete;
  GpuMemoryBufferHandleHolder& operator=(const GpuMemoryBufferHandleHolder&) =
      delete;

  ~GpuMemoryBufferHandleHolder() override {
    if (!context_provider_) {
      return;
    }

    context_provider_->RemoveObserver(this);

    gpu::SharedImageInterface* shared_image_interface =
        context_provider_->SharedImageInterface();
    DCHECK(shared_image_interface);

    for (auto& si : shared_images_) {
      if (!si) {
        continue;
      }
      shared_image_interface->DestroySharedImage(release_sync_token_,
                                                 std::move(si));
    }
  }

  // BufferHandleHolder:
  scoped_refptr<media::VideoFrame> OnFrameReadyInBuffer(
      video_capture::mojom::ReadyFrameInBufferPtr buffer) override {
    if (!context_provider_) {
      LOG(ERROR) << "GPU context lost.";
      return {};
    }

    const auto& frame_info = buffer->frame_info;
    if (!MaybeCreateSharedImages(frame_info)) {
      LOG(ERROR) << "Failed to initialize GpuMemoryBufferHandleHolder.";
      return {};
    }

    return WrapMailboxesInVideoFrame(frame_info);
  }

  // viz::ContextLostObserver:
  void OnContextLost() override {
    DCHECK(context_provider_);
    context_provider_->RemoveObserver(this);

    // Clear the mailboxes so that we can recreate the shared images.
    should_create_shared_images_ = true;
    for (auto& si : shared_images_) {
      si.reset();
    }
    release_sync_token_ = gpu::SyncToken();

    context_provider_ =
        context_factory_->SharedMainThreadRasterContextProvider();
    if (context_provider_) {
      context_provider_->AddObserver(this);
    }
  }

  const gfx::GpuMemoryBufferHandle& GetGpuMemoryBufferHandle() const {
    return gpu_memory_buffer_handle_;
  }

  base::UnsafeSharedMemoryRegion TakeGpuMemoryBufferHandleRegion() {
    return std::move(gpu_memory_buffer_handle_.region);
  }

 private:
  // Creates and returns a new `GpuMemoryBuffer` from a cloned handle of our
  // `gpu_memory_buffer_handle_`. The type of the buffer depends on the type of
  // the handle and can only be either `NATIVE_PIXMAP` or
  // `SHARED_MEMORY_BUFFER`.
  std::unique_ptr<gfx::GpuMemoryBuffer> CreateGpuMemoryBufferFromHandle(
      const gfx::Size& size) {
    const auto buffer_format = GetBufferFormat();
    const auto buffer_usage = GetBufferUsage();
    if (gpu_memory_buffer_handle_.type ==
        gpu::GpuMemoryBufferSupport::GetNativeGpuMemoryBufferType()) {
      return gpu_memory_buffer_support_.CreateGpuMemoryBufferImplFromHandle(
          gpu_memory_buffer_handle_.Clone(), size, buffer_format, buffer_usage,
          base::DoNothing());
    }

    DCHECK_EQ(gpu_memory_buffer_handle_.type, gfx::SHARED_MEMORY_BUFFER);
    DCHECK(g_force_use_gpu_memory_buffer_for_test);
    return gpu::GpuMemoryBufferImplSharedMemory::CreateFromHandle(
        gpu_memory_buffer_handle_.Clone(), size, buffer_format, buffer_usage,
        base::DoNothing());
  }

  // Initializes this holder by creating shared images and storing them in
  // `shared_images_`. These shared images are backed by a GpuMemoryBuffer whose
  // handle is a clone of our `gpu_memory_buffer_handle_`. This operation should
  // only be done the first ever time, or whenever the gpu context is lost.
  // Returns true if shared images are already created or creation is
  // successful. False otherwise.
  bool MaybeCreateSharedImages(
      const media::mojom::VideoFrameInfoPtr& frame_info) {
    DCHECK(context_provider_);

    if (!should_create_shared_images_) {
      return true;
    }

    // We clone our handle `gpu_memory_buffer_handle_` and use the cloned handle
    // to create a new GpuMemoryBuffer which will be used to create the shared
    // images. This way, the lifetime of our `gpu_memory_buffer_handle_` remains
    // tied to the lieftime of this object (i.e. until `OnBufferRetired()` is
    // called).
    std::unique_ptr<gfx::GpuMemoryBuffer> gmb =
        CreateGpuMemoryBufferFromHandle(frame_info->coded_size);

    if (!gmb) {
      LOG(ERROR) << "Failed to create a GpuMemoryBuffer.";
      return false;
    }

    gpu::SharedImageInterface* shared_image_interface =
        context_provider_->SharedImageInterface();
    DCHECK(shared_image_interface);

    if (CreateNonLegacyMultiPlaneSharedImage()) {
      auto format = GetSharedImageFormat();
#if BUILDFLAG(IS_OZONE)
      // If format is not multiplanar it must be used for testing.
      CHECK(format.is_multi_plane() || g_force_use_gpu_memory_buffer_for_test);
      if (!UsePerPlaneSampling() && format.is_multi_plane()) {
        format.SetPrefersExternalSampler();
      }
#endif
      CHECK_EQ(buffer_planes_.size(), 1u);
      shared_images_[0] = shared_image_interface->CreateSharedImage(
          {format, gmb->GetSize(), frame_info->color_space, kSharedImageUsage,
           "CameraVideoFrame"},
          gmb->CloneHandle());
      CHECK(shared_images_[0]);
    } else {
      gpu::GpuMemoryBufferManager* gmb_manager =
          context_factory_->GetGpuMemoryBufferManager();
      for (size_t plane = 0; plane < buffer_planes_.size(); ++plane) {
        shared_images_[plane] = shared_image_interface->CreateSharedImage(
            gmb.get(), gmb_manager, buffer_planes_[plane],
            {frame_info->color_space, kTopLeft_GrSurfaceOrigin,
             kPremul_SkAlphaType, kSharedImageUsage, "CameraVideoFrame"});
        CHECK(shared_images_[plane]);
      }
    }

    // Since this is the first time we create the shared images in
    // `shared_images_`, we need to guarantee that the shared images are created
    // before they're used.
    mailbox_holder_sync_token_ = shared_image_interface->GenVerifiedSyncToken();

    should_create_shared_images_ = false;
    return true;
  }

  // Wraps the shared images in `shared_images_` in a video frame and returns it
  // if wrapping was successful, or an empty refptr otherwise.
  scoped_refptr<media::VideoFrame> WrapMailboxesInVideoFrame(
      const media::mojom::VideoFrameInfoPtr& frame_info) {
    DCHECK(!should_create_shared_images_);

    if (frame_info->pixel_format !=
            media::VideoPixelFormat::PIXEL_FORMAT_NV12 &&
        frame_info->pixel_format !=
            media::VideoPixelFormat::PIXEL_FORMAT_ARGB) {
      LOG(ERROR) << "Unsupported pixel format";
      return {};
    }

#if !BUILDFLAG(IS_WIN)
    // The camera GpuMemoryBuffer is backed by a DMA-buff, and doesn't use a
    // pre-mapped shared memory region.
    DCHECK(!frame_info->is_premapped);
#endif

    auto buffer_texture_target = shared_images_[0]->GetTextureTarget(
        GetBufferUsage(), GetBufferFormat());

    gpu::MailboxHolder mailbox_holder_array[media::VideoFrame::kMaxPlanes];
    for (size_t plane = 0; plane < buffer_planes_.size(); ++plane) {
      DCHECK(shared_images_[plane]);
      mailbox_holder_array[plane] =
          gpu::MailboxHolder(shared_images_[plane]->mailbox(),
                             mailbox_holder_sync_token_, buffer_texture_target);
    }
    mailbox_holder_sync_token_.Clear();

    auto frame = media::VideoFrame::WrapNativeTextures(
        frame_info->pixel_format, mailbox_holder_array,
        base::BindOnce(&GpuMemoryBufferHandleHolder::OnMailboxReleased,
                       weak_ptr_factory_.GetWeakPtr()),
        frame_info->coded_size, frame_info->visible_rect,
        frame_info->visible_rect.size(), frame_info->timestamp);

    if (!frame) {
      LOG(ERROR) << "Failed to create a video frame.";
      return frame;
    }

    if (CreateNonLegacyMultiPlaneSharedImage()) {
      auto format = GetSharedImageFormat();
      // If format is not multiplanar it must be used for testing.
      CHECK(format.is_multi_plane() || g_force_use_gpu_memory_buffer_for_test);
      if (!UsePerPlaneSampling() && format.is_multi_plane()) {
        frame->set_shared_image_format_type(
            media::SharedImageFormatType::kSharedImageFormatExternalSampler);
      } else {
        frame->set_shared_image_format_type(
            media::SharedImageFormatType::kSharedImageFormat);
      }
    }

    if (frame_info->color_space.IsValid()) {
      frame->set_color_space(frame_info->color_space);
    }
    frame->metadata().allow_overlay = true;
    frame->metadata().read_lock_fences_enabled = true;
    frame->metadata().MergeMetadataFrom(frame_info->metadata);

    return frame;
  }

  // Called when the video frame is destroyed.
  void OnMailboxReleased(const gpu::SyncToken& release_sync_token) {
    release_sync_token_ = release_sync_token;
  }

  // The held GPU buffer handle associated with this object.
  gfx::GpuMemoryBufferHandle gpu_memory_buffer_handle_;

  // The buffer planes for each we need to create a shared image and store it in
  // `shared_images_`.
  const std::vector<gfx::BufferPlane> buffer_planes_;

  const raw_ptr<ui::ContextFactory> context_factory_;

  // Used to create a GPU memory buffer from its handle.
  gpu::GpuMemoryBufferSupport gpu_memory_buffer_support_;

  scoped_refptr<viz::RasterContextProvider> context_provider_;

  // Contains the shared images of the video frame planes created from the GPU
  // memory buffer.
  std::vector<scoped_refptr<gpu::ClientSharedImage>> shared_images_{
      media::VideoFrame::kMaxPlanes};

  // The sync token used when creating a `MailboxHolder`. This will be a
  // verified sync token the first time we wrap a video frame around a mailbox.
  gpu::SyncToken mailbox_holder_sync_token_;

  // The release sync token of the above `shared_images_`.
  gpu::SyncToken release_sync_token_;

  bool should_create_shared_images_ = true;

  base::WeakPtrFactory<GpuMemoryBufferHandleHolder> weak_ptr_factory_{this};
};

#if BUILDFLAG(IS_MAC)
// -----------------------------------------------------------------------------
// MacGpuMemoryBufferHandleHolder:

// Defines an implementation for a `BufferHandleHolder` that can extract a video
// frame that is backed by an IOSurface on mac. It allows the possibility to map
// the memory and produce a software video frame.
class MacGpuMemoryBufferHandleHolder : public BufferHandleHolder {
 public:
  explicit MacGpuMemoryBufferHandleHolder(
      media::mojom::VideoBufferHandlePtr buffer_handle,
      ui::ContextFactory* context_factory)
      : context_factory_(context_factory),
        gmb_holder_(std::move(buffer_handle), context_factory) {}
  MacGpuMemoryBufferHandleHolder(const MacGpuMemoryBufferHandleHolder&) =
      delete;
  MacGpuMemoryBufferHandleHolder& operator=(
      const MacGpuMemoryBufferHandleHolder&) = delete;
  ~MacGpuMemoryBufferHandleHolder() override = default;

  // BufferHandleHolder:
  scoped_refptr<media::VideoFrame> OnFrameReadyInBuffer(
      video_capture::mojom::ReadyFrameInBufferPtr buffer) override {
    // TODO: This isn't required to be checked every frame and only changes
    // after context loss.
    if (IsGpuRasterizationSupported(context_factory_)) {
      return gmb_holder_.OnFrameReadyInBuffer(std::move(buffer));
    }
    auto frame = media::VideoFrame::WrapUnacceleratedIOSurface(
        gmb_holder_.GetGpuMemoryBufferHandle().Clone(),
        buffer->frame_info->visible_rect, buffer->frame_info->timestamp);

    if (!frame) {
      VLOG(0) << "Failed to create a video frame.";
    }
    return frame;
  }

 private:
  const raw_ptr<ui::ContextFactory> context_factory_;

  // A GMB handle holder used to extract and return the video frame when Gpu
  // rasterization is supported.
  GpuMemoryBufferHandleHolder gmb_holder_;
};
#endif

#if BUILDFLAG(IS_WIN)
// -----------------------------------------------------------------------------
// WinGpuMemoryBufferHandleHolder:

// Defines an implementation for a `BufferHandleHolder` that can extract a video
// frame that is backed by a `kGpuMemoryBuffer` buffer type on Win. It allows
// the possibility to map the memory and produce a software video frame.
class WinGpuMemoryBufferHandleHolder : public BufferHandleHolder {
 public:
  explicit WinGpuMemoryBufferHandleHolder(
      base::RepeatingClosure require_mapped_frame_callback,
      media::mojom::VideoBufferHandlePtr buffer_handle,
      ui::ContextFactory* context_factory)
      : context_factory_(context_factory),
        gmb_holder_(std::move(buffer_handle), context_factory),
        sh_mem_holder_(gmb_holder_.TakeGpuMemoryBufferHandleRegion()),
        require_mapped_frame_callback_(
            std::move(require_mapped_frame_callback)) {
    CHECK_EQ(gmb_holder_.GetGpuMemoryBufferHandle().type,
             gfx::DXGI_SHARED_HANDLE);
    CHECK(require_mapped_frame_callback_);
  }
  WinGpuMemoryBufferHandleHolder(const WinGpuMemoryBufferHandleHolder&) =
      delete;
  WinGpuMemoryBufferHandleHolder& operator=(
      const WinGpuMemoryBufferHandleHolder&) = delete;
  ~WinGpuMemoryBufferHandleHolder() override = default;

  // BufferHandleHolder:
  scoped_refptr<media::VideoFrame> OnFrameReadyInBuffer(
      video_capture::mojom::ReadyFrameInBufferPtr buffer) override {
    // TODO: This isn't required to be checked every frame and only changes
    // after context loss.
    if (IsGpuRasterizationSupported(context_factory_) &&
        IsD3DSharedImageSupported(context_factory_)) {
      return gmb_holder_.OnFrameReadyInBuffer(std::move(buffer));
    }
    if (buffer->frame_info->is_premapped) {
      return sh_mem_holder_.OnFrameReadyInBuffer(std::move(buffer));
    }
    require_mapped_frame_callback_.Run();
    return {};
  }

 private:
  const raw_ptr<ui::ContextFactory> context_factory_;

  // A GMB handle holder used to extract and return the video frame when both
  // Gpu rasterization and D3dSharedImage are supported.
  GpuMemoryBufferHandleHolder gmb_holder_;

  // A shared memory handle holder used to extract and return the video frame
  // when frame is pre-mapped.
  SharedMemoryBufferHandleHolder sh_mem_holder_;

  // A callback that is used to request pre-mapped frames.
  const base::RepeatingClosure require_mapped_frame_callback_;
};
#endif

// Notifies the passed `VideoFrameAccessHandler` that the handler is done using
// the buffer.
void OnFrameGone(
    mojo::SharedRemote<video_capture::mojom::VideoFrameAccessHandler>
        video_frame_access_handler_remote,
    const int buffer_id) {
  video_frame_access_handler_remote->OnFinishedConsumingBuffer(buffer_id);
}

}  // namespace

// -----------------------------------------------------------------------------
// BufferHandleHolder:

BufferHandleHolder::~BufferHandleHolder() = default;

// static
std::unique_ptr<BufferHandleHolder> BufferHandleHolder::Create(
    media::mojom::VideoBufferHandlePtr buffer_handle,
    ui::ContextFactory* context_factory,
    base::RepeatingClosure require_mapped_frame_callback) {
  if (buffer_handle->is_unsafe_shmem_region()) {
    return std::make_unique<SharedMemoryBufferHandleHolder>(
        std::move(buffer_handle));
  }

  DCHECK(buffer_handle->is_gpu_memory_buffer_handle());
#if BUILDFLAG(IS_MAC)
  return std::make_unique<MacGpuMemoryBufferHandleHolder>(
      std::move(buffer_handle), context_factory);
#elif BUILDFLAG(IS_WIN)
  return std::make_unique<WinGpuMemoryBufferHandleHolder>(
      std::move(require_mapped_frame_callback), std::move(buffer_handle),
      context_factory);
#else
  return std::make_unique<GpuMemoryBufferHandleHolder>(std::move(buffer_handle),
                                                       context_factory);
#endif
}

// -----------------------------------------------------------------------------
// CameraVideoFrameHandler:

CameraVideoFrameHandler::CameraVideoFrameHandler(
    ui::ContextFactory* context_factory,
    mojo::Remote<video_capture::mojom::VideoSource> camera_video_source,
    const media::VideoCaptureFormat& capture_format,
    const std::string& device_id)
    : context_factory_(context_factory),
      camera_video_source_remote_(std::move(camera_video_source)) {
  CHECK(camera_video_source_remote_);

  camera_video_source_remote_.set_disconnect_handler(
      base::BindOnce(&CameraVideoFrameHandler::OnFatalErrorOrDisconnection,
                     base::Unretained(this)));

  media::VideoCaptureParams capture_params;
  capture_params.requested_format = capture_format;
#if BUILDFLAG(IS_CHROMEOS)
  AdjustParamsForCurrentConfig(&capture_params);
#elif BUILDFLAG(IS_WIN)
  AdjustWinParamsForCurrentConfig(&capture_params, context_factory_);
#elif BUILDFLAG(IS_MAC)
  AdjustMacParamsForCurrentConfig(&capture_params, device_id);
#endif

  camera_video_source_remote_->CreatePushSubscription(
      video_frame_handler_receiver_.BindNewPipeAndPassRemote(), capture_params,
      // The Camera app, or some other camera capture operation may already be
      // running with certain settings. We don't want to reopen the camera
      // device with our settings, since our requirements are usually low in
      // terms of frame rate and size. So we'll use whatever settings available
      // if any.
      /*force_reopen_with_new_settings=*/false,
      camera_video_stream_subsciption_remote_.BindNewPipeAndPassReceiver(),
      base::BindOnce(
          [](video_capture::mojom::CreatePushSubscriptionResultCodePtr
                 result_code,
             const media::VideoCaptureParams& actual_params) {
            if (result_code->is_error_code()) {
              LOG(ERROR) << "Error in creating push subscription: "
                         << static_cast<int>(result_code->get_error_code());
            }
          }));
}

CameraVideoFrameHandler::~CameraVideoFrameHandler() = default;

void CameraVideoFrameHandler::StartHandlingFrames(Delegate* delegate) {
  CHECK(delegate);
  CHECK(camera_video_stream_subsciption_remote_);
  delegate_ = delegate;
  active_ = true;
  camera_video_stream_subsciption_remote_->Activate();
}

void CameraVideoFrameHandler::Close(base::OnceClosure close_complete_callback) {
  active_ = false;
  // `delegate_` might be freed any time after this point, so nullify it to
  // reflect that.
  delegate_ = nullptr;
  camera_video_stream_subsciption_remote_->Close(
      std::move(close_complete_callback));
}

void CameraVideoFrameHandler::OnCaptureConfigurationChanged() {}

void CameraVideoFrameHandler::OnNewBuffer(
    int buffer_id,
    media::mojom::VideoBufferHandlePtr buffer_handle) {
  const auto pair = buffer_map_.emplace(
      buffer_id,
      BufferHandleHolder::Create(
          std::move(buffer_handle), context_factory_.get(),
          base::BindRepeating(&CameraVideoFrameHandler::RequireMappedFrame,
                              base::Unretained(this))));
  DCHECK(pair.second);
}

void CameraVideoFrameHandler::OnFrameAccessHandlerReady(
    mojo::PendingRemote<video_capture::mojom::VideoFrameAccessHandler>
        pending_frame_access_handler) {
  video_frame_access_handler_remote_.Bind(
      std::move(pending_frame_access_handler),
      base::SequencedTaskRunner::GetCurrentDefault());
}

void CameraVideoFrameHandler::OnFrameReadyInBuffer(
    video_capture::mojom::ReadyFrameInBufferPtr buffer) {
  CHECK(video_frame_access_handler_remote_);

  const int buffer_id = buffer->buffer_id;

  if (!active_) {
    video_frame_access_handler_remote_->OnFinishedConsumingBuffer(buffer_id);
    return;
  }
  // `delegate_` should still exist if the handler is active.
  CHECK(delegate_);

  const auto& iter = buffer_map_.find(buffer_id);
  CHECK(iter != buffer_map_.end());

  const auto& buffer_handle_holder = iter->second;
  scoped_refptr<media::VideoFrame> frame =
      buffer_handle_holder->OnFrameReadyInBuffer(std::move(buffer));
  if (!frame) {
    video_frame_access_handler_remote_->OnFinishedConsumingBuffer(buffer_id);
    return;
  }

  frame->AddDestructionObserver(base::BindOnce(
      &OnFrameGone, video_frame_access_handler_remote_, buffer_id));

  delegate_->OnCameraVideoFrame(std::move(frame));
}

void CameraVideoFrameHandler::OnBufferRetired(int buffer_id) {
  DCHECK(buffer_map_.contains(buffer_id));
  buffer_map_.erase(buffer_id);
}

void CameraVideoFrameHandler::OnError(media::VideoCaptureError error) {
  LOG(ERROR) << "Recieved error: " << static_cast<int>(error);
  if (IsFatalError(error)) {
    OnFatalErrorOrDisconnection();
  }
}

void CameraVideoFrameHandler::OnFrameDropped(
    media::VideoCaptureFrameDropReason reason) {
  DLOG(ERROR) << "A camera video frame was dropped due to: "
              << static_cast<int>(reason);
}

void CameraVideoFrameHandler::OnNewSubCaptureTargetVersion(
    uint32_t sub_capture_target_version) {}

void CameraVideoFrameHandler::OnFrameWithEmptyRegionCapture() {}

void CameraVideoFrameHandler::OnLog(const std::string& message) {
  DVLOG(1) << message;
}

void CameraVideoFrameHandler::OnStarted() {}

void CameraVideoFrameHandler::OnStartedUsingGpuDecode() {}

void CameraVideoFrameHandler::OnStopped() {}

// static
void CameraVideoFrameHandler::SetForceUseGpuMemoryBufferForTest(bool value) {
  g_force_use_gpu_memory_buffer_for_test = value;
}

void CameraVideoFrameHandler::OnVideoFrameGone(int buffer_id) {
  CHECK(video_frame_access_handler_remote_);
  video_frame_access_handler_remote_->OnFinishedConsumingBuffer(buffer_id);
}

void CameraVideoFrameHandler::OnFatalErrorOrDisconnection() {
  active_ = false;
  buffer_map_.clear();
  weak_ptr_factory_.InvalidateWeakPtrs();
  video_frame_handler_receiver_.reset();
  camera_video_source_remote_.reset();

  // There is a chance where resetting `camera_video_stream_subsciption_remote_`
  // causes the deletion of `this`. That is the reason why we need to use a weak
  // pointer, as to avoid unexpected behaviour.
  // For more info check `Close()` function.
  auto weak_ptr = weak_ptr_factory_.GetWeakPtr();
  camera_video_stream_subsciption_remote_.reset();
  if (!weak_ptr) {
    return;
  }

  if (delegate_) {
    delegate_->OnFatalErrorOrDisconnection();
    // Caution as the delegate may choose to delete `this` after the above call.
  }
}

void CameraVideoFrameHandler::RequireMappedFrame() {
#if BUILDFLAG(IS_WIN)
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (camera_video_stream_subsciption_remote_.is_bound()) {
    media::VideoCaptureFeedback feedback;
    feedback.require_mapped_frame = true;
    camera_video_stream_subsciption_remote_->ProcessFeedback(feedback);
  }
#endif
}

}  // namespace capture_mode
