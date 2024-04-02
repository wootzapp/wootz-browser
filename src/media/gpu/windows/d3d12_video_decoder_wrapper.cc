// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/gpu/windows/d3d12_video_decoder_wrapper.h"

#include <Windows.h>
#include <d3d12.h>

#include "base/check_op.h"
#include "base/notreached.h"
#include "base/win/scoped_handle.h"
#include "media/gpu/windows/d3d11_picture_buffer.h"
#include "media/gpu/windows/d3d12_helpers.h"
#include "media/gpu/windows/scoped_d3d_buffers.h"
#include "media/gpu/windows/supported_profile_helpers.h"

#define RETURN_IF_FAILED(message, status_code, hr) \
  do {                                             \
    if (FAILED(hr)) {                              \
      RecordFailure(message, status_code, hr);     \
      return false;                                \
    }                                              \
  } while (0)

#define RETURN_IF_FAILED2(hr, message)                                \
  do {                                                                \
    if (FAILED(hr)) {                                                 \
      MEDIA_LOG(ERROR, media_log)                                     \
          << message << ": " << logging::SystemErrorCodeToString(hr); \
      return nullptr;                                                 \
    }                                                                 \
  } while (0)

namespace media {

namespace {

class ScopedD3D12MemoryBuffer;
class ScopedD3D12ResourceBuffer;

class D3D12VideoDecoderWrapperImpl : public D3D12VideoDecoderWrapper {
 public:
  D3D12VideoDecoderWrapperImpl(
      MediaLog* media_log,
      Microsoft::WRL::ComPtr<ID3D12Device> device,
      Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue,
      Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator,
      Microsoft::WRL::ComPtr<ID3D12Fence> fence,
      Microsoft::WRL::ComPtr<ID3D12VideoDevice> video_device,
      Microsoft::WRL::ComPtr<ID3D12VideoDecoder> video_decoder,
      Microsoft::WRL::ComPtr<ID3D12VideoDecoderHeap> video_decoder_heap,
      Microsoft::WRL::ComPtr<ID3D12VideoDecodeCommandList> command_list,
      uint8_t num_planes)
      : D3D12VideoDecoderWrapper(media_log),
        device_(std::move(device)),
        video_device_(std::move(video_device)),
        command_queue_(std::move(command_queue)),
        command_allocator_(std::move(command_allocator)),
        command_list_(std::move(command_list)),
        fence_(std::move(fence)),
        video_decoder_(std::move(video_decoder)),
        video_decoder_heap_(std::move(video_decoder_heap)),
        num_planes_(num_planes),
        reference_frame_list_(std::move(video_decoder_heap)) {
    input_stream_arguments_.pHeap = video_decoder_heap_.Get();
  }

  ~D3D12VideoDecoderWrapperImpl() override = default;

  std::optional<bool> UseSingleTexture() const override { return true; }

  void Reset() override {
    picture_parameters_buffer_.clear();
    inverse_quantization_matrix_buffer_.clear();
    slice_control_buffer_.clear();
    input_stream_arguments_.NumFrameArguments = 0;
    compressed_bitstream_.Reset();
    bitstream_buffer_.reset();
  }

  bool WaitForFrameBegins(D3D11PictureBuffer* output_picture) override {
    Reset();
    HRESULT hr = command_allocator_->Reset();
    RETURN_IF_FAILED("Failed to reset command allocator",
                     D3D11StatusCode::kDecoderBeginFrameFailed, hr);
    hr = command_list_->Reset(command_allocator_.Get());
    RETURN_IF_FAILED("Failed to reset command list",
                     D3D11StatusCode::kDecoderBeginFrameFailed, hr);

    // Previous output texture will be read as a reference frame now.
    if (output_stream_arguments_.pOutputTexture2D) {
      auto barriers = CreateD3D12TransitionBarriersForAllPlanes(
          output_stream_arguments_.pOutputTexture2D,
          output_stream_arguments_.OutputSubresource, num_planes_,
          D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_VIDEO_DECODE_READ);
      command_list_->ResourceBarrier(barriers.size(), barriers.data());
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> d3d11_texture2d =
        output_picture->Texture();
    D3D11_TEXTURE2D_DESC desc;
    d3d11_texture2d->GetDesc(&desc);
    CHECK(desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED_NTHANDLE)
        << "Cannot create shared handle for d3d11 texture2d without "
           "D3D11_RESOURCE_MISC_SHARED_NTHANDLE flag.";

    auto d3d12_resource_or_error =
        output_picture->ToD3D12Resource(device_.Get());
    if (!d3d12_resource_or_error.has_value()) {
      return false;
    }
    Microsoft::WRL::ComPtr<ID3D12Resource> d3d12_resource =
        std::move(d3d12_resource_or_error).value();

    output_stream_arguments_ = {
        .pOutputTexture2D = d3d12_resource.Get(),
        .OutputSubresource = output_picture->array_slice()};
    reference_frame_list_.emplace(output_picture->picture_index(),
                                  d3d12_resource.Get(),
                                  output_picture->array_slice());

    return true;
  }

  bool HasPendingBuffer(BufferType type) override {
    switch (type) {
      case BufferType::kPictureParameters:
        return !picture_parameters_buffer_.empty();
      case BufferType::kInverseQuantizationMatrix:
        return !inverse_quantization_matrix_buffer_.empty();
      case BufferType::kSliceControl:
        return !slice_control_buffer_.empty();
      case BufferType::kBitstream:
        return bitstream_buffer_.has_value();
    }
    NOTREACHED_NORETURN();
  }

  bool SubmitSlice() override {
    if (!slice_info_bytes_.empty()) {
      // In D3D12 we need to submit the frame at once, so SubmitSlice() is
      // expected to be called only once per frame.
      CHECK(slice_control_buffer_.empty());
      slice_info_bytes_.swap(slice_control_buffer_);
      slice_info_bytes_.clear();
      CHECK_LT(input_stream_arguments_.NumFrameArguments,
               std::size(input_stream_arguments_.FrameArguments));
      input_stream_arguments_
          .FrameArguments[input_stream_arguments_.NumFrameArguments++] = {
          .Type = D3D12_VIDEO_DECODE_ARGUMENT_TYPE_SLICE_CONTROL,
          .Size = static_cast<UINT>(slice_control_buffer_.size()),
          .pData = slice_control_buffer_.data(),
      };
    }

    CHECK_LE(input_stream_arguments_.NumFrameArguments, 3u);

    reference_frame_list_.WriteTo(&input_stream_arguments_.ReferenceFrames);

    CHECK(bitstream_buffer_);
    bitstream_buffer_.reset();
    CHECK(compressed_bitstream_);
    input_stream_arguments_.CompressedBitstream = {
        .pBuffer = compressed_bitstream_.Get(),
        // The size of a buffer resource is its width.
        .Size = compressed_bitstream_->GetDesc().Width,
    };

    return true;
  }

  bool SubmitDecode() override {
    auto barriers = CreateD3D12TransitionBarriersForAllPlanes(
        output_stream_arguments_.pOutputTexture2D,
        output_stream_arguments_.OutputSubresource, num_planes_,
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE);
    command_list_->ResourceBarrier(barriers.size(), barriers.data());

    command_list_->DecodeFrame(video_decoder_.Get(), &output_stream_arguments_,
                               &input_stream_arguments_);

    for (D3D12_RESOURCE_BARRIER& barrier : barriers) {
      std::swap(barrier.Transition.StateBefore, barrier.Transition.StateAfter);
    }
    command_list_->ResourceBarrier(barriers.size(), barriers.data());

    HRESULT hr = command_list_->Close();
    RETURN_IF_FAILED("Failed to close command list",
                     D3D11StatusCode::kSubmitDecoderBuffersFailed, hr);

    ID3D12CommandList* command_lists[] = {command_list_.Get()};
    command_queue_->ExecuteCommandLists(std::size(command_lists),
                                        command_lists);
    hr = command_queue_->Signal(fence_.Get(), ++fence_id_);
    RETURN_IF_FAILED("D3D12CommandQueue Signal() failed",
                     D3D11StatusCode::kDecoderEndFrameFailed, hr);

    // Just wait here like D3D11's behavior before render side supports D3D12
    if (fence_->GetCompletedValue() >= fence_id_) {
      return true;
    }
    base::win::ScopedHandle fence_event{::CreateEvent(
        nullptr, /*bManualReset*/ TRUE, /*bInitialState*/ FALSE, nullptr)};
    hr = fence_->SetEventOnCompletion(fence_id_, fence_event.get());
    RETURN_IF_FAILED("D3D12Fence SetEventOnCompletion() failed",
                     D3D11StatusCode::kDecoderEndFrameFailed, hr);

    // TODO(crbug.com/1348104): Let ID3D11DeviceContext4::Wait() for a
    // ID3D11Fence instead.
    return WaitForSingleObject(fence_event.get(), INFINITE) == WAIT_OBJECT_0;
  }

 private:
  friend class ScopedD3D12MemoryBuffer;
  friend class ScopedD3D12ResourceBuffer;

  std::unique_ptr<ScopedD3DBuffer> GetBuffer(BufferType type,
                                             uint32_t desired_size) override;

  Microsoft::WRL::ComPtr<ID3D12Device> device_;
  Microsoft::WRL::ComPtr<ID3D12VideoDevice> video_device_;
  Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue_;
  Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator_;
  Microsoft::WRL::ComPtr<ID3D12VideoDecodeCommandList> command_list_;
  Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
  UINT64 fence_id_ = 0;

  Microsoft::WRL::ComPtr<ID3D12VideoDecoder> video_decoder_;
  Microsoft::WRL::ComPtr<ID3D12VideoDecoderHeap> video_decoder_heap_;
  const uint8_t num_planes_;
  D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS input_stream_arguments_{};
  D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS output_stream_arguments_{};
  std::vector<uint8_t> picture_parameters_buffer_;
  std::vector<uint8_t> inverse_quantization_matrix_buffer_;
  std::vector<uint8_t> slice_control_buffer_;
  Microsoft::WRL::ComPtr<ID3D12Resource> compressed_bitstream_;
  D3D12ReferenceFrameList reference_frame_list_;
};

class ScopedD3D12MemoryBuffer : public ScopedD3DBuffer {
 public:
  ScopedD3D12MemoryBuffer(D3D12VideoDecoderWrapperImpl* decoder,
                          D3D12_VIDEO_DECODE_ARGUMENT_TYPE type,
                          base::span<uint8_t> data)
      : ScopedD3DBuffer(data), decoder_(*decoder), type_(type) {}
  ~ScopedD3D12MemoryBuffer() override { ScopedD3D12MemoryBuffer::Commit(); }

  ScopedD3D12MemoryBuffer(ScopedD3D12MemoryBuffer&) = delete;
  ScopedD3D12MemoryBuffer& operator=(ScopedD3D12MemoryBuffer&) = delete;

  bool Commit() override { return Commit(data_.size()); }

  bool Commit(uint32_t written_size) override {
    if (data_.empty()) {
      return false;
    }
    CHECK_LE(written_size, data_.size());
    CHECK_LT(decoder_->input_stream_arguments_.NumFrameArguments,
             std::size(decoder_->input_stream_arguments_.FrameArguments));
    decoder_->input_stream_arguments_
        .FrameArguments[decoder_->input_stream_arguments_.NumFrameArguments++] =
        {
            .Type = type_,
            .Size = written_size,
            .pData = data_.data(),
        };
    data_ = base::span<uint8_t>();
    return true;
  }

 private:
  const raw_ref<D3D12VideoDecoderWrapperImpl> decoder_;
  const D3D12_VIDEO_DECODE_ARGUMENT_TYPE type_;
};

class ScopedD3D12ResourceBuffer : public ScopedD3DBuffer {
 public:
  ScopedD3D12ResourceBuffer(D3D12VideoDecoderWrapperImpl* decoder,
                            ID3D12Resource* resource)
      : resource_(resource) {
    void* mapped_data;
    HRESULT hr = resource_->Map(0, nullptr, &mapped_data);
    if (FAILED(hr)) {
      decoder->RecordFailure("Mapping data of ID3D12Resource failed",
                             D3D11StatusCode::kGetBitstreamBufferFailed, hr);
      return;
    }
    data_ = base::span(reinterpret_cast<uint8_t*>(mapped_data),
                       static_cast<size_t>(resource_->GetDesc().Width));
  }
  ~ScopedD3D12ResourceBuffer() override { ScopedD3D12ResourceBuffer::Commit(); }

  ScopedD3D12ResourceBuffer(ScopedD3D12ResourceBuffer&) = delete;
  ScopedD3D12ResourceBuffer& operator=(ScopedD3D12ResourceBuffer&) = delete;

  bool Commit() override { return Commit(resource_->GetDesc().Width); }

  bool Commit(uint32_t written_size) override {
    if (data_.empty()) {
      return false;
    }
    CHECK_LE(written_size, data_.size());
    D3D12_RANGE range{0, written_size};
    resource_->Unmap(0, &range);
    data_ = base::span<uint8_t>();
    return true;
  }

 protected:
  raw_ptr<ID3D12Resource> resource_;
};

std::unique_ptr<ScopedD3DBuffer> D3D12VideoDecoderWrapperImpl::GetBuffer(
    BufferType type,
    uint32_t desired_size) {
  switch (type) {
    case BufferType::kPictureParameters:
      picture_parameters_buffer_.resize(desired_size);
      return std::make_unique<ScopedD3D12MemoryBuffer>(
          this, D3D12_VIDEO_DECODE_ARGUMENT_TYPE_PICTURE_PARAMETERS,
          picture_parameters_buffer_);
    case BufferType::kInverseQuantizationMatrix:
      inverse_quantization_matrix_buffer_.resize(desired_size);
      return std::make_unique<ScopedD3D12MemoryBuffer>(
          this, D3D12_VIDEO_DECODE_ARGUMENT_TYPE_INVERSE_QUANTIZATION_MATRIX,
          inverse_quantization_matrix_buffer_);
    case BufferType::kSliceControl:
      slice_control_buffer_.resize(desired_size);
      return std::make_unique<ScopedD3D12MemoryBuffer>(
          this, D3D12_VIDEO_DECODE_ARGUMENT_TYPE_SLICE_CONTROL,
          slice_control_buffer_);
    case BufferType::kBitstream:
      // Make sure we don't create more than one resource for a frame.
      CHECK(!compressed_bitstream_);
      // Create a general buffer resource for uploading
      D3D12_HEAP_PROPERTIES heap_properties{.Type = D3D12_HEAP_TYPE_UPLOAD};
      D3D12_RESOURCE_DESC desc{.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                               .Width = desired_size,
                               .Height = 1,
                               .DepthOrArraySize = 1,
                               .MipLevels = 1,
                               .SampleDesc = {1},
                               .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR};
      HRESULT hr = device_->CreateCommittedResource(
          &heap_properties, {}, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
          nullptr, IID_PPV_ARGS(&compressed_bitstream_));
      if (FAILED(hr)) {
        RecordFailure("Failed to CreateCommittedResource",
                      D3D11StatusCode::kCreateVideoProcessorInputViewFailed,
                      hr);
        return std::make_unique<ScopedD3D12MemoryBuffer>(
            this, D3D12_VIDEO_DECODE_ARGUMENT_TYPE{}, base::span<uint8_t>{});
      }
      return std::make_unique<ScopedD3D12ResourceBuffer>(
          this, compressed_bitstream_.Get());
  }
  NOTREACHED_NORETURN();
}

}  // namespace

// static
std::unique_ptr<D3D12VideoDecoderWrapper> D3D12VideoDecoderWrapper::Create(
    MediaLog* media_log,
    Microsoft::WRL::ComPtr<ID3D12VideoDevice> video_device,
    VideoDecoderConfig config,
    uint8_t bit_depth,
    VideoChromaSampling chroma_sampling) {
  GUID guid =
      GetD3D12VideoDecodeGUID(config.profile(), bit_depth, chroma_sampling);
  DXGI_FORMAT decode_format = GetOutputDXGIFormat(bit_depth, chroma_sampling);
#if BUILDFLAG(ENABLE_HEVC_PARSER_AND_HW_DECODER)
  // For D3D11/D3D12, 8b/10b-422 HEVC will share 10b-422 GUID no matter
  // it is defined by Intel or DXVA spec(as part of Windows SDK).
  if (guid == DXVA_ModeHEVC_VLD_Main422_10_Intel) {
    decode_format = DXGI_FORMAT_Y210;
  }
#endif  // BUILDFLAG(ENABLE_HEVC_PARSER_AND_HW_DECODER)

  D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT feature{
      .Configuration = {guid},
      .Width = static_cast<UINT>(config.coded_size().width()),
      .Height = static_cast<UINT>(config.coded_size().height()),
      .DecodeFormat = decode_format,
  };
  HRESULT hr = video_device->CheckFeatureSupport(
      D3D12_FEATURE_VIDEO_DECODE_SUPPORT, &feature, sizeof(feature));
  RETURN_IF_FAILED2(hr, "D3D12VideoDevice CheckFeatureSupport failed");
  if (feature.SupportFlags != D3D12_VIDEO_DECODE_SUPPORT_FLAG_SUPPORTED) {
    MEDIA_LOG(ERROR, media_log)
        << "D3D12VideoDecoder does not support profile "
        << GetProfileName(config.profile()) << " with bit depth "
        << base::strict_cast<int>(bit_depth) << ", chroma subsampling format "
        << VideoChromaSamplingToString(chroma_sampling) << ", coded size "
        << config.coded_size().ToString() << ", and output dxgi format "
        << decode_format;
    return nullptr;
  }

  Microsoft::WRL::ComPtr<ID3D12VideoDecoder> video_decoder;
  D3D12_VIDEO_DECODER_DESC desc{.Configuration = {guid}};
  hr = video_device->CreateVideoDecoder(&desc, IID_PPV_ARGS(&video_decoder));
  RETURN_IF_FAILED2(hr, "D3D12VideoDevice CreateVideoDecoder failed");

  D3D12_VIDEO_DECODER_HEAP_DESC heap_desc{
      .Configuration = {guid},
      .DecodeWidth = static_cast<UINT>(config.coded_size().width()),
      .DecodeHeight = static_cast<UINT>(config.coded_size().height()),
      .Format = decode_format,
  };
  Microsoft::WRL::ComPtr<ID3D12VideoDecoderHeap> video_decoder_heap;
  hr = video_device->CreateVideoDecoderHeap(&heap_desc,
                                            IID_PPV_ARGS(&video_decoder_heap));
  RETURN_IF_FAILED2(hr, "D3D12VideoDevice CreateVideoDecoderHeap failed");

  Microsoft::WRL::ComPtr<ID3D12Device> device;
  hr = video_decoder->GetDevice(IID_PPV_ARGS(&device));
  CHECK_EQ(hr, S_OK);

  // TODO(crbug.com/1348104): Share the command queue across video decoders.
  Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue;
  D3D12_COMMAND_QUEUE_DESC command_queue_desc{
      D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE};
  hr = device->CreateCommandQueue(&command_queue_desc,
                                  IID_PPV_ARGS(&command_queue));
  RETURN_IF_FAILED2(hr, "D3D12Device CreateCommandQueue failed");

  Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator;
  hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE,
                                      IID_PPV_ARGS(&command_allocator));
  RETURN_IF_FAILED2(hr, "D3D12Device CreateCommandAllocator failed");

  Microsoft::WRL::ComPtr<ID3D12VideoDecodeCommandList> command_list;
  hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE,
                                 command_allocator.Get(), nullptr,
                                 IID_PPV_ARGS(&command_list));
  RETURN_IF_FAILED2(hr, "D3D12Device CreateCommandList failed");

  CHECK_EQ(command_list->Close(), S_OK);

  Microsoft::WRL::ComPtr<ID3D12Fence> fence;
  hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
  RETURN_IF_FAILED2(hr, "D3D12Device CreateFence failed");

  D3D12_FEATURE_DATA_FORMAT_INFO format_info{.Format = decode_format};
  hr = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO, &format_info,
                                   sizeof(format_info));
  RETURN_IF_FAILED2(hr, "D3D12Device CheckFeatureSupport failed");

  return std::make_unique<D3D12VideoDecoderWrapperImpl>(
      media_log, std::move(device), std::move(command_queue),
      std::move(command_allocator), std::move(fence), std::move(video_device),
      std::move(video_decoder), std::move(video_decoder_heap),
      std::move(command_list), format_info.PlaneCount);
}

D3D12VideoDecoderWrapper::D3D12VideoDecoderWrapper(MediaLog* media_log)
    : D3DVideoDecoderWrapper(media_log) {}

}  // namespace media
