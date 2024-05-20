// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/filters/hls_test_helpers.h"

#include "base/files/file_util.h"
#include "media/base/test_data_util.h"
#include "media/filters/hls_data_source_provider.h"

namespace media {
using testing::_;

MockHlsDataSourceProvider::MockHlsDataSourceProvider() = default;
MockHlsDataSourceProvider::~MockHlsDataSourceProvider() = default;

// static
std::unique_ptr<HlsDataSourceStream>
StringHlsDataSourceStreamFactory::CreateStream(std::string content,
                                               bool taint_origin) {
  HlsDataSourceProvider::SegmentQueue segments;
  auto stream = std::make_unique<HlsDataSourceStream>(
      HlsDataSourceStream::StreamId::FromUnsafeValue(42), std::move(segments),
      base::DoNothing());
  auto* buffer = stream->LockStreamForWriting(content.length());
  memcpy(buffer, content.c_str(), content.length());
  stream->UnlockStreamPostWrite(content.length(), true);
  if (taint_origin) {
    stream->set_would_taint_origin();
  }
  return stream;
}

// static
std::unique_ptr<HlsDataSourceStream>
FileHlsDataSourceStreamFactory::CreateStream(std::string filename,
                                             bool taint_origin) {
  base::FilePath file_path = GetTestDataFilePath(filename);
  int64_t file_size = 0;
  CHECK(base::GetFileSize(file_path, &file_size))
      << "Failed to get file size for '" << filename << "'";
  HlsDataSourceProvider::SegmentQueue segments;
  auto stream = std::make_unique<HlsDataSourceStream>(
      HlsDataSourceStream::StreamId::FromUnsafeValue(42), std::move(segments),
      base::DoNothing());
  auto* buffer = stream->LockStreamForWriting(file_size);
  CHECK_EQ(file_size, base::ReadFile(file_path, reinterpret_cast<char*>(buffer),
                                     file_size));
  stream->UnlockStreamPostWrite(file_size, true);
  if (taint_origin) {
    stream->set_would_taint_origin();
  }
  return stream;
}

MockManifestDemuxerEngineHost::MockManifestDemuxerEngineHost() = default;
MockManifestDemuxerEngineHost::~MockManifestDemuxerEngineHost() = default;

MockHlsRenditionHost::MockHlsRenditionHost() {}
MockHlsRenditionHost::~MockHlsRenditionHost() {}

MockHlsRendition::MockHlsRendition() = default;
MockHlsRendition::~MockHlsRendition() = default;

}  // namespace media
