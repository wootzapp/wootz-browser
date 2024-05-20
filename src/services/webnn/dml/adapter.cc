// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/webnn/dml/adapter.h"

#include <dxcore.h>
#include <string.h>
#include <string_view>

#include "base/check_is_test.h"
#include "base/logging.h"
#include "services/webnn/dml/command_queue.h"
#include "services/webnn/dml/error.h"
#include "services/webnn/dml/platform_functions.h"
#include "services/webnn/dml/utils.h"
#include "services/webnn/public/mojom/webnn_error.mojom.h"

namespace webnn::dml {

namespace {

base::unexpected<mojom::ErrorPtr> HandleAdapterFailure(
    mojom::Error::Code error_code,
    std::string_view error_message,
    HRESULT hr = S_OK) {
  if (FAILED(hr)) {
    LOG(ERROR) << "[WebNN] " << error_message << " "
               << logging::SystemErrorCodeToString(hr);
  } else {
    LOG(ERROR) << "[WebNN] " << error_message;
  }
  return base::unexpected(
      CreateError(error_code, "Unable to find a capable adapter."));
}

}  // namespace

// static
base::expected<scoped_refptr<Adapter>, mojom::ErrorPtr> Adapter::GetGpuInstance(
    DML_FEATURE_LEVEL min_required_dml_feature_level,
    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter) {
  // If the `Adapter` instance is created, add a reference and return it.
  if (gpu_instance_) {
    if (!gpu_instance_->IsDMLFeatureLevelSupported(
            min_required_dml_feature_level)) {
      return HandleAdapterFailure(mojom::Error::Code::kNotSupportedError,
                                  "The DirectML feature level on this platform "
                                  "is lower than the minimum required one.");
    }
    return base::WrapRefCounted(gpu_instance_);
  }

  return Adapter::Create(std::move(dxgi_adapter),
                         min_required_dml_feature_level);
}

// static
base::expected<scoped_refptr<Adapter>, mojom::ErrorPtr>
Adapter::GetInstanceForTesting(
    DML_FEATURE_LEVEL min_required_dml_feature_level) {
  CHECK_IS_TEST();

  Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
  HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
  if (FAILED(hr)) {
    return HandleAdapterFailure(
        mojom::Error::Code::kUnknownError,
        "Failed to create an IDXGIFactory1 for testing.", hr);
  }
  Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter;
  hr = factory->EnumAdapters(0, &dxgi_adapter);
  if (FAILED(hr)) {
    return HandleAdapterFailure(
        mojom::Error::Code::kUnknownError,
        "Failed to get an IDXGIAdapter from EnumAdapters for testing.", hr);
  }

  return Adapter::GetGpuInstance(min_required_dml_feature_level,
                                 std::move(dxgi_adapter));
}

// static
base::expected<scoped_refptr<Adapter>, mojom::ErrorPtr> Adapter::GetNpuInstance(
    DML_FEATURE_LEVEL min_required_dml_feature_level) {
  // If the `Adapter` instance is created, add a reference and return it.
  if (npu_instance_) {
    if (!npu_instance_->IsDMLFeatureLevelSupported(
            min_required_dml_feature_level)) {
      return HandleAdapterFailure(mojom::Error::Code::kNotSupportedError,
                                  "The DirectML feature level on this platform "
                                  "is lower than the minimum required one.");
    }
    return base::WrapRefCounted(npu_instance_);
  }

  // Otherwise, enumerate all dxcore adapters to select the npu adapter.
  PlatformFunctions* platform_functions = PlatformFunctions::GetInstance();
  if (!platform_functions || !platform_functions->IsDXCoreSupported()) {
    return HandleAdapterFailure(mojom::Error::Code::kNotSupportedError,
                                "DXCore is not supported on this platform.");
  }

  PlatformFunctions::DXCoreCreateAdapterFactoryProc
      dxcore_create_adapter_factory_proc =
          platform_functions->dxcore_create_adapter_factory_proc();
  if (!dxcore_create_adapter_factory_proc) {
    return HandleAdapterFailure(
        mojom::Error::Code::kUnknownError,
        "Failed to get DXCoreCreateAdapterFactory function.");
  }

  Microsoft::WRL::ComPtr<IDXCoreAdapterFactory> dxcore_factory;
  HRESULT hr =
      dxcore_create_adapter_factory_proc(IID_PPV_ARGS(&dxcore_factory));
  if (FAILED(hr)) {
    return HandleAdapterFailure(mojom::Error::Code::kUnknownError,
                                "Failed to create adapter factory.", hr);
  }
  // With the `DXCORE_ADAPTER_ATTRIBUTE_D3D12_CORE_COMPUTE` filter, the
  // adapter list only contains core-compute capable devices.
  const std::array<GUID, 1> dx_guids = {
      DXCORE_ADAPTER_ATTRIBUTE_D3D12_CORE_COMPUTE};
  Microsoft::WRL::ComPtr<IDXCoreAdapterList> adapter_list;
  hr = dxcore_factory->CreateAdapterList(dx_guids.size(), dx_guids.data(),
                                         IID_PPV_ARGS(&adapter_list));
  if (FAILED(hr)) {
    return HandleAdapterFailure(mojom::Error::Code::kUnknownError,
                                "Failed to create adapter list.", hr);
  }

  const uint32_t adapter_count = adapter_list->GetAdapterCount();
  Microsoft::WRL::ComPtr<IDXCoreAdapter> dxcore_npu_adapter;
  for (uint32_t adapter_index = 0; adapter_index < adapter_count;
       ++adapter_index) {
    Microsoft::WRL::ComPtr<IDXCoreAdapter> dxcore_adapter;
    hr = adapter_list->GetAdapter(adapter_index, IID_PPV_ARGS(&dxcore_adapter));
    if (FAILED(hr)) {
      return HandleAdapterFailure(mojom::Error::Code::kUnknownError,
                                  "Failed to get DXCore adapter.", hr);
    }

    // Because GPUs usually also have the core-compute capability, then we need
    // to filter out the GPUs with `DXCORE_ADAPTER_ATTRIBUTE_D3D12_GRAPHICS` to
    // just get the first NPU.
    bool is_hardware;
    if (SUCCEEDED(dxcore_adapter->GetProperty(DXCoreAdapterProperty::IsHardware,
                                              &is_hardware)) &&
        is_hardware &&
        !dxcore_adapter->IsAttributeSupported(
            DXCORE_ADAPTER_ATTRIBUTE_D3D12_GRAPHICS)) {
      dxcore_npu_adapter = std::move(dxcore_adapter);
      break;
    }
  }

  if (!dxcore_npu_adapter) {
    return HandleAdapterFailure(mojom::Error::Code::kUnknownError,
                                "Unable to find a capable adapter.");
  }
  return Adapter::Create(std::move(dxcore_npu_adapter),
                         min_required_dml_feature_level);
}

// static
base::expected<scoped_refptr<Adapter>, mojom::ErrorPtr> Adapter::Create(
    Microsoft::WRL::ComPtr<IUnknown> dxgi_or_dxcore_adapter,
    DML_FEATURE_LEVEL min_required_dml_feature_level) {
  PlatformFunctions* platform_functions = PlatformFunctions::GetInstance();
  if (!platform_functions) {
    return HandleAdapterFailure(
        mojom::Error::Code::kNotSupportedError,
        "Failed to load required DML/DXCore/D3D12 libraries on this platform.");
  }

  bool is_d3d12_debug_layer_enabled = false;
  // Enable the d3d12 debug layer mainly for services_unittests.exe.
  if (enable_d3d12_debug_layer_for_testing_) {
    // Enable the D3D12 debug layer.
    // Must be called before the D3D12 device is created.
    auto d3d12_get_debug_interface_proc =
        platform_functions->d3d12_get_debug_interface_proc();
    Microsoft::WRL::ComPtr<ID3D12Debug> d3d12_debug;
    if (SUCCEEDED(d3d12_get_debug_interface_proc(IID_PPV_ARGS(&d3d12_debug)))) {
      d3d12_debug->EnableDebugLayer();
      is_d3d12_debug_layer_enabled = true;
    }
  }

  // D3D_FEATURE_LEVEL_1_0_CORE allows Microsoft Compute Driver Model (MCDM)
  // devices (NPUs) to be used. D3D_FEATURE_LEVEL_11_0 targets features
  // supported by Direct3D 11.0.
  // https://learn.microsoft.com/en-us/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_feature_level
  //
  // Prefer using D3D12_COMMAND_LIST_TYPE_COMPUTE over a direct command queue
  // as these are easier for the windows kernel to preempt. For NPU devices,
  // only compute command queues are supported.
  // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_command_list_type
  D3D_FEATURE_LEVEL d3d_feature_level = D3D_FEATURE_LEVEL_11_0;
  Microsoft::WRL::ComPtr<IDXCoreAdapter> dxcore_adapter;
  if (SUCCEEDED(dxgi_or_dxcore_adapter->QueryInterface(
          IID_PPV_ARGS(&dxcore_adapter)))) {
    d3d_feature_level = D3D_FEATURE_LEVEL_1_0_CORE;
  }

  auto d3d12_create_device_proc =
      platform_functions->d3d12_create_device_proc();
  // Create d3d12 device.
  Microsoft::WRL::ComPtr<ID3D12Device> d3d12_device;
  HRESULT hr =
      d3d12_create_device_proc(dxgi_or_dxcore_adapter.Get(), d3d_feature_level,
                               IID_PPV_ARGS(&d3d12_device));
  if (FAILED(hr)) {
    return HandleAdapterFailure(mojom::Error::Code::kUnknownError,
                                "Failed to create D3D12 device.", hr);
  }

  // The d3d12 debug layer can also be enabled via Microsoft (R) DirectX Control
  // Panel (dxcpl.exe) for any executable apps by users.
  if (!is_d3d12_debug_layer_enabled) {
    Microsoft::WRL::ComPtr<ID3D12DebugDevice> debug_device;
    // Ignore failure.
    d3d12_device->QueryInterface(IID_PPV_ARGS(&debug_device));
    is_d3d12_debug_layer_enabled = (debug_device != nullptr);
  }

  // Enable the DML debug layer if the D3D12 debug layer was enabled.
  DML_CREATE_DEVICE_FLAGS flags = DML_CREATE_DEVICE_FLAG_NONE;
  if (is_d3d12_debug_layer_enabled) {
    flags |= DML_CREATE_DEVICE_FLAG_DEBUG;
  }

  // Create dml device.
  Microsoft::WRL::ComPtr<IDMLDevice> dml_device;
  auto dml_create_device1_proc = platform_functions->dml_create_device1_proc();
  hr = dml_create_device1_proc(d3d12_device.Get(), flags,
                               min_required_dml_feature_level,
                               IID_PPV_ARGS(&dml_device));
  if (FAILED(hr)) {
    if (hr == DXGI_ERROR_SDK_COMPONENT_MISSING) {
      // DirectML debug layer can fail to load even when it has been installed
      // on the system. Try again without the debug flag and see if we're
      // successful.
      flags = flags & ~DML_CREATE_DEVICE_FLAG_DEBUG;
      hr = dml_create_device1_proc(d3d12_device.Get(), flags,
                                   min_required_dml_feature_level,
                                   IID_PPV_ARGS(&dml_device));
    }

    if (FAILED(hr)) {
      if (hr == DXGI_ERROR_UNSUPPORTED) {
        return HandleAdapterFailure(mojom::Error::Code::kNotSupportedError,
                                    "DML feature level not supported.", hr);
      } else {
        return HandleAdapterFailure(mojom::Error::Code::kUnknownError,
                                    "Failed to create DirectML device.", hr);
      }
    }
  }

  const DML_FEATURE_LEVEL max_supported_dml_feature_level =
      GetMaxSupportedDMLFeatureLevel(dml_device.Get());

  // Create command queue.
  scoped_refptr<CommandQueue> command_queue =
      CommandQueue::Create(d3d12_device.Get());
  if (!command_queue) {
    return HandleAdapterFailure(mojom::Error::Code::kUnknownError,
                                "Failed to create command queue.");
  }

  D3D12_FEATURE_DATA_ARCHITECTURE arch = {};
  if (FAILED(d3d12_device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE,
                                               &arch, sizeof(arch)))) {
    return HandleAdapterFailure(mojom::Error::Code::kUnknownError,
                                "Failed to check feature support.");
  }

  const bool is_uma = (arch.UMA == TRUE);

  return WrapRefCounted(
      new Adapter(std::move(dxgi_or_dxcore_adapter), std::move(d3d12_device),
                  std::move(dml_device), std::move(command_queue),
                  max_supported_dml_feature_level, is_uma));
}

// static
void Adapter::EnableDebugLayerForTesting() {
  CHECK_IS_TEST();
  enable_d3d12_debug_layer_for_testing_ = true;
}

Adapter::Adapter(Microsoft::WRL::ComPtr<IUnknown> dxgi_or_dxcore_adapter,
                 Microsoft::WRL::ComPtr<ID3D12Device> d3d12_device,
                 Microsoft::WRL::ComPtr<IDMLDevice> dml_device,
                 scoped_refptr<CommandQueue> command_queue,
                 DML_FEATURE_LEVEL max_supported_dml_feature_level,
                 bool is_uma)
    : dxgi_or_dxcore_adapter_(std::move(dxgi_or_dxcore_adapter)),
      d3d12_device_(std::move(d3d12_device)),
      dml_device_(std::move(dml_device)),
      command_queue_(std::move(command_queue)),
      max_supported_dml_feature_level_(max_supported_dml_feature_level),
      is_uma_(is_uma) {
  Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter;
  Microsoft::WRL::ComPtr<IDXCoreAdapter> dxcore_adapter;
  if (SUCCEEDED(dxgi_or_dxcore_adapter_->QueryInterface(
          IID_PPV_ARGS(&dxgi_adapter)))) {
    CHECK_EQ(gpu_instance_, nullptr);
    gpu_instance_ = this;
  } else if (SUCCEEDED(dxgi_or_dxcore_adapter_->QueryInterface(
                 IID_PPV_ARGS(&dxcore_adapter)))) {
    CHECK_EQ(npu_instance_, nullptr);
    npu_instance_ = this;
  } else {
    NOTREACHED_NORETURN();
  }
}

Adapter::~Adapter() {
  Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter;
  Microsoft::WRL::ComPtr<IDXCoreAdapter> dxcore_adapter;
  if (SUCCEEDED(dxgi_or_dxcore_adapter_->QueryInterface(
          IID_PPV_ARGS(&dxgi_adapter)))) {
    CHECK_EQ(gpu_instance_, this);
    gpu_instance_ = nullptr;
  } else if (SUCCEEDED(dxgi_or_dxcore_adapter_->QueryInterface(
                 IID_PPV_ARGS(&dxcore_adapter)))) {
    CHECK_EQ(npu_instance_, this);
    npu_instance_ = nullptr;
  } else {
    NOTREACHED_NORETURN();
  }
}

bool Adapter::IsDMLFeatureLevelSupported(
    DML_FEATURE_LEVEL feature_level) const {
  return feature_level <= max_supported_dml_feature_level_;
}

bool Adapter::IsDMLDeviceCompileGraphSupportedForTesting() const {
  CHECK_IS_TEST();
  // IDMLDevice1::CompileGraph was introduced in DirectML version 1.2.0 or
  // DML_FEATURE_LEVEL_2_1.
  // https://learn.microsoft.com/en-us/windows/ai/directml/dml-feature-level-history
  return IsDMLFeatureLevelSupported(DML_FEATURE_LEVEL_2_1);
}

Adapter* Adapter::gpu_instance_ = nullptr;
Adapter* Adapter::npu_instance_ = nullptr;

bool Adapter::enable_d3d12_debug_layer_for_testing_ = false;

}  // namespace webnn::dml
