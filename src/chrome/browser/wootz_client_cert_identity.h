// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WOOTZ_CLIENT_CERT_IDENTITY_H_
#define CHROME_BROWSER_WOOTZ_CLIENT_CERT_IDENTITY_H_

#include "base/memory/scoped_refptr.h"
#include "net/ssl/client_cert_identity.h"

namespace net {
class X509Certificate;
class SSLPrivateKey;
}

// ClientCertIdentity implementation for Wootz Device Identity Certificate (DIC)
// that uses hardware-backed signing via the existing wootz_keystore infrastructure.
class WootzClientCertIdentity : public net::ClientCertIdentity {
 public:
  explicit WootzClientCertIdentity(scoped_refptr<net::X509Certificate> certificate);
  ~WootzClientCertIdentity() override;

  // net::ClientCertIdentity implementation:
  void AcquirePrivateKey(base::OnceCallback<void(scoped_refptr<net::SSLPrivateKey>)> callback) override;

 private:
  WootzClientCertIdentity(const WootzClientCertIdentity&) = delete;
  WootzClientCertIdentity& operator=(const WootzClientCertIdentity&) = delete;
};

#endif  // CHROME_BROWSER_WOOTZ_CLIENT_CERT_IDENTITY_H_

