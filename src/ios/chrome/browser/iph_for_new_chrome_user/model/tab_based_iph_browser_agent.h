// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_IPH_FOR_NEW_CHROME_USER_MODEL_TAB_BASED_IPH_BROWSER_AGENT_H_
#define IOS_CHROME_BROWSER_IPH_FOR_NEW_CHROME_USER_MODEL_TAB_BASED_IPH_BROWSER_AGENT_H_

#import "ios/chrome/browser/shared/model/browser/browser_observer.h"
#import "ios/chrome/browser/shared/model/browser/browser_user_data.h"
#import "ios/chrome/browser/shared/model/web_state_list/active_web_state_observation_forwarder.h"
#import "ios/chrome/browser/url_loading/model/url_loading_observer.h"

class Browser;
@class CommandDispatcher;
@protocol HelpCommands;
class UrlLoadingNotifierBrowserAgent;
class WebStateList;

namespace feature_engagement {
class Tracker;
}  // namespace feature_engagement

// A browser agent that serves a central manager for all IPHs features that
// should be triggered by tab and/or tab list changes.
class TabBasedIPHBrowserAgent : public BrowserUserData<TabBasedIPHBrowserAgent>,
                                public BrowserObserver,
                                public UrlLoadingObserver,
                                public web::WebStateObserver {
 public:
  TabBasedIPHBrowserAgent(const TabBasedIPHBrowserAgent&) = delete;
  TabBasedIPHBrowserAgent& operator=(const TabBasedIPHBrowserAgent&) = delete;

  ~TabBasedIPHBrowserAgent() override;

#pragma mark - Public methods

  // Notifies the browser agent that the user has performed a multi-gesture tab
  // refresh, so that the related in-product help would be attempted.
  void NotifyMultiGestureRefreshEvent();

  // Notifies that the user has tapped the back/forward button in the toolbar,
  // so that the related in-product help would be attempted.
  void NotifyBackForwardButtonTap();

#pragma mark - Observer headers

  // BrowserObserver
  void BrowserDestroyed(Browser* browser) override;

  // UrlLoadingObserver
  void TabDidLoadUrl(const GURL& url,
                     ui::PageTransition transition_type) override;
  void NewTabDidLoadUrl(const GURL& url, bool user_initiated) override;

  // WebStateObserver
  void DidStartNavigation(web::WebState* web_state,
                          web::NavigationContext* navigation_context) override;
  void DidStopLoading(web::WebState* web_state) override;
  void WasHidden(web::WebState* web_state) override;
  void WebStateDestroyed(web::WebState* web_state) override;

#pragma mark - Private methods

 private:
  friend class BrowserUserData<TabBasedIPHBrowserAgent>;

  explicit TabBasedIPHBrowserAgent(Browser* browser);

  // For all IPH features managed by this class, resets their tracker variables
  // to `false`, and remove currently displaying IPH views from the view.
  void ResetFeatureStatesAndRemoveIPHViews();

  // Command handler for help commands.
  id<HelpCommands> HelpHandler();

  // Observer for the browser's web state list and the active web state.
  raw_ptr<WebStateList> web_state_list_;
  std::unique_ptr<ActiveWebStateObservationForwarder>
      active_web_state_observer_;

#pragma mark - Observers variables

  // Observer for URL loading.
  raw_ptr<UrlLoadingNotifierBrowserAgent> url_loading_notifier_;
  // Command dispatcher for the browser; used to retrieve help handler.
  CommandDispatcher* command_dispatcher_;
  // Records events for the use of in-product help.
  raw_ptr<feature_engagement::Tracker> engagement_tracker_;

#pragma mark - IPH feature invocation tracking variables

  // Whether a multi-gesture refresh is currently happening.
  bool multi_gesture_refresh_ = false;
  // Whether the user has just tapped back/forward button in the toolbar; will
  // be reset to `false` after the navigation has completed.
  bool back_forward_button_tapped_ = false;

  BROWSER_USER_DATA_KEY_DECL();
};

#endif  // IOS_CHROME_BROWSER_IPH_FOR_NEW_CHROME_USER_MODEL_TAB_BASED_IPH_BROWSER_AGENT_H_
