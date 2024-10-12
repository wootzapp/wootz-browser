// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/gwp_asan/client/gwp_asan_features.h"

#include "build/build_config.h"

namespace gwp_asan::internal {

#if BUILDFLAG(IS_WIN) || BUILDFLAG(IS_APPLE) || BUILDFLAG(IS_LINUX) || \
    BUILDFLAG(IS_CHROMEOS) ||                                          \
    (BUILDFLAG(IS_ANDROID) && defined(ARCH_CPU_64_BITS))
constexpr base::FeatureState kDefaultEnabled = base::FEATURE_ENABLED_BY_DEFAULT;
#else
constexpr base::FeatureState kDefaultEnabled =
    base::FEATURE_DISABLED_BY_DEFAULT;
#endif

BASE_FEATURE(kGwpAsanMalloc, "GwpAsanMalloc", kDefaultEnabled);
BASE_FEATURE(kGwpAsanPartitionAlloc, "GwpAsanPartitionAlloc", kDefaultEnabled);

BASE_FEATURE(kExtremeLightweightUAFDetector,
             "ExtremeLightweightUAFDetector",
             base::FEATURE_DISABLED_BY_DEFAULT);
const base::FeatureParam<int> kExtremeLightweightUAFDetectorSamplingFrequency{
    &kExtremeLightweightUAFDetector, "sampling_frequency",
    1000};  // Quarantine once per 1000 calls to `free`.
const base::FeatureParam<int>
    kExtremeLightweightUAFDetectorQuarantineCapacityInBytes{
        &kExtremeLightweightUAFDetector, "quarantine_capacity_in_bytes",
        256 * 1024};
constexpr base::FeatureParam<ExtremeLightweightUAFDetectorTargetProcesses>::
    Option kExtremeLightweightUAFDetectorTargetProcessesOptions[] = {
        {ExtremeLightweightUAFDetectorTargetProcesses::kAllProcesses, "all"},
        {ExtremeLightweightUAFDetectorTargetProcesses::kBrowserProcessOnly,
         "browser_only"},
};
const base::FeatureParam<ExtremeLightweightUAFDetectorTargetProcesses>
    kExtremeLightweightUAFDetectorTargetProcesses{
        &kExtremeLightweightUAFDetector,
        "target_processes",
        ExtremeLightweightUAFDetectorTargetProcesses::kAllProcesses,
        &kExtremeLightweightUAFDetectorTargetProcessesOptions,
    };

}  // namespace gwp_asan::internal
