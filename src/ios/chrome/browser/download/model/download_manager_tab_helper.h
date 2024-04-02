// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_DOWNLOAD_MODEL_DOWNLOAD_MANAGER_TAB_HELPER_H_
#define IOS_CHROME_BROWSER_DOWNLOAD_MODEL_DOWNLOAD_MANAGER_TAB_HELPER_H_

#include <memory>

#import <Foundation/Foundation.h>

#import "base/memory/raw_ptr.h"
#import "ios/web/public/download/download_task_observer.h"
#import "ios/web/public/web_state_observer.h"
#import "ios/web/public/web_state_user_data.h"

@protocol DownloadManagerTabHelperDelegate;
@protocol SystemIdentity;

namespace web {
class DownloadTask;
class WebState;
}  // namespace web

// TabHelper which manages a single file download.
class DownloadManagerTabHelper
    : public web::WebStateUserData<DownloadManagerTabHelper>,
      public web::WebStateObserver,
      public web::DownloadTaskObserver {
 public:
  DownloadManagerTabHelper(const DownloadManagerTabHelper&) = delete;
  DownloadManagerTabHelper& operator=(const DownloadManagerTabHelper&) = delete;

  ~DownloadManagerTabHelper() override;

  // Set the current download task for this tab.
  virtual void SetCurrentDownload(std::unique_ptr<web::DownloadTask> task);

  // Returns `true` after Download() was called, `false` after the task was
  // cancelled.
  bool has_download_task() const { return task_.get(); }

  // Returns the currently active download task.
  web::DownloadTask* GetActiveDownloadTask();

  // Sets the delegate. The tab helper will no-op if the delegate is nil.
  void SetDelegate(id<DownloadManagerTabHelperDelegate> delegate);

  // Starts the current download task. Asserts that `task == task_`.
  virtual void StartDownload(web::DownloadTask* task);

  // Sets whether the Download toolbar should adapt to the fullscreen state.
  virtual void AdaptToFullscreen(bool adapt_to_fullscreen);

 protected:
  // Allow subclassing from DownloadManagerTabHelper for testing purposes.
  explicit DownloadManagerTabHelper(web::WebState* web_state);

 private:
  friend class web::WebStateUserData<DownloadManagerTabHelper>;

  // web::WebStateObserver overrides:
  void WasShown(web::WebState* web_state) override;
  void WasHidden(web::WebState* web_state) override;
  void WebStateDestroyed(web::WebState* web_state) override;

  // web::DownloadTaskObserver overrides:
  void OnDownloadUpdated(web::DownloadTask* task) override;

  // Assigns `task` to `task_`; replaces the current download if exists;
  // instructs the delegate that download has started.
  void DidCreateDownload(std::unique_ptr<web::DownloadTask> task);
  // Returns whether `task_` still needs to be saved to Drive.
  bool WillDownloadTaskBeSavedToDrive() const;

  raw_ptr<web::WebState> web_state_ = nullptr;
  __weak id<DownloadManagerTabHelperDelegate> delegate_ = nil;
  std::unique_ptr<web::DownloadTask> task_;
  bool delegate_started_ = false;

  WEB_STATE_USER_DATA_KEY_DECL();
};

#endif  // IOS_CHROME_BROWSER_DOWNLOAD_MODEL_DOWNLOAD_MANAGER_TAB_HELPER_H_
