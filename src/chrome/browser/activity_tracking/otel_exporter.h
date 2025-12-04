// Copyright 2024 The wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ACTIVITY_TRACKING_OTEL_EXPORTER_H_
#define CHROME_BROWSER_ACTIVITY_TRACKING_OTEL_EXPORTER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/functional/callback.h"
#include "base/memory/scoped_refptr.h"
#include "base/values.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"

namespace activity_tracking {

// OpenTelemetry-based exporter for activity tracking data
// Supports multiple observability backends through OTLP (OpenTelemetry Protocol)
//
// Supported backends:
// - Splunk Observability Cloud
// - Datadog
// - New Relic
// - Grafana Cloud
// - Any OTLP-compatible collector
class OTelExporter {
 public:
  enum class ExportResult {
    SUCCESS,
    FAILURE,
    TIMEOUT,
    NETWORK_ERROR,
  };

  enum class ExportFormat {
    OTLP_HTTP_JSON,    // OTLP over HTTP with JSON encoding
    OTLP_HTTP_PROTOBUF, // OTLP over HTTP with Protobuf encoding (future)
    OTLP_GRPC,         // OTLP over gRPC (future)
  };

  using ExportCallback = base::OnceCallback<void(ExportResult)>;

  // Configuration for OpenTelemetry exporter
  struct Config {
    Config();
    ~Config();
    Config(const Config&);
    Config& operator=(const Config&);

    // Endpoint for OTLP collector (e.g., http://localhost:4318/v1/logs)
    std::string otlp_endpoint;
    
    // Authentication headers (e.g., for Splunk: X-SF-TOKEN, for Datadog: DD-API-KEY)
    std::map<std::string, std::string> headers;
    
    // Service information
    std::string service_name = "wootzapp-enterprise";
    std::string service_version = "1.0.0";
    
    // User identification (email from SAML)
    std::string user_email;
    
    // Export format
    ExportFormat format = ExportFormat::OTLP_HTTP_JSON;
    
    // Timeout in milliseconds
    int timeout_ms = 30000;
    
    // Batch settings
    int max_batch_size = 512;
    int max_queue_size = 2048;
    int scheduled_delay_ms = 5000;
  };

  OTelExporter(
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
      const Config& config);
  ~OTelExporter();

  // Export activity events as OpenTelemetry logs
  void ExportLogs(const base::Value::List& events, ExportCallback callback);

  // Export activity events as OpenTelemetry traces (spans)
  void ExportTraces(const base::Value::List& events, ExportCallback callback);
  
  // Export activity events in Splunk HEC format (for SignalFx/Splunk)
  void ExportAsSplunkEvents(const base::Value::List& events, ExportCallback callback);

  // Get configuration
  const Config& config() const { return config_; }

  // Update configuration (e.g., change endpoint)
  void UpdateConfig(const Config& config);

 private:
  // Convert wootzapp activity events to OTLP log format
  base::Value::Dict ConvertToOTLPLogs(const base::Value::List& events);

  // Convert wootzapp activity events to OTLP trace format
  base::Value::Dict ConvertToOTLPTraces(const base::Value::List& events);

  // Send data to OTLP endpoint
  void SendToOTLP(const std::string& endpoint,
                  const base::Value::Dict& payload,
                  ExportCallback callback);

  // Handle network response
  void OnExportComplete(std::unique_ptr<network::SimpleURLLoader> loader,
                        ExportCallback callback,
                        std::unique_ptr<std::string> response_body);

  scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
  Config config_;

  // Track active requests
  std::vector<std::unique_ptr<network::SimpleURLLoader>> active_loaders_;
};

}  // namespace activity_tracking

#endif  // CHROME_BROWSER_ACTIVITY_TRACKING_OTEL_EXPORTER_H_

