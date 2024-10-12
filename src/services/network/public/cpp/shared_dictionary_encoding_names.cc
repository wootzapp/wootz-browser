// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/network/public/cpp/shared_dictionary_encoding_names.h"

#include "services/network/public/cpp/features.h"

namespace network {

const char* GetSharedBrotliContentEncodingName() {
  return "br-d";
}

const char* GetSharedZstdContentEncodingName() {
  return "zstd-d";
}

}  // namespace network
