// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/action_url/core/anchor_data.h"

#include "base/base64.h"
#include "base/strings/strcat.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"

namespace action_url {

AnchorData::AnchorData() = default;

AnchorData::AnchorData(const AnchorData&) = default;

AnchorData& AnchorData::operator=(const AnchorData&) = default;

AnchorData::AnchorData(AnchorData&&) = default;

AnchorData& AnchorData::operator=(AnchorData&&) = default;

AnchorData::~AnchorData() = default;

bool AnchorData::SameAnchorAs(const AnchorData& anchor) const {
  if (id_attribute != anchor.id_attribute || url != anchor.url ||
      renderer_id != anchor.renderer_id || process_id != anchor.process_id ||
      driver_id != anchor.driver_id || host_frame != anchor.host_frame) {
    return false;
  }
  return true;
}

}  // namespace action_url
