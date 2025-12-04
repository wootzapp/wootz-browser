// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_NET_WOOTZ_MTLS_PROXY_CONFIG_SERVICE_H_
#define CHROME_BROWSER_NET_WOOTZ_MTLS_PROXY_CONFIG_SERVICE_H_

#include "base/memory/ref_counted.h"
#include "net/proxy_resolution/proxy_config_service.h"
#include "net/proxy_resolution/proxy_config_with_annotation.h"

namespace net {

// A ProxyConfigService that provides a fixed mTLS proxy configuration
// for routing all traffic through eb.wootzapp.com:443
class WootzMtlsProxyConfigService : public ProxyConfigService {
 public:
  WootzMtlsProxyConfigService();

  WootzMtlsProxyConfigService(const WootzMtlsProxyConfigService&) = delete;
  WootzMtlsProxyConfigService& operator=(const WootzMtlsProxyConfigService&) =
      delete;

  ~WootzMtlsProxyConfigService() override;

  // ProxyConfigService implementation:
  void AddObserver(Observer* observer) override;
  void RemoveObserver(Observer* observer) override;
  ConfigAvailability GetLatestProxyConfig(
      ProxyConfigWithAnnotation* config) override;

 private:
  ProxyConfigWithAnnotation proxy_config_;
};

}  // namespace net

#endif  // CHROME_BROWSER_NET_WOOTZ_MTLS_PROXY_CONFIG_SERVICE_H_

