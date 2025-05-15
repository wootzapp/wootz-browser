// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SUBRESOURCE_FILTER_CORE_BROWSER_ADBLOCK_CONTROL_H_
#define COMPONENTS_SUBRESOURCE_FILTER_CORE_BROWSER_ADBLOCK_CONTROL_H_
#include "base/logging.h"

namespace subresource_filter {

// Simple class to control ad block state to avoid circular dependencies.
// This is a singleton that can be used from both the features code and the
// throttle manager code without creating a circular dependency.
class AdBlockControl {
 public:
  static bool IsEnabled();
  static void SetEnabled(bool enabled);
  static bool global_ad_blocking_enabled_;
};

}  // namespace subresource_filter

#endif  // COMPONENTS_SUBRESOURCE_FILTER_CORE_BROWSER_ADBLOCK_CONTROL_H_