// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_GPU_CHROMEOS_MAILBOX_VIDEO_FRAME_CONVERTER_H_
#define MEDIA_GPU_CHROMEOS_MAILBOX_VIDEO_FRAME_CONVERTER_H_

#include <optional>

#include "base/containers/queue.h"
#include "base/containers/small_map.h"
#include "base/functional/callback_forward.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "gpu/command_buffer/common/shared_image_capabilities.h"
#include "gpu/ipc/common/surface_handle.h"
#include "gpu/ipc/service/shared_image_stub.h"
#include "media/gpu/chromeos/frame_resource.h"
#include "media/gpu/media_gpu_export.h"
#include "third_party/skia/include/core/SkAlphaType.h"
#include "third_party/skia/include/gpu/GrTypes.h"
#include "ui/gfx/buffer_types.h"

namespace base {
class Location;
class SingleThreadTaskRunner;
}  // namespace base

namespace gpu {
class CommandBufferStub;
}  // namespace gpu

namespace gfx {
struct GpuFenceHandle;
}  // namespace gfx

namespace media {

// This class is used for converting DMA buffer-backed FrameResources to
// gpu::Mailbox-backed FrameResources. See ConvertFrame() for more details.
// After conversion, the returned gpu::Mailbox-backed FrameResource will retain
// a reference to the FrameResource passed to ConvertFrame().
class MEDIA_GPU_EXPORT MailboxVideoFrameConverter {
 public:
  using OutputCB = base::RepeatingCallback<void(scoped_refptr<FrameResource>)>;
  using GetOriginalFrameCB = base::RepeatingCallback<FrameResource*(
      gfx::GenericSharedMemoryId frame_id)>;
  using GetCommandBufferStubCB =
      base::RepeatingCallback<gpu::CommandBufferStub*()>;

  class GpuDelegate {
   public:
    GpuDelegate() = default;
    GpuDelegate(const GpuDelegate&) = delete;
    GpuDelegate& operator=(const GpuDelegate&) = delete;
    virtual ~GpuDelegate() = default;

    virtual bool Initialize() = 0;
    virtual std::optional<gpu::SharedImageCapabilities> GetCapabilities() = 0;
    virtual gpu::SharedImageStub::SharedImageDestructionCallback
    CreateSharedImage(const gpu::Mailbox& mailbox,
                      gfx::GpuMemoryBufferHandle handle,
                      gfx::BufferFormat format,
                      gfx::BufferPlane plane,
                      const gfx::Size& size,
                      const gfx::ColorSpace& color_space,
                      GrSurfaceOrigin surface_origin,
                      SkAlphaType alpha_type,
                      uint32_t usage) = 0;
    virtual gpu::SharedImageStub::SharedImageDestructionCallback
    CreateSharedImage(const gpu::Mailbox& mailbox,
                      gfx::GpuMemoryBufferHandle handle,
                      viz::SharedImageFormat format,
                      const gfx::Size& size,
                      const gfx::ColorSpace& color_space,
                      GrSurfaceOrigin surface_origin,
                      SkAlphaType alpha_type,
                      uint32_t usage) = 0;
    virtual bool UpdateSharedImage(const gpu::Mailbox& mailbox,
                                   gfx::GpuFenceHandle in_fence_handle) = 0;
    virtual bool WaitOnSyncTokenAndReleaseFrame(
        scoped_refptr<FrameResource> frame,
        const gpu::SyncToken& sync_token) = 0;
  };

  // Creates a MailboxVideoFrameConverter instance. |gpu_task_runner| is the
  // task runner of the GPU main thread. Returns nullptr if the
  // MailboxVideoFrameConverter can't be created.
  static std::unique_ptr<MailboxVideoFrameConverter> Create(
      scoped_refptr<base::SingleThreadTaskRunner> gpu_task_runner,
      GetCommandBufferStubCB get_stub_cb);

  MailboxVideoFrameConverter(const MailboxVideoFrameConverter&) = delete;
  MailboxVideoFrameConverter& operator=(const MailboxVideoFrameConverter&) =
      delete;

  // Initializes the converter. This method must be called before any
  // ConvertFrame() is called.
  void Initialize(scoped_refptr<base::SequencedTaskRunner> parent_task_runner,
                  OutputCB output_cb);

  // Enqueues |frame| to be converted to a gpu::Mailbox-backed
  // VideoFrameResource. If set_get_original_frame_cb() was called with a
  // non-null callback, |frame| must wrap a FrameResource that is retrieved via
  // that callback. Otherwise, |frame| will be used directly. |frame| or the
  // FrameResource that it wraps must be able to be used to create a
  // GpuMemoryBufferHandle. This means that its storage type must be either
  // STORAGE_DMABUFS or STORAGE_GPU_MEMORY_BUFFER. The generated gpu::Mailbox is
  // kept alive until the underlying frame is destroyed. These methods must be
  // called on |parent_task_runner_|.
  void ConvertFrame(scoped_refptr<FrameResource> frame);
  void AbortPendingFrames();
  bool HasPendingFrames() const;

  // Sets the callback to unwrap FrameResources provided to ConvertFrame(). If
  // |get_original_frame_cb| is null or this method is never called at all,
  // ConvertFrame() assumes it's called with unwrapped FrameResources.
  //
  // This method must be called on |parent_task_runner_|.
  //
  // Note: if |get_original_frame_cb| is called at all, it will be called only
  // during a call to ConvertFrame(), so it's guaranteed to be called on
  // |parent_task_runner_|.
  void set_get_original_frame_cb(GetOriginalFrameCB get_original_frame_cb);

 private:
  friend struct std::default_delete<MailboxVideoFrameConverter>;
  friend class MailboxVideoFrameConverterTest;
  friend class MailboxVideoFrameConverterWithUnwrappedFramesTest;

  // Use FrameResource::unique_id() as internal frame indexing.
  using UniqueID = decltype(std::declval<FrameResource>().unique_id());

  // A self-cleaning SharedImage, with move-only semantics.
  class ScopedSharedImage;

  MailboxVideoFrameConverter(
      scoped_refptr<base::SingleThreadTaskRunner> gpu_task_runner,
      std::unique_ptr<GpuDelegate> gpu_delegate);
  // Destructor runs on the GPU main thread.
  ~MailboxVideoFrameConverter();

  void Destroy();
  void DestroyOnGPUThread();

  // TODO(crbug.com/998279): replace s/OnGPUThread/OnGPUTaskRunner/.
  bool InitializeOnGPUThread();

  // Wraps |mailbox| and |frame| into a new VideoFrameResource and sends it via
  // |output_cb_|.
  void WrapMailboxAndVideoFrameAndOutput(FrameResource* origin_frame,
                                         scoped_refptr<FrameResource> frame,
                                         const gpu::Mailbox& mailbox);

  // ConvertFrame() delegates to this method to GenerateSharedImageOnGPUThread()
  // or just UpdateSharedImageOnGPUThread(), then to jump back to
  // WrapMailboxAndVideoFrameAndOutput().
  void ConvertFrameOnGPUThread(FrameResource* origin_frame,
                               scoped_refptr<FrameResource> frame,
                               ScopedSharedImage* stored_shared_image);

  // Populates a ScopedSharedImage from a FrameResource. |origin_frame| must be
  // kept alive for the duration of this method. This method runs on
  // |gpu_task_runner_|. Returns true if the SharedImage could be created
  // successfully; false otherwise (and OnError() is called).
  bool GenerateSharedImageOnGPUThread(FrameResource* origin_frame,
                                      const gfx::ColorSpace& src_color_space,
                                      const gfx::Rect& destination_visible_rect,
                                      ScopedSharedImage* shared_image);

  // Registers the mapping between a FrameResource and the SharedImage.
  // |origin_frame| must be kept alive for the duration of this method. After
  // this method returns, |scoped_shared_image| will be owned by |origin_frame|.
  // This guarantees that the SharedImage lives as long as the associated buffer
  // even if MailboxVideoFrameConverter dies.
  void RegisterSharedImage(
      FrameResource* origin_frame,
      std::unique_ptr<ScopedSharedImage> scoped_shared_image);
  // Unregisters the |origin_frame_id| and associated SharedImage.
  // |scoped_shared_image| is passed to guarantee that the SharedImage is alive
  // until after we delete the pointer from |shared_images_|.
  void UnregisterSharedImage(
      UniqueID origin_frame_id,
      std::unique_ptr<ScopedSharedImage> scoped_shared_image);

  // Updates the SharedImage associated to |mailbox|. Returns true if the update
  // could be carried out, false otherwise.
  bool UpdateSharedImageOnGPUThread(const gpu::Mailbox& mailbox);

  // Waits on |sync_token|, keeping |frame| alive until it is signalled. It
  // trampolines threads to |gpu_task_runner| if necessary.
  void WaitOnSyncTokenAndReleaseFrameOnGPUThread(
      scoped_refptr<FrameResource> frame,
      const gpu::SyncToken& sync_token);

  // Invoked when any error occurs. |msg| is the error message.
  void OnError(const base::Location& location, const std::string& msg);

  // In DmabufVideoFramePool and OOPVideoDecoder, we recycle the unused frames.
  // This is done a bit differently for each case:
  //
  // - For DmabufVideoFramePool: each time a frame is requested from the pool it
  //   is wrapped inside another frame. A destruction callback is then added to
  //   this wrapped frame to automatically return it to the pool upon
  //   destruction.
  //
  // - For OOPVideoDecoder: each time we receive a frame from the remote
  //   decoder, we look it up in a cache of known, previously received buffers
  //   (or insert it into this cache if it's a new buffer). We wrap the known or
  //   new frame inside another frame. A destruction callback is then added to
  //   this wrapped frame to automatically notify the remote decoder that it can
  //   re-use the underlying buffer upon destruction.
  //
  // Unfortunately this means that a new frame is returned each time (i.e., we
  // receive a new FrameResource::unique_id() each time) and we need a way to
  // uniquely identify the underlying frame to avoid converting the same frame
  // multiple times. |get_original_frame_cb_| is used to get the original frame.
  //
  // When |get_original_frame_cb_| is null, we assume it's not necessary to get
  // the original frames, and we just use them directly.
  //
  // TODO(b/195769334): remove the null |get_original_frame_cb_| path because it
  // shouldn't be used after https://crrev.com/c/4457504.
  GetOriginalFrameCB get_original_frame_cb_;

  const scoped_refptr<base::SingleThreadTaskRunner> gpu_task_runner_;
  const std::unique_ptr<GpuDelegate> gpu_delegate_;

  // Mapping from the unique id of the frame to its corresponding SharedImage.
  // Accessed only on |parent_task_runner_|. The ScopedSharedImages are owned by
  // the unwrapped FrameResources so that they can be used even after
  // MailboxVideoFrameConverter dies (e.g., there may still be compositing
  // commands that need the shared images).
  base::small_map<std::map<UniqueID, ScopedSharedImage*>> shared_images_;

  // The queue of input frames and the unique_id of their origin frame.
  // Accessed only on |parent_task_runner_|.
  // TODO(crbug.com/998279): remove this member entirely.
  base::queue<std::pair<scoped_refptr<FrameResource>, UniqueID>>
      input_frame_queue_;

  // The working task runner.
  scoped_refptr<base::SequencedTaskRunner> parent_task_runner_;

  // The callback to return converted frames back to client. This callback will
  // be called on |parent_task_runner_|.
  OutputCB output_cb_;

  // The weak pointer of this, bound to |parent_task_runner_|. Used at the
  // in the original frame's destruction callback.
  base::WeakPtr<MailboxVideoFrameConverter> parent_weak_this_;
  // The weak pointer of this, bound to |gpu_task_runner_|.
  // Used to generate SharedImages on the GPU main thread.
  base::WeakPtr<MailboxVideoFrameConverter> gpu_weak_this_;
  base::WeakPtrFactory<MailboxVideoFrameConverter> parent_weak_this_factory_{
      this};
  base::WeakPtrFactory<MailboxVideoFrameConverter> gpu_weak_this_factory_{this};
};

}  // namespace media

namespace std {

// Specialize std::default_delete to call Destroy().
template <>
struct MEDIA_GPU_EXPORT default_delete<media::MailboxVideoFrameConverter> {
  void operator()(media::MailboxVideoFrameConverter* ptr) const;
};

}  // namespace std

#endif  // MEDIA_GPU_CHROMEOS_MAILBOX_VIDEO_FRAME_CONVERTER_H_
