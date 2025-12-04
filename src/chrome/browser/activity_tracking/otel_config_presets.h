// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ACTIVITY_TRACKING_OTEL_CONFIG_PRESETS_H_
#define CHROME_BROWSER_ACTIVITY_TRACKING_OTEL_CONFIG_PRESETS_H_

#include <string>

#include "chrome/browser/activity_tracking/otel_exporter.h"

namespace activity_tracking {

// Preset configurations for popular observability platforms
// These provide easy-to-use configurations for common backends
namespace otel_presets {

// Splunk Observability Cloud (SignalFx) - Event Format
// Using SignalFx native event format with dimensions
// Event Endpoint: https://ingest.<realm>.signalfx.com/v2/event
// Format: [ { "eventType": "...", "category": "...", "dimensions": {...}, "timestamp": ... } ]
inline OTelExporter::Config CreateSplunkConfig(const std::string& realm,
                                                const std::string& access_token) {
  OTelExporter::Config config;
  // Use event endpoint for activity tracking
  config.otlp_endpoint = "https://ingest." + realm + ".signalfx.com/v2/event";
  config.headers["X-SF-Token"] = access_token;  // Note: Mixed case
  config.headers["Content-Type"] = "application/json";
  config.service_name = "wootzapp-enterprise";
  config.service_version = "1.0.0";
  return config;
}

// Datadog
// Endpoint: https://http-intake.logs.datadoghq.com/api/v2/logs
inline OTelExporter::Config CreateDatadogConfig(const std::string& api_key,
                                                 const std::string& site = "datadoghq.com") {
  OTelExporter::Config config;
  config.otlp_endpoint = "https://http-intake.logs." + site + "/api/v2/datapoint";
  config.headers["DD-API-KEY"] = api_key;
  config.headers["Content-Type"] = "application/json";
  config.service_name = "wootzapp-enterprise";
  config.service_version = "1.0.0";
  return config;
}

// New Relic
// Endpoint: https://otlp.nr-data.net:4318/v1/logs
inline OTelExporter::Config CreateNewRelicConfig(const std::string& license_key) {
  OTelExporter::Config config;
  config.otlp_endpoint = "https://otlp.nr-data.net:4318/v1/logs";
  config.headers["api-key"] = license_key;
  config.headers["Content-Type"] = "application/json";
  config.service_name = "wootzapp-enterprise";
  config.service_version = "1.0.0";
  return config;
}

// Grafana Cloud (Loki with OTLP)
// Endpoint: https://otlp-gateway-{region}.grafana.net/otlp/v1/logs
inline OTelExporter::Config CreateGrafanaCloudConfig(
    const std::string& instance_id,
    const std::string& api_token,
    const std::string& region = "prod-us-east-0") {
  OTelExporter::Config config;
  config.otlp_endpoint = "https://otlp-gateway-" + region + 
                         ".grafana.net/otlp/v1/logs";
  config.headers["Authorization"] = "Basic " + instance_id + ":" + api_token;
  config.headers["Content-Type"] = "application/json";
  config.service_name = "wootzapp-enterprise";
  config.service_version = "1.0.0";
  return config;
}

// Honeycomb
// Endpoint: https://api.honeycomb.io/1/logs
inline OTelExporter::Config CreateHoneycombConfig(const std::string& api_key,
                                                   const std::string& dataset = "chromium") {
  OTelExporter::Config config;
  config.otlp_endpoint = "https://api.honeycomb.io/1/logs";
  config.headers["X-Honeycomb-Team"] = api_key;
  config.headers["X-Honeycomb-Dataset"] = dataset;
  config.headers["Content-Type"] = "application/json";
  config.service_name = "wootzapp-enterprise";
  config.service_version = "1.0.0";
  return config;
}

// Local OpenTelemetry Collector
// Default endpoint: http://localhost:4318/v1/logs
inline OTelExporter::Config CreateLocalCollectorConfig(
    const std::string& endpoint = "http://localhost:4318/v1/logs") {
  OTelExporter::Config config;
  config.otlp_endpoint = endpoint;
  config.headers["Content-Type"] = "application/json";
  config.service_name = "wootzapp-enterprise";
  config.service_version = "1.0.0";
  return config;
}

// Generic OTLP HTTP endpoint
// For custom backends or self-hosted collectors
inline OTelExporter::Config CreateGenericOTLPConfig(
    const std::string& endpoint,
    const std::map<std::string, std::string>& headers = {}) {
  OTelExporter::Config config;
  config.otlp_endpoint = endpoint;
  config.headers = headers;
  config.headers["Content-Type"] = "application/json";
  config.service_name = "wootzapp-enterprise";
  config.service_version = "1.0.0";
  return config;
}

}  // namespace otel_presets

}  // namespace activity_tracking

#endif  // CHROME_BROWSER_ACTIVITY_TRACKING_OTEL_CONFIG_PRESETS_H_

