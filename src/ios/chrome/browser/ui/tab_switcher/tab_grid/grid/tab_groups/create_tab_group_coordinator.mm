// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/tab_groups/create_tab_group_coordinator.h"

#import "base/check.h"
#import "ios/chrome/browser/shared/model/browser/browser.h"
#import "ios/chrome/browser/shared/model/web_state_list/tab_group.h"
#import "ios/chrome/browser/shared/public/commands/command_dispatcher.h"
#import "ios/chrome/browser/shared/public/features/features.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/tab_groups/create_tab_group_mediator.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/tab_groups/create_tab_group_view_controller.h"
#import "ios/chrome/browser/ui/tab_switcher/tab_grid/grid/tab_groups/tab_groups_commands.h"
#import "ios/web/public/web_state_id.h"

@implementation CreateTabGroupCoordinator {
  // Mediator for tab groups creation.
  CreateTabGroupMediator* _mediator;
  // View controller for tab groups creation.
  CreateTabGroupViewController* _viewController;
  // List of tabs to add to the tab group.
  std::set<web::WebStateID> _identifiers;
  // Tab group to edit.
  const TabGroup* _tabGroup;
}

#pragma mark - Public

- (instancetype)
    initTabGroupCreationWithBaseViewController:(UIViewController*)viewController
                                       browser:(Browser*)browser
                                  selectedTabs:
                                      (const std::set<web::WebStateID>&)
                                          identifiers {
  CHECK(base::FeatureList::IsEnabled(kTabGroupsInGrid))
      << "You should not be able to create a tab group outside the Tab Groups experiment.";
  CHECK(!identifiers.empty()) << "Cannot create an empty tab group.";
  self = [super initWithBaseViewController:viewController browser:browser];
  if (self) {
    _identifiers = identifiers;
  }
  return self;
}

- (instancetype)
    initTabGroupEditionWithBaseViewController:(UIViewController*)viewController
                                      browser:(Browser*)browser
                                     tabGroup:(const TabGroup*)tabGroup {
  CHECK(base::FeatureList::IsEnabled(kTabGroupsInGrid))
      << "You should not be able to edit a tab group outside the Tab Groups "
         "experiment.";
  CHECK(tabGroup) << "You need to pass a tab group in order to edit it.";
  self = [super initWithBaseViewController:viewController browser:browser];
  if (self) {
    _tabGroup = tabGroup;
  }
  return self;
}

#pragma mark - ChromeCoordinator

- (void)start {
  id<TabGroupsCommands> handler = HandlerForProtocol(
      self.browser->GetCommandDispatcher(), TabGroupsCommands);
  _viewController =
      [[CreateTabGroupViewController alloc] initWithHandler:handler
                                                   tabGroup:_tabGroup];

  if (_tabGroup) {
    _mediator = [[CreateTabGroupMediator alloc]
        initTabGroupEditionWithConsumer:_viewController
                               tabGroup:_tabGroup
                           webStateList:self.browser->GetWebStateList()];
  } else {
    _mediator = [[CreateTabGroupMediator alloc]
        initTabGroupCreationWithConsumer:_viewController
                            selectedTabs:_identifiers
                            webStateList:self.browser->GetWebStateList()];
  }
  _viewController.mutator = _mediator;

  // TODO(crbug.com/1501837): Add the create tab group animation.
  _viewController.modalPresentationStyle =
      UIModalPresentationOverCurrentContext;
  [self.baseViewController presentViewController:_viewController
                                        animated:YES
                                      completion:nil];
}

- (void)stop {
  _mediator = nil;

  // TODO(crbug.com/1501837): Make the created tab group animation.
  [_viewController dismissViewControllerAnimated:YES completion:nil];
  _viewController = nil;
}

@end
