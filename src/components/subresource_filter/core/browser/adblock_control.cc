// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/subresource_filter/core/browser/adblock_control.h"

namespace subresource_filter {

bool AdBlockControl::IsEnabled() {
  LOG(INFO) << "AdBlock: AdBlockControl::IsEnabled: " << global_ad_blocking_enabled_;
  return global_ad_blocking_enabled_;
}

void AdBlockControl::SetEnabled(bool enabled) {
  LOG(INFO) << "AdBlock: AdBlockControl::SetEnabled: " << enabled;
  global_ad_blocking_enabled_ = enabled;
}

bool AdBlockControl::global_ad_blocking_enabled_ = false;

}  // namespace subresource_filter