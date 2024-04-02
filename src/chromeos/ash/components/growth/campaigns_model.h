// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_ASH_COMPONENTS_GROWTH_CAMPAIGNS_MODEL_H_
#define CHROMEOS_ASH_COMPONENTS_GROWTH_CAMPAIGNS_MODEL_H_

#include <memory>
#include <optional>

#include "base/component_export.h"
#include "base/values.h"

namespace base {
class Time;
}

namespace growth {

// Entries should not be renumbered and numeric values should never be reused
// as it is used for logging metrics as well. Please keep in sync with
// "CampaignSlot" in src/tools/metrics/histograms/enums.xml.
enum class Slot {
  kDemoModeApp = 0,
  kDemoModeFreePlayApps = 1,
  kNudge = 2,
  kMaxValue = kNudge
};

// Dictionary of supported targetings. For example:
// {
//    "demoMode" : {...},
//    "session": {...}
// }
using Targeting = base::Value::Dict;

// List of `Targeting`.
using Targetings = base::Value::List;

// Dictionary of supported payloads. For example:
// {
//   "demoMode": {
//     "attractionLoop": {
//       "videoSrcLang1": "/asset/lang1.mp4",
//       "videoSrcLang2": "/asset/lang2.mp4"
//     }
//   }
// }
using Payload = base::Value::Dict;

// Dictionary of Campaign. For example:
// {
//    "id": 1,
//    "studyId":1,
//    "targetings": {...}
//    "payload": {...}
// }
using Campaign = base::Value::Dict;

// List of campaigns.
using Campaigns = base::Value::List;

// Lists of campaigns keyed by the targeted slot. The key is the slot ID in
// string. For example:
// {
//   "0": [...]
//   "1": [...]
// }
using CampaignsPerSlot = base::Value::Dict;

const Campaigns* GetCampaignsBySlot(const CampaignsPerSlot* campaigns_per_slot,
                                    Slot slot);

const Targetings* GetTargetings(const Campaign* campaign);

COMPONENT_EXPORT(CHROMEOS_ASH_COMPONENTS_GROWTH)
const Payload* GetPayloadBySlot(const Campaign* campaign, Slot slot);

COMPONENT_EXPORT(CHROMEOS_ASH_COMPONENTS_GROWTH)
std::optional<int> GetCampaignId(const Campaign* campaign);

COMPONENT_EXPORT(CHROMEOS_ASH_COMPONENTS_GROWTH)
std::optional<int> GetStudyId(const Campaign* campaign);

// Lists of campaigns keyed by the targeted slot. The key is the slot ID in
// string. For example:
// {
//   "0": [...]
//   "1": [...]
// }
using CampaignsPerSlot = base::Value::Dict;

const Campaigns* GetCampaignsBySlot(const CampaignsPerSlot* campaigns_per_slot,
                                    Slot slot);

const Targetings* GetTargetings(const Campaign* campaign);

const Payload* GetPayloadBySlot(const Campaign* campaign, Slot slot);

class TargetingBase {
 public:
  explicit TargetingBase(const Targeting* targeting_dict,
                         const char* targeting_path);
  TargetingBase(const TargetingBase&) = delete;
  TargetingBase& operator=(const TargetingBase) = delete;
  ~TargetingBase();

  // True if the specific targeting (e.g: demoMode) was found in the targeting
  // dictionary. The campaign will be selected if the targeted criteria is not
  // found and defer to the next criteria matching.
  bool IsValid() const;

 protected:
  const base::Value::List* GetListCriteria(const char* path_suffix) const;
  const std::optional<bool> GetBoolCriteria(const char* path_suffix) const;
  const std::optional<int> GetIntCriteria(const char* path_suffix) const;
  const std::string* GetStringCriteria(const char* path_suffix) const;

 private:
  const std::string GetCriteriaPath(const char* path_suffix) const;

  // The dictionary that contains targeting definition. Owned by
  // `CampaignsManager`.
  raw_ptr<const Targeting> targeting_;
  // The targeting path.
  const char* targeting_path_;
};

// Demo mode targeting. For example:
// {
//   "retailers": ["bb", "bsb"];
//   "storeIds": ["2", "4", "6"],
//   "country": ["US"],
//   "capability": {
//     "isFeatureAwareDevice": false,
//     "isCloudGamingDevice": true,
//   }
// }
class DemoModeTargeting : public TargetingBase {
 public:
  explicit DemoModeTargeting(const Targeting* targeting_dict);
  DemoModeTargeting(const DemoModeTargeting&) = delete;
  DemoModeTargeting& operator=(const DemoModeTargeting) = delete;
  ~DemoModeTargeting();

  const base::Value::List* GetStoreIds() const;
  const base::Value::List* GetRetailers() const;
  const base::Value::List* GetCountries() const;
  const std::string* GetAppMinVersion() const;
  const std::string* GetAppMaxVersion() const;
  const std::optional<bool> TargetCloudGamingDevice() const;
  const std::optional<bool> TargetFeatureAwareDevice() const;
};

// Wrapper around Device targeting dictionary. The structure looks like:
// {
//   "locales": ["en-US", "zh-CN"];
//   "milestone": {
//      "min": 117,
//      "max": 120
//   }
// }
class DeviceTargeting : public TargetingBase {
 public:
  explicit DeviceTargeting(const Targeting* targeting_dict);
  DeviceTargeting(const DeviceTargeting&) = delete;
  DeviceTargeting& operator=(const DeviceTargeting) = delete;
  ~DeviceTargeting();

  const base::Value::List* GetLocales() const;
  const std::optional<int> GetMinMilestone() const;
  const std::optional<int> GetMaxMilestone() const;
  const std::optional<bool> GetFeatureAwareDevice() const;
};

// Wrapper around scheduling targeting dictionary.
//
// The structure looks like:
// {
//   "start": 1697046365,
//   "end": 1697046598
// }
//
// Start and end are the number of seconds since epoch in UTC.
class SchedulingTargeting {
 public:
  explicit SchedulingTargeting(const base::Value::Dict* scheduling);
  SchedulingTargeting(const SchedulingTargeting&) = delete;
  SchedulingTargeting& operator=(const SchedulingTargeting) = delete;
  ~SchedulingTargeting();

  const base::Time GetStartTime() const;
  const base::Time GetEndTime() const;

 private:
  raw_ptr<const base::Value::Dict> scheduling_dict_;
};

// Wrapper around scheduling targeting dictionary.
//
// The structure looks like:
// {
//   "scheduling": []
// }
class SessionTargeting : public TargetingBase {
 public:
  explicit SessionTargeting(const Targeting* targeting_dict);
  SessionTargeting(const SessionTargeting&) = delete;
  SessionTargeting& operator=(const SessionTargeting) = delete;
  ~SessionTargeting();

  const std::vector<std::unique_ptr<SchedulingTargeting>> GetSchedulings()
      const;
  const base::Value::List* GetExperimentTags() const;
};

}  // namespace growth

#endif  // CHROMEOS_ASH_COMPONENTS_GROWTH_CAMPAIGNS_MODEL_H_
