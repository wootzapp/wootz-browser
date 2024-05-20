// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/gpu/v4l2/stateless/device.h"

#include <map>

// build_config.h must come before BUILDFLAG()
#include "build/build_config.h"
#if BUILDFLAG(IS_CHROMEOS)
#include <linux/media/av1-ctrls.h>
#endif

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "base/check.h"
#include "base/containers/contains.h"
#include "base/notreached.h"
#include "base/posix/eintr_wrapper.h"
#include "base/strings/string_number_conversions.h"
#include "media/base/video_types.h"
#include "media/gpu/macros.h"
#include "media/media_buildflags.h"

// This has not been accepted upstream.
#ifndef V4L2_PIX_FMT_AV1
#define V4L2_PIX_FMT_AV1 v4l2_fourcc('A', 'V', '0', '1') /* AV1 */
#endif
// This has been upstreamed and backported for ChromeOS, but has not been
// picked up by the Chromium sysroots.
#ifndef V4L2_PIX_FMT_AV1_FRAME
#define V4L2_PIX_FMT_AV1_FRAME v4l2_fourcc('A', 'V', '1', 'F')
#endif

namespace media {

namespace {

std::string IoctlToString(uint64_t request) {
#define IOCTL_TO_STR(i) \
  case i:               \
    return #i;

  switch (request) {
    IOCTL_TO_STR(VIDIOC_DECODER_CMD)
    IOCTL_TO_STR(VIDIOC_DQBUF)
    IOCTL_TO_STR(VIDIOC_DQEVENT)
    IOCTL_TO_STR(VIDIOC_ENCODER_CMD)
    IOCTL_TO_STR(VIDIOC_ENUM_FMT)
    IOCTL_TO_STR(VIDIOC_ENUM_FRAMESIZES)
    IOCTL_TO_STR(VIDIOC_EXPBUF)
    IOCTL_TO_STR(VIDIOC_G_CROP)
    IOCTL_TO_STR(VIDIOC_G_EXT_CTRLS)
    IOCTL_TO_STR(VIDIOC_G_FMT)
    IOCTL_TO_STR(VIDIOC_G_PARM)
    IOCTL_TO_STR(VIDIOC_G_SELECTION)
    IOCTL_TO_STR(VIDIOC_QBUF)
    IOCTL_TO_STR(VIDIOC_QUERYBUF)
    IOCTL_TO_STR(VIDIOC_QUERYCAP)
    IOCTL_TO_STR(VIDIOC_QUERYCTRL)
    IOCTL_TO_STR(VIDIOC_QUERYMENU)
    IOCTL_TO_STR(VIDIOC_QUERY_EXT_CTRL)
    IOCTL_TO_STR(VIDIOC_REQBUFS)
    IOCTL_TO_STR(VIDIOC_STREAMOFF)
    IOCTL_TO_STR(VIDIOC_STREAMON)
    IOCTL_TO_STR(VIDIOC_SUBSCRIBE_EVENT)
    IOCTL_TO_STR(VIDIOC_S_CROP)
    IOCTL_TO_STR(VIDIOC_S_CTRL)
    IOCTL_TO_STR(VIDIOC_S_EXT_CTRLS)
    IOCTL_TO_STR(VIDIOC_S_FMT)
    IOCTL_TO_STR(VIDIOC_S_PARM)
    IOCTL_TO_STR(VIDIOC_S_SELECTION)
    IOCTL_TO_STR(VIDIOC_TRY_DECODER_CMD)
    IOCTL_TO_STR(VIDIOC_TRY_ENCODER_CMD)
    IOCTL_TO_STR(VIDIOC_TRY_FMT)
    IOCTL_TO_STR(VIDIOC_UNSUBSCRIBE_EVENT)
  }

  return "unknown";

#undef IOCTL_TO_STR
}

// Helper functions for translating between V4L2 structs that should not
// be included in the header and external structs that are shared.
enum v4l2_buf_type BufferTypeToV4L2(BufferType type) {
  if (type == BufferType::kCompressedData) {
    return V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
  } else if (type == BufferType::kDecodedFrame) {
    return V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  }

  NOTREACHED_IN_MIGRATION();
  return V4L2_BUF_TYPE_PRIVATE;
}

enum v4l2_memory MemoryTypeToV4L2(MemoryType memory) {
  switch (memory) {
    case MemoryType::kMemoryMapped:
      return V4L2_MEMORY_MMAP;
    case MemoryType::kDmaBuf:
      return V4L2_MEMORY_DMABUF;
    case MemoryType::kInvalid:
      NOTREACHED_IN_MIGRATION();
      // V4L2_MEMORY_USERPTR is not used in our code.
      return V4L2_MEMORY_USERPTR;
  }
}

BufferType V4L2ToBufferType(unsigned int type) {
  if (type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
    return BufferType::kCompressedData;
  } else if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
    return BufferType::kDecodedFrame;
  }

  NOTREACHED_IN_MIGRATION();
  return BufferType::kInvalid;
}

MemoryType V4L2ToMemoryType(unsigned int memory) {
  switch (memory) {
    case V4L2_MEMORY_MMAP:
      return MemoryType::kMemoryMapped;
    case V4L2_MEMORY_DMABUF:
      return MemoryType::kDmaBuf;
  }

  NOTREACHED_IN_MIGRATION();
  return MemoryType::kInvalid;
}

Buffer V4L2BufferToBuffer(const struct v4l2_buffer& v4l2_buffer) {
  const BufferType buffer_type = V4L2ToBufferType(v4l2_buffer.type);
  const MemoryType memory_type = V4L2ToMemoryType(v4l2_buffer.memory);
  Buffer buffer(buffer_type, memory_type, v4l2_buffer.index, v4l2_buffer.length,
                v4l2_buffer.timestamp);
  for (uint32_t plane = 0; plane < buffer.PlaneCount(); ++plane) {
    buffer.SetupPlane(plane, v4l2_buffer.m.planes[plane].m.mem_offset,
                      v4l2_buffer.m.planes[plane].length);
  }

  return buffer;
}

void BufferToV4L2Buffer(struct v4l2_buffer* v4l2_buffer, const Buffer& buffer) {
  v4l2_buffer->length = buffer.PlaneCount();
  v4l2_buffer->type = BufferTypeToV4L2(buffer.GetBufferType());
  v4l2_buffer->memory = MemoryTypeToV4L2(buffer.GetMemoryType());
  v4l2_buffer->index = buffer.GetIndex();
  v4l2_buffer->timestamp = buffer.GetTimeval();
  for (uint32_t plane = 0; plane < buffer.PlaneCount(); ++plane) {
    v4l2_buffer->m.planes[plane].length = buffer.PlaneLength(plane);
    v4l2_buffer->m.planes[plane].bytesused = buffer.PlaneBytesUsed(plane);
    v4l2_buffer->m.planes[plane].m.mem_offset = buffer.PlaneMemOffset(plane);
  }
}

using v4l2_enum_type = decltype(V4L2_PIX_FMT_H264);
// Correspondence from V4L2 codec described as a pixel format to a Control ID.
static const std::map<v4l2_enum_type, v4l2_enum_type>
    kV4L2CodecPixFmtToProfileCID = {
        {V4L2_PIX_FMT_H264_SLICE, V4L2_CID_MPEG_VIDEO_H264_PROFILE},
#if BUILDFLAG(ENABLE_HEVC_PARSER_AND_HW_DECODER)
        {V4L2_PIX_FMT_HEVC_SLICE, V4L2_CID_MPEG_VIDEO_HEVC_PROFILE},
#endif  // BUILDFLAG(ENABLE_HEVC_PARSER_AND_HW_DECODER)
        {V4L2_PIX_FMT_VP8_FRAME, V4L2_CID_MPEG_VIDEO_VP8_PROFILE},
        {V4L2_PIX_FMT_VP9_FRAME, V4L2_CID_MPEG_VIDEO_VP9_PROFILE},
#if BUILDFLAG(IS_CHROMEOS)
        {V4L2_PIX_FMT_AV1_FRAME, V4L2_CID_MPEG_VIDEO_AV1_PROFILE},
#endif
};

// Default VideoCodecProfiles associated to a V4L2 Codec Control ID.
static const std::map<v4l2_enum_type, std::vector<VideoCodecProfile>>
    kDefaultVideoCodecProfilesForProfileCID = {
        {V4L2_CID_MPEG_VIDEO_H264_PROFILE,
         {
             H264PROFILE_BASELINE,
             H264PROFILE_MAIN,
             H264PROFILE_HIGH,
         }},
#if BUILDFLAG(ENABLE_HEVC_PARSER_AND_HW_DECODER)
        {V4L2_CID_MPEG_VIDEO_HEVC_PROFILE,
         {HEVCPROFILE_MAIN, HEVCPROFILE_MAIN10}},
#endif  // BUILDFLAG(ENABLE_HEVC_PARSER_AND_HW_DECODER)
        {V4L2_CID_MPEG_VIDEO_VP8_PROFILE, {VP8PROFILE_ANY}},
        {V4L2_CID_MPEG_VIDEO_VP9_PROFILE, {VP9PROFILE_PROFILE0}},
#if BUILDFLAG(IS_CHROMEOS)
        {V4L2_CID_MPEG_VIDEO_AV1_PROFILE, {AV1PROFILE_PROFILE_MAIN}},
#endif
};

VideoCodec V4L2PixFmtToVideoCodec(uint32_t pix_fmt) {
  switch (pix_fmt) {
    case V4L2_PIX_FMT_H264_SLICE:
      return VideoCodec::kH264;
#if BUILDFLAG(ENABLE_HEVC_PARSER_AND_HW_DECODER)
    case V4L2_PIX_FMT_HEVC_SLICE:
      return VideoCodec::kHEVC;
#endif  // BUILDFLAG(ENABLE_HEVC_PARSER_AND_HW_DECODER)
    case V4L2_PIX_FMT_VP8_FRAME:
      return VideoCodec::kVP8;
    case V4L2_PIX_FMT_VP9_FRAME:
      return VideoCodec::kVP9;
    case V4L2_PIX_FMT_AV1_FRAME:
      return VideoCodec::kAV1;
  }
  return VideoCodec::kUnknown;
}

std::optional<BufferFormat> V4L2FormatToBufferFormat(
    const struct v4l2_format& format) {
  const auto fourcc = Fourcc::FromV4L2PixFmt(format.fmt.pix_mp.pixelformat);
  if (!fourcc) {
    return std::nullopt;
  }

  const gfx::Size resolution =
      gfx::Size(format.fmt.pix_mp.width, format.fmt.pix_mp.height);
  BufferFormat buffer_format =
      BufferFormat(fourcc.value(), resolution, V4L2ToBufferType(format.type));

  if (buffer_format.buffer_type == BufferType::kInvalid) {
    DVLOGF(1) << "Invalid V4L2 buffer type (" << format.type << ").";
  }
  for (size_t i = 0; i < format.fmt.pix_mp.num_planes; ++i) {
    const v4l2_plane_pix_format& plane_format = format.fmt.pix_mp.plane_fmt[i];
    buffer_format.planes.emplace_back(plane_format.bytesperline,
                                      plane_format.sizeimage);
  }
  return buffer_format;
}

void BufferFormatToV4L2Format(struct v4l2_format& v_format,
                              const BufferFormat& b_format) {
  memset(&v_format, 0, sizeof(v_format));

  v_format.type = BufferTypeToV4L2(b_format.buffer_type);
  v_format.fmt.pix_mp.pixelformat = b_format.fourcc.ToV4L2PixFmt();
  v_format.fmt.pix_mp.width = b_format.resolution.width();
  v_format.fmt.pix_mp.height = b_format.resolution.height();
  uint32_t i = 0;
  for (const auto& plane : b_format.planes) {
    v_format.fmt.pix_mp.plane_fmt[i].bytesperline = plane.stride;
    v_format.fmt.pix_mp.plane_fmt[i].sizeimage = plane.image_size;
    ++i;
  }
}
}  // namespace

std::string BufferTypeString(const BufferType buffer_type) {
  switch (buffer_type) {
    case BufferType::kCompressedData:
      return "compressed data";
    case BufferType::kDecodedFrame:
      return "decoded frame";
    case BufferType::kInvalid:
      return "INVALID";
  }
}

Device::Device() {}

Buffer::Buffer(BufferType buffer_type,
               MemoryType memory_type,
               uint32_t index,
               uint32_t plane_count,
               struct timeval time_val)
    : buffer_type_(buffer_type),
      memory_type_(memory_type),
      index_(index),
      time_val_(time_val) {
  planes_.resize(plane_count);
}

Buffer::Buffer(const Buffer&) = default;

void* Buffer::MappedAddress(uint32_t plane) const {
  DCHECK(memory_type_ == MemoryType::kMemoryMapped);
  return planes_[plane].mapped_address;
}

void Buffer::SetMappedAddress(uint32_t plane, void* address) {
  DVLOGF(4) << plane << " : " << address;
  DCHECK(memory_type_ == MemoryType::kMemoryMapped);
  planes_[plane].mapped_address = address;
}

void Buffer::SetupPlane(uint32_t plane, size_t offset, size_t size) {
  planes_[plane].mem_offset = offset;
  planes_[plane].length = size;
}

void Buffer::SetTimeAsFrameID(uint64_t usec) {
  time_val_.tv_sec = 0;
  time_val_.tv_usec = usec;
}

struct timeval Buffer::GetTimeval() const {
  return time_val_;
}

uint64_t Buffer::GetTimeAsFrameID() const {
  DCHECK_EQ(time_val_.tv_sec, 0);

  return time_val_.tv_usec;
}

bool Buffer::CopyDataIn(const void* data, size_t length) {
  DVLOGF(4) << MappedAddress(0) << " : " << data << " : " << length;

  void* destination_addr = MappedAddress(0);
  if (!destination_addr || !data || (planes_.size() != 1) ||
      (length > planes_[0].length)) {
    return false;
  }

  memcpy(destination_addr, data, length);
  planes_[0].bytes_used = length;

  return true;
}

Buffer::~Buffer() {}

BufferFormat::BufferFormat(Fourcc fourcc,
                           gfx::Size resolution,
                           BufferType buffer_type)
    : fourcc(fourcc), resolution(resolution), buffer_type(buffer_type) {}

BufferFormat::BufferFormat(const BufferFormat& other) = default;

BufferFormat::~BufferFormat() {}

std::string BufferFormat::ToString() const {
  return std::string("(" + fourcc.ToString() + " : " + resolution.ToString() +
                     " : " + BufferTypeString(buffer_type) + ")");
}

void Device::Close() {
  device_fd_.reset();
}

// VIDIOC_ENUM_FMT
std::set<VideoCodec> Device::EnumerateInputFormats() {
  std::set<VideoCodec> pix_fmts;
  v4l2_fmtdesc fmtdesc = {.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE};
  for (; IoctlDevice(VIDIOC_ENUM_FMT, &fmtdesc); ++fmtdesc.index) {
    DVLOGF(3) << "Enumerated input format: "
              << media::FourccToString(fmtdesc.pixelformat) << " ("
              << fmtdesc.description << ")";
    VideoCodec enumerated_codec = V4L2PixFmtToVideoCodec(fmtdesc.pixelformat);

    // Not all codecs returned from the device are supported by ChromeOS
    if (VideoCodec::kUnknown != enumerated_codec) {
      pix_fmts.insert(enumerated_codec);
    }
  }

  return pix_fmts;
}

// VIDIOC_G_FMT
std::optional<BufferFormat> Device::GetOutputFormat() {
  DVLOGF(3);
  struct v4l2_format format;
  memset(&format, 0, sizeof(format));
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

  if (!IoctlDevice(VIDIOC_G_FMT, &format)) {
    return std::nullopt;
  }

  return V4L2FormatToBufferFormat(format);
}

std::optional<BufferFormat> Device::TrySetOutputFormat(
    int request,
    const BufferFormat& format) {
  struct v4l2_format v_format;

  BufferFormatToV4L2Format(v_format, format);

  if (!IoctlDevice(request, &v_format)) {
    return std::nullopt;
  }

  if (format.fourcc.ToV4L2PixFmt() != v_format.fmt.pix_mp.pixelformat) {
    VLOGF(2) << "Format tried is not the format returned.";
    return std::nullopt;
  }

  return V4L2FormatToBufferFormat(v_format);
}

bool Device::TryOutputFormat(const BufferFormat& format) {
  DVLOGF(3);
  return TrySetOutputFormat(VIDIOC_TRY_FMT, format).has_value();
}

std::optional<BufferFormat> Device::SetOutputFormat(
    const BufferFormat& format) {
  DVLOGF(3);
  return TrySetOutputFormat(VIDIOC_S_FMT, format);
}

// VIDIOC_S_FMT
bool Device::SetInputFormat(VideoCodec codec,
                            gfx::Size resolution,
                            size_t encoded_buffer_size) {
  DVLOGF(3);
  const uint32_t pix_fmt = VideoCodecToV4L2PixFmt(codec);
  struct v4l2_format format;
  memset(&format, 0, sizeof(format));
  format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
  format.fmt.pix_mp.pixelformat = pix_fmt;
  format.fmt.pix_mp.width = resolution.width();
  format.fmt.pix_mp.height = resolution.height();
  format.fmt.pix_mp.num_planes = 1;
  format.fmt.pix_mp.plane_fmt[0].sizeimage = encoded_buffer_size;

  if (!IoctlDevice(VIDIOC_S_FMT, &format)) {
    return false;
  }

  if (format.fmt.pix_mp.pixelformat != pix_fmt) {
    DVLOGF(1) << "Failed to set format fourcc: " << FourccToString(pix_fmt);
    return false;
  }

  if (format.fmt.pix_mp.plane_fmt[0].sizeimage < encoded_buffer_size) {
    DVLOGF(1) << "Requested a buffer of size (" << encoded_buffer_size
              << ") but only (" << format.fmt.pix_mp.plane_fmt[0].sizeimage
              << ") provided.";
    return false;
  }

  return true;
}

// VIDIOC_STREAMON
bool Device::StreamOn(BufferType type) {
  enum v4l2_buf_type buf_type = BufferTypeToV4L2(type);

  return IoctlDevice(VIDIOC_STREAMON, &buf_type);
}

// VIDIOC_STREAMOFF
bool Device::StreamOff(BufferType type) {
  enum v4l2_buf_type buf_type = BufferTypeToV4L2(type);

  return IoctlDevice(VIDIOC_STREAMOFF, &buf_type);
}

// VIDIOC_EXPBUF
std::vector<base::ScopedFD> Device::ExportAsDMABUF(const Buffer& buffer) {
  DVLOGF(4);

  std::vector<base::ScopedFD> dmabuf_fds;
  for (uint32_t i = 0; i < buffer.PlaneCount(); ++i) {
    struct v4l2_exportbuffer expbuf;
    memset(&expbuf, 0, sizeof(expbuf));
    expbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    expbuf.index = buffer.GetIndex();
    expbuf.plane = i;
    expbuf.flags = O_CLOEXEC;
    if (!IoctlDevice(VIDIOC_EXPBUF, &expbuf)) {
      DVLOGF(1) << "VIDIOC_EXPBUF failed to export " << i << " of "
                << buffer.PlaneCount() << " planes";
      dmabuf_fds.clear();
      break;
    }

    dmabuf_fds.push_back(base::ScopedFD(expbuf.fd));
  }

  return dmabuf_fds;
}

// VIDIOC_REQBUFS
std::optional<uint32_t> Device::RequestBuffers(BufferType type,
                                               MemoryType memory,
                                               uint32_t count) {
  DVLOGF(4);
  struct v4l2_requestbuffers reqbufs;
  memset(&reqbufs, 0, sizeof(reqbufs));

  reqbufs.count = count;
  reqbufs.type = BufferTypeToV4L2(type);
  reqbufs.memory = MemoryTypeToV4L2(memory);

  if (!IoctlDevice(VIDIOC_REQBUFS, &reqbufs)) {
    return std::nullopt;
  }

  return reqbufs.count;
}

// VIDIOC_QUERYBUF
std::optional<Buffer> Device::QueryBuffer(BufferType buffer_type,
                                          MemoryType memory_type,
                                          uint32_t index,
                                          uint32_t num_planes) {
  struct v4l2_buffer v4l2_buffer;
  struct v4l2_plane v4l2_planes[VIDEO_MAX_PLANES];
  memset(&v4l2_buffer, 0, sizeof(v4l2_buffer));
  memset(v4l2_planes, 0, sizeof(v4l2_planes));
  v4l2_buffer.m.planes = v4l2_planes;
  v4l2_buffer.length = num_planes;

  v4l2_buffer.type = BufferTypeToV4L2(buffer_type);
  v4l2_buffer.memory = MemoryTypeToV4L2(memory_type);
  v4l2_buffer.index = index;

  if (!IoctlDevice(VIDIOC_QUERYBUF, &v4l2_buffer)) {
    return std::nullopt;
  }

  return V4L2BufferToBuffer(v4l2_buffer);
}

// VIDIOC_QBUF
bool Device::QueueBuffer(const Buffer& buffer,
                         const base::ScopedFD& request_fd) {
  struct v4l2_buffer v4l2_buffer;
  struct v4l2_plane v4l2_planes[VIDEO_MAX_PLANES];
  memset(&v4l2_buffer, 0, sizeof(v4l2_buffer));
  memset(v4l2_planes, 0, sizeof(v4l2_planes));
  v4l2_buffer.m.planes = v4l2_planes;

  BufferToV4L2Buffer(&v4l2_buffer, buffer);

  // TODO(frkoenig): This should be in the stateless driver. It is not currently
  // because BufferToV4L2Buffer is a function that is only available to this
  // file
  if (BufferType::kCompressedData == buffer.GetBufferType()) {
    v4l2_buffer.flags |= V4L2_BUF_FLAG_REQUEST_FD;
    v4l2_buffer.request_fd = request_fd.get();
  }

  return IoctlDevice(VIDIOC_QBUF, &v4l2_buffer);
}

// VIDIOC_DQBUF
std::optional<Buffer> Device::DequeueBuffer(BufferType buffer_type,
                                            MemoryType memory_type,
                                            uint32_t num_planes) {
  DVLOGF(4) << BufferTypeString(buffer_type);
  struct v4l2_buffer v4l2_buffer;
  struct v4l2_plane v4l2_planes[VIDEO_MAX_PLANES];
  memset(&v4l2_buffer, 0, sizeof(v4l2_buffer));
  memset(v4l2_planes, 0, sizeof(v4l2_planes));
  v4l2_buffer.m.planes = v4l2_planes;

  v4l2_buffer.length = num_planes;
  v4l2_buffer.type = BufferTypeToV4L2(buffer_type);
  v4l2_buffer.memory = MemoryTypeToV4L2(memory_type);

  if (!IoctlDevice(VIDIOC_DQBUF, &v4l2_buffer)) {
    return std::nullopt;
  }

  return V4L2BufferToBuffer(v4l2_buffer);
}

// VIDIOC_ENUM_FRAMESIZES
std::pair<gfx::Size, gfx::Size> Device::GetFrameResolutionRange(
    VideoCodec codec) {
  constexpr gfx::Size kDefaultMaxCodedSize(1920, 1088);
  constexpr gfx::Size kDefaultMinCodedSize(16, 16);

  v4l2_frmsizeenum frame_size;
  memset(&frame_size, 0, sizeof(frame_size));
  frame_size.pixel_format = VideoCodecToV4L2PixFmt(codec);
  if (IoctlDevice(VIDIOC_ENUM_FRAMESIZES, &frame_size)) {
#if BUILDFLAG(IS_CHROMEOS)
    // All of Chrome-supported implementations support STEPWISE only.
    CHECK_EQ(frame_size.type, V4L2_FRMSIZE_TYPE_STEPWISE);
#endif
    if (frame_size.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
      return std::make_pair(gfx::Size(frame_size.stepwise.min_width,
                                      frame_size.stepwise.min_height),
                            gfx::Size(frame_size.stepwise.max_width,
                                      frame_size.stepwise.max_height));
    }
  }

  VLOGF(1) << "VIDIOC_ENUM_FRAMESIZES failed for "
           << media::FourccToString(frame_size.pixel_format)
           << ", using default values";
  return std::make_pair(kDefaultMinCodedSize, kDefaultMaxCodedSize);
}

bool Device::OpenDevice() {
  DVLOGF(3);
  static const std::string kDecoderDevicePrefix = "/dev/video-dec";

  // We are sandboxed, so we can't query directory contents to check which
  // devices are actually available. Try to open the first 10; if not present,
  // we will just fail to open immediately.
  for (int i = 0; i < 10; ++i) {
    const auto path = kDecoderDevicePrefix + base::NumberToString(i);
    device_fd_.reset(
        HANDLE_EINTR(open(path.c_str(), O_RDWR | O_NONBLOCK | O_CLOEXEC, 0)));
    if (!device_fd_.is_valid()) {
      LOG(ERROR) << "Failed to open media device: " << path;
      continue;
    }

    break;
  }

  if (!device_fd_.is_valid()) {
    LOG(ERROR) << "Failed to open device fd.";
    return false;
  }

  return true;
}

bool Device::MmapBuffer(Buffer& buffer) {
  for (uint32_t plane = 0; plane < buffer.PlaneCount(); ++plane) {
    void* p = mmap(nullptr, buffer.PlaneLength(plane), PROT_READ | PROT_WRITE,
                   MAP_SHARED, device_fd_.get(), buffer.PlaneMemOffset(plane));
    if (p == MAP_FAILED) {
      MunmapBuffer(buffer);
      return false;
    }
    buffer.SetMappedAddress(plane, p);
  }

  return true;
}

void Device::MunmapBuffer(Buffer& buffer) {
  for (uint32_t plane = 0; plane < buffer.PlaneCount(); plane++) {
    if (buffer.MappedAddress(plane) != nullptr) {
      munmap(buffer.MappedAddress(plane), buffer.PlaneLength(plane));
      buffer.SetMappedAddress(plane, nullptr);
    }
  }
}

Device::~Device() {}

bool Device::Ioctl(const base::ScopedFD& fd, uint64_t request, void* arg) {
  DCHECK(fd.is_valid());
  constexpr int kIoctlOk = 0;
  const int ret = HANDLE_EINTR(ioctl(fd.get(), request, arg));
  if (ret != kIoctlOk) {
    const logging::SystemErrorCode err = logging::GetLastSystemErrorCode();
    if (err == EAGAIN && request == VIDIOC_DQBUF) {
      DVLOGF(1) << IoctlToString(request)
                << " failed: " << logging::SystemErrorCodeToString(err)
                << ": This is _usually_ an expected failure from trying to "
                   "VIDIOC_DQBUF a buffer that is not done being processed.";
    } else if (err == EINVAL &&
               (request == VIDIOC_ENUM_FMT || request == VIDIOC_QUERYMENU ||
                request == VIDIOC_QUERYCTRL || request == VIDIOC_DQBUF)) {
      VLOGF(2) << IoctlToString(request)
               << " failed: " << logging::SystemErrorCodeToString(err)
               << " : This is _usually_ an expected error.";
    } else {
      PLOG(ERROR) << IoctlToString(request);
    }
  }
  return ret == kIoctlOk;
}

bool Device::IoctlDevice(uint64_t request, void* arg) {
  return Ioctl(device_fd_, request, arg);
}

}  //  namespace media
