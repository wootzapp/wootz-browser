// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_COMMAND_BUFFER_COMMON_SHARED_IMAGE_USAGE_H_
#define GPU_COMMAND_BUFFER_COMMON_SHARED_IMAGE_USAGE_H_

#include <stdint.h>
#include <string>

#include "gpu/gpu_export.h"

namespace gpu {

// Please update the function, CreateLabelForSharedImageUsage, when adding a new
// enum value.
enum SharedImageUsage : uint32_t {
  // Image will be read via GLES2Interface
  SHARED_IMAGE_USAGE_GLES2_READ = 1 << 0,
  // Image will be read via RasterInterface
  SHARED_IMAGE_USAGE_RASTER_READ = 1 << 1,
  // Image will be read from inside Display Compositor
  SHARED_IMAGE_USAGE_DISPLAY_READ = 1 << 2,
  // Image will be written to inside Display Compositor
  SHARED_IMAGE_USAGE_DISPLAY_WRITE = 1 << 3,
  // Image will be used as a scanout buffer (overlay)
  SHARED_IMAGE_USAGE_SCANOUT = 1 << 4,
  // Image will be used in OOP rasterization. This flag is used on top of
  // SHARED_IMAGE_USAGE_RASTER_{READ, WRITE} to indicate that the client will
  // only use RasterInterface for OOP rasterization. TODO(backer): Eliminate
  // once we can CPU raster to SkImage via RasterInterface.
  SHARED_IMAGE_USAGE_OOP_RASTERIZATION = 1 << 5,
  // Image will be read by Dawn (for WebGPU)
  SHARED_IMAGE_USAGE_WEBGPU_READ = 1 << 6,
  // Image may use concurrent read/write access. Used by single buffered canvas.
  // TODO(crbug.com/41462072): This usage is currently not supported in
  // GL/Vulkan
  // interop cases.
  SHARED_IMAGE_USAGE_CONCURRENT_READ_WRITE = 1 << 7,
  // Image will be used for video decode acceleration on Chrome OS.
  SHARED_IMAGE_USAGE_VIDEO_DECODE = 1 << 8,
  // Image will be used as a WebGPU swapbuffer
  SHARED_IMAGE_USAGE_WEBGPU_SWAP_CHAIN_TEXTURE = 1 << 9,
  // The image was created by VideoToolbox on macOS, and is backed by a
  // CVPixelBuffer's IOSurface. Because of this backing, IOSurfaceIsInUse will
  // always return true.
  SHARED_IMAGE_USAGE_MACOS_VIDEO_TOOLBOX = 1 << 10,
  // Image will be used with mipmap enabled
  SHARED_IMAGE_USAGE_MIPMAP = 1 << 11,
  // Image will be used for CPU Writes by client
  SHARED_IMAGE_USAGE_CPU_WRITE = 1 << 12,
  // Image will be used in RasterInterface with RawDraw.
  SHARED_IMAGE_USAGE_RAW_DRAW = 1 << 13,
  // Image will be used in RasterInterface for DelegatedCompositing.
  // This is intended to avoid the overhead of a GPU fence per tile.
  // TODO(crbug.com/41492887): In order to delegate buffers we need all buffer
  // allocations to be set as SCANOUT. This will cause a fence per rastered
  // tiled. A new buffer concept that avoids scanout but allows delegation might
  // enable us to remove this usage.
  SHARED_IMAGE_USAGE_RASTER_DELEGATED_COMPOSITING = 1 << 14,
  // Image will be created on the high performance GPU if supported.
  SHARED_IMAGE_USAGE_HIGH_PERFORMANCE_GPU = 1 << 15,
  // Windows only: image will be backed by a DComp surface. A swap chain is
  // preferred when an image is opaque and expected to update frequently and
  // independently of other overlays. This flag is incompatible with
  // DISPLAY_READ.
  SHARED_IMAGE_USAGE_SCANOUT_DCOMP_SURFACE = 1 << 16,

  // Image will be used as a WebGPU storage texture.
  SHARED_IMAGE_USAGE_WEBGPU_STORAGE_TEXTURE = 1 << 17,

  // Image will be written via GLES2Interface
  SHARED_IMAGE_USAGE_GLES2_WRITE = 1 << 18,

  // Image will be written via RasterInterface
  SHARED_IMAGE_USAGE_RASTER_WRITE = 1 << 19,

  // Image will be written by Dawn (for WebGPU)
  SHARED_IMAGE_USAGE_WEBGPU_WRITE = 1 << 20,

  // The image will be used by GLES2 only for raster over the GLES2 interface.
  // Specified in conjunction with GLES2_READ and/or GLES2_WRITE.
  SHARED_IMAGE_USAGE_GLES2_FOR_RASTER_ONLY = 1 << 21,

  // The image will be used by raster only over the GLES2 interface.
  // Specified in conjunction with RASTER_READ and/or RASTER_WRITE.
  SHARED_IMAGE_USAGE_RASTER_OVER_GLES2_ONLY = 1 << 22,

  // Image will contain protected content to be scanned out. Note that this type
  // of image
  // won't necessarily be written to by a hardware video decoder, but will
  // instead be written
  // to by a preprocessing step that converts the image's pixel format into
  // something the
  // display controller understands.
  SHARED_IMAGE_USAGE_PROTECTED_VIDEO = 1 << 23,

  // Start service side only usage flags after this entry. They must be larger
  // than `LAST_CLIENT_USAGE`.
  LAST_CLIENT_USAGE = SHARED_IMAGE_USAGE_PROTECTED_VIDEO,

  // Image will have pixels uploaded from CPU. The backing must implement
  // `UploadFromMemory()` if it supports this usage. Clients should specify
  // SHARED_IMAGE_USAGE_CPU_WRITE if they need to write pixels to the image.
  SHARED_IMAGE_USAGE_CPU_UPLOAD = 1 << 24,

  LAST_SHARED_IMAGE_USAGE = SHARED_IMAGE_USAGE_CPU_UPLOAD
};

// This is used as the debug_label prefix for all shared images created by
// importing buffers in Exo. This prefix is checked in the GPU process when
// reporting if memory for shared images is attributed to exo imports or not.
GPU_EXPORT extern const char kExoTextureLabelPrefix[];

// Returns true if usage is a valid client usage.
GPU_EXPORT bool IsValidClientUsage(uint32_t usage);

// Returns true iff usage includes SHARED_IMAGE_USAGE_GLES2_READ or
// SHARED_IMAGE_USAGE_GLES2_WRITE.
GPU_EXPORT bool HasGLES2ReadOrWriteUsage(uint32_t usage);

// Create a string to label SharedImageUsage.
GPU_EXPORT std::string CreateLabelForSharedImageUsage(uint32_t usage);

}  // namespace gpu

#endif  // GPU_COMMAND_BUFFER_COMMON_SHARED_IMAGE_USAGE_H_
