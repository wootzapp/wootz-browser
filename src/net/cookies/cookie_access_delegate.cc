// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/cookies/cookie_access_delegate.h"

class GURL;

namespace net {

CookieAccessDelegate::CookieAccessDelegate() = default;

CookieAccessDelegate::~CookieAccessDelegate() = default;

bool CookieAccessDelegate::ShouldTreatUrlAsTrustworthy(const GURL& url) const {
  return false;
}

bool CookieAccessDelegate::ShouldUseEphemeralStorage(
    const GURL& url,
    const net::SiteForCookies& site_for_cookies,
    const std::optional<url::Origin>& top_frame_origin) const {
  NOTREACHED() << "Should be overridden";
}


}  // namespace net
