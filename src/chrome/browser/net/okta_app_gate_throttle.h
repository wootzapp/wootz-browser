// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_NET_OKTA_APP_GATE_THROTTLE_H_
#define CHROME_BROWSER_NET_OKTA_APP_GATE_THROTTLE_H_

#include <memory>
#include <string>
#include <vector>

#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "third_party/blink/public/common/loader/url_loader_throttle.h"

namespace content {
class BrowserContext;
}

namespace net {
struct RedirectInfo;
class HttpRequestHeaders;
}  // namespace net

namespace network {
class SimpleURLLoader;
struct ResourceRequest;
namespace mojom {
class URLResponseHead;
}
}  // namespace network

class OktaCertificateManager;

class OktaAppGateThrottle : public blink::URLLoaderThrottle {
 public:
  explicit OktaAppGateThrottle(content::BrowserContext* context);

  OktaAppGateThrottle(const OktaAppGateThrottle&) = delete;
  OktaAppGateThrottle& operator=(const OktaAppGateThrottle&) = delete;

  ~OktaAppGateThrottle() override;

  // blink::URLLoaderThrottle implementation:
  void WillStartRequest(network::ResourceRequest* request,
                        bool* defer) override;
  void WillRedirectRequest(
      net::RedirectInfo* redirect_info,
      const network::mojom::URLResponseHead& response_head,
      bool* defer,
      std::vector<std::string>* to_be_removed_request_headers,
      net::HttpRequestHeaders* modified_request_headers,
      net::HttpRequestHeaders* modified_cors_exempt_request_headers) override;

 private:
  void ShowAccessDeniedPage();

  // Callback for certificate request
  void OnCertificateReceived(bool success, const std::string& certificate_pem);

  // Callback for certificate request triggered by eb.wootzapp.com visit
  void OnCertificateReceivedForFacebook(bool success,
                                        const std::string& certificate_pem);

  raw_ptr<content::BrowserContext> context_;
  std::unique_ptr<OktaCertificateManager> certificate_manager_;
  bool gated_ = false;  // ensure we gate once per chain

  base::WeakPtrFactory<OktaAppGateThrottle> weak_factory_{this};
};

#endif  // CHROME_BROWSER_NET_OKTA_APP_GATE_THROTTLE_H_
