// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/wootz_offline_pages/wootz_offline_page_saver_installer.h"

#include "chrome/browser/wootz_offline_pages/wootz_offline_page_saver.h"
#include "content/public/browser/web_contents.h"

WootzOfflinePageSaverInstaller::WootzOfflinePageSaverInstaller(
    content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      content::WebContentsUserData<WootzOfflinePageSaverInstaller>(
          *web_contents) {
  WootzOfflinePageSaver::CreateForWebContents(web_contents);
}

WootzOfflinePageSaverInstaller::~WootzOfflinePageSaverInstaller() = default;

void WootzOfflinePageSaverInstaller::PrimaryMainFrameRenderProcessGone(
    base::TerminationStatus status) {
  web_contents()->RemoveUserData(
      WootzOfflinePageSaverInstaller::UserDataKey());
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(WootzOfflinePageSaverInstaller);
