// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/gpu/chromeos/mailbox_video_frame_converter.h"

#include <optional>

#include "base/containers/contains.h"
#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/location.h"
#include "base/memory/ptr_util.h"
#include "base/task/sequenced_task_runner.h"
#include "base/task/single_thread_task_runner.h"
#include "base/trace_event/trace_event.h"
#include "components/viz/common/resources/shared_image_format.h"
#include "gpu/command_buffer/client/client_shared_image.h"
#include "gpu/command_buffer/common/shared_image_usage.h"
#include "gpu/command_buffer/service/scheduler.h"
#include "gpu/command_buffer/service/shared_image/shared_image_factory.h"
#include "gpu/ipc/service/gpu_channel.h"
#include "media/base/format_utils.h"
#include "media/base/media_switches.h"
#include "media/base/video_util.h"
#include "media/gpu/chromeos/platform_video_frame_utils.h"
#include "media/gpu/chromeos/video_frame_resource.h"
#include "media/gpu/macros.h"
#include "ui/gfx/buffer_format_util.h"
#include "ui/gfx/gpu_memory_buffer.h"
#include "ui/gl/gl_bindings.h"

namespace {

// Based on `buffer_format` support by VideoPixelFormatToGfxBufferFormat.
viz::SharedImageFormat GetSharedImageFormat(gfx::BufferFormat buffer_format) {
  viz::SharedImageFormat format;
  switch (buffer_format) {
    case gfx::BufferFormat::RGBA_8888:
      format = viz::SinglePlaneFormat::kRGBA_8888;
      break;
    case gfx::BufferFormat::RGBX_8888:
      format = viz::SinglePlaneFormat::kRGBX_8888;
      break;
    case gfx::BufferFormat::BGRA_8888:
      format = viz::SinglePlaneFormat::kBGRA_8888;
      break;
    case gfx::BufferFormat::BGRX_8888:
      format = viz::SinglePlaneFormat::kBGRX_8888;
      break;
    case gfx::BufferFormat::YVU_420:
      format = viz::MultiPlaneFormat::kYV12;
      break;
    case gfx::BufferFormat::YUV_420_BIPLANAR:
      format = viz::MultiPlaneFormat::kNV12;
      break;
    case gfx::BufferFormat::YUVA_420_TRIPLANAR:
      format = viz::MultiPlaneFormat::kNV12A;
      break;
    case gfx::BufferFormat::P010:
      format = viz::MultiPlaneFormat::kP010;
      break;
    default:
      DLOG(WARNING) << "Unsupported buffer_format: "
                    << static_cast<int>(buffer_format);
      NOTREACHED_NORETURN();
  }
#if BUILDFLAG(IS_CHROMEOS) || BUILDFLAG(IS_LINUX)
  // If format is true multiplanar format, we prefer external sampler on
  // ChromeOS and Linux.
  if (format.is_multi_plane()) {
    format.SetPrefersExternalSampler();
  }
#endif
  return format;
}

}  // namespace

namespace media {

class GpuDelegateImpl : public MailboxVideoFrameConverter::GpuDelegate {
 public:
  using GetGpuChannelCB =
      base::RepeatingCallback<base::WeakPtr<gpu::GpuChannel>()>;

  GpuDelegateImpl(scoped_refptr<base::SingleThreadTaskRunner> gpu_task_runner,
                  GetGpuChannelCB get_gpu_channel_cb)
      : gpu_task_runner_(std::move(gpu_task_runner)),
        get_gpu_channel_cb_(std::move(get_gpu_channel_cb)) {}
  GpuDelegateImpl(const GpuDelegateImpl&) = delete;
  GpuDelegateImpl& operator=(const GpuDelegateImpl&) = delete;
  ~GpuDelegateImpl() override = default;

  bool Initialize() override {
    DCHECK(gpu_task_runner_->BelongsToCurrentThread());

    // Use |gpu_channel_| as a marker that we have been initialized already.
    if (gpu_channel_)
      return true;

    gpu_channel_ = get_gpu_channel_cb_.Run();
    return !!gpu_channel_;
  }

  std::optional<gpu::SharedImageCapabilities> GetCapabilities() override {
    DCHECK(gpu_task_runner_->BelongsToCurrentThread());
    if (!gpu_channel_) {
      return std::nullopt;
    }

    gpu::SharedImageStub* shared_image_stub = gpu_channel_->shared_image_stub();
    DCHECK(shared_image_stub);
    gpu::SharedImageFactory* factory = shared_image_stub->factory();
    CHECK(factory);
    return factory->MakeCapabilities();
  }

  gpu::SharedImageStub::SharedImageDestructionCallback CreateSharedImage(
      const gpu::Mailbox& mailbox,
      gfx::GpuMemoryBufferHandle handle,
      gfx::BufferFormat format,
      gfx::BufferPlane plane,
      const gfx::Size& size,
      const gfx::ColorSpace& color_space,
      GrSurfaceOrigin surface_origin,
      SkAlphaType alpha_type,
      uint32_t usage) override {
    DCHECK(gpu_task_runner_->BelongsToCurrentThread());

    if (!gpu_channel_)
      return base::NullCallback();

    gpu::SharedImageStub* shared_image_stub = gpu_channel_->shared_image_stub();
    DCHECK(shared_image_stub);

    if (!shared_image_stub->CreateSharedImage(
            mailbox, std::move(handle), format, plane, size, color_space,
            surface_origin, alpha_type, usage, "MailboxVideoFrameConverter")) {
      return base::NullCallback();
    }

    return shared_image_stub->GetSharedImageDestructionCallback(mailbox);
  }

  gpu::SharedImageStub::SharedImageDestructionCallback CreateSharedImage(
      const gpu::Mailbox& mailbox,
      gfx::GpuMemoryBufferHandle handle,
      viz::SharedImageFormat format,
      const gfx::Size& size,
      const gfx::ColorSpace& color_space,
      GrSurfaceOrigin surface_origin,
      SkAlphaType alpha_type,
      uint32_t usage) override {
    DCHECK(gpu_task_runner_->BelongsToCurrentThread());

    if (!gpu_channel_) {
      return base::NullCallback();
    }

    gpu::SharedImageStub* shared_image_stub = gpu_channel_->shared_image_stub();
    DCHECK(shared_image_stub);

    if (!shared_image_stub->CreateSharedImage(
            mailbox, std::move(handle), format, size, color_space,
            surface_origin, alpha_type, usage, "MailboxVideoFrameConverter")) {
      return base::NullCallback();
    }

    return shared_image_stub->GetSharedImageDestructionCallback(mailbox);
  }

  bool UpdateSharedImage(const gpu::Mailbox& mailbox,
                         gfx::GpuFenceHandle in_fence_handle) override {
    DCHECK(gpu_task_runner_->BelongsToCurrentThread());

    if (!gpu_channel_)
      return false;

    gpu::SharedImageStub* shared_image_stub = gpu_channel_->shared_image_stub();
    DCHECK(shared_image_stub);

    return shared_image_stub->UpdateSharedImage(mailbox,
                                                std::move(in_fence_handle));
  }

  bool WaitOnSyncTokenAndReleaseFrame(
      scoped_refptr<FrameResource> frame,
      const gpu::SyncToken& sync_token) override {
    DCHECK(gpu_task_runner_->BelongsToCurrentThread());

    if (!gpu_channel_)
      return false;

    gpu::SharedImageStub* shared_image_stub = gpu_channel_->shared_image_stub();
    DCHECK(shared_image_stub);

    auto keep_video_frame_alive =
        base::DoNothingWithBoundArgs(std::move(frame));
    auto* scheduler = gpu_channel_->scheduler();
    DCHECK(scheduler);
    scheduler->ScheduleTask(gpu::Scheduler::Task(
        shared_image_stub->sequence(), std::move(keep_video_frame_alive),
        std::vector<gpu::SyncToken>({sync_token})));
    return true;
  }

 private:
  const scoped_refptr<base::SingleThreadTaskRunner> gpu_task_runner_;
  const GetGpuChannelCB get_gpu_channel_cb_;

  // |gpu_channel_| will outlive CommandBufferStub, keep the former as a WeakPtr
  // to guarantee proper resource cleanup. To be dereferenced on
  // |gpu_task_runner_| only.
  base::WeakPtr<gpu::GpuChannel> gpu_channel_;
};

// A SharedImage wrapper that calls |destroy_shared_image_cb| in dtor on
// |gpu_task_runner|.
class MailboxVideoFrameConverter::ScopedSharedImage {
 public:
  ScopedSharedImage(scoped_refptr<base::SingleThreadTaskRunner> gpu_task_runner)
      : destruction_task_runner_(std::move(gpu_task_runner)) {}

  ScopedSharedImage(const ScopedSharedImage&) = delete;
  ScopedSharedImage& operator=(const ScopedSharedImage&) = delete;

  ~ScopedSharedImage() { Destroy(); }

  void Reset(const gpu::Mailbox& mailbox,
             const gfx::Size& size,
             const gfx::ColorSpace& color_space,
             gpu::SharedImageStub::SharedImageDestructionCallback
                 destroy_shared_image_cb) {
    Destroy();
    DCHECK(!mailbox.IsZero());
    mailbox_ = mailbox;
    size_ = size;
    color_space_ = color_space;
    destroy_shared_image_cb_ = std::move(destroy_shared_image_cb);
  }

  bool HasData() const { return !mailbox_.IsZero(); }
  const gpu::Mailbox& mailbox() const { return mailbox_; }
  const gfx::Size& size() const { return size_; }
  const gfx::ColorSpace& color_space() const { return color_space_; }

 private:
  void Destroy() {
    if (destroy_shared_image_cb_) {
      if (destruction_task_runner_->RunsTasksInCurrentSequence()) {
        std::move(destroy_shared_image_cb_).Run(gpu::SyncToken());
        return;
      }
      destruction_task_runner_->PostTask(
          FROM_HERE, base::BindOnce(std::move(destroy_shared_image_cb_),
                                    gpu::SyncToken()));
    }
  }

  gpu::Mailbox mailbox_;
  gfx::Size size_;
  gfx::ColorSpace color_space_;
  gpu::SharedImageStub::SharedImageDestructionCallback destroy_shared_image_cb_;
  const scoped_refptr<base::SequencedTaskRunner> destruction_task_runner_;
};

// static
std::unique_ptr<MailboxVideoFrameConverter> MailboxVideoFrameConverter::Create(
    scoped_refptr<base::SingleThreadTaskRunner> gpu_task_runner,
    GetCommandBufferStubCB get_stub_cb) {
  DCHECK(gpu_task_runner);
  DCHECK(get_stub_cb);

  auto get_gpu_channel_cb = base::BindRepeating(
      [](base::RepeatingCallback<gpu::CommandBufferStub*()> get_stub_cb) {
        gpu::CommandBufferStub* stub = get_stub_cb.Run();
        if (!stub)
          return base::WeakPtr<gpu::GpuChannel>();
        DCHECK(stub->channel());
        return stub->channel()->AsWeakPtr();
      },
      get_stub_cb);
  auto gpu_delegate = std::make_unique<GpuDelegateImpl>(
      gpu_task_runner, std::move(get_gpu_channel_cb));

  return base::WrapUnique(new MailboxVideoFrameConverter(
      std::move(gpu_task_runner), std::move(gpu_delegate)));
}

MailboxVideoFrameConverter::MailboxVideoFrameConverter(
    scoped_refptr<base::SingleThreadTaskRunner> gpu_task_runner,
    std::unique_ptr<GpuDelegate> gpu_delegate)
    : gpu_task_runner_(std::move(gpu_task_runner)),
      gpu_delegate_(std::move(gpu_delegate)) {
  DVLOGF(2);

  parent_weak_this_ = parent_weak_this_factory_.GetWeakPtr();
  gpu_weak_this_ = gpu_weak_this_factory_.GetWeakPtr();
}

void MailboxVideoFrameConverter::Initialize(
    scoped_refptr<base::SequencedTaskRunner> parent_task_runner,
    OutputCB output_cb) {
  parent_task_runner_ = std::move(parent_task_runner);
  output_cb_ = std::move(output_cb);
}

void MailboxVideoFrameConverter::Destroy() {
  DCHECK(!parent_task_runner_ ||
         parent_task_runner_->RunsTasksInCurrentSequence());
  DVLOGF(2);

  parent_weak_this_factory_.InvalidateWeakPtrs();
  gpu_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&MailboxVideoFrameConverter::DestroyOnGPUThread,
                                gpu_weak_this_));
}

void MailboxVideoFrameConverter::DestroyOnGPUThread() {
  DCHECK(gpu_task_runner_->RunsTasksInCurrentSequence());
  DVLOGF(2);

  gpu_weak_this_factory_.InvalidateWeakPtrs();
  delete this;
}

MailboxVideoFrameConverter::~MailboxVideoFrameConverter() {
  // |gpu_weak_this_factory_| is already invalidated here.
  DCHECK(gpu_task_runner_->RunsTasksInCurrentSequence());
  DVLOGF(2);
}

bool MailboxVideoFrameConverter::InitializeOnGPUThread() {
  DVLOGF(4);
  DCHECK(gpu_task_runner_->BelongsToCurrentThread());
  return gpu_delegate_->Initialize();
}

void MailboxVideoFrameConverter::ConvertFrame(
    scoped_refptr<FrameResource> frame) {
  DCHECK(parent_task_runner_->RunsTasksInCurrentSequence());
  DVLOGF(4);

  if (!frame ||
      (frame->storage_type() != VideoFrame::STORAGE_DMABUFS &&
       frame->storage_type() != VideoFrame::STORAGE_GPU_MEMORY_BUFFER)) {
    return OnError(FROM_HERE, "Invalid frame.");
  }

  FrameResource* origin_frame =
      !get_original_frame_cb_.is_null()
          ? get_original_frame_cb_.Run(frame->GetSharedMemoryId())
          : frame.get();
  if (!origin_frame)
    return OnError(FROM_HERE, "Failed to get origin frame.");

  ScopedSharedImage* shared_image = nullptr;
  const UniqueID origin_frame_id = origin_frame->unique_id();
  auto shared_image_it = shared_images_.find(origin_frame_id);
  if (shared_image_it != shared_images_.end())
    shared_image = shared_image_it->second;

  input_frame_queue_.emplace(frame, origin_frame_id);

  // |frame| always carries a reference to |origin_frame|, directly or
  // indirectly. Therefore, |origin_frame| is guaranteed to be valid by carrying
  // |frame|. Maintaining a reference |origin_frame| also guarantees that
  // |shared_image| is alive, so as long as |frame| lives, |shared_image| is
  // valid. Hence, it's safe to use base::Unretained here.
  gpu_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&MailboxVideoFrameConverter::ConvertFrameOnGPUThread,
                     gpu_weak_this_, base::Unretained(origin_frame),
                     std::move(frame), base::Unretained(shared_image)));
}

void MailboxVideoFrameConverter::WrapMailboxAndVideoFrameAndOutput(
    FrameResource* origin_frame,
    scoped_refptr<FrameResource> frame,
    const gpu::Mailbox& mailbox) {
  DCHECK(parent_task_runner_->RunsTasksInCurrentSequence());
  DCHECK(!mailbox.IsZero());

  const UniqueID origin_frame_id = origin_frame->unique_id();
  DCHECK(base::Contains(shared_images_, origin_frame_id));

  // While we were on |gpu_task_runner_|, AbortPendingFrames() might have been
  // called and/or possibly different frames enqueued in |input_frame_queue_|.
  if (input_frame_queue_.empty())
    return;
  if (input_frame_queue_.front().second != origin_frame_id)
    return;
  input_frame_queue_.pop();

  const auto buffer_format = VideoPixelFormatToGfxBufferFormat(frame->format());
  // GenerateSharedImageOnGPUThread() should have checked the |origin_frame|'s
  // format (which should be the same as the |frame|'s format).
  CHECK_EQ(frame->format(), origin_frame->format());
  CHECK(buffer_format);

  gpu::MailboxHolder mailbox_holders[VideoFrame::kMaxPlanes];

  uint32_t texture_target = GL_TEXTURE_2D;
  // NOTE: The change to simplify logic for computing the texture target here is
  // under the `kUseUniversalGetTextureTargetFunction` killswitch as it is
  // following the same logic that ClientSharedImage::GetTextureTarget() is
  // using, and thus it makes sense to roll out the changes together.
  if (base::FeatureList::IsEnabled(
          gpu::kUseUniversalGetTextureTargetFunction)) {
    // The target should be GL_TEXTURE_2D unless external sampling is being
    // used, which in this context is equivalent to the passed-in buffer format
    // being multiplanar as we never use per-plane sampling in this context.
    texture_target = gfx::BufferFormatIsMultiplanar(*buffer_format)
                         ? GL_TEXTURE_EXTERNAL_OES
                         : GL_TEXTURE_2D;
  } else {
    texture_target = gpu::NativeBufferNeedsPlatformSpecificTextureTarget(
                         *buffer_format, gfx::BufferPlane::DEFAULT)
                         ? GL_TEXTURE_EXTERNAL_OES
                         : GL_TEXTURE_2D;
  }
  mailbox_holders[0] =
      gpu::MailboxHolder(mailbox, gpu::SyncToken(), texture_target);

  VideoFrame::ReleaseMailboxCB release_mailbox_cb = base::BindOnce(
      [](scoped_refptr<base::SequencedTaskRunner> gpu_task_runner,
         base::WeakPtr<MailboxVideoFrameConverter> gpu_weak_ptr,
         scoped_refptr<FrameResource> frame, const gpu::SyncToken& sync_token) {
        if (!sync_token.HasData()) {
          return;
        }

        if (gpu_task_runner->RunsTasksInCurrentSequence()) {
          if (gpu_weak_ptr) {
            gpu_weak_ptr->WaitOnSyncTokenAndReleaseFrameOnGPUThread(
                std::move(frame), sync_token);
          }
          return;
        }
        gpu_task_runner->PostTask(
            FROM_HERE,
            base::BindOnce(&MailboxVideoFrameConverter::
                               WaitOnSyncTokenAndReleaseFrameOnGPUThread,
                           gpu_weak_ptr, std::move(frame), sync_token));
      },
      gpu_task_runner_, gpu_weak_this_, frame);

  // Note the use of GetRectSizeFromOrigin() as the coded size. The reason is
  // that the coded_size() of the outgoing FrameResource tells the client what
  // the "usable area" of the frame's buffer is so that it issues rendering
  // commands correctly. For most videos, this usable area is simply
  // frame->visible_rect().size(). However, some H.264 videos define a visible
  // rectangle that doesn't start at (0, 0). For these frames, the usable area
  // includes the non-visible area on the left and on top of the visible area
  // (so that the client can calculate the UV coordinates correctly). Hence the
  // use of GetRectSizeFromOrigin().
  //
  // Most video frames should use visible size instead of coded size because
  // some videos use 0s to pad the frames to coded size, which will cause
  // artifacting along the edges of the image when we scale using bilinear
  // filtering. Tiled protected content is an exception though, because we have
  // a custom Vulkan shader pipeline for scanning out these buffers that needs
  // to know the underlying coded buffer size for detiling computations.
  //
  // The metadata field |needs_detiling| technically comes from an untrusted
  // source, but we don't believe this is a security risk since the worst case
  // scenario simply involves video corruption when the Vulkan detiler
  // misinterprets the frame.
  const gfx::Size coded_size =
      frame->metadata().needs_detiling
          ? frame->coded_size()
          : GetRectSizeFromOrigin(frame->visible_rect());
  scoped_refptr<VideoFrame> mailbox_frame = VideoFrame::WrapNativeTextures(
      frame->format(), mailbox_holders, std::move(release_mailbox_cb),
      coded_size, frame->visible_rect(), frame->natural_size(),
      frame->timestamp());
  mailbox_frame->set_color_space(frame->ColorSpace());
  mailbox_frame->set_hdr_metadata(frame->hdr_metadata());
  mailbox_frame->set_metadata(frame->metadata());

  auto si_format = GetSharedImageFormat(*buffer_format);
  mailbox_frame->set_shared_image_format_type(
      media::SharedImageFormatType::kSharedImageFormat);
  if (si_format.PrefersExternalSampler()) {
    mailbox_frame->set_shared_image_format_type(
        media::SharedImageFormatType::kSharedImageFormatExternalSampler);
  }
  mailbox_frame->metadata().read_lock_fences_enabled = true;
  mailbox_frame->metadata().is_webgpu_compatible =
      frame->metadata().is_webgpu_compatible;

  output_cb_.Run(VideoFrameResource::Create(std::move(mailbox_frame)));
}

void MailboxVideoFrameConverter::ConvertFrameOnGPUThread(
    FrameResource* origin_frame,
    scoped_refptr<FrameResource> frame,
    ScopedSharedImage* stored_shared_image) {
  DCHECK(gpu_task_runner_->BelongsToCurrentThread());
  TRACE_EVENT1("media,gpu", "ConvertFrameOnGPUThread", "FrameResource id",
               origin_frame->unique_id());
  const gfx::ColorSpace src_color_space = frame->ColorSpace();
  const gfx::Rect visible_rect = frame->visible_rect();

  // |origin_frame| is valid as long as |frame| is carried.
  auto wrap_mailbox_and_video_frame_and_output_cb = base::BindOnce(
      &MailboxVideoFrameConverter::WrapMailboxAndVideoFrameAndOutput,
      parent_weak_this_, base::Unretained(origin_frame), std::move(frame));

  // If there's a |stored_shared_image| associated with |origin_frame|, update
  // it and call the continuation callback, otherwise create a SharedImage and
  // register it.
  if (stored_shared_image) {
    DCHECK(!stored_shared_image->mailbox().IsZero());
    bool res;
    if (stored_shared_image->size() == GetRectSizeFromOrigin(visible_rect) &&
        stored_shared_image->color_space() == src_color_space) {
      res = UpdateSharedImageOnGPUThread(stored_shared_image->mailbox());
    } else {
      // Either the existing shared image's size is no longer good enough or the
      // color space has changed. Let's create a new shared image.
      res = GenerateSharedImageOnGPUThread(origin_frame, src_color_space,
                                           visible_rect, stored_shared_image);
    }
    if (res) {
      DCHECK(stored_shared_image->HasData());
      parent_task_runner_->PostTask(
          FROM_HERE,
          base::BindOnce(std::move(wrap_mailbox_and_video_frame_and_output_cb),
                         stored_shared_image->mailbox()));
    }
    return;
  }

  // There was no existing SharedImage: create a new one.
  auto new_shared_image = std::make_unique<ScopedSharedImage>(gpu_task_runner_);
  if (!GenerateSharedImageOnGPUThread(origin_frame, src_color_space,
                                      visible_rect, new_shared_image.get())) {
    return;
  }
  DCHECK(new_shared_image->HasData());

  const gpu::Mailbox mailbox = new_shared_image->mailbox();
  // |origin_frame| is valid as long as |frame| lives. |frame| is kept alive in
  // |wrap_mailbox_and_video_frame_and_output_cb|.
  parent_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&MailboxVideoFrameConverter::RegisterSharedImage,
                     parent_weak_this_, base::Unretained(origin_frame),
                     std::move(new_shared_image)));
  parent_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(std::move(wrap_mailbox_and_video_frame_and_output_cb),
                     mailbox));
}

bool MailboxVideoFrameConverter::GenerateSharedImageOnGPUThread(
    FrameResource* origin_frame,
    const gfx::ColorSpace& src_color_space,
    const gfx::Rect& destination_visible_rect,
    ScopedSharedImage* shared_image) {
  DCHECK(gpu_task_runner_->BelongsToCurrentThread());
  DCHECK(shared_image);
  DVLOGF(4) << "frame: " << origin_frame->unique_id();

  // TODO(crbug.com/998279): consider eager initialization.
  if (!InitializeOnGPUThread()) {
    OnError(FROM_HERE, "InitializeOnGPUThread failed");
    return false;
  }

  const auto buffer_format =
      VideoPixelFormatToGfxBufferFormat(origin_frame->format());
  if (!buffer_format) {
    OnError(FROM_HERE, "Unsupported format: " +
                           VideoPixelFormatToString(origin_frame->format()));
    return false;
  }

  auto gpu_memory_buffer_handle = origin_frame->CreateGpuMemoryBufferHandle();
  DCHECK(!gpu_memory_buffer_handle.is_null());
  DCHECK_EQ(gpu_memory_buffer_handle.type, gfx::NATIVE_PIXMAP);

  gpu::Mailbox mailbox = gpu::Mailbox::GenerateForSharedImage();

  // The SharedImage size ultimately must correspond to the size used to import
  // the decoded frame into a graphics API (e.g., the EGL image size when using
  // OpenGL). For most videos, this is simply |destination_visible_rect|.size().
  // However, some H.264 videos specify a visible rectangle that doesn't start
  // at (0, 0). Since clients are expected to calculate UV coordinates to handle
  // these exotic visible rectangles, we must include the area on the left and
  // on the top of the frames when computing the SharedImage size.
  const gfx::Size shared_image_size =
      origin_frame->metadata().needs_detiling
          ? origin_frame->coded_size()
          : GetRectSizeFromOrigin(destination_visible_rect);

  const std::optional<gpu::SharedImageCapabilities> shared_image_caps =
      gpu_delegate_->GetCapabilities();

  if (!shared_image_caps.has_value()) {
    OnError(FROM_HERE, "Can't get the SharedImageCapabilities");
    return false;
  }

  // The allocated SharedImages should be usable for the (Display) compositor
  // and, potentially, for overlays (Scanout).
  uint32_t shared_image_usage =
      gpu::SHARED_IMAGE_USAGE_DISPLAY_READ | gpu::SHARED_IMAGE_USAGE_SCANOUT;

  // These SharedImages might also be used for zero-copy import into WebGPU to
  // serve as the sources of WebGPU reads (e.g., for video effects processing).
  if (origin_frame->metadata().is_webgpu_compatible &&
      !shared_image_caps->disable_webgpu_shared_images) {
    shared_image_usage |= gpu::SHARED_IMAGE_USAGE_WEBGPU_READ;
  }

  gpu::SharedImageStub::SharedImageDestructionCallback destroy_shared_image_cb =
      gpu_delegate_->CreateSharedImage(
          mailbox, std::move(gpu_memory_buffer_handle),
          GetSharedImageFormat(*buffer_format), shared_image_size,
          src_color_space, kTopLeft_GrSurfaceOrigin, kPremul_SkAlphaType,
          shared_image_usage);
  if (destroy_shared_image_cb.is_null()) {
    OnError(FROM_HERE, "Failed to create shared image.");
    return false;
  }
  // There's no need to UpdateSharedImage() after CreateSharedImage().

  shared_image->Reset(mailbox, shared_image_size, src_color_space,
                      std::move(destroy_shared_image_cb));
  return true;
}

void MailboxVideoFrameConverter::RegisterSharedImage(
    FrameResource* origin_frame,
    std::unique_ptr<ScopedSharedImage> scoped_shared_image) {
  DVLOGF(4) << "frame: " << origin_frame->unique_id();
  DCHECK(parent_task_runner_->RunsTasksInCurrentSequence());
  DCHECK(scoped_shared_image);
  DCHECK(!scoped_shared_image->mailbox().IsZero());
  DCHECK(!base::Contains(shared_images_, origin_frame->unique_id()));

  shared_images_[origin_frame->unique_id()] = scoped_shared_image.get();
  origin_frame->AddDestructionObserver(base::BindOnce(
      [](std::unique_ptr<ScopedSharedImage> shared_image,
         scoped_refptr<base::SequencedTaskRunner> parent_task_runner,
         base::WeakPtr<MailboxVideoFrameConverter> parent_weak_ptr,
         UniqueID origin_frame_id) {
        if (parent_task_runner->RunsTasksInCurrentSequence()) {
          if (parent_weak_ptr)
            parent_weak_ptr->UnregisterSharedImage(origin_frame_id,
                                                   std::move(shared_image));
          return;
        }
        parent_task_runner->PostTask(
            FROM_HERE,
            base::BindOnce(&MailboxVideoFrameConverter::UnregisterSharedImage,
                           parent_weak_ptr, origin_frame_id,
                           std::move(shared_image)));
      },
      std::move(scoped_shared_image), parent_task_runner_, parent_weak_this_,
      origin_frame->unique_id()));
}

bool MailboxVideoFrameConverter::UpdateSharedImageOnGPUThread(
    const gpu::Mailbox& mailbox) {
  DCHECK(gpu_task_runner_->BelongsToCurrentThread());
  if (!gpu_delegate_->UpdateSharedImage(mailbox, gfx::GpuFenceHandle())) {
    OnError(FROM_HERE, "Could not update shared image");
    return false;
  }
  return true;
}

void MailboxVideoFrameConverter::WaitOnSyncTokenAndReleaseFrameOnGPUThread(
    scoped_refptr<FrameResource> frame,
    const gpu::SyncToken& sync_token) {
  DCHECK(gpu_task_runner_->BelongsToCurrentThread());
  if (!gpu_delegate_->WaitOnSyncTokenAndReleaseFrame(std::move(frame),
                                                     sync_token)) {
    return OnError(FROM_HERE,
                   "Could not schedule a task to wait on SyncToken!");
  }
}

void MailboxVideoFrameConverter::UnregisterSharedImage(
    UniqueID origin_frame_id,
    std::unique_ptr<ScopedSharedImage> scoped_shared_image) {
  DCHECK(parent_task_runner_->RunsTasksInCurrentSequence());
  DVLOGF(4);

  auto it = shared_images_.find(origin_frame_id);
  DCHECK(it != shared_images_.end());
  DCHECK(it->second == scoped_shared_image.get());
  shared_images_.erase(it);
}

void MailboxVideoFrameConverter::AbortPendingFrames() {
  DCHECK(parent_task_runner_->RunsTasksInCurrentSequence());
  DVLOGF(4) << "Number of pending frames: " << input_frame_queue_.size();

  input_frame_queue_ = {};
}

bool MailboxVideoFrameConverter::HasPendingFrames() const {
  DCHECK(parent_task_runner_->RunsTasksInCurrentSequence());
  DVLOGF(4) << "Number of pending frames: " << input_frame_queue_.size();

  return !input_frame_queue_.empty();
}

void MailboxVideoFrameConverter::set_get_original_frame_cb(
    GetOriginalFrameCB get_original_frame_cb) {
  DCHECK(parent_task_runner_->RunsTasksInCurrentSequence());
  get_original_frame_cb_ = std::move(get_original_frame_cb);
}

void MailboxVideoFrameConverter::OnError(const base::Location& location,
                                         const std::string& msg) {
  VLOGF(1) << "(" << location.ToString() << ") " << msg;

  parent_task_runner_->PostTask(
      FROM_HERE, base::BindOnce(&MailboxVideoFrameConverter::AbortPendingFrames,
                                parent_weak_this_));
  // Currently we don't have a dedicated callback to notify client that error
  // occurs. Output a null frame to indicate any error occurs.
  // TODO(akahuang): Create an error notification callback.
  parent_task_runner_->PostTask(FROM_HERE, base::BindOnce(output_cb_, nullptr));
}

}  // namespace media

namespace std {

void default_delete<media::MailboxVideoFrameConverter>::operator()(
    media::MailboxVideoFrameConverter* ptr) const {
  ptr->Destroy();
}

}  // namespace std
