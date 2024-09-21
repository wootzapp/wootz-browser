/* Copyright (c) 2023 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_EPHEMERAL_STORAGE_WOOTZ_EPHEMERAL_STORAGE_SERVICE_DELEGATE_H_
#define CHROME_BROWSER_EPHEMERAL_STORAGE_WOOTZ_EPHEMERAL_STORAGE_SERVICE_DELEGATE_H_

#include <string>

#include "base/memory/raw_ptr.h"
#include "base/memory/scoped_refptr.h"
#include "components/ephemeral_storage/ephemeral_storage_service_delegate.h"
#include "components/content_settings/core/browser/cookie_settings.h"

namespace content {
class BrowserContext;
}

class HostContentSettingsMap;

namespace ephemeral_storage {

class WootzEphemeralStorageServiceDelegate
    : public EphemeralStorageServiceDelegate {
 public:
  WootzEphemeralStorageServiceDelegate(
      content::BrowserContext* context,
      HostContentSettingsMap* host_content_settings_map,
      scoped_refptr<content_settings::CookieSettings> cookie_settings);
  ~WootzEphemeralStorageServiceDelegate() override;

  void CleanupTLDEphemeralArea(const TLDEphemeralAreaKey& key) override;
  void CleanupFirstPartyStorageArea(
      const std::string& registerable_domain) override;

 private:
  raw_ptr<content::BrowserContext> context_ = nullptr;
  raw_ptr<HostContentSettingsMap> host_content_settings_map_ = nullptr;
  scoped_refptr<content_settings::CookieSettings> cookie_settings_;
};

}  // namespace ephemeral_storage

#endif  // CHROME_BROWSER_EPHEMERAL_STORAGE_WOOTZ_EPHEMERAL_STORAGE_SERVICE_DELEGATE_H_
