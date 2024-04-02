// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/command_buffer/service/shared_image/d3d_image_backing.h"

#include <d3d11_3.h>

#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/memory/ptr_util.h"
#include "base/memory/raw_ptr.h"
#include "gpu/command_buffer/common/shared_image_trace_utils.h"
#include "gpu/command_buffer/common/shared_image_usage.h"
#include "gpu/command_buffer/service/dxgi_shared_handle_manager.h"
#include "gpu/command_buffer/service/shared_image/d3d_image_representation.h"
#include "gpu/command_buffer/service/shared_image/d3d_image_utils.h"
#include "gpu/command_buffer/service/shared_image/shared_image_format_service_utils.h"
#include "gpu/command_buffer/service/shared_image/skia_gl_image_representation.h"
#include "gpu/command_buffer/service/shared_image/skia_graphite_dawn_image_representation.h"
#include "third_party/libyuv/include/libyuv/planar_functions.h"
#include "ui/gl/egl_util.h"
#include "ui/gl/gl_angle_util_win.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_surface_egl.h"
#include "ui/gl/scoped_restore_texture.h"

#if BUILDFLAG(SKIA_USE_DAWN)
#include "gpu/command_buffer/service/dawn_context_provider.h"
#endif

#if BUILDFLAG(USE_DAWN) && BUILDFLAG(DAWN_ENABLE_BACKEND_OPENGLES)
#include "gpu/command_buffer/service/shared_image/dawn_egl_image_representation.h"
#endif

#if BUILDFLAG(USE_DAWN)
#include <dawn/native/D3D11Backend.h>
#include <dawn/native/D3DBackend.h>
using dawn::native::ExternalImageDescriptor;
using dawn::native::d3d::ExternalImageDescriptorD3D11Texture;
using dawn::native::d3d::ExternalImageDescriptorDXGISharedHandle;
using dawn::native::d3d::ExternalImageDXGI;
using dawn::native::d3d::ExternalImageDXGIBeginAccessDescriptor;
using dawn::native::d3d::ExternalImageDXGIFenceDescriptor;
#endif  // BUILDFLAG(USE_DAWN)

#ifndef EGL_ANGLE_image_d3d11_texture
#define EGL_D3D11_TEXTURE_ANGLE 0x3484
#define EGL_TEXTURE_INTERNAL_FORMAT_ANGLE 0x345D
#define EGL_D3D11_TEXTURE_PLANE_ANGLE 0x3492
#define EGL_D3D11_TEXTURE_ARRAY_SLICE_ANGLE 0x3493
#endif /* EGL_ANGLE_image_d3d11_texture */

namespace gpu {

namespace {

size_t NumPlanes(DXGI_FORMAT dxgi_format) {
  switch (dxgi_format) {
    case DXGI_FORMAT_NV12:
    case DXGI_FORMAT_P010:
      return 2;
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
      return 1;
    default:
      NOTREACHED_NORETURN() << "Unsupported DXGI format: " << dxgi_format;
  }
}

viz::SharedImageFormat VideoPlaneFormat(DXGI_FORMAT dxgi_format, size_t plane) {
  DCHECK_LT(plane, NumPlanes(dxgi_format));
  viz::SharedImageFormat format;
  switch (dxgi_format) {
    case DXGI_FORMAT_NV12:
      // Y plane is accessed as R8 and UV plane is accessed as R8G8 in D3D.
      return plane == 0 ? viz::SinglePlaneFormat::kR_8
                        : viz::SinglePlaneFormat::kRG_88;
    case DXGI_FORMAT_P010:
      // Y plane is accessed as R16 and UV plane is accessed as R16G16 in D3D.
      return plane == 0 ? viz::SinglePlaneFormat::kR_16
                        : viz::SinglePlaneFormat::kRG_1616;
    case DXGI_FORMAT_B8G8R8A8_UNORM:
      return viz::SinglePlaneFormat::kBGRA_8888;
    case DXGI_FORMAT_R10G10B10A2_UNORM:
      return viz::SinglePlaneFormat::kRGBA_1010102;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
      return viz::SinglePlaneFormat::kRGBA_F16;
    default:
      NOTREACHED_NORETURN() << "Unsupported DXGI video format: " << dxgi_format;
  }
}

gfx::Size VideoPlaneSize(DXGI_FORMAT dxgi_format,
                         const gfx::Size& size,
                         size_t plane) {
  DCHECK_LT(plane, NumPlanes(dxgi_format));
  switch (dxgi_format) {
    case DXGI_FORMAT_NV12:
    case DXGI_FORMAT_P010:
      // Y plane is full size and UV plane is accessed as half size in D3D.
      return plane == 0 ? size : gfx::Size(size.width() / 2, size.height() / 2);
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
      return size;
    default:
      NOTREACHED_NORETURN() << "Unsupported DXGI video format: " << dxgi_format;
  }
}

// `row_bytes` is the number of bytes that need to be copied in each row, which
// can be smaller than `source_stride` or `dest_stride`.
void CopyPlane(const uint8_t* source_memory,
               size_t source_stride,
               uint8_t* dest_memory,
               size_t dest_stride,
               size_t row_bytes,
               const gfx::Size& size) {
  libyuv::CopyPlane(source_memory, source_stride, dest_memory, dest_stride,
                    row_bytes, size.height());
}

bool BindEGLImageToTexture(GLenum texture_target, void* egl_image) {
  if (!egl_image) {
    LOG(ERROR) << "EGL image is null";
    return false;
  }
  glEGLImageTargetTexture2DOES(texture_target, egl_image);
  if (eglGetError() != static_cast<EGLint>(EGL_SUCCESS)) {
    LOG(ERROR) << "Failed to bind EGL image to the texture"
               << ui::GetLastEGLErrorString();
    return false;
  }
  return true;
}

}  // namespace

D3DImageBacking::GLTextureHolder::GLTextureHolder(
    base::PassKey<D3DImageBacking>,
    scoped_refptr<gles2::TexturePassthrough> texture_passthrough,
    gl::ScopedEGLImage egl_image)
    : texture_passthrough_(std::move(texture_passthrough)),
      egl_image_(std::move(egl_image)) {}

bool D3DImageBacking::GLTextureHolder::BindEGLImageToTexture() {
  if (!needs_rebind_) {
    return true;
  }

  gl::GLApi* const api = gl::g_current_gl_context;
  gl::ScopedRestoreTexture scoped_restore(api, GL_TEXTURE_2D);

  DCHECK_EQ(texture_passthrough_->target(),
            static_cast<unsigned>(GL_TEXTURE_2D));
  api->glBindTextureFn(GL_TEXTURE_2D, texture_passthrough_->service_id());

  if (!::gpu::BindEGLImageToTexture(GL_TEXTURE_2D, egl_image_.get())) {
    return false;
  }

  needs_rebind_ = false;
  return true;
}

void D3DImageBacking::GLTextureHolder::MarkContextLost() {
  if (texture_passthrough_) {
    texture_passthrough_->MarkContextLost();
  }
}

D3DImageBacking::GLTextureHolder::~GLTextureHolder() = default;

// static
scoped_refptr<D3DImageBacking::GLTextureHolder>
D3DImageBacking::CreateGLTexture(
    const GLFormatDesc& gl_format_desc,
    const gfx::Size& size,
    const gfx::ColorSpace& color_space,
    Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture,
    GLenum texture_target,
    unsigned array_slice,
    unsigned plane_index,
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain) {
  gl::GLApi* const api = gl::g_current_gl_context;
  gl::ScopedRestoreTexture scoped_restore(api, texture_target);

  GLuint service_id = 0;
  api->glGenTexturesFn(1, &service_id);
  api->glBindTextureFn(texture_target, service_id);

  // These need to be set for the texture to be considered mipmap complete.
  api->glTexParameteriFn(texture_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  api->glTexParameteriFn(texture_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // These are not strictly required but guard against some checks if NPOT
  // texture support is disabled.
  api->glTexParameteriFn(texture_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  api->glTexParameteriFn(texture_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  const EGLint egl_attrib_list[] = {
      EGL_TEXTURE_INTERNAL_FORMAT_ANGLE,
      static_cast<EGLint>(gl_format_desc.image_internal_format),
      EGL_D3D11_TEXTURE_ARRAY_SLICE_ANGLE,
      static_cast<EGLint>(array_slice),
      EGL_D3D11_TEXTURE_PLANE_ANGLE,
      static_cast<EGLint>(plane_index),
      EGL_NONE};

  auto egl_image = gl::MakeScopedEGLImage(
      EGL_NO_CONTEXT, EGL_D3D11_TEXTURE_ANGLE,
      static_cast<EGLClientBuffer>(d3d11_texture.Get()), egl_attrib_list);

  if (!egl_image.get()) {
    LOG(ERROR) << "Failed to create an EGL image";
    api->glDeleteTexturesFn(1, &service_id);
    return nullptr;
  }

  if (!BindEGLImageToTexture(texture_target, egl_image.get())) {
    return nullptr;
  }

  auto texture = base::MakeRefCounted<gles2::TexturePassthrough>(
      service_id, texture_target);
  GLint texture_memory_size = 0;
  api->glGetTexParameterivFn(texture_target, GL_MEMORY_SIZE_ANGLE,
                             &texture_memory_size);
  texture->SetEstimatedSize(texture_memory_size);

  return base::MakeRefCounted<GLTextureHolder>(base::PassKey<D3DImageBacking>(),
                                               std::move(texture),
                                               std::move(egl_image));
}

// static
std::unique_ptr<D3DImageBacking> D3DImageBacking::CreateFromSwapChainBuffer(
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
    bool is_back_buffer) {
  DCHECK(format.is_single_plane());
  return base::WrapUnique(new D3DImageBacking(
      mailbox, format, size, color_space, surface_origin, alpha_type, usage,
      "SwapChainBuffer", std::move(d3d11_texture),
      /*dxgi_shared_handle_state=*/nullptr, gl_format_caps, GL_TEXTURE_2D,
      /*array_slice=*/0u, /*plane_index=*/0u, std::move(swap_chain),
      is_back_buffer));
}

// static
std::unique_ptr<D3DImageBacking> D3DImageBacking::Create(
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
    size_t plane_index) {
  const bool has_webgpu_usage = !!(usage & (SHARED_IMAGE_USAGE_WEBGPU_READ |
                                            SHARED_IMAGE_USAGE_WEBGPU_WRITE));
  // DXGI shared handle is required for WebGPU/Dawn/D3D12 interop.
  CHECK(!has_webgpu_usage || dxgi_shared_handle_state);
  auto backing = base::WrapUnique(new D3DImageBacking(
      mailbox, format, size, color_space, surface_origin, alpha_type, usage,
      std::move(debug_label), std::move(d3d11_texture),
      std::move(dxgi_shared_handle_state), gl_format_caps, texture_target,
      array_slice, plane_index));
  return backing;
}

// static
std::vector<std::unique_ptr<SharedImageBacking>>
D3DImageBacking::CreateFromVideoTexture(
    base::span<const Mailbox> mailboxes,
    DXGI_FORMAT dxgi_format,
    const gfx::Size& size,
    uint32_t usage,
    unsigned array_slice,
    const GLFormatCaps& gl_format_caps,
    Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture,
    scoped_refptr<DXGISharedHandleState> dxgi_shared_handle_state) {
  CHECK(d3d11_texture);
  CHECK_EQ(mailboxes.size(), NumPlanes(dxgi_format));

  // DXGI shared handle is required for WebGPU/Dawn/D3D12 interop.
  const bool has_webgpu_usage = usage & (SHARED_IMAGE_USAGE_WEBGPU_READ |
                                         SHARED_IMAGE_USAGE_WEBGPU_WRITE);
  CHECK(!has_webgpu_usage || dxgi_shared_handle_state);

  std::vector<std::unique_ptr<SharedImageBacking>> shared_images(
      NumPlanes(dxgi_format));
  for (size_t plane_index = 0; plane_index < shared_images.size();
       plane_index++) {
    const auto& mailbox = mailboxes[plane_index];

    const auto plane_format = VideoPlaneFormat(dxgi_format, plane_index);
    const auto plane_size = VideoPlaneSize(dxgi_format, size, plane_index);

    // Shared image does not need to store the colorspace since it is already
    // stored on the VideoFrame which is provided upon presenting the overlay.
    // To prevent the developer from mistakenly using it, provide the invalid
    // value from default-construction.
    constexpr gfx::ColorSpace kInvalidColorSpace;

    // The target must be GL_TEXTURE_EXTERNAL_OES as the texture is not created
    // with D3D11_BIND_RENDER_TARGET bind flag and so it cannot be bound to the
    // framebuffer. To prevent Skia trying to bind it for read pixels, we need
    // it to be GL_TEXTURE_EXTERNAL_OES.
    constexpr GLenum kTextureTarget = GL_TEXTURE_EXTERNAL_OES;

    // Do not cache GL textures in the backing since it's owned by the video
    // decoder, and there could be no GL context to MakeCurrent in the
    // destructor.
    shared_images[plane_index] = base::WrapUnique(new D3DImageBacking(
        mailbox, plane_format, plane_size, kInvalidColorSpace,
        kTopLeft_GrSurfaceOrigin, kPremul_SkAlphaType, usage, "VideoTexture",
        d3d11_texture, dxgi_shared_handle_state, gl_format_caps, kTextureTarget,
        array_slice, plane_index));
    if (!shared_images[plane_index])
      return {};

    shared_images[plane_index]->SetCleared();
  }

  return shared_images;
}

D3DImageBacking::D3DImageBacking(
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
    size_t plane_index,
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain,
    bool is_back_buffer)
    : ClearTrackingSharedImageBacking(mailbox,
                                      format,
                                      size,
                                      color_space,
                                      surface_origin,
                                      alpha_type,
                                      usage,
                                      std::move(debug_label),
                                      format.EstimatedSizeInBytes(size),
                                      /*is_thread_safe=*/false),
      d3d11_texture_(std::move(d3d11_texture)),
      dxgi_shared_handle_state_(std::move(dxgi_shared_handle_state)),
      gl_format_caps_(gl_format_caps),
      texture_target_(texture_target),
      array_slice_(array_slice),
      plane_index_(plane_index),
      swap_chain_(std::move(swap_chain)),
      is_back_buffer_(is_back_buffer),
      angle_d3d11_device_(gl::QueryD3D11DeviceObjectFromANGLE()) {
  if (d3d11_texture_) {
    d3d11_texture_->GetDevice(&texture_d3d11_device_);
    d3d11_texture_->GetDesc(&d3d11_texture_desc_);
  }
}

D3DImageBacking::~D3DImageBacking() {
  if (!have_context()) {
    for (auto& texture_holder : gl_texture_holders_) {
      if (texture_holder) {
        texture_holder->MarkContextLost();
      }
    }
  }
}

ID3D11Texture2D* D3DImageBacking::GetOrCreateStagingTexture() {
  if (!staging_texture_) {
    D3D11_TEXTURE2D_DESC staging_desc = {};
    staging_desc.Width = d3d11_texture_desc_.Width;
    staging_desc.Height = d3d11_texture_desc_.Height;
    staging_desc.Format = d3d11_texture_desc_.Format;
    staging_desc.MipLevels = 1;
    staging_desc.ArraySize = 1;
    staging_desc.SampleDesc.Count = 1;
    staging_desc.Usage = D3D11_USAGE_STAGING;
    staging_desc.CPUAccessFlags =
        D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

    CHECK(texture_d3d11_device_);
    HRESULT hr = texture_d3d11_device_->CreateTexture2D(&staging_desc, nullptr,
                                                        &staging_texture_);
    if (FAILED(hr)) {
      LOG(ERROR) << "Failed to create staging texture. hr=" << std::hex << hr;
      return nullptr;
    }

    constexpr char kStagingTextureLabel[] = "SharedImageD3D_StagingTexture";
    // Add debug label to the long lived texture.
    staging_texture_->SetPrivateData(WKPDID_D3DDebugObjectName,
                                     strlen(kStagingTextureLabel),
                                     kStagingTextureLabel);
  }
  return staging_texture_.Get();
}

SharedImageBackingType D3DImageBacking::GetType() const {
  return SharedImageBackingType::kD3D;
}

void D3DImageBacking::Update(std::unique_ptr<gfx::GpuFence> in_fence) {
  // Do nothing since D3DImageBackings are only ever backed by DXGI GMB handles,
  // which are synonymous with D3D textures, and no explicit update is needed.
}

bool D3DImageBacking::UploadFromMemory(const std::vector<SkPixmap>& pixmaps) {
  DCHECK_EQ(pixmaps.size(), static_cast<size_t>(format().NumberOfPlanes()));

  CHECK(texture_d3d11_device_);
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context;
  texture_d3d11_device_->GetImmediateContext(&device_context);

  if (d3d11_texture_desc_.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE) {
    // D3D doesn't support mappable+default YUV textures.
    DCHECK(format().is_single_plane());

    Microsoft::WRL::ComPtr<ID3D11Device3> device3;
    HRESULT hr = texture_d3d11_device_.As(&device3);
    if (FAILED(hr)) {
      LOG(ERROR) << "Failed to retrieve ID3D11Device3. hr=" << std::hex << hr;
      return false;
    }
    hr = device_context->Map(d3d11_texture_.Get(), 0, D3D11_MAP_WRITE, 0,
                             nullptr);
    if (FAILED(hr)) {
      LOG(ERROR) << "Failed to map texture for write. hr = " << std::hex << hr;
      return false;
    }

    const uint8_t* source_memory =
        static_cast<const uint8_t*>(pixmaps[0].addr());
    const size_t source_stride = pixmaps[0].rowBytes();
    device3->WriteToSubresource(d3d11_texture_.Get(), 0, nullptr, source_memory,
                                source_stride, 0);
    device_context->Unmap(d3d11_texture_.Get(), 0);
  } else {
    ID3D11Texture2D* staging_texture = GetOrCreateStagingTexture();
    if (!staging_texture) {
      return false;
    }
    D3D11_MAPPED_SUBRESOURCE mapped_resource = {};
    HRESULT hr = device_context->Map(staging_texture, 0, D3D11_MAP_WRITE, 0,
                                     &mapped_resource);
    if (FAILED(hr)) {
      LOG(ERROR) << "Failed to map texture for write. hr=" << std::hex << hr;
      return false;
    }

    // The mapped staging texture pData points to the first plane's data so an
    // offset is needed for subsequent planes.
    size_t dest_offset = 0;

    for (int plane = 0; plane < format().NumberOfPlanes(); ++plane) {
      auto& pixmap = pixmaps[plane];
      const uint8_t* source_memory = static_cast<const uint8_t*>(pixmap.addr());
      const size_t source_stride = pixmap.rowBytes();

      uint8_t* dest_memory =
          static_cast<uint8_t*>(mapped_resource.pData) + dest_offset;
      const size_t dest_stride = mapped_resource.RowPitch;

      gfx::Size plane_size = format().GetPlaneSize(plane, size());
      CopyPlane(source_memory, source_stride, dest_memory, dest_stride,
                pixmap.info().minRowBytes(), plane_size);

      dest_offset += mapped_resource.RowPitch * plane_size.height();
    }

    device_context->Unmap(staging_texture, 0);
    device_context->CopyResource(d3d11_texture_.Get(), staging_texture);
  }
  return true;
}

bool D3DImageBacking::ReadbackToMemory(const std::vector<SkPixmap>& pixmaps) {
  CHECK(texture_d3d11_device_);
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context;
  texture_d3d11_device_->GetImmediateContext(&device_context);

  if (d3d11_texture_desc_.CPUAccessFlags & D3D11_CPU_ACCESS_READ) {
    // D3D doesn't support mappable+default YUV textures.
    DCHECK(format().is_single_plane());

    Microsoft::WRL::ComPtr<ID3D11Device3> device3;
    HRESULT hr = texture_d3d11_device_.As(&device3);
    if (FAILED(hr)) {
      LOG(ERROR) << "Failed to retrieve ID3D11Device3. hr=" << std::hex << hr;
      return false;
    }
    hr = device_context->Map(d3d11_texture_.Get(), 0, D3D11_MAP_READ, 0,
                             nullptr);
    if (FAILED(hr)) {
      LOG(ERROR) << "Failed to map texture for read. hr=" << std::hex << hr;
      return false;
    }

    uint8_t* dest_memory = static_cast<uint8_t*>(pixmaps[0].writable_addr());
    const size_t dest_stride = pixmaps[0].rowBytes();
    device3->ReadFromSubresource(dest_memory, dest_stride, 0,
                                 d3d11_texture_.Get(), 0, nullptr);
    device_context->Unmap(d3d11_texture_.Get(), 0);
  } else {
    ID3D11Texture2D* staging_texture = GetOrCreateStagingTexture();
    if (!staging_texture) {
      return false;
    }
    device_context->CopyResource(staging_texture, d3d11_texture_.Get());
    D3D11_MAPPED_SUBRESOURCE mapped_resource = {};
    HRESULT hr = device_context->Map(staging_texture, 0, D3D11_MAP_READ, 0,
                                     &mapped_resource);
    if (FAILED(hr)) {
      LOG(ERROR) << "Failed to map texture for read. hr=" << std::hex << hr;
      return false;
    }

    // The mapped staging texture pData points to the first plane's data so an
    // offset is needed for subsequent planes.
    size_t source_offset = 0;

    for (int plane = 0; plane < format().NumberOfPlanes(); ++plane) {
      auto& pixmap = pixmaps[plane];
      uint8_t* dest_memory = static_cast<uint8_t*>(pixmap.writable_addr());
      const size_t dest_stride = pixmap.rowBytes();

      const uint8_t* source_memory =
          static_cast<uint8_t*>(mapped_resource.pData) + source_offset;
      const size_t source_stride = mapped_resource.RowPitch;

      gfx::Size plane_size = format().GetPlaneSize(plane, size());
      CopyPlane(source_memory, source_stride, dest_memory, dest_stride,
                pixmap.info().minRowBytes(), plane_size);

      source_offset += mapped_resource.RowPitch * plane_size.height();
    }

    device_context->Unmap(staging_texture, 0);
  }
  return true;
}

#if BUILDFLAG(USE_DAWN)
std::unique_ptr<DawnImageRepresentation> D3DImageBacking::ProduceDawn(
    SharedImageManager* manager,
    MemoryTypeTracker* tracker,
    const wgpu::Device& device,
    wgpu::BackendType backend_type,
    std::vector<wgpu::TextureFormat> view_formats,
    scoped_refptr<SharedContextState> context_state) {
#if BUILDFLAG(DAWN_ENABLE_BACKEND_OPENGLES)
  if (backend_type == wgpu::BackendType::OpenGLES) {
    std::unique_ptr<GLTextureImageRepresentationBase> gl_representation =
        ProduceGLTexturePassthrough(manager, tracker);
    auto* d3d_representation =
        static_cast<GLTexturePassthroughD3DImageRepresentation*>(
            gl_representation.get());
    void* egl_image = d3d_representation->GetEGLImage();
    if (!egl_image) {
      LOG(ERROR) << "EGL image is null.";
      return nullptr;
    }
    return std::make_unique<DawnEGLImageRepresentation>(
        std::move(gl_representation), egl_image, manager, this, tracker,
        device);
  }
#endif

  if (backend_type != wgpu::BackendType::D3D11 &&
      backend_type != wgpu::BackendType::D3D12) {
    LOG(ERROR) << "Unsupported Dawn backend: "
               << static_cast<WGPUBackendType>(backend_type);
    return nullptr;
  }

  // Persistently open the shared handle by caching it on this backing.
  auto& external_image = GetDawnExternalImage(device.Get());
  if (!external_image) {
    Microsoft::WRL::ComPtr<ID3D11Device> dawn_d3d11_device;
    if (backend_type == wgpu::BackendType::D3D11) {
      dawn_d3d11_device = dawn::native::d3d11::GetD3D11Device(device.Get());
    }
    if (dawn_d3d11_device == texture_d3d11_device_) {
      external_image = CreateDawnExternalImageDXGI(
          device, usage(), d3d11_texture_desc_, d3d11_texture_, view_formats);
    } else {
      CHECK(dxgi_shared_handle_state_);
      const HANDLE shared_handle = dxgi_shared_handle_state_->GetSharedHandle();
      CHECK(base::win::HandleTraits::IsHandleValid(shared_handle));
      external_image = CreateDawnExternalImageDXGI(
          device, usage(), d3d11_texture_desc_, shared_handle, view_formats);
    }

    if (!external_image) {
      return nullptr;
    }
  }

  return std::make_unique<DawnD3DImageRepresentation>(manager, this, tracker,
                                                      device, backend_type);
}
#endif  // BUILDFLAG(USE_DAWN)

void D3DImageBacking::UpdateExternalFence(
    scoped_refptr<gfx::D3DSharedFence> external_fence) {
  if (!write_fence_) {
    write_fence_ = std::move(external_fence);
  }

  // TODO(crbug.com/1236801): Handle cases that write_fence_ exists.
}

std::unique_ptr<VideoDecodeImageRepresentation>
D3DImageBacking::ProduceVideoDecode(SharedImageManager* manager,
                                    MemoryTypeTracker* tracker,
                                    VideoDecodeDevice device) {
  return std::make_unique<D3D11VideoDecodeImageRepresentation>(
      manager, this, tracker, device, d3d11_texture_);
}

std::vector<scoped_refptr<gfx::D3DSharedFence>>
D3DImageBacking::GetPendingWaitFences(
    const Microsoft::WRL::ComPtr<ID3D11Device>& wait_d3d11_device,
    const wgpu::Device& wait_dawn_device,
    bool write_access) {
  // We don't need to use fences for single device scenarios (no shared handle),
  // or if we're using a keyed mutex instead.
  if (!use_fence_synchronization()) {
    return {};
  }

  // Lazily create and signal the D3D11 fence on the texture's original device
  // if not present and we're using the backing on another device.
  auto& texture_device_fence = d3d11_signaled_fence_map_[texture_d3d11_device_];
  if (wait_d3d11_device != texture_d3d11_device_ && !texture_device_fence) {
    texture_device_fence =
        gfx::D3DSharedFence::CreateForD3D11(texture_d3d11_device_);
    if (!texture_device_fence) {
      LOG(ERROR) << "Failed to retrieve D3D11 signal fence";
      return {};
    }
    // Make D3D11 device wait for |write_fence_| since we'll replace it below.
    if (write_fence_ && !write_fence_->WaitD3D11(texture_d3d11_device_)) {
      LOG(ERROR) << "Failed to wait for write fence";
      return {};
    }
    if (!texture_device_fence->IncrementAndSignalD3D11()) {
      LOG(ERROR) << "Failed to signal D3D11 signal fence";
      return {};
    }
    // Store it in |write_fence_| so it's waited on for all subsequent access.
    write_fence_ = texture_device_fence;
  }

#if BUILDFLAG(USE_DAWN)
  const gfx::D3DSharedFence* dawn_signaled_fence =
      wait_dawn_device ? dawn_signaled_fence_map_[wait_dawn_device.Get()].get()
                       : nullptr;
#else
  const gfx::D3DSharedFence* dawn_signaled_fence = nullptr;
#endif

  auto should_wait_on_fence = [&](gfx::D3DSharedFence* wait_fence) -> bool {
    // Skip the wait if it's for the fence last signaled by the Dawn device, or
    // for D3D11 if the fence was issued for the same device since D3D11 uses a
    // single immediate context for issuing commands on a device.
    CHECK(wait_fence);
    return (wait_dawn_device && dawn_signaled_fence != wait_fence) ||
           (wait_d3d11_device &&
            wait_d3d11_device != wait_fence->GetD3D11Device());
  };

  std::vector<scoped_refptr<gfx::D3DSharedFence>> wait_fences;
  // Always wait for previous write for both read-only or read-write access.
  // Skip the wait if it's for the fence last signaled by the Dawn device, or
  // for D3D11 if the fence was issued for the same device since D3D11 has a
  // single immediate context for issuing commands.
  if (write_fence_ && should_wait_on_fence(write_fence_.get())) {
    wait_fences.push_back(write_fence_);
  }
  // Also wait for all previous reads for read-write access.
  if (write_access) {
    for (const auto& read_fence : read_fences_) {
      if (should_wait_on_fence(read_fence.get())) {
        wait_fences.push_back(read_fence);
      }
    }
  }
  return wait_fences;
}

#if BUILDFLAG(USE_DAWN)
wgpu::Texture D3DImageBacking::BeginAccessDawn(const wgpu::Device& device,
                                               wgpu::BackendType backend_type,
                                               wgpu::TextureUsage wgpu_usage) {
  const bool write_access = wgpu_usage & (wgpu::TextureUsage::CopyDst |
                                          wgpu::TextureUsage::StorageBinding |
                                          wgpu::TextureUsage::RenderAttachment);

  if (!ValidateBeginAccess(write_access)) {
    return nullptr;
  }

  Microsoft::WRL::ComPtr<ID3D11Device> dawn_d3d11_device;
  if (backend_type == wgpu::BackendType::D3D11) {
    dawn_d3d11_device = dawn::native::d3d11::GetD3D11Device(device.Get());
    CHECK(dawn_d3d11_device);
  }

  // Dawn access is allowed without shared handle for single device scenarios.
  CHECK(dxgi_shared_handle_state_ ||
        dawn_d3d11_device == texture_d3d11_device_);

  auto& external_image = GetDawnExternalImage(device);
  CHECK(external_image);

  ExternalImageDXGIBeginAccessDescriptor desc;
  desc.isInitialized = IsCleared();
  desc.isSwapChainTexture =
      (usage() & SHARED_IMAGE_USAGE_WEBGPU_SWAP_CHAIN_TEXTURE);
  desc.usage = static_cast<WGPUTextureUsage>(wgpu_usage);

  // Defer clearing fences until later to handle Dawn failure to import texture.
  std::vector<scoped_refptr<gfx::D3DSharedFence>> wait_fences =
      GetPendingWaitFences(dawn_d3d11_device, device, write_access);
  for (auto& wait_fence : wait_fences) {
    desc.waitFences.push_back(ExternalImageDXGIFenceDescriptor{
        wait_fence->GetSharedHandle(), wait_fence->GetFenceValue()});
  }

  auto texture = wgpu::Texture::Acquire(external_image->BeginAccess(&desc));
  if (!texture) {
    LOG(ERROR) << "Failed to begin access and produce WGPUTexture";
    return nullptr;
  }

  // Clear fences and update state iff Dawn BeginAccess succeeds.
  BeginAccessCommon(write_access);
  return texture;
}

void D3DImageBacking::EndAccessDawn(const wgpu::Device& device,
                                    wgpu::Texture texture) {
  DCHECK(texture);
  if (dawn::native::IsTextureSubresourceInitialized(texture.Get(), 0, 1, 0,
                                                    1)) {
    SetCleared();
  }

  // External image is removed from cache on first EndAccess after device is
  // lost. It's ok to skip synchronization because it should've already been
  // synchronized before the entry was removed from the cache.
  if (auto& external_image = GetDawnExternalImage(device.Get())) {
    // EndAccess returns a null fence handle if the device was lost, but that's
    // OK since we check for it explicitly below.
    ExternalImageDXGIFenceDescriptor descriptor;
    external_image->EndAccess(texture.Get(), &descriptor);

    scoped_refptr<gfx::D3DSharedFence> signaled_fence;
    if (use_fence_synchronization() && descriptor.fenceHandle != nullptr) {
      auto& cached_fence = dawn_signaled_fence_map_[device.Get()];
      // Try to reuse the last signaled fence if it's the same fence.
      if (!cached_fence ||
          !cached_fence->IsSameFenceAsHandle(descriptor.fenceHandle)) {
        cached_fence = gfx::D3DSharedFence::CreateFromUnownedHandle(
            descriptor.fenceHandle);
      }
      if (cached_fence) {
        cached_fence->Update(descriptor.fenceValue);
        signaled_fence = cached_fence;
      } else {
        LOG(ERROR) << "Failed to import D3D fence from Dawn on EndAccess";
      }
    }

    if (!external_image->IsValid()) {
      // Erase from cache if external image is invalid i.e. device was lost.
      dawn_signaled_fence_map_.erase(device.Get());
      dxgi_shared_handle_state_->EraseDawnExternalImage(device.Get());
    }

    EndAccessCommon(std::move(signaled_fence));
  }
}

std::unique_ptr<ExternalImageDXGI>& D3DImageBacking::GetDawnExternalImage(
    const wgpu::Device& device) {
  return dxgi_shared_handle_state_
             ? dxgi_shared_handle_state_->GetDawnExternalImage(device.Get())
             : dawn_external_image_;
}
#endif

bool D3DImageBacking::BeginAccessD3D11(
    Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device,
    bool write_access) {
  if (!ValidateBeginAccess(write_access)) {
    return false;
  }

  // Defer clearing fences until later to handle D3D11 failure to synchronize.
  std::vector<scoped_refptr<gfx::D3DSharedFence>> wait_fences =
      GetPendingWaitFences(d3d11_device, /*dawn_device=*/nullptr, write_access);
  for (auto& wait_fence : wait_fences) {
    if (!wait_fence->WaitD3D11(d3d11_device)) {
      LOG(ERROR) << "Failed to wait for fence";
      return false;
    }
  }

  // D3D11 access is allowed without shared handle for single device scenarios.
  CHECK(dxgi_shared_handle_state_ || d3d11_device == texture_d3d11_device_);
  if (dxgi_shared_handle_state_ &&
      !dxgi_shared_handle_state_->AcquireKeyedMutex(d3d11_device)) {
    LOG(ERROR) << "Failed to synchronize using keyed mutex";
    return false;
  }

  // Clear fences and update state iff D3D11 BeginAccess succeeds.
  BeginAccessCommon(write_access);
  return true;
}

void D3DImageBacking::EndAccessD3D11(
    Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device) {
  const bool is_texture_device = d3d11_device == texture_d3d11_device_;
  // If shared handle is not present, we can only access on the same device.
  CHECK(dxgi_shared_handle_state_ || is_texture_device);
  // Do not create a fence for the texture's original device if we're only using
  // the texture on one device or using a keyed mutex. The fence is lazily
  // created on the first access from another device in GetPendingWaitFences().
  scoped_refptr<gfx::D3DSharedFence> signaled_fence;
  if (use_fence_synchronization()) {
    auto& d3d11_signal_fence = d3d11_signaled_fence_map_[d3d11_device];
    if (!d3d11_signal_fence) {
      d3d11_signal_fence = gfx::D3DSharedFence::CreateForD3D11(d3d11_device);
    }
    if (d3d11_signal_fence && d3d11_signal_fence->IncrementAndSignalD3D11()) {
      signaled_fence = d3d11_signal_fence;
    } else {
      LOG(ERROR) << "Failed to signal D3D11 device fence on EndAccess";
    }
  }

  if (dxgi_shared_handle_state_) {
    dxgi_shared_handle_state_->ReleaseKeyedMutex(d3d11_device);
  }

  EndAccessCommon(std::move(signaled_fence));
}

bool D3DImageBacking::ValidateBeginAccess(bool write_access) const {
  if (in_write_access_) {
    LOG(ERROR) << "Already being accessed for write";
    return false;
  }
  if (write_access && num_readers_ > 0) {
    LOG(ERROR) << "Already being accessed for read";
    return false;
  }
  return true;
}

void D3DImageBacking::BeginAccessCommon(bool write_access) {
  if (write_access) {
    // For read-write access, we wait for all previous reads and reset fences
    // since all subsequent access will wait on |write_fence_| generated when
    // this access ends.
    write_fence_.reset();
    read_fences_.clear();
    in_write_access_ = true;
  } else {
    num_readers_++;
  }
}

void D3DImageBacking::EndAccessCommon(
    scoped_refptr<gfx::D3DSharedFence> signaled_fence) {
  if (in_write_access_) {
    DCHECK(!write_fence_);
    DCHECK(read_fences_.empty());
    in_write_access_ = false;
    write_fence_ = std::move(signaled_fence);
  } else {
    num_readers_--;
    if (signaled_fence)
      read_fences_.insert(std::move(signaled_fence));
  }
}

void* D3DImageBacking::GetEGLImage() const {
  DCHECK(format().is_single_plane());
  return gl_texture_holders_[0] ? gl_texture_holders_[0]->egl_image() : nullptr;
}

bool D3DImageBacking::PresentSwapChain() {
  TRACE_EVENT0("gpu", "D3DImageBacking::PresentSwapChain");
  if (!swap_chain_ || !is_back_buffer_) {
    LOG(ERROR) << "Backing does not correspond to back buffer of swap chain";
    return false;
  }

  constexpr UINT kFlags = DXGI_PRESENT_ALLOW_TEARING;
  constexpr DXGI_PRESENT_PARAMETERS kParams = {};

  HRESULT hr = swap_chain_->Present1(/*interval=*/0, kFlags, &kParams);
  if (FAILED(hr)) {
    LOG(ERROR) << "Present1 failed with error " << std::hex << hr;
    return false;
  }

  DCHECK(format().is_single_plane());

  // we're rebinding to ensure that underlying D3D11 resource views are
  // recreated in ANGLE.
  if (gl_texture_holders_[0]) {
    gl_texture_holders_[0]->set_needs_rebind(true);
  }

  // Flush device context otherwise present could be deferred.
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11_device_context;
  texture_d3d11_device_->GetImmediateContext(&d3d11_device_context);
  d3d11_device_context->Flush();

  return true;
}

std::unique_ptr<GLTexturePassthroughImageRepresentation>
D3DImageBacking::ProduceGLTexturePassthrough(SharedImageManager* manager,
                                             MemoryTypeTracker* tracker) {
  TRACE_EVENT0("gpu", "D3DImageBacking::ProduceGLTexturePassthrough");

  const auto number_of_planes = static_cast<size_t>(format().NumberOfPlanes());
  std::vector<scoped_refptr<GLTextureHolder>> gl_texture_holders(
      number_of_planes);
  DCHECK_GE(gl_texture_holders_.size(), number_of_planes);

  // If DXGI shared handle is present, the |d3d11_texture_| might belong to a
  // different device with Graphite so retrieve the ANGLE specific D3D11
  // texture from the |dxgi_shared_handle_state_|.
  const bool is_angle_texture = texture_d3d11_device_ == angle_d3d11_device_;
  CHECK(is_angle_texture || dxgi_shared_handle_state_);
  auto d3d11_texture = is_angle_texture
                           ? d3d11_texture_
                           : dxgi_shared_handle_state_->GetOrCreateD3D11Texture(
                                 angle_d3d11_device_);
  if (!d3d11_texture) {
    LOG(ERROR) << "Failed to open DXGI shared handle";
    return nullptr;
  }

  for (int plane = 0; plane < format().NumberOfPlanes(); ++plane) {
    auto& holder = gl_texture_holders[plane];
    if (gl_texture_holders_[plane]) {
      holder = gl_texture_holders_[plane].get();
      continue;
    }

    // The GL internal format can differ from the underlying swap chain or
    // texture format e.g. RGBA or RGB instead of BGRA or RED/RG for NV12
    // texture planes. See EGL_ANGLE_d3d_texture_client_buffer spec for format
    // restrictions.
    GLFormatDesc gl_format_desc;
    if (format().is_multi_plane()) {
      gl_format_desc = gl_format_caps_.ToGLFormatDesc(format(), plane);
    } else {
      // For legacy multiplanar formats, `format` is already plane format (eg.
      // RED, RG), so we pass plane_index=0.
      gl_format_desc =
          gl_format_caps_.ToGLFormatDesc(format(), /*plane_index=*/0);
    }

    gfx::Size plane_size = format().GetPlaneSize(plane, size());
    // For legacy multiplanar formats, format() is plane format (eg. RED, RG)
    // which is_single_plane(), but the real plane is in plane_index_ so we
    // pass that.
    unsigned plane_id = format().is_single_plane() ? plane_index_ : plane;
    // Creating the GL texture doesn't require exclusive access to the
    // underlying D3D11 texture.
    holder = CreateGLTexture(gl_format_desc, plane_size, color_space(),
                             d3d11_texture, texture_target_, array_slice_,
                             plane_id, swap_chain_);
    if (!holder) {
      LOG(ERROR) << "Failed to create GL texture for plane: " << plane;
      return nullptr;
    }
    // Cache the gl textures using weak pointers.
    gl_texture_holders_[plane] = holder->GetWeakPtr();
  }

  return std::make_unique<GLTexturePassthroughD3DImageRepresentation>(
      manager, this, tracker, angle_d3d11_device_,
      std::move(gl_texture_holders));
}

std::unique_ptr<SkiaGaneshImageRepresentation>
D3DImageBacking::ProduceSkiaGanesh(
    SharedImageManager* manager,
    MemoryTypeTracker* tracker,
    scoped_refptr<SharedContextState> context_state) {
  auto gl_representation = ProduceGLTexturePassthrough(manager, tracker);
  if (!gl_representation) {
    return nullptr;
  }
  return SkiaGLImageRepresentation::Create(std::move(gl_representation),
                                           std::move(context_state), manager,
                                           this, tracker);
}

#if BUILDFLAG(SKIA_USE_DAWN)
std::unique_ptr<SkiaGraphiteImageRepresentation>
D3DImageBacking::ProduceSkiaGraphite(
    SharedImageManager* manager,
    MemoryTypeTracker* tracker,
    scoped_refptr<SharedContextState> context_state) {
  auto device = context_state->dawn_context_provider()->GetDevice();
  wgpu::AdapterProperties adapter_properties;
  device.GetAdapter().GetProperties(&adapter_properties);
  auto dawn_representation =
      ProduceDawn(manager, tracker, device.Get(),
                  adapter_properties.backendType, {}, context_state);
  const bool is_yuv_plane = NumPlanes(d3d11_texture_desc_.Format) > 1;
  return SkiaGraphiteDawnImageRepresentation::Create(
      std::move(dawn_representation), context_state,
      context_state->gpu_main_graphite_recorder(), manager, this, tracker,
      is_yuv_plane, plane_index_);
}
#endif  // BUILDFLAG(SKIA_USE_DAWN)

std::unique_ptr<OverlayImageRepresentation> D3DImageBacking::ProduceOverlay(
    SharedImageManager* manager,
    MemoryTypeTracker* tracker) {
  TRACE_EVENT0("gpu", "D3DImageBacking::ProduceOverlay");
  return std::make_unique<OverlayD3DImageRepresentation>(manager, this, tracker,
                                                         texture_d3d11_device_);
}

std::optional<gl::DCLayerOverlayImage>
D3DImageBacking::GetDCLayerOverlayImage() {
  if (swap_chain_) {
    return std::make_optional<gl::DCLayerOverlayImage>(size(), swap_chain_);
  }
  return std::make_optional<gl::DCLayerOverlayImage>(size(), d3d11_texture_,
                                                     array_slice_);
}

}  // namespace gpu
