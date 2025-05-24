// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ACTION_URL_CORE_BROWSER_ACTION_URL_DRIVER_H_
#define COMPONENTS_ACTION_URL_CORE_BROWSER_ACTION_URL_DRIVER_H_

namespace action_url {

class ActionUrlDriver {
 public:
  ActionUrlDriver() = default;

  ActionUrlDriver(const ActionUrlDriver&) = delete;
  ActionUrlDriver& operator=(const ActionUrlDriver&) = delete;

  virtual ~ActionUrlDriver() = default;

  // Returns driver id which is unique in the current tab.
  virtual int GetId() const = 0;
};

}  // namespace action_url

#endif  // COMPONENTS_ACTION_URL_CORE_BROWSER_ACTION_URL_DRIVER_H_
