// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_WEBNN_DML_UTILS_H_
#define SERVICES_WEBNN_DML_UTILS_H_

#include <DirectML.h>
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>

#include "base/component_export.h"
#include "base/containers/span.h"
#include "services/webnn/dml/command_recorder.h"
#include "services/webnn/public/mojom/webnn_context_provider.mojom.h"
#include "services/webnn/public/mojom/webnn_error.mojom.h"

namespace webnn::dml {

using Microsoft::WRL::ComPtr;

uint64_t CalculateDMLBufferTensorSize(DML_TENSOR_DATA_TYPE data_type,
                                      const std::vector<uint32_t>& dimensions,
                                      const std::vector<uint32_t>& strides);

std::vector<uint32_t> CalculateStrides(base::span<const uint32_t> dimensions);

// The length of `permutation` must be the same as `array`. The values in
// `permutation` must be within the range [0, N-1] where N is the length of
// `array`. There must be no two or more same values in `permutation`.
//
// e.g., Given an array of [10, 11, 12, 13] and a permutation of [0, 2, 3, 1],
// the permuted array would be [10, 12, 13, 11].
std::vector<uint32_t> PermuteArray(base::span<const uint32_t> array,
                                   base::span<const uint32_t> permutation);

// Gets the ID3D12Device used to create the IDMLDevice.
ComPtr<ID3D12Device> GetD3D12Device(IDMLDevice* dml_device);

// Returns the maximum feature level supported by the DML device.
DML_FEATURE_LEVEL GetMaxSupportedDMLFeatureLevel(IDMLDevice* dml_device);

// Creates a transition barrier which is used to specify the resource is
// transitioning from `before` to `after` states.
D3D12_RESOURCE_BARRIER COMPONENT_EXPORT(WEBNN_SERVICE)
    CreateTransitionBarrier(ID3D12Resource* resource,
                            D3D12_RESOURCE_STATES before,
                            D3D12_RESOURCE_STATES after);

// Helper function to upload data from CPU to GPU, the resource can be created
// for a single buffer or a big buffer combined from multiple buffers.
void COMPONENT_EXPORT(WEBNN_SERVICE)
    UploadBufferWithBarrier(CommandRecorder* command_recorder,
                            ComPtr<ID3D12Resource> dst_buffer,
                            ComPtr<ID3D12Resource> src_buffer,
                            size_t buffer_size);

// Helper function to readback data from GPU to CPU, the resource can be created
// for a single buffer or a big buffer combined from multiple buffers.
void COMPONENT_EXPORT(WEBNN_SERVICE)
    ReadbackBufferWithBarrier(CommandRecorder* command_recorder,
                              ComPtr<ID3D12Resource> readback_buffer,
                              ComPtr<ID3D12Resource> default_buffer,
                              size_t buffer_size);

mojom::ErrorPtr CreateError(mojom::Error::Code error_code,
                            const std::string& error_message);

}  // namespace webnn::dml

#endif  // SERVICES_WEBNN_DML_UTILS_H_
