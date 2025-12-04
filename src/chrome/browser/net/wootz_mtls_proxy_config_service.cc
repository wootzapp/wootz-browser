// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/net/wootz_mtls_proxy_config_service.h"

#include "base/logging.h"
#include "net/base/host_port_pair.h"
#include "net/base/proxy_chain.h"
#include "net/base/proxy_server.h"
#include "net/proxy_resolution/proxy_config.h"
#include "net/traffic_annotation/network_traffic_annotation.h"

namespace net {

WootzMtlsProxyConfigService::WootzMtlsProxyConfigService() {
  // Hardcoded mTLS proxy endpoint (eb.wootzapp.com:443)
  HostPortPair proxy_endpoint("eb.wootzapp.com", 443);
  
  LOG(ERROR) << "Configuring mTLS proxy: " << proxy_endpoint.ToString();

  // Create an HTTPS proxy server
  ProxyServer proxy_server(ProxyServer::SCHEME_HTTPS, proxy_endpoint);
  
  // Create a proxy chain with this server
  ProxyChain proxy_chain(proxy_server);

  // Configure the proxy
  ProxyConfig proxy_config;
  proxy_config.proxy_rules().type =
      ProxyConfig::ProxyRules::Type::PROXY_LIST;
  proxy_config.proxy_rules().single_proxies.AddProxyChain(proxy_chain);
  
  // No bypass rules - route everything through the proxy
  // If you want to exclude certain hosts (like localhost), add them here:
  // proxy_config.proxy_rules().bypass_rules.AddRuleFromString("localhost");
  // proxy_config.proxy_rules().bypass_rules.AddRuleFromString("127.0.0.1");
  
  // Traffic annotation for the proxy
  static const net::NetworkTrafficAnnotationTag kTrafficAnnotation =
      net::DefineNetworkTrafficAnnotation("wootz_mtls_proxy", R"(
        semantics {
          sender: "Wootz mTLS Proxy"
          description:
            "Routes all browser traffic through an mTLS-authenticated proxy "
            "at eb.wootzapp.com:443 for security and monitoring."
          trigger: "Any network request from the browser."
          data: "All HTTP/HTTPS traffic from the browser."
          destination: OTHER
          destination_other: 'eb.wootzapp.com:443 (Wootz mTLS Proxy)'
        }
        policy {
          cookies_allowed: YES
          cookies_store: "user"
          setting: "This feature cannot be disabled."
          policy_exception_justification:
            "Required for Wootz browser security infrastructure."
        }
      )");

  proxy_config_ = ProxyConfigWithAnnotation(proxy_config, kTrafficAnnotation);
  
  LOG(ERROR) << "mTLS proxy configured successfully: " 
            << proxy_server.host_port_pair().ToString();
}

WootzMtlsProxyConfigService::~WootzMtlsProxyConfigService() = default;

void WootzMtlsProxyConfigService::AddObserver(Observer* observer) {
  // Static configuration, no need to notify observers
}

void WootzMtlsProxyConfigService::RemoveObserver(Observer* observer) {
  // Static configuration, no observers
}

ProxyConfigService::ConfigAvailability
WootzMtlsProxyConfigService::GetLatestProxyConfig(
    ProxyConfigWithAnnotation* config) {
  *config = proxy_config_;
  return CONFIG_VALID;
}

}  // namespace net

