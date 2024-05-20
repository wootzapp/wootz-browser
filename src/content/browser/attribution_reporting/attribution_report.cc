// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/attribution_reporting/attribution_report.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <utility>

#include "base/check.h"
#include "base/check_op.h"
#include "base/functional/overloaded.h"
#include "base/numerics/checked_math.h"
#include "base/strings/strcat.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "components/attribution_reporting/aggregatable_trigger_config.h"
#include "components/attribution_reporting/source_type.h"
#include "components/attribution_reporting/suitable_origin.h"
#include "content/browser/attribution_reporting/aggregatable_attribution_utils.h"
#include "content/browser/attribution_reporting/common_source_info.h"
#include "content/browser/attribution_reporting/stored_source.h"
#include "net/http/http_request_headers.h"
#include "third_party/abseil-cpp/absl/types/variant.h"
#include "third_party/blink/public/mojom/aggregation_service/aggregatable_report.mojom.h"
#include "url/gurl.h"
#include "url/origin.h"
#include "url/url_canon.h"

namespace content {

namespace {

void PopulateSourceDebugKey(base::Value::Dict& dict,
                            std::optional<uint64_t> debug_key) {
  if (debug_key.has_value()) {
    dict.Set("source_debug_key", base::NumberToString(*debug_key));
  }
}

void PopulateTriggerDebugKey(base::Value::Dict& dict,
                             std::optional<uint64_t> debug_key) {
  if (debug_key.has_value()) {
    dict.Set("trigger_debug_key", base::NumberToString(*debug_key));
  }
}

void PopulateReportBody(base::Value::Dict& dict,
                        const AttributionReport::CommonAggregatableData& data,
                        std::optional<uint64_t> trigger_debug_key) {
  if (const auto& assembled_report = data.assembled_report;
      assembled_report.has_value()) {
    dict = assembled_report->GetAsJson();
  } else {
    // This generally should only be called when displaying the report
    // for debugging/internals.
    dict.Set("shared_info", "not generated prior to send");
    dict.Set("aggregation_service_payloads", "not generated prior to send");
  }

  PopulateTriggerDebugKey(dict, trigger_debug_key);

  if (const auto& trigger_context_id =
          data.aggregatable_trigger_config.trigger_context_id();
      trigger_context_id.has_value()) {
    dict.Set("trigger_context_id", *trigger_context_id);
  }
}

}  // namespace

AttributionReport::EventLevelData::EventLevelData(uint32_t trigger_data,
                                                  int64_t priority,
                                                  StoredSource source)
    : trigger_data(trigger_data),
      priority(priority),
      source(std::move(source)) {}

AttributionReport::EventLevelData::EventLevelData(const EventLevelData&) =
    default;

AttributionReport::EventLevelData& AttributionReport::EventLevelData::operator=(
    const EventLevelData&) = default;

AttributionReport::EventLevelData::EventLevelData(EventLevelData&&) = default;

AttributionReport::EventLevelData& AttributionReport::EventLevelData::operator=(
    EventLevelData&&) = default;

AttributionReport::EventLevelData::~EventLevelData() = default;

AttributionReport::CommonAggregatableData::CommonAggregatableData(
    std::optional<attribution_reporting::SuitableOrigin>
        aggregation_coordinator_origin,
    std::optional<std::string> verification_token,
    attribution_reporting::AggregatableTriggerConfig
        aggregatable_trigger_config)
    : aggregation_coordinator_origin(std::move(aggregation_coordinator_origin)),
      verification_token(std::move(verification_token)),
      aggregatable_trigger_config(std::move(aggregatable_trigger_config)) {}

AttributionReport::CommonAggregatableData::CommonAggregatableData() = default;

AttributionReport::CommonAggregatableData::CommonAggregatableData(
    const CommonAggregatableData&) = default;

AttributionReport::CommonAggregatableData&
AttributionReport::CommonAggregatableData::operator=(
    const CommonAggregatableData&) = default;

AttributionReport::CommonAggregatableData::CommonAggregatableData(
    CommonAggregatableData&&) = default;

AttributionReport::CommonAggregatableData&
AttributionReport::CommonAggregatableData::operator=(CommonAggregatableData&&) =
    default;

AttributionReport::CommonAggregatableData::~CommonAggregatableData() = default;

AttributionReport::AggregatableAttributionData::AggregatableAttributionData(
    CommonAggregatableData common_data,
    std::vector<blink::mojom::AggregatableReportHistogramContribution>
        contributions,
    StoredSource source)
    : common_data(std::move(common_data)),
      contributions(std::move(contributions)),
      source(std::move(source)) {}

AttributionReport::AggregatableAttributionData::AggregatableAttributionData(
    const AggregatableAttributionData&) = default;

AttributionReport::AggregatableAttributionData&
AttributionReport::AggregatableAttributionData::operator=(
    const AggregatableAttributionData&) = default;

AttributionReport::AggregatableAttributionData::AggregatableAttributionData(
    AggregatableAttributionData&&) = default;

AttributionReport::AggregatableAttributionData&
AttributionReport::AggregatableAttributionData::operator=(
    AggregatableAttributionData&&) = default;

AttributionReport::AggregatableAttributionData::~AggregatableAttributionData() =
    default;

base::CheckedNumeric<int64_t>
AttributionReport::AggregatableAttributionData::BudgetRequired() const {
  return GetTotalAggregatableValues(contributions);
}

AttributionReport::NullAggregatableData::NullAggregatableData(
    CommonAggregatableData common_data,
    attribution_reporting::SuitableOrigin reporting_origin,
    base::Time fake_source_time)
    : common_data(std::move(common_data)),
      reporting_origin(std::move(reporting_origin)),
      fake_source_time(fake_source_time) {}

AttributionReport::NullAggregatableData::NullAggregatableData(
    const NullAggregatableData&) = default;

AttributionReport::NullAggregatableData::NullAggregatableData(
    NullAggregatableData&&) = default;

AttributionReport::NullAggregatableData&
AttributionReport::NullAggregatableData::operator=(
    const NullAggregatableData&) = default;

AttributionReport::NullAggregatableData&
AttributionReport::NullAggregatableData::operator=(NullAggregatableData&&) =
    default;

AttributionReport::NullAggregatableData::~NullAggregatableData() = default;

AttributionReport::AttributionReport(AttributionInfo attribution_info,
                                     Id id,
                                     base::Time report_time,
                                     base::Time initial_report_time,
                                     base::Uuid external_report_id,
                                     int failed_send_attempts,
                                     Data data)
    : attribution_info_(std::move(attribution_info)),
      id_(id),
      report_time_(report_time),
      initial_report_time_(initial_report_time),
      external_report_id_(std::move(external_report_id)),
      failed_send_attempts_(failed_send_attempts),
      data_(std::move(data)) {
  DCHECK(external_report_id_.is_valid());
  DCHECK_GE(failed_send_attempts_, 0);
}

AttributionReport::AttributionReport(const AttributionReport&) = default;

AttributionReport& AttributionReport::operator=(const AttributionReport&) =
    default;

AttributionReport::AttributionReport(AttributionReport&&) = default;

AttributionReport& AttributionReport::operator=(AttributionReport&&) = default;

AttributionReport::~AttributionReport() = default;

GURL AttributionReport::ReportURL(bool debug) const {
  static constexpr char kBasePath[] = "/.well-known/attribution-reporting/";
  static constexpr char kDebugPath[] = "debug/";

  const char* endpoint_path;
  switch (GetReportType()) {
    case Type::kEventLevel:
      endpoint_path = "report-event-attribution";
      break;
    case Type::kAggregatableAttribution:
    case Type::kNullAggregatable:
      endpoint_path = "report-aggregate-attribution";
      break;
  }

  std::string path =
      base::StrCat({kBasePath, debug ? kDebugPath : "", endpoint_path});

  GURL::Replacements replacements;
  replacements.SetPathStr(path);
  return GetReportingOrigin()->GetURL().ReplaceComponents(replacements);
}

base::Value::Dict AttributionReport::ReportBody() const {
  base::Value::Dict dict;

  absl::visit(
      base::Overloaded{
          [&](const EventLevelData& data) {
            const StoredSource& source = data.source;

            dict.Set("attribution_destination",
                     source.destination_sites().ToJson());

            // The API denotes these values as strings; a `uint64_t` cannot be
            // put in a dict as an integer in order to be opaque to various API
            // configurations.
            dict.Set("source_event_id",
                     base::NumberToString(source.source_event_id()));

            dict.Set("trigger_data", base::NumberToString(data.trigger_data));

            dict.Set("source_type", attribution_reporting::SourceTypeName(
                                        source.common_info().source_type()));

            dict.Set("report_id", external_report_id_.AsLowercaseString());

            // Round to 7 digits of precision, which allows us to express binary
            // randomized response with epsilon = 14 without rounding to 0
            // (0.00000166305 -> 0.0000017).
            double rounded_rate =
                round(source.randomized_response_rate() * 10000000) /
                10000000.0;
            dict.Set("randomized_trigger_rate", rounded_rate);

            PopulateSourceDebugKey(dict, source.debug_key());
            PopulateTriggerDebugKey(dict, attribution_info_.debug_key);

            dict.Set("scheduled_report_time",
                     base::NumberToString(
                         (initial_report_time_ - base::Time::UnixEpoch())
                             .InSeconds()));
          },

          [&](const AggregatableAttributionData& data) {
            PopulateReportBody(dict, data.common_data,
                               attribution_info_.debug_key);

            PopulateSourceDebugKey(dict, data.source.debug_key());
          },

          [&](const NullAggregatableData& data) {
            PopulateReportBody(dict, data.common_data,
                               attribution_info_.debug_key);
          },
      },
      data_);

  return dict;
}

void AttributionReport::set_report_time(base::Time report_time) {
  report_time_ = report_time;
}

void AttributionReport::set_external_report_id(base::Uuid external_report_id) {
  DCHECK(external_report_id.is_valid());
  external_report_id_ = std::move(external_report_id);
}

// static
std::optional<base::Time> AttributionReport::MinReportTime(
    std::optional<base::Time> a,
    std::optional<base::Time> b) {
  if (!a.has_value()) {
    return b;
  }

  if (!b.has_value()) {
    return a;
  }

  return std::min(*a, *b);
}

void AttributionReport::PopulateAdditionalHeaders(
    net::HttpRequestHeaders& headers) const {
  const std::optional<std::string>* verification_token = absl::visit(
      base::Overloaded{
          [](const EventLevelData&) -> const std::optional<std::string>* {
            return nullptr;
          },

          [](const AggregatableAttributionData& data) {
            return &data.common_data.verification_token;
          },

          [](const NullAggregatableData& data) {
            return &data.common_data.verification_token;
          },
      },
      data_);

  if (verification_token && verification_token->has_value()) {
    headers.SetHeader("Sec-Attribution-Reporting-Private-State-Token",
                      **verification_token);
  }
}

const StoredSource* AttributionReport::GetStoredSource() const {
  return absl::visit(
      base::Overloaded{
          [](const EventLevelData& data) { return &data.source; },
          [](const AggregatableAttributionData& data) { return &data.source; },
          [](const NullAggregatableData& data) -> const StoredSource* {
            return nullptr;
          },
      },
      data_);
}

const attribution_reporting::SuitableOrigin&
AttributionReport::GetReportingOrigin() const {
  return absl::visit(base::Overloaded{
                         [](const EventLevelData& data)
                             -> const attribution_reporting::SuitableOrigin& {
                           return data.source.common_info().reporting_origin();
                         },
                         [](const AggregatableAttributionData& data)
                             -> const attribution_reporting::SuitableOrigin& {
                           return data.source.common_info().reporting_origin();
                         },
                         [](const NullAggregatableData& data)
                             -> const attribution_reporting::SuitableOrigin& {
                           return data.reporting_origin;
                         },
                     },
                     data_);
}

}  // namespace content
