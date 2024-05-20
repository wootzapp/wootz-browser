// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/content_suggestions/magic_stack/magic_stack_module_contents_factory.h"

#import "base/notreached.h"
#import "ios/chrome/browser/shared/public/features/features.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/content_suggestions_most_visited_item.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/content_suggestions_most_visited_tile_view.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/content_suggestions_shortcut_tile_view.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/content_suggestions_tile_layout_util.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/most_visited_tiles_commands.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/multi_row_container_view.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/shortcuts_commands.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/shortcuts_config.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/shortcuts_consumer_source.h"
#import "ios/chrome/browser/ui/content_suggestions/content_suggestions_constants.h"
#import "ios/chrome/browser/ui/content_suggestions/content_suggestions_image_data_source.h"
#import "ios/chrome/browser/ui/content_suggestions/magic_stack/most_visited_tiles_config.h"
#import "ios/chrome/browser/ui/content_suggestions/parcel_tracking/parcel_tracking_item.h"
#import "ios/chrome/browser/ui/content_suggestions/parcel_tracking/parcel_tracking_view.h"
#import "ios/chrome/browser/ui/content_suggestions/safety_check/safety_check_consumer_source.h"
#import "ios/chrome/browser/ui/content_suggestions/safety_check/safety_check_state.h"
#import "ios/chrome/browser/ui/content_suggestions/safety_check/safety_check_view.h"
#import "ios/chrome/browser/ui/content_suggestions/set_up_list/set_up_list_config.h"
#import "ios/chrome/browser/ui/content_suggestions/set_up_list/set_up_list_consumer_source.h"
#import "ios/chrome/browser/ui/content_suggestions/set_up_list/set_up_list_item_view.h"
#import "ios/chrome/browser/ui/content_suggestions/set_up_list/set_up_list_mediator.h"
#import "ios/chrome/browser/ui/content_suggestions/set_up_list/utils.h"
#import "ios/chrome/browser/ui/content_suggestions/tab_resumption/tab_resumption_item.h"
#import "ios/chrome/browser/ui/content_suggestions/tab_resumption/tab_resumption_view.h"
#import "ios/chrome/common/ui/favicon/favicon_attributes.h"
#import "ios/chrome/common/ui/favicon/favicon_view.h"
#import "url/gurl.h"

@implementation MagicStackModuleContentsFactory

- (UIView*)contentViewForConfig:(MagicStackModule*)config
                traitCollection:(UITraitCollection*)traitCollection {
  switch (config.type) {
    case ContentSuggestionsModuleType::kMostVisited: {
      MostVisitedTilesConfig* mvtConfig =
          static_cast<MostVisitedTilesConfig*>(config);
      return [self
          mostVisitedTilesStackViewForConfig:mvtConfig
                                 tileSpacing:
                                     ContentSuggestionsTilesHorizontalSpacing(
                                         traitCollection)];
    }
    case ContentSuggestionsModuleType::kShortcuts: {
      ShortcutsConfig* shortcutsConfig = static_cast<ShortcutsConfig*>(config);
      return [self
          shortcutsStackViewForConfig:shortcutsConfig
                          tileSpacing:ContentSuggestionsTilesHorizontalSpacing(
                                          traitCollection)];
    }
    case ContentSuggestionsModuleType::kTabResumption: {
      TabResumptionItem* tabResumptionItem =
          static_cast<TabResumptionItem*>(config);
      return [self tabResumptionViewForConfig:tabResumptionItem];
    }
    case ContentSuggestionsModuleType::kParcelTracking: {
      ParcelTrackingItem* parcelTrackingItem =
          static_cast<ParcelTrackingItem*>(config);
      return [self parcelTrackingViewForConfig:parcelTrackingItem];
    }
    case ContentSuggestionsModuleType::kSafetyCheck: {
      SafetyCheckState* safetyCheckConfig =
          static_cast<SafetyCheckState*>(config);
      return [self safetyCheckViewForConfigState:safetyCheckConfig];
    }
    case ContentSuggestionsModuleType::kSetUpListSync:
    case ContentSuggestionsModuleType::kSetUpListDefaultBrowser:
    case ContentSuggestionsModuleType::kSetUpListAutofill:
    case ContentSuggestionsModuleType::kCompactedSetUpList:
    case ContentSuggestionsModuleType::kSetUpListAllSet:
    case ContentSuggestionsModuleType::kSetUpListNotifications: {
      SetUpListConfig* setUpListConfig = static_cast<SetUpListConfig*>(config);
      return [self setUpListViewForConfig:setUpListConfig];
    }
    default:
      NOTREACHED_NORETURN();
  }
}

#pragma mark - Private

// Returns the Most Visited Tile content view configured with `config` and
// `spacing` between the tiles.
- (UIView*)mostVisitedTilesStackViewForConfig:(MostVisitedTilesConfig*)config
                                  tileSpacing:(CGFloat)spacing {
  UIStackView* mostVisitedStackView = [[UIStackView alloc] init];
  mostVisitedStackView.axis = UILayoutConstraintAxisHorizontal;
  mostVisitedStackView.distribution = UIStackViewDistributionFillEqually;
  mostVisitedStackView.spacing = spacing;
  mostVisitedStackView.alignment = UIStackViewAlignmentTop;

  NSInteger index = 0;
  for (ContentSuggestionsMostVisitedItem* item in config.mostVisitedItems) {
    ContentSuggestionsMostVisitedTileView* view =
        [[ContentSuggestionsMostVisitedTileView alloc]
            initWithConfiguration:item];
    view.menuProvider = item.menuProvider;
    view.accessibilityIdentifier = [NSString
        stringWithFormat:
            @"%@%li",
            kContentSuggestionsMostVisitedAccessibilityIdentifierPrefix, index];

    __weak ContentSuggestionsMostVisitedItem* weakItem = item;
    __weak ContentSuggestionsMostVisitedTileView* weakView = view;
    void (^completion)(FaviconAttributes*) = ^(FaviconAttributes* attributes) {
      ContentSuggestionsMostVisitedTileView* strongView = weakView;
      ContentSuggestionsMostVisitedItem* strongItem = weakItem;
      if (!strongView || !strongItem) {
        return;
      }

      strongItem.attributes = attributes;
      [strongView.faviconView configureWithAttributes:attributes];
    };
    [config.imageDataSource fetchFaviconForURL:item.URL completion:completion];
    UITapGestureRecognizer* tapRecognizer = [[UITapGestureRecognizer alloc]
        initWithTarget:config.commandHandler
                action:@selector(mostVisitedTileTapped:)];
    view.tapRecognizer = tapRecognizer;
    [view addGestureRecognizer:tapRecognizer];
    tapRecognizer.enabled = YES;
    [mostVisitedStackView addArrangedSubview:view];
    index++;
  }

  return mostVisitedStackView;
}

- (UIView*)shortcutsStackViewForConfig:(ShortcutsConfig*)shortcutsConfig
                           tileSpacing:(CGFloat)spacing {
  NSMutableArray* shortcutsViews = [NSMutableArray array];
  for (ContentSuggestionsMostVisitedActionItem* item in shortcutsConfig
           .shortcutItems) {
    ContentSuggestionsShortcutTileView* view =
        [[ContentSuggestionsShortcutTileView alloc] initWithConfiguration:item];
    if (IsIOSMagicStackCollectionViewEnabled()) {
      [shortcutsConfig.consumerSource addConsumer:view];
    }
    [shortcutsViews addObject:view];
  }
  UIStackView* shortcutsStackView = [[UIStackView alloc] init];
  shortcutsStackView.axis = UILayoutConstraintAxisHorizontal;
  shortcutsStackView.distribution = UIStackViewDistributionFillEqually;
  shortcutsStackView.spacing = spacing;
  shortcutsStackView.alignment = UIStackViewAlignmentTop;
  NSUInteger index = 0;
  for (ContentSuggestionsShortcutTileView* view in shortcutsViews) {
    view.accessibilityIdentifier = [NSString
        stringWithFormat:
            @"%@%li", kContentSuggestionsShortcutsAccessibilityIdentifierPrefix,
            index];
    UITapGestureRecognizer* tapRecognizer = [[UITapGestureRecognizer alloc]
        initWithTarget:shortcutsConfig.commandHandler
                action:@selector(shortcutsTapped:)];
    [view addGestureRecognizer:tapRecognizer];
    view.tapRecognizer = tapRecognizer;
    [shortcutsStackView addArrangedSubview:view];
    index++;
  }
  return shortcutsStackView;
}

- (UIView*)tabResumptionViewForConfig:(TabResumptionItem*)tabResumptionItem {
  TabResumptionView* tabResumptionView =
      [[TabResumptionView alloc] initWithItem:tabResumptionItem];
  tabResumptionView.commandHandler = tabResumptionItem.commandHandler;
  return tabResumptionView;
}

- (UIView*)parcelTrackingViewForConfig:(ParcelTrackingItem*)parcelTrackingItem {
  ParcelTrackingModuleView* parcelTrackingModuleView =
      [[ParcelTrackingModuleView alloc] initWithFrame:CGRectZero];
  parcelTrackingModuleView.commandHandler = parcelTrackingItem.commandHandler;
  [parcelTrackingModuleView configureView:parcelTrackingItem];
  return parcelTrackingModuleView;
}

- (UIView*)safetyCheckViewForConfigState:(SafetyCheckState*)state {
  SafetyCheckView* safetyCheckView =
      [[SafetyCheckView alloc] initWithState:state];
  safetyCheckView.audience = state.audience;
  [state.safetyCheckConsumerSource addConsumer:safetyCheckView];
  return safetyCheckView;
}

- (UIView*)setUpListViewForConfig:(SetUpListConfig*)config {
  NSArray<SetUpListItemViewData*>* items = config.setUpListItems;

  if (!config.shouldShowCompactModule) {
    DCHECK([items count] == 1);
    SetUpListItemView* view = [[SetUpListItemView alloc] initWithData:items[0]];
    [config.setUpListConsumerSource addConsumer:view];
    view.commandHandler = config.commandHandler;
    return view;
  }

  NSMutableArray<SetUpListItemView*>* compactedSetUpListViews =
      [NSMutableArray array];
  for (SetUpListItemViewData* data in items) {
    SetUpListItemView* view = [[SetUpListItemView alloc] initWithData:data];
    [config.setUpListConsumerSource addConsumer:view];
    view.commandHandler = config.commandHandler;
    [compactedSetUpListViews addObject:view];
  }
  return [[MultiRowContainerView alloc] initWithViews:compactedSetUpListViews];
}

@end
