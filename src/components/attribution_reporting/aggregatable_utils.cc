// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/attribution_reporting/aggregatable_utils.h"

#include <optional>
#include <vector>

#include "base/check.h"
#include "base/functional/function_ref.h"
#include "base/time/time.h"
#include "components/attribution_reporting/aggregatable_trigger_config.h"
#include "components/attribution_reporting/constants.h"
#include "components/attribution_reporting/source_registration_time_config.mojom.h"

namespace attribution_reporting {

namespace {

std::vector<NullAggregatableReport> GetNullAggregatableReportsForLookback(
    base::Time trigger_time,
    std::optional<base::Time> attributed_source_time,
    int days_lookback,
    GenerateNullAggregatableReportFunc generate_func) {
  std::vector<NullAggregatableReport> reports;
  for (int i = 0; i <= days_lookback; i++) {
    base::Time fake_source_time = trigger_time - base::Days(i);
    if (attributed_source_time &&
        RoundDownToWholeDaySinceUnixEpoch(fake_source_time) ==
            *attributed_source_time) {
      continue;
    }

    if (generate_func(i)) {
      reports.emplace_back(fake_source_time);
    }
  }
  return reports;
}

}  // namespace

base::Time RoundDownToWholeDaySinceUnixEpoch(base::Time time) {
  return base::Time::UnixEpoch() +
         (time - base::Time::UnixEpoch()).FloorToMultiple(base::Days(1));
}

std::vector<NullAggregatableReport> GetNullAggregatableReports(
    const AggregatableTriggerConfig& config,
    base::Time trigger_time,
    std::optional<base::Time> attributed_source_time,
    GenerateNullAggregatableReportFunc generate_func) {
  // See spec
  // https://wicg.github.io/attribution-reporting-api/#generate-null-reports.

  bool has_trigger_context_id = config.trigger_context_id().has_value();

  mojom::SourceRegistrationTimeConfig source_registration_time_config =
      config.source_registration_time_config();

  static_assert((attribution_reporting::kMaxSourceExpiry.InDays() + 1) *
                    kNullReportsRateIncludeSourceRegistrationTime >
                kNullReportsRateExcludeSourceRegistrationTime);

  switch (source_registration_time_config) {
    case mojom::SourceRegistrationTimeConfig::kInclude: {
      std::optional<base::Time> rounded_attributed_source_time;
      if (attributed_source_time) {
        rounded_attributed_source_time =
            RoundDownToWholeDaySinceUnixEpoch(*attributed_source_time);
      }

      CHECK(!has_trigger_context_id);

      return GetNullAggregatableReportsForLookback(
          trigger_time, rounded_attributed_source_time,
          /*days_lookback=*/
          kMaxSourceExpiry.InDays(), generate_func);
    }
    case mojom::SourceRegistrationTimeConfig::kExclude: {
      const bool has_real_report = attributed_source_time.has_value();
      if (has_real_report) {
        return {};
      }

      if (has_trigger_context_id) {
        return {
            NullAggregatableReport{
                .fake_source_time = trigger_time,
            },
        };
      }

      return GetNullAggregatableReportsForLookback(
          trigger_time, attributed_source_time, /*days_lookback=*/0,
          generate_func);
    }
  }
}

}  // namespace attribution_reporting
