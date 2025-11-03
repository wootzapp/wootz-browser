// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_SAVER_INSTALLER_H_
#define CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_SAVER_INSTALLER_H_

#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

class WootzOfflinePageSaverInstaller
    : public content::WebContentsObserver,
      public content::WebContentsUserData<WootzOfflinePageSaverInstaller> {
 public:
  ~WootzOfflinePageSaverInstaller() override;

  // content::WebContentsObserver:
  void PrimaryMainFrameRenderProcessGone(
      base::TerminationStatus status) override;

 private:
  friend class content::WebContentsUserData<WootzOfflinePageSaverInstaller>;
  explicit WootzOfflinePageSaverInstaller(content::WebContents* web_contents);

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

#endif  // CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_SAVER_INSTALLER_H_
