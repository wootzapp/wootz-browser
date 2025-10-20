// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ACTIVITY_TRACKING_SPLUNK_OBSERVABILITY_EXPORTER_H_
#define CHROME_BROWSER_ACTIVITY_TRACKING_SPLUNK_OBSERVABILITY_EXPORTER_H_

#include <memory>
#include <string>

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/scoped_refptr.h"
#include "base/values.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"

namespace activity_tracking {

// Exports navigation telemetry to Splunk Observability Cloud (SignalFx)
// Uses Chromium's network stack to send data to the SignalFx ingest API
class SplunkObservabilityExporter {
 public:
  enum class ExportResult {
    SUCCESS,
    FAILURE,
    TIMEOUT,
    NETWORK_ERROR,
  };

  using ExportCallback = base::OnceCallback<void(ExportResult)>;

  // Configuration for Splunk Observability Cloud
  struct Config {
    Config();
    ~Config();
    Config(const Config&);
    Config& operator=(const Config&);
    
    std::string realm;                // e.g., "sg0" for Singapore
    std::string access_token;         // SignalFx access token (sent as X-SF-TOKEN header)
    std::string ingest_endpoint;      // Ingest endpoint: https://ingest.<realm>.signalfx.com
    int timeout_ms = 30000;           // 30 seconds default
    bool enable_metrics = true;       // Send to /v2/datapoint
    bool enable_events = true;        // Send to /v2/event
    bool enable_traces = false;       // Send to /v2/trace (not implemented yet)
  };

  SplunkObservabilityExporter(
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
      const Config& config);
  ~SplunkObservabilityExporter();

  // Export OpenTelemetry JSON logs as SignalFx events
  void ExportEvents(const base::Value::Dict& otel_json, 
                    ExportCallback callback);

  // Export OpenTelemetry JSON logs as SignalFx metrics (datapoints)
  void ExportMetrics(const base::Value::Dict& otel_json,
                     ExportCallback callback);

  // Get configuration
  const Config& config() const { return config_; }

 private:
  // Convert OpenTelemetry log format to SignalFx event format
  base::Value::List ConvertOTelToSignalFxEvents(
      const base::Value::Dict& otel_json);

  // Convert OpenTelemetry log format to SignalFx datapoint format
  base::Value::List ConvertOTelToSignalFxDatapoints(
      const base::Value::Dict& otel_json);

  // Send data to SignalFx API
  void SendToSignalFx(const std::string& endpoint,
                      const base::Value::List& payload,
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

#endif  // CHROME_BROWSER_ACTIVITY_TRACKING_SPLUNK_OBSERVABILITY_EXPORTER_H_

