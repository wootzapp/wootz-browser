// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/metrics_internals/metrics_internals_handler.h"

#include "base/functional/bind.h"
#include "base/values.h"
#include "chrome/browser/browser_process.h"
#include "components/metrics/debug/metrics_internals_utils.h"
#include "components/metrics/metrics_service.h"
#include "components/metrics/metrics_service_observer.h"
#include "components/metrics/structured/buildflags/buildflags.h"

#if BUILDFLAG(STRUCTURED_METRICS_DEBUG_ENABLED)
#include "components/metrics/debug/structured/structured_metrics_utils.h"
#endif  // BUILDFLAG(STRUCTURED_METRICS_DEBUG_ENABLED)

MetricsInternalsHandler::MetricsInternalsHandler() {
  if (!ShouldUseMetricsServiceObserver()) {
    uma_log_observer_ = std::make_unique<metrics::MetricsServiceObserver>(
        metrics::MetricsServiceObserver::MetricsServiceType::UMA);
    g_browser_process->metrics_service()->AddLogsObserver(
        uma_log_observer_.get());
  }

#if BUILDFLAG(STRUCTURED_METRICS_DEBUG_ENABLED)
  metrics::structured::StructuredMetricsService* service =
      g_browser_process->GetMetricsServicesManager()
          ->GetStructuredMetricsService();
  if (service) {
    structured_metrics_debug_provider_ =
        std::make_unique<metrics::structured::StructuredMetricsDebugProvider>(
            service);
  }
#endif  // BUILDFLAG(STRUCTURED_METRICS_DEBUG_ENABLED)
}

MetricsInternalsHandler::~MetricsInternalsHandler() {
  if (uma_log_observer_) {
    g_browser_process->metrics_service()->RemoveLogsObserver(
        uma_log_observer_.get());
  }
}

void MetricsInternalsHandler::OnJavascriptAllowed() {
  uma_log_notified_subscription_ = GetUmaObserver()->AddNotifiedCallback(
      base::BindRepeating(&MetricsInternalsHandler::OnUmaLogCreatedOrEvent,
                          weak_ptr_factory_.GetWeakPtr()));
}

void MetricsInternalsHandler::OnJavascriptDisallowed() {
  weak_ptr_factory_.InvalidateWeakPtrs();
  uma_log_notified_subscription_ = {};
}

void MetricsInternalsHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "fetchVariationsSummary",
      base::BindRepeating(
          &MetricsInternalsHandler::HandleFetchVariationsSummary,
          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "fetchUmaSummary",
      base::BindRepeating(&MetricsInternalsHandler::HandleFetchUmaSummary,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "fetchUmaLogsData",
      base::BindRepeating(&MetricsInternalsHandler::HandleFetchUmaLogsData,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "isUsingMetricsServiceObserver",
      base::BindRepeating(
          &MetricsInternalsHandler::HandleIsUsingMetricsServiceObserver,
          base::Unretained(this)));

#if BUILDFLAG(STRUCTURED_METRICS_DEBUG_ENABLED)
  web_ui()->RegisterMessageCallback(
      "fetchStructuredMetricsEvents",
      base::BindRepeating(
          &MetricsInternalsHandler::HandleFetchStructuredMetricsEvents,
          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "fetchStructuredMetricsSummary",
      base::BindRepeating(
          &MetricsInternalsHandler::HandleFetchStructuredMetricsSummary,
          base::Unretained(this)));
#endif  // BUILDFLAG(STRUCTURED_METRICS_DEBUG_ENABLED)
}

bool MetricsInternalsHandler::ShouldUseMetricsServiceObserver() {
  return g_browser_process->metrics_service()->logs_event_observer() != nullptr;
}

metrics::MetricsServiceObserver* MetricsInternalsHandler::GetUmaObserver() {
  return ShouldUseMetricsServiceObserver()
             ? g_browser_process->metrics_service()->logs_event_observer()
             : uma_log_observer_.get();
}

void MetricsInternalsHandler::HandleFetchVariationsSummary(
    const base::Value::List& args) {
  AllowJavascript();
  const base::Value& callback_id = args[0];
  ResolveJavascriptCallback(
      callback_id, metrics::GetVariationsSummary(
                       g_browser_process->GetMetricsServicesManager()));
}

void MetricsInternalsHandler::HandleFetchUmaSummary(
    const base::Value::List& args) {
  AllowJavascript();
  const base::Value& callback_id = args[0];
  ResolveJavascriptCallback(
      callback_id,
      metrics::GetUmaSummary(
          g_browser_process->GetMetricsServicesManager()->GetMetricsService()));
}

void MetricsInternalsHandler::HandleFetchUmaLogsData(
    const base::Value::List& args) {
  AllowJavascript();
  // |args| should have two elements: the callback ID, and a bool parameter that
  // determines whether we should include log proto data.
  DCHECK_EQ(args.size(), 2U);
  const base::Value& callback_id = args[0];
  const bool include_log_proto_data = args[1].GetBool();

  std::string logs_json;
  bool result =
      GetUmaObserver()->ExportLogsAsJson(include_log_proto_data, &logs_json);
  DCHECK(result);
  ResolveJavascriptCallback(callback_id, base::Value(std::move(logs_json)));
}

void MetricsInternalsHandler::HandleIsUsingMetricsServiceObserver(
    const base::Value::List& args) {
  AllowJavascript();
  const base::Value& callback_id = args[0];
  ResolveJavascriptCallback(callback_id,
                            base::Value(ShouldUseMetricsServiceObserver()));
}

void MetricsInternalsHandler::OnUmaLogCreatedOrEvent() {
  FireWebUIListener("uma-log-created-or-event");
}

#if BUILDFLAG(STRUCTURED_METRICS_DEBUG_ENABLED)
void MetricsInternalsHandler::HandleFetchStructuredMetricsEvents(
    const base::Value::List& args) {
  AllowJavascript();
  const base::Value& callback_id = args[0];
  ResolveJavascriptCallback(
      callback_id, structured_metrics_debug_provider_
                       ? structured_metrics_debug_provider_->events().Clone()
                       : base::Value::List());
}

void MetricsInternalsHandler::HandleFetchStructuredMetricsSummary(
    const base::Value::List& args) {
  AllowJavascript();
  const base::Value& callback_id = args[0];
  ResolveJavascriptCallback(callback_id,
                            metrics::structured::GetStructuredMetricsSummary(
                                g_browser_process->GetMetricsServicesManager()
                                    ->GetStructuredMetricsService()));
}

#endif  // BUILDFLAG(STRUCTURED_METRICS_DEBUG_ENABLED)
