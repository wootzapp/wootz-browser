// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_API_DECLARATIVE_NET_REQUEST_RULESET_INSTALL_PREF_H_
#define EXTENSIONS_BROWSER_API_DECLARATIVE_NET_REQUEST_RULESET_INSTALL_PREF_H_

#include <optional>
#include <vector>

#include "extensions/common/api/declarative_net_request/constants.h"

namespace extensions::declarative_net_request {

struct RulesetInstallPref {
  // ID of the ruleset.
  RulesetID ruleset_id;
  // Checksum of the indexed ruleset if specified.
  std::optional<int> checksum;

  // If set to true, then the ruleset was ignored and not indexed.
  bool ignored;

  RulesetInstallPref(RulesetID ruleset_id,
                     std::optional<int> checksum,
                     bool ignored);
};

using RulesetInstallPrefs = std::vector<RulesetInstallPref>;

}  // namespace extensions::declarative_net_request

#endif  // EXTENSIONS_BROWSER_API_DECLARATIVE_NET_REQUEST_RULESET_INSTALL_PREF_H_
