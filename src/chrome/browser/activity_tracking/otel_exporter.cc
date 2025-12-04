// Copyright 2024 The wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/activity_tracking/otel_exporter.h"

#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
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

// Network traffic annotation for OTLP exports
const net::NetworkTrafficAnnotationTag kOTLPTrafficAnnotation =
    net::DefineNetworkTrafficAnnotation("otel_activity_export", R"(
        semantics {
          sender: "Activity Tracking Service"
          description:
            "Exports browser activity telemetry data using OpenTelemetry Protocol "
            "(OTLP) to configurable observability backends for analytics, "
            "monitoring, and observability."
          trigger:
            "Triggered when activity events are collected and ready to export."
          data:
            "Navigation URLs (filtered for privacy), page titles, timestamps, "
            "navigation states, interaction types, and performance metrics."
          destination: OTHER
          destination_other: "Configurable OTLP-compatible observability backend"
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

OTelExporter::Config::Config() = default;
OTelExporter::Config::~Config() = default;
OTelExporter::Config::Config(const Config&) = default;
OTelExporter::Config& OTelExporter::Config::operator=(const Config&) = default;

OTelExporter::OTelExporter(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    const Config& config)
    : url_loader_factory_(std::move(url_loader_factory)), config_(config) {}

OTelExporter::~OTelExporter() = default;

void OTelExporter::ExportLogs(const base::Value::List& events,
                               ExportCallback callback) {
  if (events.empty()) {
    std::move(callback).Run(ExportResult::SUCCESS);
    return;
  }

  base::Value::Dict otlp_payload = ConvertToOTLPLogs(events);
  SendToOTLP(config_.otlp_endpoint, otlp_payload, std::move(callback));
}

void OTelExporter::ExportTraces(const base::Value::List& events,
                                 ExportCallback callback) {
  if (events.empty()) {
    std::move(callback).Run(ExportResult::SUCCESS);
    return;
  }

  base::Value::Dict otlp_payload = ConvertToOTLPTraces(events);
  SendToOTLP(config_.otlp_endpoint, otlp_payload, std::move(callback));
}

void OTelExporter::UpdateConfig(const Config& config) {
  config_ = config;
}

void OTelExporter::ExportAsSplunkEvents(const base::Value::List& events,
                                         ExportCallback callback) {
  if (events.empty()) {
    std::move(callback).Run(ExportResult::SUCCESS);
    return;
  }

  // SignalFx event format: send events with dimensions
  // Format: [ { "eventType": "...", "category": "...", "dimensions": {...}, "timestamp": ... } ]
  
  base::Value::List signalfx_events;
  
  int64_t current_time_ms = 
      (base::Time::Now() - base::Time::UnixEpoch()).InMilliseconds();
  
  for (const auto& event : events) {
    const base::Value::Dict* event_dict = event.GetIfDict();
    if (!event_dict) {
      continue;
    }
    
    // Create SignalFx event structure
    base::Value::Dict signalfx_event;
    
    // Event type and category
    signalfx_event.Set("eventType", "activity_tracking");
    signalfx_event.Set("category", "INFO");
    
    // Timestamp in milliseconds - must be a JSON number (integer), not string
    // Store as double (base::Value only supports int or double)
    // Will be serialized as integer using OPTIONS_OMIT_DOUBLE_TYPE_PRESERVATION
    signalfx_event.Set("timestamp", static_cast<double>(current_time_ms));
    
    // Dimensions - all event data goes here as strings
    base::Value::Dict dimensions;
    
    // Service metadata
    dimensions.Set("service.name", config_.service_name);
    dimensions.Set("service.version", config_.service_version);
    dimensions.Set("platform", "android");
    
    // User identification (email from SAML)
    if (!config_.user_email.empty()) {
      dimensions.Set("user.email", config_.user_email);
    }
    
    // Navigation data
    if (const std::string* url = event_dict->FindString("url")) {
      dimensions.Set("url", *url);
    }
    if (const std::string* title = event_dict->FindString("title")) {
      dimensions.Set("title", *title);
    }
    if (const std::string* state = event_dict->FindString("state")) {
      dimensions.Set("state", *state);
    }
    if (const std::string* interaction = event_dict->FindString("interaction_type")) {
      dimensions.Set("interaction_type", *interaction);
    }
    if (std::optional<double> duration = event_dict->FindDouble("duration_ms")) {
      // Convert to string as dimensions are typically strings
      dimensions.Set("duration_ms", base::NumberToString(*duration));
    }
    
    // Security policies - convert booleans to strings
    if (const base::Value::Dict* policies = event_dict->FindDict("security_policies")) {
      if (std::optional<bool> val = policies->FindBool("content_privacy_enabled")) {
        dimensions.Set("content_privacy_enabled", *val ? "true" : "false");
      }
      if (std::optional<bool> val = policies->FindBool("copy_paste_blocked")) {
        dimensions.Set("copy_paste_blocked", *val ? "true" : "false");
      }
      if (std::optional<bool> val = policies->FindBool("upload_blocked")) {
        dimensions.Set("upload_blocked", *val ? "true" : "false");
      }
      if (std::optional<bool> val = policies->FindBool("download_blocked")) {
        dimensions.Set("download_blocked", *val ? "true" : "false");
      }
      if (std::optional<bool> val = policies->FindBool("domain_blocked")) {
        dimensions.Set("domain_blocked", *val ? "true" : "false");
      }
    }
    
    signalfx_event.Set("dimensions", std::move(dimensions));
    signalfx_events.Append(std::move(signalfx_event));
  }
  
  // Convert to JSON array format
  // Use OPTIONS_OMIT_DOUBLE_TYPE_PRESERVATION to write large doubles (timestamps)
  // as integers without scientific notation
  std::string json_payload;
  if (!base::JSONWriter::WriteWithOptions(
          signalfx_events, 
          base::JSONWriter::OPTIONS_OMIT_DOUBLE_TYPE_PRESERVATION,
          &json_payload)) {
    LOG(ERROR) << "[OTelExporter] Failed to serialize SignalFx events";
    std::move(callback).Run(ExportResult::FAILURE);
    return;
  }

  LOG(INFO) << "[OTelExporter] ========================================";
  LOG(INFO) << "[OTelExporter] Exporting " << signalfx_events.size() 
            << " events in SignalFx format to: " << config_.otlp_endpoint;
  LOG(INFO) << "[OTelExporter] Payload size: " << json_payload.size() << " bytes";
  LOG(INFO) << "[OTelExporter] ========================================";
  LOG(INFO) << "[OTelExporter] FULL JSON PAYLOAD (pretty-printed below):";
  LOG(INFO) << json_payload;
  LOG(INFO) << "[OTelExporter] ========================================";
  
  // Log structure validation
  LOG(INFO) << "[OTelExporter] JSON Structure Check:";
  LOG(INFO) << "[OTelExporter]   - Array Size: " << signalfx_events.size();
  if (!signalfx_events.empty()) {
    const base::Value& first_event = signalfx_events[0];
    LOG(INFO) << "[OTelExporter]   - First Event is Dict: " << first_event.is_dict();
    if (first_event.is_dict()) {
      const base::Value::Dict& event_dict = first_event.GetDict();
      LOG(INFO) << "[OTelExporter]   - Has 'eventType': " << (event_dict.Find("eventType") != nullptr);
      LOG(INFO) << "[OTelExporter]   - Has 'category': " << (event_dict.Find("category") != nullptr);
      LOG(INFO) << "[OTelExporter]   - Has 'timestamp': " << (event_dict.Find("timestamp") != nullptr);
      LOG(INFO) << "[OTelExporter]   - Has 'dimensions': " << (event_dict.Find("dimensions") != nullptr);
      
      const base::Value* timestamp = event_dict.Find("timestamp");
      if (timestamp) {
        LOG(INFO) << "[OTelExporter]   - Timestamp Type: " 
                  << (timestamp->is_string() ? "STRING" : 
                      timestamp->is_int() ? "INT" : 
                      timestamp->is_double() ? "DOUBLE" : "OTHER");
        if (timestamp->is_string()) {
          LOG(INFO) << "[OTelExporter]   - Timestamp Value: " << timestamp->GetString();
        }
      }
      
      const base::Value::Dict* dimensions = event_dict.FindDict("dimensions");
      if (dimensions) {
        LOG(INFO) << "[OTelExporter]   - Dimensions Count: " << dimensions->size();
      }
    }
  }
  LOG(INFO) << "[OTelExporter] ========================================";

  // Create resource request
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(config_.otlp_endpoint);
  resource_request->method = "POST";
  resource_request->load_flags =
      net::LOAD_BYPASS_CACHE | net::LOAD_DISABLE_CACHE;
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;

  // Set headers
  resource_request->headers.SetHeader("Content-Type", "application/json");
  resource_request->headers.SetHeader("User-Agent",
                                      "wootzapp-Activity-Tracker-SignalFx/1.0");

  // Add custom headers (authentication)
  for (const auto& [key, value] : config_.headers) {
    resource_request->headers.SetHeader(key, value);
  }

  // Create URL loader
  auto url_loader = network::SimpleURLLoader::Create(
      std::move(resource_request), kOTLPTrafficAnnotation);

  // Attach payload
  url_loader->AttachStringForUpload(json_payload, "application/json");

  // Set timeout
  url_loader->SetTimeoutDuration(base::Milliseconds(config_.timeout_ms));

  // Keep loader alive
  auto* url_loader_ptr = url_loader.get();

  // Start request
  url_loader_ptr->DownloadToString(
      url_loader_factory_.get(),
      base::BindOnce(&OTelExporter::OnExportComplete, base::Unretained(this),
                     std::move(url_loader), std::move(callback)),
      1024 * 1024);  // 1MB max response
}

base::Value::Dict OTelExporter::ConvertToOTLPLogs(
    const base::Value::List& events) {
  // OTLP Log format according to:
  // https://opentelemetry.io/docs/specs/otlp/#otlpgrpc
  // https://github.com/open-telemetry/opentelemetry-proto/blob/main/opentelemetry/proto/logs/v1/logs.proto

  base::Value::Dict root;

  // Resource attributes (describes the source of the telemetry)
  base::Value::List resource_logs;
  base::Value::Dict resource_log;

  base::Value::Dict resource;
  base::Value::List resource_attributes;

  // Service name
  base::Value::Dict service_name_attr;
  service_name_attr.Set("key", "service.name");
  base::Value::Dict service_name_value;
  service_name_value.Set("stringValue", config_.service_name);
  service_name_attr.Set("value", std::move(service_name_value));
  resource_attributes.Append(std::move(service_name_attr));

  // Service version
  base::Value::Dict service_version_attr;
  service_version_attr.Set("key", "service.version");
  base::Value::Dict service_version_value;
  service_version_value.Set("stringValue", config_.service_version);
  service_version_attr.Set("value", std::move(service_version_value));
  resource_attributes.Append(std::move(service_version_attr));

  // Platform
  base::Value::Dict platform_attr;
  platform_attr.Set("key", "platform");
  base::Value::Dict platform_value;
  platform_value.Set("stringValue", "android");
  platform_attr.Set("value", std::move(platform_value));
  resource_attributes.Append(std::move(platform_attr));

  resource.Set("attributes", std::move(resource_attributes));
  resource_log.Set("resource", std::move(resource));

  // Scope logs
  base::Value::List scope_logs;
  base::Value::Dict scope_log;

  base::Value::Dict scope;
  scope.Set("name", "activity_tracking");
  scope.Set("version", "1.0.0");
  scope_log.Set("scope", std::move(scope));

  // Log records
  base::Value::List log_records;
  int64_t current_time_ns =
      (base::Time::Now() - base::Time::UnixEpoch()).InMicroseconds() * 1000;

  for (const auto& event : events) {
    const base::Value::Dict* event_dict = event.GetIfDict();
    if (!event_dict) {
      continue;
    }

    base::Value::Dict log_record;

    // Time (in nanoseconds since Unix epoch)
    const std::string* timestamp_str = event_dict->FindString("timestamp");
    if (timestamp_str && !timestamp_str->empty()) {
      // TODO: Parse ISO 8601 timestamp properly
      // For now, use current time
      log_record.Set("timeUnixNano",
                     base::NumberToString(current_time_ns));
    } else {
      log_record.Set("timeUnixNano",
                     base::NumberToString(current_time_ns));
    }

    // Observed time (same as event time for now)
    log_record.Set("observedTimeUnixNano",
                   base::NumberToString(current_time_ns));

    // Severity (informational by default)
    log_record.Set("severityNumber", 9);  // INFO
    log_record.Set("severityText", "INFO");

    // Body (main event data)
    const std::string* event_type = event_dict->FindString("event_type");
    if (event_type) {
      base::Value::Dict body;
      body.Set("stringValue", *event_type);
      log_record.Set("body", std::move(body));
    }

    // Attributes (all event data as key-value pairs)
    base::Value::List attributes;

    // Add event type as attribute
    if (event_type) {
      base::Value::Dict attr;
      attr.Set("key", "event.type");
      base::Value::Dict attr_value;
      attr_value.Set("stringValue", *event_type);
      attr.Set("value", std::move(attr_value));
      attributes.Append(std::move(attr));
    }

    // Add all other fields from the event
    const base::Value::Dict* data = event_dict->FindDict("data");
    if (data) {
      for (const auto [key, value] : *data) {
        base::Value::Dict attr;
        attr.Set("key", key);

        base::Value::Dict attr_value;
        if (value.is_string()) {
          attr_value.Set("stringValue", value.GetString());
        } else if (value.is_int()) {
          attr_value.Set("intValue",
                         base::NumberToString(value.GetInt()));
        } else if (value.is_double()) {
          attr_value.Set("doubleValue", value.GetDouble());
        } else if (value.is_bool()) {
          attr_value.Set("boolValue", value.GetBool());
        }

        attr.Set("value", std::move(attr_value));
        attributes.Append(std::move(attr));
      }
    }

    log_record.Set("attributes", std::move(attributes));

    log_records.Append(std::move(log_record));
  }

  scope_log.Set("logRecords", std::move(log_records));
  scope_logs.Append(std::move(scope_log));
  resource_log.Set("scopeLogs", std::move(scope_logs));
  resource_logs.Append(std::move(resource_log));
  root.Set("resourceLogs", std::move(resource_logs));

  return root;
}

base::Value::Dict OTelExporter::ConvertToOTLPTraces(
    const base::Value::List& events) {
  // OTLP Trace format according to:
  // https://github.com/open-telemetry/opentelemetry-proto/blob/main/opentelemetry/proto/trace/v1/trace.proto

  base::Value::Dict root;

  // Resource attributes
  base::Value::List resource_spans;
  base::Value::Dict resource_span;

  base::Value::Dict resource;
  base::Value::List resource_attributes;

  // Service name
  base::Value::Dict service_name_attr;
  service_name_attr.Set("key", "service.name");
  base::Value::Dict service_name_value;
  service_name_value.Set("stringValue", config_.service_name);
  service_name_attr.Set("value", std::move(service_name_value));
  resource_attributes.Append(std::move(service_name_attr));

  resource.Set("attributes", std::move(resource_attributes));
  resource_span.Set("resource", std::move(resource));

  // Scope spans
  base::Value::List scope_spans;
  base::Value::Dict scope_span;

  base::Value::Dict scope;
  scope.Set("name", "activity_tracking");
  scope.Set("version", "1.0.0");
  scope_span.Set("scope", std::move(scope));

  // Spans (one per event)
  base::Value::List spans;
  int64_t current_time_ns =
      (base::Time::Now() - base::Time::UnixEpoch()).InMicroseconds() * 1000;

  for (const auto& event : events) {
    const base::Value::Dict* event_dict = event.GetIfDict();
    if (!event_dict) {
      continue;
    }

    base::Value::Dict span;

    // Generate trace and span IDs (simplified for now)
    span.Set("traceId", "00000000000000000000000000000001");
    span.Set("spanId", "0000000000000001");

    // Span name
    const std::string* event_type = event_dict->FindString("event_type");
    if (event_type) {
      span.Set("name", *event_type);
    } else {
      span.Set("name", "activity_event");
    }

    // Span kind (INTERNAL = 1)
    span.Set("kind", 1);

    // Timestamps
    span.Set("startTimeUnixNano", base::NumberToString(current_time_ns));
    span.Set("endTimeUnixNano",
             base::NumberToString(current_time_ns + 1000000));  // +1ms

    // Attributes
    base::Value::List attributes;
    const base::Value::Dict* data = event_dict->FindDict("data");
    if (data) {
      for (const auto [key, value] : *data) {
        base::Value::Dict attr;
        attr.Set("key", key);

        base::Value::Dict attr_value;
        if (value.is_string()) {
          attr_value.Set("stringValue", value.GetString());
        } else if (value.is_int()) {
          attr_value.Set("intValue",
                         base::NumberToString(value.GetInt()));
        } else if (value.is_double()) {
          attr_value.Set("doubleValue", value.GetDouble());
        } else if (value.is_bool()) {
          attr_value.Set("boolValue", value.GetBool());
        }

        attr.Set("value", std::move(attr_value));
        attributes.Append(std::move(attr));
      }
    }

    span.Set("attributes", std::move(attributes));
    spans.Append(std::move(span));
  }

  scope_span.Set("spans", std::move(spans));
  scope_spans.Append(std::move(scope_span));
  resource_span.Set("scopeSpans", std::move(scope_spans));
  resource_spans.Append(std::move(resource_span));
  root.Set("resourceSpans", std::move(resource_spans));

  return root;
}

void OTelExporter::SendToOTLP(const std::string& endpoint,
                               const base::Value::Dict& payload,
                               ExportCallback callback) {
  // Convert payload to JSON string
  std::string json_payload;
  if (!base::JSONWriter::Write(payload, &json_payload)) {
    LOG(ERROR) << "[OTelExporter] Failed to serialize OTLP payload";
    std::move(callback).Run(ExportResult::FAILURE);
    return;
  }

  LOG(INFO) << "[OTelExporter] Exporting to: " << endpoint;
  LOG(INFO) << "[OTelExporter] Payload size: " << json_payload.size()
            << " bytes";

  // Create resource request
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(endpoint);
  resource_request->method = "POST";
  resource_request->load_flags =
      net::LOAD_BYPASS_CACHE | net::LOAD_DISABLE_CACHE;
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;

  // Set OTLP headers
  resource_request->headers.SetHeader("Content-Type", "application/json");
  resource_request->headers.SetHeader("User-Agent",
                                      "wootzapp-Activity-Tracker-OTLP/1.0");

  // Add custom headers (e.g., authentication tokens)
  LOG(INFO) << "[OTelExporter] Adding " << config_.headers.size() << " custom headers";
  for (const auto& [key, value] : config_.headers) {
    // Mask sensitive values in logs
    std::string masked_value = value;
    if (key.find("Token") != std::string::npos || 
        key.find("TOKEN") != std::string::npos ||
        key.find("Key") != std::string::npos || 
        key.find("KEY") != std::string::npos) {
      masked_value = value.substr(0, 4) + "..." + value.substr(value.length() - 4);
    }
    LOG(INFO) << "[OTelExporter] Header: " << key << " = " << masked_value;
    resource_request->headers.SetHeader(key, value);
  }

  // Create URL loader
  auto url_loader = network::SimpleURLLoader::Create(
      std::move(resource_request), kOTLPTrafficAnnotation);

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
      base::BindOnce(&OTelExporter::OnExportComplete, base::Unretained(this),
                     std::move(url_loader), std::move(callback)),
      1024 * 1024);  // 1MB max response
}

void OTelExporter::OnExportComplete(
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
    LOG(INFO) << "[OTelExporter] Export successful - HTTP " << response_code;
    result = ExportResult::SUCCESS;
  } else if (net_error == net::ERR_TIMED_OUT) {
    LOG(ERROR) << "[OTelExporter] Export timeout";
    result = ExportResult::TIMEOUT;
  } else {
    LOG(ERROR) << "[OTelExporter] ========================================";
    LOG(ERROR) << "[OTelExporter] Export FAILED";
    LOG(ERROR) << "[OTelExporter] HTTP Response Code: " << response_code;
    LOG(ERROR) << "[OTelExporter] Net Error Code: " << net_error;
    
    if (loader->ResponseInfo() && loader->ResponseInfo()->headers) {
      LOG(ERROR) << "[OTelExporter] Response Headers:";
      std::string headers_str = loader->ResponseInfo()->headers->GetStatusLine();
      LOG(ERROR) << "[OTelExporter]   Status Line: " << headers_str;
      
      size_t iter = 0;
      std::string name, value;
      while (loader->ResponseInfo()->headers->EnumerateHeaderLines(&iter, &name, &value)) {
        LOG(ERROR) << "[OTelExporter]   " << name << ": " << value;
      }
    }
    
    if (response_body && !response_body->empty()) {
      LOG(ERROR) << "[OTelExporter] ========================================";
      LOG(ERROR) << "[OTelExporter] Response Body:";
      LOG(ERROR) << *response_body;
      LOG(ERROR) << "[OTelExporter] ========================================";
    } else {
      LOG(ERROR) << "[OTelExporter] (No response body)";
    }
    LOG(ERROR) << "[OTelExporter] ========================================";
    result = ExportResult::FAILURE;
  }

  std::move(callback).Run(result);
}

}  // namespace activity_tracking

