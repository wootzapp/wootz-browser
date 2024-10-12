// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/attribution_reporting/attribution_debug_report.h"

#include <stdint.h>

#include <optional>
#include <sstream>
#include <string_view>
#include <utility>

#include "base/check.h"
#include "base/check_op.h"
#include "base/feature_list.h"
#include "base/functional/function_ref.h"
#include "base/functional/overloaded.h"
#include "base/metrics/histogram_functions.h"
#include "base/notreached.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "components/attribution_reporting/constants.h"
#include "components/attribution_reporting/destination_set.h"
#include "components/attribution_reporting/os_registration.h"
#include "components/attribution_reporting/registration_header_error.h"
#include "components/attribution_reporting/source_registration.h"
#include "components/attribution_reporting/suitable_origin.h"
#include "components/attribution_reporting/trigger_registration.h"
#include "content/browser/attribution_reporting/attribution_constants.h"
#include "content/browser/attribution_reporting/attribution_features.h"
#include "content/browser/attribution_reporting/attribution_reporting.mojom.h"
#include "content/browser/attribution_reporting/attribution_trigger.h"
#include "content/browser/attribution_reporting/common_source_info.h"
#include "content/browser/attribution_reporting/create_report_result.h"
#include "content/browser/attribution_reporting/os_registration.h"
#include "content/browser/attribution_reporting/storable_source.h"
#include "content/browser/attribution_reporting/store_source_result.h"
#include "net/base/schemeful_site.h"
#include "third_party/abseil-cpp/absl/numeric/int128.h"
#include "third_party/abseil-cpp/absl/types/variant.h"
#include "url/gurl.h"

namespace content {

namespace {

using EventLevelResult = ::content::AttributionTrigger::EventLevelResult;
using AggregatableResult = ::content::AttributionTrigger::AggregatableResult;

constexpr char kAttributionDestination[] = "attribution_destination";

// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum class DebugDataType {
  kSourceDestinationLimit = 0,
  kSourceNoised = 1,
  kSourceStorageLimit = 2,
  kSourceSuccess = 3,
  kSourceUnknownError = 4,
  kSourceDestinationRateLimit = 5,
  kTriggerNoMatchingSource = 6,
  kTriggerNoMatchingFilterData = 8,
  kTriggerReportingOriginLimit = 9,
  kTriggerEventDeduplicated = 10,
  kTriggerEventNoMatchingConfigurations = 11,
  kTriggerEventNoise = 12,
  kTriggerEventLowPriority = 13,
  kTriggerEventExcessiveReports = 14,
  kTriggerEventStorageLimit = 15,
  kTriggerEventReportWindowPassed = 16,
  kTriggerAggregateDeduplicated = 17,
  kTriggerAggregateNoContributions = 18,
  kTriggerAggregateInsufficientBudget = 19,
  kTriggerAggregateStorageLimit = 20,
  kTriggerAggregateReportWindowPassed = 21,
  kTriggerAggregateExcessiveReports = 22,
  kTriggerUnknownError = 23,
  kOsSourceDelegated = 24,
  kOsTriggerDelegated = 25,
  kTriggerEventReportWindowNotStarted = 26,
  kTriggerEventNoMatchingTriggerData = 27,
  kHeaderParsingError = 28,
  kSourceReportingOriginPerSiteLimit = 29,
  kTriggerEventAttributionsPerSourceDestinationLimit = 30,
  kTriggerAggregateAttributionsPerSourceDestinationLimit = 31,
  kSourceMaxChannelCapacityReached = 32,
  kSourceMaxTriggerDataCardinalityReached = 33,
  kMaxValue = kSourceMaxTriggerDataCardinalityReached,
};

struct DebugDataTypeAndBody {
  DebugDataType debug_data_type;
  base::Value limit;

  explicit DebugDataTypeAndBody(DebugDataType debug_data_type,
                                base::Value limit = base::Value())
      : debug_data_type(debug_data_type), limit(std::move(limit)) {}
};

// This is a temporary measure until we phase out the use of uint128.
std::string EncodeUint128ToString(absl::uint128 value) {
  std::ostringstream out;
  out << value;
  return out.str();
}

base::Value GetLimit(int limit) {
  return base::Value(base::NumberToString(limit));
}

base::Value GetLimit(absl::uint128 limit) {
  return base::Value(EncodeUint128ToString(limit));
}

std::optional<DebugDataTypeAndBody> GetReportDataBody(
    const StoreSourceResult& result) {
  return absl::visit(
      base::Overloaded{
          [](StoreSourceResult::ProhibitedByBrowserPolicy) {
            return std::optional<DebugDataTypeAndBody>();
          },
          [&](absl::variant<StoreSourceResult::Success,
                            // `kSourceSuccess` is sent for a few errors as well
                            // to mitigate the security concerns on reporting
                            // these errors. Because these errors are thrown
                            // based on information across reporting origins,
                            // reporting on them would violate the same-origin
                            // policy.
                            StoreSourceResult::ExcessiveReportingOrigins,
                            StoreSourceResult::DestinationGlobalLimitReached>) {
            return result.is_noised()
                       ? std::make_optional<DebugDataTypeAndBody>(
                             DebugDataType::kSourceNoised)
                       : std::make_optional<DebugDataTypeAndBody>(
                             DebugDataType::kSourceSuccess);
          },
          [](StoreSourceResult::InsufficientUniqueDestinationCapacity v) {
            return std::make_optional<DebugDataTypeAndBody>(
                DebugDataType::kSourceDestinationLimit, GetLimit(v.limit));
          },
          [](absl::variant<StoreSourceResult::DestinationReportingLimitReached,
                           StoreSourceResult::DestinationBothLimitsReached> v) {
            return std::make_optional<DebugDataTypeAndBody>(
                DebugDataType::kSourceDestinationRateLimit,
                absl::visit([](auto v) { return GetLimit(v.limit); }, v));
          },
          [](StoreSourceResult::InsufficientSourceCapacity v) {
            return std::make_optional<DebugDataTypeAndBody>(
                DebugDataType::kSourceStorageLimit, GetLimit(v.limit));
          },
          [](StoreSourceResult::InternalError) {
            return std::make_optional<DebugDataTypeAndBody>(
                DebugDataType::kSourceUnknownError);
          },
          [](StoreSourceResult::ReportingOriginsPerSiteLimitReached v) {
            return std::make_optional<DebugDataTypeAndBody>(
                DebugDataType::kSourceReportingOriginPerSiteLimit,
                GetLimit(v.limit));
          },
          [](StoreSourceResult::ExceedsMaxChannelCapacity v) {
            return std::make_optional<DebugDataTypeAndBody>(
                DebugDataType::kSourceMaxChannelCapacityReached,
                base::Value(v.limit));
          },
          [](StoreSourceResult::ExceedsMaxTriggerStateCardinality v) {
            return std::make_optional<DebugDataTypeAndBody>(
                DebugDataType::kSourceMaxTriggerDataCardinalityReached,
                GetLimit(v.limit));
          },
      },
      result.result());
}

std::optional<DebugDataTypeAndBody> GetReportDataTypeAndLimit(
    EventLevelResult result,
    const CreateReportResult::Limits& limits) {
  switch (result) {
    case EventLevelResult::kSuccess:
    case EventLevelResult::kProhibitedByBrowserPolicy:
    case EventLevelResult::kSuccessDroppedLowerPriority:
    case EventLevelResult::kNotRegistered:
      return std::nullopt;
    case EventLevelResult::kInternalError:
      return DebugDataTypeAndBody(DebugDataType::kTriggerUnknownError);
    case EventLevelResult::kNoCapacityForConversionDestination:
      return DebugDataTypeAndBody(
          DebugDataType::kTriggerEventStorageLimit,
          GetLimit(limits.max_event_level_reports_per_destination.value()));
    case EventLevelResult::kExcessiveReportingOrigins:
      return DebugDataTypeAndBody(
          DebugDataType::kTriggerReportingOriginLimit,
          GetLimit(
              limits.rate_limits_max_attribution_reporting_origins.value()));
    case EventLevelResult::kNoMatchingImpressions:
      return DebugDataTypeAndBody(DebugDataType::kTriggerNoMatchingSource);
    case EventLevelResult::kExcessiveAttributions:
      return DebugDataTypeAndBody(
          DebugDataType::kTriggerEventAttributionsPerSourceDestinationLimit,
          GetLimit(limits.rate_limits_max_attributions.value()));
    case EventLevelResult::kNoMatchingSourceFilterData:
      return DebugDataTypeAndBody(DebugDataType::kTriggerNoMatchingFilterData);
    case EventLevelResult::kDeduplicated:
      return DebugDataTypeAndBody(DebugDataType::kTriggerEventDeduplicated);
    case EventLevelResult::kNoMatchingConfigurations:
      return DebugDataTypeAndBody(
          DebugDataType::kTriggerEventNoMatchingConfigurations);
    case EventLevelResult::kNeverAttributedSource:
    case EventLevelResult::kFalselyAttributedSource:
      return DebugDataTypeAndBody(DebugDataType::kTriggerEventNoise);
    case EventLevelResult::kPriorityTooLow:
      return DebugDataTypeAndBody(DebugDataType::kTriggerEventLowPriority);
    case EventLevelResult::kExcessiveReports:
      return DebugDataTypeAndBody(DebugDataType::kTriggerEventExcessiveReports);
    case EventLevelResult::kReportWindowNotStarted:
      return DebugDataTypeAndBody(
          DebugDataType::kTriggerEventReportWindowNotStarted);
    case EventLevelResult::kReportWindowPassed:
      return DebugDataTypeAndBody(
          DebugDataType::kTriggerEventReportWindowPassed);
    case EventLevelResult::kNoMatchingTriggerData:
      return DebugDataTypeAndBody(
          DebugDataType::kTriggerEventNoMatchingTriggerData);
  }
}

std::optional<DebugDataTypeAndBody> GetReportDataTypeAndLimit(
    AggregatableResult result,
    const CreateReportResult::Limits& limits) {
  switch (result) {
    case AggregatableResult::kSuccess:
    case AggregatableResult::kNotRegistered:
    case AggregatableResult::kProhibitedByBrowserPolicy:
      return std::nullopt;
    case AggregatableResult::kInternalError:
      return DebugDataTypeAndBody(DebugDataType::kTriggerUnknownError);
    case AggregatableResult::kNoCapacityForConversionDestination:
      return DebugDataTypeAndBody(
          DebugDataType::kTriggerAggregateStorageLimit,
          GetLimit(limits.max_aggregatable_reports_per_destination.value()));
    case AggregatableResult::kExcessiveReportingOrigins:
      return DebugDataTypeAndBody(
          DebugDataType::kTriggerReportingOriginLimit,
          GetLimit(
              limits.rate_limits_max_attribution_reporting_origins.value()));
    case AggregatableResult::kNoMatchingImpressions:
      return DebugDataTypeAndBody(DebugDataType::kTriggerNoMatchingSource);
    case AggregatableResult::kExcessiveAttributions:
      return DebugDataTypeAndBody(
          DebugDataType::kTriggerAggregateAttributionsPerSourceDestinationLimit,
          GetLimit(limits.rate_limits_max_attributions.value()));
    case AggregatableResult::kNoMatchingSourceFilterData:
      return DebugDataTypeAndBody(DebugDataType::kTriggerNoMatchingFilterData);
    case AggregatableResult::kDeduplicated:
      return DebugDataTypeAndBody(DebugDataType::kTriggerAggregateDeduplicated);
    case AggregatableResult::kNoHistograms:
      return DebugDataTypeAndBody(
          DebugDataType::kTriggerAggregateNoContributions);
    case AggregatableResult::kInsufficientBudget:
      return DebugDataTypeAndBody(
          DebugDataType::kTriggerAggregateInsufficientBudget,
          GetLimit(attribution_reporting::kMaxAggregatableValue));
    case AggregatableResult::kReportWindowPassed:
      return DebugDataTypeAndBody(
          DebugDataType::kTriggerAggregateReportWindowPassed);
    case AggregatableResult::kExcessiveReports:
      return DebugDataTypeAndBody(
          DebugDataType::kTriggerAggregateExcessiveReports,
          GetLimit(limits.max_aggregatable_reports_per_source.value()));
  }
}

std::string_view SerializeReportDataType(DebugDataType data_type) {
  switch (data_type) {
    case DebugDataType::kSourceDestinationLimit:
      return "source-destination-limit";
    case DebugDataType::kSourceNoised:
      return "source-noised";
    case DebugDataType::kSourceStorageLimit:
      return "source-storage-limit";
    case DebugDataType::kSourceSuccess:
      return "source-success";
    case DebugDataType::kSourceDestinationRateLimit:
      return "source-destination-rate-limit";
    case DebugDataType::kSourceUnknownError:
      return "source-unknown-error";
    case DebugDataType::kTriggerNoMatchingSource:
      return "trigger-no-matching-source";
    case DebugDataType::kTriggerEventAttributionsPerSourceDestinationLimit:
      return "trigger-event-attributions-per-source-destination-limit";
    case DebugDataType::kTriggerAggregateAttributionsPerSourceDestinationLimit:
      return "trigger-aggregate-attributions-per-source-destination-limit";
    case DebugDataType::kTriggerNoMatchingFilterData:
      return "trigger-no-matching-filter-data";
    case DebugDataType::kTriggerReportingOriginLimit:
      return "trigger-reporting-origin-limit";
    case DebugDataType::kTriggerEventDeduplicated:
      return "trigger-event-deduplicated";
    case DebugDataType::kTriggerEventNoMatchingConfigurations:
      return "trigger-event-no-matching-configurations";
    case DebugDataType::kTriggerEventNoise:
      return "trigger-event-noise";
    case DebugDataType::kTriggerEventLowPriority:
      return "trigger-event-low-priority";
    case DebugDataType::kTriggerEventExcessiveReports:
      return "trigger-event-excessive-reports";
    case DebugDataType::kTriggerEventStorageLimit:
      return "trigger-event-storage-limit";
    case DebugDataType::kTriggerEventReportWindowNotStarted:
      return "trigger-event-report-window-not-started";
    case DebugDataType::kTriggerEventReportWindowPassed:
      return "trigger-event-report-window-passed";
    case DebugDataType::kTriggerEventNoMatchingTriggerData:
      return "trigger-event-no-matching-trigger-data";
    case DebugDataType::kTriggerAggregateDeduplicated:
      return "trigger-aggregate-deduplicated";
    case DebugDataType::kTriggerAggregateNoContributions:
      return "trigger-aggregate-no-contributions";
    case DebugDataType::kTriggerAggregateInsufficientBudget:
      return "trigger-aggregate-insufficient-budget";
    case DebugDataType::kTriggerAggregateStorageLimit:
      return "trigger-aggregate-storage-limit";
    case DebugDataType::kTriggerAggregateReportWindowPassed:
      return "trigger-aggregate-report-window-passed";
    case DebugDataType::kTriggerAggregateExcessiveReports:
      return "trigger-aggregate-excessive-reports";
    case DebugDataType::kTriggerUnknownError:
      return "trigger-unknown-error";
    case DebugDataType::kOsSourceDelegated:
      return "os-source-delegated";
    case DebugDataType::kOsTriggerDelegated:
      return "os-trigger-delegated";
    case DebugDataType::kHeaderParsingError:
      return "header-parsing-error";
    case DebugDataType::kSourceReportingOriginPerSiteLimit:
      return "source-reporting-origin-per-site-limit";
    case DebugDataType::kSourceMaxChannelCapacityReached:
      return "source-channel-capacity-limit";
    case DebugDataType::kSourceMaxTriggerDataCardinalityReached:
      return "source-trigger-state-cardinality-limit";
  }
}

void SetSourceData(base::Value::Dict& data_body,
                   uint64_t source_event_id,
                   const net::SchemefulSite& source_site,
                   std::optional<uint64_t> source_debug_key) {
  data_body.Set("source_event_id", base::NumberToString(source_event_id));
  data_body.Set("source_site", source_site.Serialize());
  if (source_debug_key) {
    data_body.Set("source_debug_key", base::NumberToString(*source_debug_key));
  }
}

void SetLimit(base::Value::Dict& data_body, base::Value limit) {
  data_body.Set("limit", std::move(limit));
}

base::Value::Dict GetReportDataBody(DebugDataTypeAndBody data,
                                    const CreateReportResult& result) {
  if (data.debug_data_type == DebugDataType::kTriggerEventExcessiveReports ||
      data.debug_data_type == DebugDataType::kTriggerEventLowPriority) {
    DCHECK(result.dropped_event_level_report());
    return result.dropped_event_level_report()->ReportBody();
  }

  base::Value::Dict data_body;
  data_body.Set(
      kAttributionDestination,
      net::SchemefulSite(result.trigger().destination_origin()).Serialize());
  if (std::optional<uint64_t> debug_key =
          result.trigger().registration().debug_key) {
    data_body.Set("trigger_debug_key", base::NumberToString(*debug_key));
  }

  if (const std::optional<StoredSource>& source = result.source()) {
    SetSourceData(data_body, source->source_event_id(),
                  source->common_info().source_site(), source->debug_key());
  }

  if (!data.limit.is_none()) {
    SetLimit(data_body, std::move(data.limit));
  }

  return data_body;
}

base::Value::Dict GetReportData(DebugDataType type, base::Value::Dict body) {
  base::Value::Dict dict;
  dict.Set("type", SerializeReportDataType(type));
  dict.Set("body", std::move(body));
  return dict;
}

void RecordVerboseDebugReportType(DebugDataType type) {
  static_assert(DebugDataType::kMaxValue ==
                    DebugDataType::kSourceMaxTriggerDataCardinalityReached,
                "Update ConversionVerboseDebugReportType enum.");
  base::UmaHistogramEnumeration("Conversions.SentVerboseDebugReportType4",
                                type);
}

}  // namespace

GURL AttributionDebugReport::ReportUrl() const {
  static constexpr char kPath[] =
      "/.well-known/attribution-reporting/debug/verbose";

  GURL::Replacements replacements;
  replacements.SetPathStr(kPath);
  return reporting_origin_->GetURL().ReplaceComponents(replacements);
}

// static
std::optional<AttributionDebugReport> AttributionDebugReport::Create(
    base::FunctionRef<bool()> is_operation_allowed,
    const StoreSourceResult& result) {
  const StorableSource& source = result.source();
  if (!source.registration().debug_reporting ||
      !source.common_info().debug_cookie_set() ||
      source.is_within_fenced_frame() || !is_operation_allowed()) {
    return std::nullopt;
  }

  std::optional<DebugDataTypeAndBody> data = GetReportDataBody(result);
  if (!data) {
    return std::nullopt;
  }

  RecordVerboseDebugReportType(data->debug_data_type);

  base::Value::Dict body;
  if (!data->limit.is_none()) {
    SetLimit(body, std::move(data->limit));
  }

  const attribution_reporting::SourceRegistration& registration =
      source.registration();

  body.Set(kAttributionDestination, registration.destination_set.ToJson());
  SetSourceData(body, registration.source_event_id,
                source.common_info().source_site(), registration.debug_key);

  base::Value::List report_body;
  report_body.Append(GetReportData(data->debug_data_type, std::move(body)));
  return AttributionDebugReport(std::move(report_body),
                                source.common_info().reporting_origin());
}

// static
std::optional<AttributionDebugReport> AttributionDebugReport::Create(
    base::FunctionRef<bool()> is_operation_allowed,
    bool is_debug_cookie_set,
    const CreateReportResult& result) {
  if (!result.trigger().registration().debug_reporting ||
      !is_debug_cookie_set || result.trigger().is_within_fenced_frame() ||
      !is_operation_allowed()) {
    return std::nullopt;
  }

  if (result.source() && !result.source()->common_info().debug_cookie_set()) {
    return std::nullopt;
  }

  base::Value::List report_body;

  std::optional<DebugDataType> event_level_type;
  if (std::optional<DebugDataTypeAndBody> event_level_data_type_limit =
          GetReportDataTypeAndLimit(result.event_level_status(),
                                    result.limits())) {
    event_level_type = event_level_data_type_limit->debug_data_type;
    report_body.Append(GetReportData(
        event_level_data_type_limit->debug_data_type,
        GetReportDataBody(std::move(*event_level_data_type_limit), result)));
    RecordVerboseDebugReportType(event_level_data_type_limit->debug_data_type);
  }

  if (std::optional<DebugDataTypeAndBody> aggregatable_data_type_limit =
          GetReportDataTypeAndLimit(result.aggregatable_status(),
                                    result.limits());
      aggregatable_data_type_limit &&
      aggregatable_data_type_limit->debug_data_type != event_level_type) {
    report_body.Append(GetReportData(
        aggregatable_data_type_limit->debug_data_type,
        GetReportDataBody(std::move(*aggregatable_data_type_limit), result)));
    RecordVerboseDebugReportType(aggregatable_data_type_limit->debug_data_type);
  }

  if (report_body.empty()) {
    return std::nullopt;
  }

  return AttributionDebugReport(std::move(report_body),
                                result.trigger().reporting_origin());
}

// static
std::optional<AttributionDebugReport> AttributionDebugReport::Create(
    const OsRegistration& registration,
    size_t item_index,
    base::FunctionRef<bool(const url::Origin&)> is_operation_allowed) {
  CHECK_LT(item_index, registration.registration_items.size());
  const auto& registration_item = registration.registration_items[item_index];
  if (!registration_item.debug_reporting ||
      registration.is_within_fenced_frame) {
    return std::nullopt;
  }

  auto registration_origin =
      attribution_reporting::SuitableOrigin::Create(registration_item.url);
  if (!registration_origin.has_value() ||
      !is_operation_allowed(*registration_origin)) {
    return std::nullopt;
  }

  DebugDataType data_type;
  switch (registration.GetType()) {
    case attribution_reporting::mojom::RegistrationType::kSource:
      data_type = DebugDataType::kOsSourceDelegated;
      break;
    case attribution_reporting::mojom::RegistrationType::kTrigger:
      data_type = DebugDataType::kOsTriggerDelegated;
      break;
  }

  base::Value::Dict data_body;
  data_body.Set("context_site",
                net::SchemefulSite(registration.top_level_origin).Serialize());
  data_body.Set("registration_url", registration_item.url.spec());

  base::Value::List report_body;
  report_body.Append(GetReportData(data_type, std::move(data_body)));

  RecordVerboseDebugReportType(data_type);

  return AttributionDebugReport(std::move(report_body),
                                std::move(*registration_origin));
}

std::optional<AttributionDebugReport> AttributionDebugReport::Create(
    attribution_reporting::SuitableOrigin reporting_origin,
    const attribution_reporting::RegistrationHeaderError& error,
    const attribution_reporting::SuitableOrigin& context_origin,
    bool is_within_fenced_frame,
    base::FunctionRef<bool(const url::Origin&)> is_operation_allowed) {
  if (is_within_fenced_frame || !is_operation_allowed(*reporting_origin)) {
    return std::nullopt;
  }

  base::Value::Dict data_body;
  data_body.Set("context_site", net::SchemefulSite(context_origin).Serialize());
  data_body.Set("header", error.HeaderName());
  data_body.Set("value", error.header_value);

  if (base::FeatureList::IsEnabled(kAttributionHeaderErrorDetails)) {
    if (base::Value error_details = error.ErrorDetails();
        !error_details.is_none()) {
      data_body.Set("error", std::move(error_details));
    }
  }

  const DebugDataType data_type = DebugDataType::kHeaderParsingError;

  base::Value::List report_body;
  report_body.Append(GetReportData(data_type, std::move(data_body)));

  RecordVerboseDebugReportType(data_type);

  return AttributionDebugReport(std::move(report_body),
                                std::move(reporting_origin));
}

AttributionDebugReport::AttributionDebugReport(
    base::Value::List report_body,
    attribution_reporting::SuitableOrigin reporting_origin)
    : report_body_(std::move(report_body)),
      reporting_origin_(std::move(reporting_origin)) {
  DCHECK(!report_body_.empty());
}

AttributionDebugReport::~AttributionDebugReport() = default;

AttributionDebugReport::AttributionDebugReport(AttributionDebugReport&&) =
    default;

AttributionDebugReport& AttributionDebugReport::operator=(
    AttributionDebugReport&&) = default;

}  // namespace content
