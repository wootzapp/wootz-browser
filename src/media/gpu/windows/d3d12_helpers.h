// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_GPU_WINDOWS_D3D12_HELPERS_H_
#define MEDIA_GPU_WINDOWS_D3D12_HELPERS_H_

#include <d3d12.h>
#include <d3d12video.h>
#include <dxgi.h>
#include <wrl.h>

#include <array>
#include <memory>
#include <vector>

#include "media/base/limits.h"
#include "media/base/video_codecs.h"
#include "media/base/video_types.h"
#include "media/filters/vp9_parser.h"
#include "media/gpu/h264_dpb.h"
#include "media/gpu/media_gpu_export.h"
#include "media/video/h265_parser.h"
#include "third_party/abseil-cpp/absl/container/inlined_vector.h"
#include "third_party/libgav1/src/src/utils/constants.h"

namespace media {

// Manages reference frame buffers, for reference frame descriptors to index on.
class MEDIA_GPU_EXPORT D3D12ReferenceFrameList {
 public:
  explicit D3D12ReferenceFrameList(
      Microsoft::WRL::ComPtr<ID3D12VideoDecoderHeap> heap);
  ~D3D12ReferenceFrameList();

  void WriteTo(D3D12_VIDEO_DECODE_REFERENCE_FRAMES* dest);

  void emplace(size_t index, ID3D12Resource* resource, UINT subresource);

 private:
  // Max size of picture_buffers_ that D3D11VideoDecoder may create.
  static constexpr size_t kMaxSize =
      std::max({static_cast<size_t>(H264DPB::kDPBMaxSize),
                static_cast<size_t>(/*H265*/ kMaxDpbSize),
                static_cast<size_t>(kVp9NumRefFrames),
                static_cast<size_t>(libgav1::kNumReferenceFrameTypes)}) +
      limits::kMaxVideoFrames + 2;

  Microsoft::WRL::ComPtr<ID3D12VideoDecoderHeap> heap_;
  size_t size_ = 0;
  // D3D12_VIDEO_DECODE_REFERENCE_FRAMES has ID3D12Resource** ppTexture2Ds, so
  // we have to store raw pointer array here. The pointers are only passed to
  // D3D12 API and we never dereference it in Chromium.
  // The lifetime of these |resources_| is managed by |D3D11VideoDecoder|'s
  // |picture_buffers_|. When picture buffers are invalidated, the
  // D3D12VideoDecoderWrapper and its ID3D12VideoDecoder instance will be
  // destroyed ensuring these values are no longer referenced.
  std::array<ID3D12Resource*, kMaxSize> resources_;
  std::array<UINT, kMaxSize> subresources_;
  // D3D12_VIDEO_DECODE_REFERENCE_FRAMES also has ID3D12VideoDecoderHeap**
  // ppHeaps. The items in |heaps_| are always |heap_.Get()|.
  std::array<ID3D12VideoDecoderHeap*, kMaxSize> heaps_;
};

MEDIA_GPU_EXPORT Microsoft::WRL::ComPtr<ID3D12Device> CreateD3D12Device(
    IDXGIAdapter* adapter);

MEDIA_GPU_EXPORT constexpr UINT D3D12CalcSubresource(UINT mip_slice,
                                                     UINT array_slice,
                                                     UINT plane_slice,
                                                     UINT mip_levels,
                                                     UINT array_size);

// In D3D12 resource barriers, subresource id is not only being composed of mip
// level id and array index id, but also plane id. This method is to create an
// vector of transition barriers for all planes if there is more than one (like
// Y plane and UV plane for NV12).
MEDIA_GPU_EXPORT absl::InlinedVector<D3D12_RESOURCE_BARRIER, 2>
CreateD3D12TransitionBarriersForAllPlanes(ID3D12Resource* resource,
                                          UINT subresource,
                                          uint8_t num_planes,
                                          D3D12_RESOURCE_STATES state_before,
                                          D3D12_RESOURCE_STATES state_after);

// Get the profile GUID for creating ID3D12VideoDecoder. The |bitdepth| and
// |chroma_sampling| will only be checked when the |profile| is
// HEVCPROFILE_REXT.
MEDIA_GPU_EXPORT GUID
GetD3D12VideoDecodeGUID(VideoCodecProfile profile,
                        uint8_t bitdepth,
                        VideoChromaSampling chroma_sampling);

}  // namespace media

#endif  // MEDIA_GPU_WINDOWS_D3D12_HELPERS_H_
