// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/saml_verifier/saml_verifier.h"

#include "base/logging.h"
#include "net/http/http_request_headers.h"
#include "net/url_request/url_request.h"
#include "url/gurl.h"

namespace saml_verifier {

// Global function to inject WootzApp headers into HTTP requests
// This can be called from a NetworkDelegate or URLRequestInterceptor
bool InjectWootzAppHeaders(net::URLRequest* request) {
  if (!request || !request->url().is_valid()) {
    return false;
  }
  
  std::string hostname = request->url().host();
  
  // Get headers from SAML verifier
  auto headers = SamlVerifier::GetWootzAppHeaders(hostname);
  
  if (headers.empty()) {
    return false;  // No headers to inject
  }
  
  // Inject headers into the request
  for (const auto& [name, value] : headers) {
    request->SetExtraRequestHeaderByName(name, value, true);
    LOG(INFO) << "[WootzApp] Injected header: " << name << " = " << value;
  }
  
  return true;
}

// Function to check if a request should have WootzApp headers injected
bool ShouldInjectWootzAppHeaders(const GURL& url) {
  if (!url.is_valid()) {
    return false;
  }
  
  std::string hostname = url.host();
  
  // Check if header injection is enabled
  if (!SamlVerifier::IsHeaderInjectionEnabled()) {
    return false;
  }
  
  // Check if this is an internal domain
  return SamlVerifier::ShouldInjectWootzAppHeaders(hostname);
  
  return false;
}



}  // namespace saml_verifier
