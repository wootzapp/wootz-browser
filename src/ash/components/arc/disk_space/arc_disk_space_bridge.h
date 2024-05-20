// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_COMPONENTS_ARC_DISK_SPACE_ARC_DISK_SPACE_BRIDGE_H_
#define ASH_COMPONENTS_ARC_DISK_SPACE_ARC_DISK_SPACE_BRIDGE_H_

#include <optional>

#include "ash/components/arc/mojom/disk_space.mojom.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "components/keyed_service/core/keyed_service.h"

namespace content {
class BrowserContext;
}  // namespace content

namespace arc {

// The uid shift of ARC.
constexpr uid_t kArcUidShift = 655360;
// The gid shift of ARC.
constexpr gid_t kArcGidShift = 655360;

// The constants below describes the ranges of valid ID to query (based on
// what is tracked by installd).These numbers are from
// system/core/libcutils/include/private/android_filesystem_config.h in
// Android codebase.

// The smallest UID in Android that is tracked by installd. This is set to be
// the minimum possible uid that Android process can have.
constexpr uid_t kAndroidUidStart = 0;
// The largest Android UID for which GetQuotaCurrentSpaceForUid() could be
// called by installd, based on the numbers in android_filesystem_config.h. The
// limit differs before and after T; it has been AID_APP_END before T, but from
// T it's AID_SDK_SANDBOX_PROCESS_END.
constexpr uid_t kAndroidUidEndBeforeT = 19999;
constexpr uid_t kAndroidUidEndAfterT = 29999;

// The following section describes the GID that are tracked by installd.
// Installd tracks different kinds of GID types: Cache, External, Shared, and
// other Android processes GID that are smaller than Cache GID. The smallest
// amongst them is 0 and the largest is Shared hence the covered range is
// between 0 and AID_SHARED_GID_END (inclusive).

// The smallest GID in Android that is tracked by installd. This is set to be
// the minimum possible gid that Android process can have.
constexpr gid_t kAndroidGidStart = 0;
// The largest GID in Android that is tracked by installd. This is from
// AID_SHARED_GID_END in android_filesystem_config.h.
constexpr gid_t kAndroidGidEnd = 59999;

// Project IDs reserved for Android files on external storage.
// Total 100 IDs are reserved from PROJECT_ID_EXT_DEFAULT (1000)
// in android_projectid_config.h
constexpr int kProjectIdForAndroidFilesStart = 1000;
constexpr int kProjectIdForAndroidFilesEnd = 1099;

// Project IDs reserved for Android apps, taken from android_projectid_config.h.
// The lower-limit of the range is PROJECT_ID_EXT_DATA_START.
// The upper-limit of the range differs before and after T. The limit has been
// PROJECT_ID_EXT_OBB_END until T, but from T it's PROJECT_ID_APP_CACHE_END.
constexpr int kProjectIdForAndroidAppsStart = 20000;
constexpr int kProjectIdForAndroidAppsEndBeforeT = 49999;
constexpr int kProjectIdForAndroidAppsEndAfterT = 69999;

class ArcBridgeService;

// This class proxies disk space related requests between CrOS and Android.
class ArcDiskSpaceBridge : public KeyedService, public mojom::DiskSpaceHost {
 public:
  // Returns singleton instance for the given BrowserContext,
  // or nullptr if the browser |context| is not allowed to use ARC.
  static ArcDiskSpaceBridge* GetForBrowserContext(
      content::BrowserContext* context);
  static ArcDiskSpaceBridge* GetForBrowserContextForTesting(
      content::BrowserContext* context);

  ArcDiskSpaceBridge(content::BrowserContext* context,
                     ArcBridgeService* bridge_service);

  ArcDiskSpaceBridge(const ArcDiskSpaceBridge&) = delete;
  ArcDiskSpaceBridge& operator=(const ArcDiskSpaceBridge&) = delete;

  ~ArcDiskSpaceBridge() override;

  // mojom::DiskSpaceHost overrides:
  void IsQuotaSupported(IsQuotaSupportedCallback callback) override;

  void GetQuotaCurrentSpaceForUid(
      uint32_t uid,
      GetQuotaCurrentSpaceForUidCallback callback) override;

  void GetQuotaCurrentSpaceForGid(
      uint32_t gid,
      GetQuotaCurrentSpaceForGidCallback callback) override;

  void GetQuotaCurrentSpaceForProjectId(
      uint32_t project_id,
      GetQuotaCurrentSpaceForProjectIdCallback callback) override;

  void GetFreeDiskSpace(GetFreeDiskSpaceCallback) override;

  static void EnsureFactoryBuilt();

 private:
  void OnGetFreeDiskSpace(GetFreeDiskSpaceCallback callback,
                          std::optional<int64_t> reply);

  const raw_ptr<ArcBridgeService>
      arc_bridge_service_;  // Owned by ArcServiceManager.

  // WeakPtrFactory to use for callbacks.
  base::WeakPtrFactory<ArcDiskSpaceBridge> weak_factory_{this};
};

}  // namespace arc

#endif  // ASH_COMPONENTS_ARC_DISK_SPACE_ARC_DISK_SPACE_BRIDGE_H_
