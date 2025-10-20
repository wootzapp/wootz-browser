// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/activity_tracking/splunk_observability_exporter.h"

#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "net/base/load_flags.h"
#include "net/http/http_status_code.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "url/gurl.h"

namespace activity_tracking {

namespace {

// Network traffic annotation for SignalFx exports
const net::NetworkTrafficAnnotationTag kSignalFxTrafficAnnotation =
    net::DefineNetworkTrafficAnnotation("splunk_observability_export", R"(
        semantics {
          sender: "Activity Tracking Service"
          description:
            "Exports browser navigation telemetry data to Splunk Observability "
            "Cloud (SignalFx) for analytics, monitoring, and observability."
          trigger:
            "Triggered when navigation events are collected and ready to export."
          data:
            "Navigation URLs (filtered for privacy), page titles, timestamps, "
            "navigation states, and interaction types."
          destination: OTHER
          destination_other: "Splunk Observability Cloud SignalFx"
        }
        policy {
          cookies_allowed: NO
          setting:
            "This feature can be controlled via enterprise policy or "
            "command-line flags."
          policy_exception_justification:
            "Not implemented yet. This is for development and monitoring."
        })");

}  // namespace

SplunkObservabilityExporter::Config::Config() = default;
SplunkObservabilityExporter::Config::~Config() = default;
SplunkObservabilityExporter::Config::Config(const Config&) = default;
SplunkObservabilityExporter::Config& SplunkObservabilityExporter::Config::operator=(
    const Config&) = default;

SplunkObservabilityExporter::SplunkObservabilityExporter(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    const Config& config)
    : url_loader_factory_(std::move(url_loader_factory)),
      config_(config) {
}

SplunkObservabilityExporter::~SplunkObservabilityExporter() = default;

void SplunkObservabilityExporter::ExportEvents(
    const base::Value::Dict& otel_json,
    ExportCallback callback) {
  
  if (!config_.enable_events) {
    std::move(callback).Run(ExportResult::SUCCESS);
    return;
  }

  base::Value::List signalfx_events = ConvertOTelToSignalFxEvents(otel_json);
  
  if (signalfx_events.empty()) {
    std::move(callback).Run(ExportResult::SUCCESS);
    return;
  }

  // Build event endpoint
  std::string event_endpoint = config_.ingest_endpoint;
  if (event_endpoint.find("/v2/") == std::string::npos) {
    // Append /v2/event if not present
    if (event_endpoint.back() != '/') {
      event_endpoint += '/';
    }
    event_endpoint += "v2/event";
  }

  SendToSignalFx(event_endpoint, signalfx_events, std::move(callback));
}

void SplunkObservabilityExporter::ExportMetrics(
    const base::Value::Dict& otel_json,
    ExportCallback callback) {
  
  if (!config_.enable_metrics) {
    std::move(callback).Run(ExportResult::SUCCESS);
    return;
  }

  base::Value::List signalfx_datapoints = 
      ConvertOTelToSignalFxDatapoints(otel_json);
  
  if (signalfx_datapoints.empty()) {
    std::move(callback).Run(ExportResult::SUCCESS);
    return;
  }

  // Build datapoint endpoint
  std::string datapoint_endpoint = config_.ingest_endpoint;
  if (datapoint_endpoint.find("/v2/") == std::string::npos) {
    // Append /v2/datapoint if not present
    if (datapoint_endpoint.back() != '/') {
      datapoint_endpoint += '/';
    }
    datapoint_endpoint += "v2/datapoint";
  }

  SendToSignalFx(datapoint_endpoint, signalfx_datapoints, std::move(callback));
}

base::Value::List SplunkObservabilityExporter::ConvertOTelToSignalFxEvents(
    const base::Value::Dict& otel_json) {
  
  base::Value::List signalfx_events;

  // Extract log records from OpenTelemetry format
  const base::Value::List* resource_logs = otel_json.FindList("resourceLogs");
  if (!resource_logs) {
    return signalfx_events;
  }

  for (const auto& resource_log : *resource_logs) {
    const base::Value::Dict* resource_log_dict = resource_log.GetIfDict();
    if (!resource_log_dict) continue;

    const base::Value::List* scope_logs = 
        resource_log_dict->FindList("scopeLogs");
    if (!scope_logs) continue;

    for (const auto& scope_log : *scope_logs) {
      const base::Value::Dict* scope_log_dict = scope_log.GetIfDict();
      if (!scope_log_dict) continue;

      const base::Value::List* log_records = 
          scope_log_dict->FindList("logRecords");
      if (!log_records) continue;

      for (const auto& log_record : *log_records) {
        const base::Value::Dict* log_record_dict = log_record.GetIfDict();
        if (!log_record_dict) continue;

        // Convert to SignalFx Event format
        base::Value::Dict signalfx_event;

        // Extract attributes
        const base::Value::Dict* attributes = 
            log_record_dict->FindDict("attributes");
        if (!attributes) continue;

        // Required fields for SignalFx events
        const std::string* url = attributes->FindString("url");
        const std::string* state = attributes->FindString("state");
        const std::string* timestamp = attributes->FindString("timestamp");
        const std::string* interaction_type = 
            attributes->FindString("interaction_type");

        if (!url || !state) continue;

        // Event category (required)
        signalfx_event.Set("category", "USER_DEFINED");

        // Event type (required) - use state as event type
        signalfx_event.Set("eventType", *state);

        // Timestamp (milliseconds since epoch)
        // TODO: Parse timestamp string; for now use current time
        int64_t ts = base::Time::Now().InMillisecondsSinceUnixEpoch();
        signalfx_event.Set("timestamp", static_cast<double>(ts));

        // Properties (dimensions and custom properties)
        base::Value::Dict properties;
        properties.Set("url", *url);
        properties.Set("state", *state);
        
        if (interaction_type) {
          properties.Set("interaction_type", *interaction_type);
        }

        const std::string* title = attributes->FindString("title");
        if (title) {
          properties.Set("title", *title);
        }

        std::optional<double> duration_ms = attributes->FindDouble("duration_ms");
        if (duration_ms) {
          properties.Set("duration_ms", *duration_ms);
        }

        // Add common dimensions
        properties.Set("service", "chromium-browser");
        properties.Set("component", "navigation-tracker");
        properties.Set("platform", "android");

        signalfx_event.Set("properties", std::move(properties));

        signalfx_events.Append(std::move(signalfx_event));
      }
    }
  }

  return signalfx_events;
}

base::Value::List SplunkObservabilityExporter::ConvertOTelToSignalFxDatapoints(
    const base::Value::Dict& otel_json) {
  
  base::Value::List signalfx_datapoints;

  // Extract log records from OpenTelemetry format
  const base::Value::List* resource_logs = otel_json.FindList("resourceLogs");
  if (!resource_logs) {
    return signalfx_datapoints;
  }

  for (const auto& resource_log : *resource_logs) {
    const base::Value::Dict* resource_log_dict = resource_log.GetIfDict();
    if (!resource_log_dict) continue;

    const base::Value::List* scope_logs = 
        resource_log_dict->FindList("scopeLogs");
    if (!scope_logs) continue;

    for (const auto& scope_log : *scope_logs) {
      const base::Value::Dict* scope_log_dict = scope_log.GetIfDict();
      if (!scope_log_dict) continue;

      const base::Value::List* log_records = 
          scope_log_dict->FindList("logRecords");
      if (!log_records) continue;

      for (const auto& log_record : *log_records) {
        const base::Value::Dict* log_record_dict = log_record.GetIfDict();
        if (!log_record_dict) continue;

        const base::Value::Dict* attributes = 
            log_record_dict->FindDict("attributes");
        if (!attributes) continue;

        // Create a gauge metric for navigation duration
        std::optional<double> duration_ms = attributes->FindDouble("duration_ms");
        if (!duration_ms || *duration_ms <= 0) continue;

        base::Value::Dict datapoint;

        // Metric name (required)
        datapoint.Set("metric", "navigation.duration");

        // Value (required)
        base::Value::Dict value;
        value.Set("doubleValue", *duration_ms);
        datapoint.Set("value", std::move(value));

        // Metric type (required): GAUGE, COUNTER, or CUMULATIVE_COUNTER
        datapoint.Set("metricType", "GAUGE");

        // Timestamp (milliseconds since epoch)
        // TODO: Parse timestamp string; for now use current time
        int64_t ts = base::Time::Now().InMillisecondsSinceUnixEpoch();
        datapoint.Set("timestamp", static_cast<double>(ts));

        // Dimensions (tags)
        base::Value::List dimensions;

        const std::string* state = attributes->FindString("state");
        if (state) {
          base::Value::Dict dim;
          dim.Set("key", "state");
          dim.Set("value", *state);
          dimensions.Append(std::move(dim));
        }

        const std::string* interaction_type = 
            attributes->FindString("interaction_type");
        if (interaction_type) {
          base::Value::Dict dim;
          dim.Set("key", "interaction_type");
          dim.Set("value", *interaction_type);
          dimensions.Append(std::move(dim));
        }

        // Add common dimensions
        base::Value::Dict service_dim;
        service_dim.Set("key", "service");
        service_dim.Set("value", "chromium-browser");
        dimensions.Append(std::move(service_dim));

        base::Value::Dict platform_dim;
        platform_dim.Set("key", "platform");
        platform_dim.Set("value", "android");
        dimensions.Append(std::move(platform_dim));

        datapoint.Set("dimensions", std::move(dimensions));

        signalfx_datapoints.Append(std::move(datapoint));
      }
    }
  }

  return signalfx_datapoints;
}

void SplunkObservabilityExporter::SendToSignalFx(
    const std::string& endpoint,
    const base::Value::List& payload,
    ExportCallback callback) {
  
  // Convert payload to JSON string
  std::string json_payload;
  if (!base::JSONWriter::Write(payload, &json_payload)) {
    LOG(ERROR) << "[SplunkExporter] Failed to serialize JSON payload";
    std::move(callback).Run(ExportResult::FAILURE);
    return;
  }

  // Create resource request
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(endpoint);
  resource_request->method = "POST";
  resource_request->load_flags = 
      net::LOAD_BYPASS_CACHE | net::LOAD_DISABLE_CACHE;
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;

  // Set SignalFx headers (X-SF-TOKEN for ingest API)
  resource_request->headers.SetHeader("Content-Type", "application/json");
  resource_request->headers.SetHeader("X-SF-TOKEN", config_.access_token);
  resource_request->headers.SetHeader("User-Agent", "Chromium-Activity-Tracker/1.0");

  // Create URL loader
  auto url_loader = network::SimpleURLLoader::Create(
      std::move(resource_request), kSignalFxTrafficAnnotation);

  // Attach payload
  url_loader->AttachStringForUpload(json_payload, "application/json");

  // Set timeout
  url_loader->SetTimeoutDuration(base::Milliseconds(config_.timeout_ms));

  // Allow retry on network change
  url_loader->SetRetryOptions(
      1, network::SimpleURLLoader::RETRY_ON_NETWORK_CHANGE);

  // Keep loader alive
  auto* url_loader_ptr = url_loader.get();
  
  // Start request
  url_loader_ptr->DownloadToString(
      url_loader_factory_.get(),
      base::BindOnce(&SplunkObservabilityExporter::OnExportComplete,
                     base::Unretained(this),
                     std::move(url_loader),
                     std::move(callback)),
      1024 * 1024);  // 1MB max response
}

void SplunkObservabilityExporter::OnExportComplete(
    std::unique_ptr<network::SimpleURLLoader> loader,
    ExportCallback callback,
    std::unique_ptr<std::string> response_body) {
  
  int response_code = -1;
  if (loader->ResponseInfo() && loader->ResponseInfo()->headers) {
    response_code = loader->ResponseInfo()->headers->response_code();
  }

  int net_error = loader->NetError();

  ExportResult result;

  if (net_error == net::OK && response_code >= 200 && response_code < 300) {
    result = ExportResult::SUCCESS;
  } else if (net_error == net::ERR_TIMED_OUT) {
    LOG(ERROR) << "[SplunkExporter] Export timeout";
    result = ExportResult::TIMEOUT;
  } else {
    LOG(ERROR) << "[SplunkExporter] Export failed - HTTP " << response_code 
               << ", Net error " << net_error;
    result = ExportResult::FAILURE;
  }

  std::move(callback).Run(result);
}

}  // namespace activity_tracking

