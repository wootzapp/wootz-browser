// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_ATTRIBUTION_REPORTING_ATTRIBUTION_DEBUG_REPORT_H_
#define CONTENT_BROWSER_ATTRIBUTION_REPORTING_ATTRIBUTION_DEBUG_REPORT_H_

#include <stddef.h>

#include <optional>

#include "base/values.h"
#include "components/attribution_reporting/suitable_origin.h"
#include "content/common/content_export.h"

class GURL;

namespace attribution_reporting {
struct RegistrationHeaderError;
}  // namespace attribution_reporting

namespace content {

class AttributionTrigger;
class CreateReportResult;
class StorableSource;
class StoreSourceResult;

struct OsRegistration;

// Class that contains all the data needed to serialize and send an attribution
// debug report.
class CONTENT_EXPORT AttributionDebugReport {
 public:
  static std::optional<AttributionDebugReport> Create(
      const StorableSource& source,
      bool is_debug_cookie_set,
      const StoreSourceResult& result);

  static std::optional<AttributionDebugReport> Create(
      const AttributionTrigger& trigger,
      bool is_debug_cookie_set,
      const CreateReportResult& result);

  static std::optional<AttributionDebugReport> Create(const OsRegistration&,
                                                      size_t item_index);

  static std::optional<AttributionDebugReport> Create(
      attribution_reporting::SuitableOrigin reporting_origin,
      const attribution_reporting::RegistrationHeaderError&,
      const attribution_reporting::SuitableOrigin& context_origin,
      bool is_within_fenced_frame);

  ~AttributionDebugReport();

  AttributionDebugReport(const AttributionDebugReport&) = delete;
  AttributionDebugReport& operator=(const AttributionDebugReport&) = delete;

  AttributionDebugReport(AttributionDebugReport&&);
  AttributionDebugReport& operator=(AttributionDebugReport&&);

  const base::Value::List& ReportBody() const { return report_body_; }

  const attribution_reporting::SuitableOrigin& reporting_origin() const {
    return reporting_origin_;
  }

  GURL ReportUrl() const;

 private:
  AttributionDebugReport(
      base::Value::List report_body,
      attribution_reporting::SuitableOrigin reporting_origin);

  base::Value::List report_body_;
  attribution_reporting::SuitableOrigin reporting_origin_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_ATTRIBUTION_REPORTING_ATTRIBUTION_DEBUG_REPORT_H_
