// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_NET_OKTA_GATE_UTIL_H_
#define CHROME_BROWSER_NET_OKTA_GATE_UTIL_H_

class GURL;

// Determines if the given URL is an Okta app URL that should be gated.
bool IsOktaAppUrl(const GURL& url);

// Determines if the given URL is eb.wootzapp.com (triggers certificate
// provisioning)
bool IsFacebookUrl(const GURL& url);

#endif  // CHROME_BROWSER_NET_OKTA_GATE_UTIL_H_
