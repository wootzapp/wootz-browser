// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_ASH_COMPONENTS_GROWTH_ACTION_PERFORMER_H_
#define CHROMEOS_ASH_COMPONENTS_GROWTH_ACTION_PERFORMER_H_

#include <optional>

#include "base/functional/callback.h"
#include "base/values.h"

namespace growth {

// The different actions that the Growth framework can run.
enum class ActionType {
  kInstallWebApp = 0,
  kPinWebApp = 1,
  kOpenUrl = 2,
};

enum class ActionResult {
  kSuccess = 0,
  kFailure = 1,
};

enum class ActionResultReason {
  kParsingActionFailed = 0,

  // For kInstallWebApp action
  kWebAppProviderNotAvailable = 1,
  kWebAppInstallFailedOther = 2,
};

// Abstract interface for the different actions that Growth framework
// can make.
class ActionPerformer {
 public:
  using Callback =
      base::OnceCallback<void(ActionResult, std::optional<ActionResultReason>)>;
  ActionPerformer() = default;
  ActionPerformer(const ActionPerformer&) = delete;
  ActionPerformer& operator=(const ActionPerformer&) = delete;
  virtual ~ActionPerformer() = default;

  virtual void Run(const base::Value::Dict* action_params,
                   Callback callback) = 0;

  // Returns what type of action the subclass can run.
  virtual ActionType ActionType() const = 0;
};

}  // namespace growth

#endif  // CHROMEOS_ASH_COMPONENTS_GROWTH_ACTION_PERFORMER_H_
