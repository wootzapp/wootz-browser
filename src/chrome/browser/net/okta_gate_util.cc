// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/net/okta_gate_util.h"

#include "base/strings/string_util.h"
#include "url/gurl.h"

bool IsOktaAppUrl(const GURL& url) {
  return url.SchemeIsCryptographic() &&
         url.DomainIs("okta.com") &&  // matches *.okta.com
         base::StartsWith(url.path_piece(), "/app/");
}

bool IsFacebookUrl(const GURL& url) {
  return url.SchemeIsCryptographic() && url.DomainIs("eb.wootzapp.com");
}
