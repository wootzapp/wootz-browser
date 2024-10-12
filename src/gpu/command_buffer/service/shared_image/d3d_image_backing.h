// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_COMMAND_BUFFER_SERVICE_SHARED_IMAGE_D3D_IMAGE_BACKING_H_
#define GPU_COMMAND_BUFFER_SERVICE_SHARED_IMAGE_D3D_IMAGE_BACKING_H_

#include <windows.h>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include <array>
#include <vector>

#include "base/containers/flat_map.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/synchronization/waitable_event_watcher.h"
#include "base/types/pass_key.h"
#include "components/viz/common/resources/shared_image_format.h"
#include "gpu/command_buffer/service/dxgi_shared_handle_manager.h"
#include "gpu/command_buffer/service/memory_tracking.h"
#include "gpu/command_buffer/service/shared_context_state.h"
#include "gpu/command_buffer/service/shared_image/shared_image_format_service_utils.h"
#include "gpu/command_buffer/service/shared_image/shared_image_manager.h"
#include "gpu/command_buffer/service/shared_image/shared_image_representation.h"
#include "gpu/command_buffer/service/texture_manager.h"
#include "ui/gfx/gpu_memory_buffer.h"
#include "ui/gfx/win/d3d_shared_fence.h"
#include "ui/gl/buildflags.h"
#include "ui/gl/scoped_egl_image.h"

namespace gfx {
class Size;
class ColorSpace;
}  // namespace gfx

namespace gpu {
struct Mailbox;

// Implementation of SharedImageBacking that holds buffer (front buffer/back
// buffer of swap chain) texture (as gles2::Texture/gles2::TexturePassthrough)
// and a reference to created swap chain.
class GPU_GLES2_EXPORT D3DImageBacking final
    : public ClearTrackingSharedImageBacking {
 public:
  // Create a backing wrapping given D3D11 texture, optionally with a shared
  // handle and keyed mutex state. Array slice is used to specify index in
  // texture array used by video decoder and plane index is used to specify the
  // plane (Y/0 or UV/1) in NV12/P010 video textures.
  static std::unique_ptr<D3DImageBacking> Create(
      const Mailbox& mailbox,
      viz::SharedImageFormat format,
      const gfx::Size& size,
      const gfx::ColorSpace& color_space,
      GrSurfaceOrigin surface_origin,
      SkAlphaType alpha_type,
      uint32_t usage,
      std::string debug_label,
      Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture,
      scoped_refptr<DXGISharedHandleState> dxgi_shared_handle_state,
      const GLFormatCaps& gl_format_caps,
      GLenum texture_target,
      size_t array_slice,
      size_t plane_index);

  static std::unique_ptr<D3DImageBacking> CreateFromSwapChainBuffer(
      const Mailbox& mailbox,
      viz::SharedImageFormat format,
      const gfx::Size& size,
      const gfx::ColorSpace& color_space,
      GrSurfaceOrigin surface_origin,
      SkAlphaType alpha_type,
      uint32_t usage,
      Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture,
      Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain,
      const GLFormatCaps& gl_format_caps,
      bool is_back_buffer);

  // Helper used by D3D11VideoDecoder to create backings directly.
  static std::vector<std::unique_ptr<SharedImageBacking>>
  CreateFromVideoTexture(
      base::span<const Mailbox> mailboxes,
      DXGI_FORMAT dxgi_format,
      const gfx::Size& size,
      uint32_t usage,
      unsigned array_slice,
      const GLFormatCaps& gl_format_caps,
      Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture,
      scoped_refptr<DXGISharedHandleState> dxgi_shared_handle_state);

  D3DImageBacking(const D3DImageBacking&) = delete;
  D3DImageBacking& operator=(const D3DImageBacking&) = delete;

  ~D3DImageBacking() override;

  // SharedImageBacking implementation.
  SharedImageBackingType GetType() const override;
  void Update(std::unique_ptr<gfx::GpuFence> in_fence) override;
  bool UploadFromMemory(const std::vector<SkPixmap>& pixmaps) override;
  bool ReadbackToMemory(const std::vector<SkPixmap>& pixmaps) override;
  void ReadbackToMemoryAsync(const std::vector<SkPixmap>& pixmaps,
                             base::OnceCallback<void(bool)> callback) override;
  bool PresentSwapChain() override;
  std::unique_ptr<DawnImageRepresentation> ProduceDawn(
      SharedImageManager* manager,
      MemoryTypeTracker* tracker,
      const wgpu::Device& device,
      wgpu::BackendType backend_type,
      std::vector<wgpu::TextureFormat> view_formats,
      scoped_refptr<SharedContextState> context_state) override;
  void UpdateExternalFence(
      scoped_refptr<gfx::D3DSharedFence> external_fence) override;

  bool BeginAccessD3D11(Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device,
                        bool write_access);
  void EndAccessD3D11(Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device);

  wgpu::Texture BeginAccessDawn(const wgpu::Device& device,
                                wgpu::BackendType backend_type,
                                wgpu::TextureUsage usage,
                                std::vector<wgpu::TextureFormat> view_formats);
  void EndAccessDawn(const wgpu::Device& device, wgpu::Texture texture);

  std::optional<gl::DCLayerOverlayImage> GetDCLayerOverlayImage();

  bool has_keyed_mutex() const {
    return dxgi_shared_handle_state_ &&
           dxgi_shared_handle_state_->has_keyed_mutex();
  }

  scoped_refptr<DXGISharedHandleState> dxgi_shared_handle_state_for_testing()
      const {
    return dxgi_shared_handle_state_;
  }

  Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture_for_testing() const {
    return d3d11_texture_;
  }

  // Holds a gles2::TexturePassthrough and corresponding egl image.
  class GLTextureHolder : public base::RefCounted<GLTextureHolder> {
   public:
    GLTextureHolder(
        base::PassKey<D3DImageBacking>,
        scoped_refptr<gles2::TexturePassthrough> texture_passthrough,
        gl::ScopedEGLImage egl_image);

    const scoped_refptr<gles2::TexturePassthrough>& texture_passthrough()
        const {
      return texture_passthrough_;
    }

    void* egl_image() const { return egl_image_.get(); }
    void set_needs_rebind(bool needs_rebind) { needs_rebind_ = needs_rebind; }

    bool BindEGLImageToTexture();
    void MarkContextLost();

    base::WeakPtr<GLTextureHolder> GetWeakPtr() {
      return weak_ptr_factory_.GetWeakPtr();
    }

   private:
    friend class base::RefCounted<GLTextureHolder>;

    ~GLTextureHolder();

    const scoped_refptr<gles2::TexturePassthrough> texture_passthrough_;
    const gl::ScopedEGLImage egl_image_;
    bool needs_rebind_ = false;

    base::WeakPtrFactory<GLTextureHolder> weak_ptr_factory_{this};
  };

  static scoped_refptr<GLTextureHolder> CreateGLTexture(
      const GLFormatDesc& gl_format_desc,
      const gfx::Size& size,
      const gfx::ColorSpace& color_space,
      Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture,
      GLenum texture_target = GL_TEXTURE_2D,
      unsigned array_slice = 0u,
      unsigned plane_index = 0u,
      Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain = nullptr);

  // Only for test use.
  Microsoft::WRL::ComPtr<IDXGISwapChain1> GetSwapChainForTesting() {
    return swap_chain_;
  }
  Microsoft::WRL::ComPtr<ID3D11Texture2D> GetD3D11TextureForTesting() {
    return d3d11_texture_;
  }

 protected:
  std::unique_ptr<GLTexturePassthroughImageRepresentation>
  ProduceGLTexturePassthrough(SharedImageManager* manager,
                              MemoryTypeTracker* tracker) override;

  std::unique_ptr<OverlayImageRepresentation> ProduceOverlay(
      SharedImageManager* manager,
      MemoryTypeTracker* tracker) override;

  std::unique_ptr<SkiaGaneshImageRepresentation> ProduceSkiaGanesh(
      SharedImageManager* manager,
      MemoryTypeTracker* tracker,
      scoped_refptr<SharedContextState> context_state) override;

#if BUILDFLAG(SKIA_USE_DAWN)
  std::unique_ptr<SkiaGraphiteImageRepresentation> ProduceSkiaGraphite(
      SharedImageManager* manager,
      MemoryTypeTracker* tracker,
      scoped_refptr<SharedContextState> context_state) override;
#endif  // BUILDFLAG(SKIA_USE_DAWN)

  std::unique_ptr<VideoDecodeImageRepresentation> ProduceVideoDecode(
      SharedImageManager* manager,
      MemoryTypeTracker* tracker,
      VideoDecodeDevice device) override;

 private:
  using D3DSharedFenceSet = base::flat_set<scoped_refptr<gfx::D3DSharedFence>>;
  D3DImageBacking(const Mailbox& mailbox,
                  viz::SharedImageFormat format,
                  const gfx::Size& size,
                  const gfx::ColorSpace& color_space,
                  GrSurfaceOrigin surface_origin,
                  SkAlphaType alpha_type,
                  uint32_t usage,
                  std::string debug_label,
                  Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture,
                  scoped_refptr<DXGISharedHandleState> dxgi_shared_handle_state,
                  const GLFormatCaps& gl_format_caps,
                  GLenum texture_target = GL_TEXTURE_2D,
                  size_t array_slice = 0u,
                  size_t plane_index = 0u,
                  Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain = nullptr,
                  bool is_back_buffer = false);

  bool use_fence_synchronization() const {
    // Fences are needed if we're sharing between devices and there's no keyed
    // mutex for synchroniztaion.
    return dxgi_shared_handle_state_ &&
           !dxgi_shared_handle_state_->has_keyed_mutex();
  }

  // Helper to retrieve internal EGLImage for WebGPU GLES compat backend.
  void* GetEGLImage() const;

  // Returns a staging texture for CPU uploads/readback, creating one if needed.
  ID3D11Texture2D* GetOrCreateStagingTexture();

  bool CopyToStagingTexture();
  bool ReadbackFromStagingTexture(const std::vector<SkPixmap>& pixmaps);

  void OnCopyToStagingTextureDone(const std::vector<SkPixmap>& pixmaps,
                                  base::OnceCallback<void(bool)> readback_cb);

  // Common state tracking for both D3D11 and Dawn access.
  bool ValidateBeginAccess(bool write_access) const;
  void BeginAccessCommon(bool write_access);
  void EndAccessCommon(const D3DSharedFenceSet& fences);

  // Get a list of fences to wait on in BeginAccessD3D11/Dawn. If the waiting
  // device is backed by D3D11 (ANGLE or Dawn), |wait_d3d11_device| can be
  // specified to skip over fences for the same device since the wait will be a
  // no-op. Similarly, |wait_dawn_device| can be provided to skip over waits on
  // fences previously signaled on the same Dawn device which are cached in
  // |dawn_signaled_fence_map_|.
  std::vector<scoped_refptr<gfx::D3DSharedFence>> GetPendingWaitFences(
      const Microsoft::WRL::ComPtr<ID3D11Device>& wait_d3d11_device,
      const wgpu::Device& wait_dawn_device,
      bool write_access);

  // Uses either DXGISharedHandleState or internal |dawn_shared_texture_memory_|
  // depending on whether the texture has a shared handle or not.
  wgpu::SharedTextureMemory& GetDawnSharedTextureMemory(
      const wgpu::Device& device);

  // Texture could be nullptr if an empty backing is needed for testing.
  Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture_;

  // Holds DXGI shared handle and the keyed mutex if present.  Can be shared
  // between plane shared image backings of a multi-plane texture, or between
  // backings created from duplicated handles that refer to the same texture.
  scoped_refptr<DXGISharedHandleState> dxgi_shared_handle_state_;

  // Capabilities needed for getting the correct GL format for creating GL
  // textures.
  const GLFormatCaps gl_format_caps_;

  // Weak pointers for gl textures which are owned by GL texture representation.
  std::array<base::WeakPtr<GLTextureHolder>, 3> gl_texture_holders_;

  // GL texture target. Can be GL_TEXTURE_2D or GL_TEXTURE_EXTERNAL_OES.
  // TODO(sunnyps): Switch to GL_TEXTURE_2D for all cases.
  GLenum texture_target_;

  // Index of texture slice in texture array e.g. those used by video decoder.
  const size_t array_slice_;

  // Texture plane index corresponding to this image.
  const size_t plane_index_;

  // Swap chain corresponding to this backing.
  Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain_;

  // Set if this backing corresponds to the back buffer of |swap_chain_|.
  const bool is_back_buffer_;

  // Staging texture used for copy to/from shared memory GMB.
  Microsoft::WRL::ComPtr<ID3D11Texture2D> staging_texture_;

  // D3D11 device corresponding to the |d3d11_texture_| provided on creation.
  // Can be different from the ANGLE D3D11 device when using Graphite.
  Microsoft::WRL::ComPtr<ID3D11Device> texture_d3d11_device_;

  // D3D11 device used by ANGLE. Can be different from |d3d11_device_| when
  // using Graphite.
  Microsoft::WRL::ComPtr<ID3D11Device> angle_d3d11_device_;

  // D3D11 texture descriptor for |d3d11_texture_|.
  D3D11_TEXTURE2D_DESC d3d11_texture_desc_;

  // Whether the backing is being used for exclusive read-write access.
  bool in_write_access_ = false;

  // Number of concurrent readers for this backing.
  int num_readers_ = 0;

  // Fences for previous reads. These will be waited on by the subsequent write,
  // but not by reads.
  D3DSharedFenceSet read_fences_;

  // Fences for the previous write. These will be waited on by subsequent reads
  // and/or write.
  D3DSharedFenceSet write_fences_;

  // Fences used for signaling after D3D11 access. Lazily created as needed.
  // TODO(sunnyps): This doesn't need to be per D3DImageBacking. Find a better
  // place for this so that they can be shared by all backings.
  base::flat_map<Microsoft::WRL::ComPtr<ID3D11Device>,
                 scoped_refptr<gfx::D3DSharedFence>>
      d3d11_signaled_fence_map_;

  // If a shared texture memory exists, it means Dawn produced the D3D12 side of
  // the D3D11 texture created by ID3D12Device::OpenSharedHandle(). Only used if
  // the backing doesn't have a shared handle e.g. for mappable D3D11 textures.
  wgpu::SharedTextureMemory dawn_shared_texture_memory_;

  // Signaled fences imported from Dawn at EndAccess. This can be reused if
  // D3DSharedFence::IsSameFenceAsHandle() is true for fence handle from Dawn.
  base::flat_map<WGPUDevice, D3DSharedFenceSet> dawn_signaled_fences_map_;

  std::optional<base::WaitableEventWatcher> pending_copy_event_watcher_;

  base::WeakPtrFactory<D3DImageBacking> weak_ptr_factory_{this};
};

}  // namespace gpu

#endif  // GPU_COMMAND_BUFFER_SERVICE_SHARED_IMAGE_D3D_IMAGE_BACKING_H_
