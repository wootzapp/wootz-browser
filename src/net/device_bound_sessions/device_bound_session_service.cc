// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/device_bound_sessions/device_bound_session_service.h"

#include <memory>

#include "base/memory/ptr_util.h"
#include "net/base/features.h"

namespace net {

namespace {
class DeviceBoundSessionServiceImpl : public DeviceBoundSessionService {
 public:
  DeviceBoundSessionServiceImpl() = default;
  // TODO(kristianm): Implement RegisterBoundSession
  void RegisterBoundSession(
      DeviceBoundSessionRegistrationFetcherParam registration_params,
      const IsolationInfo& isolation_info) override {}
};
}  // namespace

std::unique_ptr<DeviceBoundSessionService> DeviceBoundSessionService::Create(
    const URLRequestContext* request_context) {
  return std::make_unique<DeviceBoundSessionServiceImpl>();
}

}  // namespace net
