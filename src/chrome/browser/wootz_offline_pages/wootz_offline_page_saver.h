// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_SAVER_H_
#define CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_SAVER_H_

#include <vector>

#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"
#include "url/gurl.h"

class WootzOfflinePageService;

class WootzOfflinePageSaver
    : public content::WebContentsObserver,
      public content::WebContentsUserData<WootzOfflinePageSaver> {
 public:
  ~WootzOfflinePageSaver() override;

  // content::WebContentsObserver:
  void DidFinishNavigation(
      content::NavigationHandle* navigation_handle) override;
  void DidStopLoading() override;

 private:
  friend class content::WebContentsUserData<WootzOfflinePageSaver>;
  explicit WootzOfflinePageSaver(content::WebContents* web_contents);

  WootzOfflinePageService* GetService();

  // Stores redirect chain from the last navigation to use when page finishes loading
  std::vector<GURL> last_redirect_chain_;
  
  // Track if a save operation is in progress to prevent concurrent saves
  bool save_in_progress_ = false;
  
  // Track the last saved URL to prevent duplicate saves
  GURL last_saved_url_;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

#endif  // CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_SAVER_H_
