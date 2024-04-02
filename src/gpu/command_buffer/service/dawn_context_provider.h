// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_COMMAND_BUFFER_SERVICE_DAWN_CONTEXT_PROVIDER_H_
#define GPU_COMMAND_BUFFER_SERVICE_DAWN_CONTEXT_PROVIDER_H_

#include <dawn/platform/DawnPlatform.h>

#include <memory>
#include <optional>

#include "base/memory/raw_ptr.h"
#include "base/synchronization/lock.h"
#include "base/thread_annotations.h"
#include "build/build_config.h"
#include "gpu/command_buffer/common/constants.h"
#include "gpu/command_buffer/service/dawn_caching_interface.h"
#include "gpu/config/gpu_driver_bug_workarounds.h"
#include "gpu/config/gpu_preferences.h"
#include "gpu/gpu_gles2_export.h"
#include "third_party/dawn/include/dawn/native/DawnNative.h"
#include "third_party/skia/include/gpu/graphite/ContextOptions.h"
#include "third_party/skia/include/gpu/graphite/dawn/DawnTypes.h"

#if BUILDFLAG(IS_WIN)
#include <d3d11.h>
#include <wrl/client.h>
#endif

namespace skgpu::graphite {
class Context;
}  // namespace skgpu::graphite

namespace gpu {
namespace webgpu {
class DawnInstance;
}  // namespace webgpu

class GPU_GLES2_EXPORT DawnContextProvider {
 public:
  using CacheBlobCallback = webgpu::DawnCachingInterface::CacheBlobCallback;
  static std::unique_ptr<DawnContextProvider> Create(
      const GpuPreferences& gpu_preferences = GpuPreferences(),
      const GpuDriverBugWorkarounds& gpu_driver_workarounds =
          GpuDriverBugWorkarounds());
  static std::unique_ptr<DawnContextProvider> CreateWithBackend(
      wgpu::BackendType backend_type,
      bool force_fallback_adapter = false,
      const GpuPreferences& gpu_preferences = GpuPreferences(),
      const GpuDriverBugWorkarounds& gpu_driver_workarounds =
          GpuDriverBugWorkarounds());

  static wgpu::BackendType GetDefaultBackendType();
  static bool DefaultForceFallbackAdapter();

  DawnContextProvider(const DawnContextProvider&) = delete;
  DawnContextProvider& operator=(const DawnContextProvider&) = delete;

  ~DawnContextProvider();

  wgpu::Device GetDevice() const { return device_; }
  wgpu::BackendType backend_type() const { return backend_type_; }
  bool is_vulkan_swiftshader_adapter() const {
    return is_vulkan_swiftshader_adapter_;
  }
  wgpu::Instance GetInstance() const;

  void SetCachingInterface(
      std::unique_ptr<webgpu::DawnCachingInterface> caching_interface);

  bool InitializeGraphiteContext(
      const skgpu::graphite::ContextOptions& options);

  skgpu::graphite::Context* GetGraphiteContext() const {
    return graphite_context_.get();
  }

#if BUILDFLAG(IS_WIN)
  Microsoft::WRL::ComPtr<ID3D11Device> GetD3D11Device() const;
#endif

  bool SupportsFeature(wgpu::FeatureName feature);

  std::optional<error::ContextLostReason> GetResetStatus() const;

  void OnError(WGPUErrorType error_type, const char* message);

 private:
  // Cache functions for Dawn device to use.
  static size_t LoadCachedData(const void* key,
                               size_t key_size,
                               void* value,
                               size_t value_size,
                               void* userdata);
  static void StoreCachedData(const void* key,
                              size_t key_size,
                              const void* value,
                              size_t value_size,
                              void* userdata);

  explicit DawnContextProvider();

  bool Initialize(wgpu::BackendType backend_type,
                  bool force_fallback_adapter,
                  const GpuPreferences& gpu_preferences,
                  const GpuDriverBugWorkarounds& gpu_driver_workarounds);

  std::unique_ptr<webgpu::DawnCachingInterface> caching_interface_;
  std::unique_ptr<dawn::platform::Platform> platform_;
  std::unique_ptr<webgpu::DawnInstance> instance_;
  wgpu::Device device_;
  wgpu::BackendType backend_type_;
  bool is_vulkan_swiftshader_adapter_ = false;
  std::unique_ptr<skgpu::graphite::Context> graphite_context_;

  mutable base::Lock context_lost_lock_;
  std::optional<error::ContextLostReason> context_lost_reason_
      GUARDED_BY(context_lost_lock_);
};

}  // namespace gpu

#endif  // GPU_COMMAND_BUFFER_SERVICE_DAWN_CONTEXT_PROVIDER_H_
