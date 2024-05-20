// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/components/arc/disk_space/arc_disk_space_bridge.h"

#include <utility>

#include "ash/components/arc/arc_browser_context_keyed_service_factory_base.h"
#include "ash/components/arc/arc_util.h"
#include "ash/components/arc/session/arc_bridge_service.h"
#include "base/functional/bind.h"
#include "base/memory/singleton.h"
#include "chromeos/ash/components/dbus/spaced/spaced_client.h"
#include "chromeos/ash/components/dbus/userdataauth/userdataauth_client.h"

namespace arc {

namespace {

// Path to query disk space and disk quota for ARC.
constexpr char kArcDiskHome[] = "/home/chronos/user";

// Singleton factory for ArcDiskSpaceBridge.
class ArcDiskSpaceBridgeFactory
    : public internal::ArcBrowserContextKeyedServiceFactoryBase<
          ArcDiskSpaceBridge,
          ArcDiskSpaceBridgeFactory> {
 public:
  // Factory name used by ArcBrowserContextKeyedServiceFactoryBase.
  static constexpr const char* kName = "ArcDiskSpaceBridgeFactory";

  static ArcDiskSpaceBridgeFactory* GetInstance() {
    return base::Singleton<ArcDiskSpaceBridgeFactory>::get();
  }

 private:
  friend base::DefaultSingletonTraits<ArcDiskSpaceBridgeFactory>;
  ArcDiskSpaceBridgeFactory() = default;
  ~ArcDiskSpaceBridgeFactory() override = default;
};

bool IsAndroidUid(uint32_t uid) {
  const uint32_t android_uid_end = GetArcAndroidSdkVersionAsInt() < kArcVersionT
                                       ? kAndroidUidEndBeforeT
                                       : kAndroidUidEndAfterT;
  return kAndroidUidStart <= uid && uid <= android_uid_end;
}

bool IsAndroidGid(uint32_t gid) {
  return kAndroidGidStart <= gid && gid <= kAndroidGidEnd;
}

bool IsAndroidProjectId(uint32_t project_id) {
  const uint32_t project_id_for_android_apps_end =
      GetArcAndroidSdkVersionAsInt() < kArcVersionT
          ? kProjectIdForAndroidAppsEndBeforeT
          : kProjectIdForAndroidAppsEndAfterT;
  return (project_id >= kProjectIdForAndroidFilesStart &&
          project_id <= kProjectIdForAndroidFilesEnd) ||
         (project_id >= kProjectIdForAndroidAppsStart &&
          project_id <= project_id_for_android_apps_end);
}

void IsQuotaSupportedOnArcDiskHome(
    ArcDiskSpaceBridge::IsQuotaSupportedCallback callback) {
  ash::SpacedClient::Get()->IsQuotaSupported(
      kArcDiskHome,
      base::BindOnce(
          [](ArcDiskSpaceBridge::IsQuotaSupportedCallback callback,
             std::optional<bool> reply) {
            LOG_IF(ERROR, !reply.has_value())
                << "Failed to retrieve result from IsQuotaSupported";
            std::move(callback).Run(reply.value_or(false));
          },
          std::move(callback)));
}

}  // namespace

// static
ArcDiskSpaceBridge* ArcDiskSpaceBridge::GetForBrowserContext(
    content::BrowserContext* context) {
  return ArcDiskSpaceBridgeFactory::GetForBrowserContext(context);
}

// static
ArcDiskSpaceBridge* ArcDiskSpaceBridge::GetForBrowserContextForTesting(
    content::BrowserContext* context) {
  return ArcDiskSpaceBridgeFactory::GetForBrowserContextForTesting(context);
}

ArcDiskSpaceBridge::ArcDiskSpaceBridge(content::BrowserContext* context,
                                       ArcBridgeService* bridge_service)
    : arc_bridge_service_(bridge_service) {
  arc_bridge_service_->disk_space()->SetHost(this);
}

ArcDiskSpaceBridge::~ArcDiskSpaceBridge() {
  arc_bridge_service_->disk_space()->SetHost(nullptr);
}

void ArcDiskSpaceBridge::IsQuotaSupported(IsQuotaSupportedCallback callback) {
  // Whether ARC quota is supported is an AND of the following two booleans:
  // * Whether there are no unmounted Android users (from cryptohome)
  // * Whether |kArcDiskHome| is mounted with quota enabled (from spaced)
  // Query cryptohome first, as the first one is more likely to be false.
  ash::UserDataAuthClient::Get()->GetArcDiskFeatures(
      user_data_auth::GetArcDiskFeaturesRequest(),
      base::BindOnce(
          [](IsQuotaSupportedCallback callback,
             std::optional<user_data_auth::GetArcDiskFeaturesReply> reply) {
            LOG_IF(ERROR, !reply.has_value())
                << "Failed to retrieve result from GetArcDiskFeatures call.";
            if (!reply.has_value() || !reply->quota_supported()) {
              std::move(callback).Run(false);
              return;
            }
            IsQuotaSupportedOnArcDiskHome(std::move(callback));
          },
          std::move(callback)));
}

void ArcDiskSpaceBridge::GetQuotaCurrentSpaceForUid(
    uint32_t android_uid,
    GetQuotaCurrentSpaceForUidCallback callback) {
  if (!IsAndroidUid(android_uid)) {
    LOG(ERROR) << "Android uid " << android_uid
               << " is outside the allowed query range";
    std::move(callback).Run(-1);
    return;
  }

  const uint32_t cros_uid = android_uid + kArcUidShift;
  ash::SpacedClient::Get()->GetQuotaCurrentSpaceForUid(
      kArcDiskHome, cros_uid,
      base::BindOnce(
          [](GetQuotaCurrentSpaceForUidCallback callback, int cros_uid,
             std::optional<int64_t> reply) {
            LOG_IF(ERROR, !reply.has_value())
                << "Failed to retrieve result from GetQuotaCurrentSpaceForUid "
                << "for uid=" << cros_uid;
            std::move(callback).Run(reply.value_or(-1));
          },
          std::move(callback), cros_uid));
}

void ArcDiskSpaceBridge::GetQuotaCurrentSpaceForGid(
    uint32_t android_gid,
    GetQuotaCurrentSpaceForGidCallback callback) {
  if (!IsAndroidGid(android_gid)) {
    LOG(ERROR) << "Android gid " << android_gid
               << " is outside the allowed query range";
    std::move(callback).Run(-1);
    return;
  }

  const uint32_t cros_gid = android_gid + kArcGidShift;
  ash::SpacedClient::Get()->GetQuotaCurrentSpaceForGid(
      kArcDiskHome, cros_gid,
      base::BindOnce(
          [](GetQuotaCurrentSpaceForGidCallback callback, int cros_gid,
             std::optional<int64_t> reply) {
            LOG_IF(ERROR, !reply.has_value())
                << "Failed to retrieve result from GetQuotaCurrentSpaceForGid "
                << "for gid=" << cros_gid;
            std::move(callback).Run(reply.value_or(-1));
          },
          std::move(callback), cros_gid));
}

void ArcDiskSpaceBridge::GetQuotaCurrentSpaceForProjectId(
    uint32_t project_id,
    GetQuotaCurrentSpaceForProjectIdCallback callback) {
  if (!IsAndroidProjectId(project_id)) {
    LOG(ERROR) << "Android project id " << project_id
               << " is outside the allowed query range";
    std::move(callback).Run(-1);
    return;
  }
  ash::SpacedClient::Get()->GetQuotaCurrentSpaceForProjectId(
      kArcDiskHome, project_id,
      base::BindOnce(
          [](GetQuotaCurrentSpaceForProjectIdCallback callback, int project_id,
             std::optional<int64_t> reply) {
            LOG_IF(ERROR, !reply.has_value())
                << "Failed to retrieve result from "
                   "GetQuotaCurrentSpaceForProjectId for project_id="
                << project_id;
            std::move(callback).Run(reply.value_or(-1));
          },
          std::move(callback), project_id));
}

void ArcDiskSpaceBridge::GetFreeDiskSpace(GetFreeDiskSpaceCallback callback) {
  ash::SpacedClient::Get()->GetFreeDiskSpace(
      kArcDiskHome,
      base::BindOnce(&ArcDiskSpaceBridge::OnGetFreeDiskSpace,
                     weak_factory_.GetWeakPtr(), std::move(callback)));
}

void ArcDiskSpaceBridge::OnGetFreeDiskSpace(GetFreeDiskSpaceCallback callback,
                                            std::optional<int64_t> reply) {
  if (!reply.has_value()) {
    LOG(ERROR) << "spaced::GetFreeDiskSpace failed";
    std::move(callback).Run(nullptr);
    return;
  }

  mojom::DiskSpacePtr disk_space = mojom::DiskSpace::New();
  disk_space->space_in_bytes = reply.value();
  std::move(callback).Run(std::move(disk_space));
}

// static
void ArcDiskSpaceBridge::EnsureFactoryBuilt() {
  ArcDiskSpaceBridgeFactory::GetInstance();
}

}  // namespace arc
