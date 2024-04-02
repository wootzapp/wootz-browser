// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/content_suggestions/magic_stack/magic_stack_ranking_model.h"

#import "components/segmentation_platform/public/constants.h"
#import "components/segmentation_platform/public/features.h"
#import "components/segmentation_platform/public/segmentation_platform_service.h"
#import "ios/chrome/browser/ntp_tiles/model/tab_resumption/tab_resumption_prefs.h"
#import "ios/chrome/browser/parcel_tracking/parcel_tracking_prefs.h"
#import "ios/chrome/browser/parcel_tracking/parcel_tracking_util.h"
#import "ios/chrome/browser/safety_check/model/ios_chrome_safety_check_manager_constants.h"
#import "ios/chrome/browser/shared/model/application_context/application_context.h"
#import "ios/chrome/browser/shared/model/prefs/pref_names.h"
#import "ios/chrome/browser/shared/public/features/features.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/most_visited_tiles_mediator.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/shortcuts_config.h"
#import "ios/chrome/browser/ui/content_suggestions/cells/shortcuts_mediator.h"
#import "ios/chrome/browser/ui/content_suggestions/content_suggestions_constants.h"
#import "ios/chrome/browser/ui/content_suggestions/content_suggestions_consumer.h"
#import "ios/chrome/browser/ui/content_suggestions/content_suggestions_metrics_recorder.h"
#import "ios/chrome/browser/ui/content_suggestions/magic_stack/magic_stack_ranking_model_delegate.h"
#import "ios/chrome/browser/ui/content_suggestions/magic_stack/most_visited_tiles_config.h"
#import "ios/chrome/browser/ui/content_suggestions/parcel_tracking/parcel_tracking_item.h"
#import "ios/chrome/browser/ui/content_suggestions/parcel_tracking/parcel_tracking_mediator.h"
#import "ios/chrome/browser/ui/content_suggestions/safety_check/safety_check_magic_stack_mediator.h"
#import "ios/chrome/browser/ui/content_suggestions/safety_check/safety_check_prefs.h"
#import "ios/chrome/browser/ui/content_suggestions/safety_check/safety_check_state.h"
#import "ios/chrome/browser/ui/content_suggestions/set_up_list/set_up_list_config.h"
#import "ios/chrome/browser/ui/content_suggestions/set_up_list/set_up_list_item_view_data.h"
#import "ios/chrome/browser/ui/content_suggestions/set_up_list/set_up_list_mediator.h"
#import "ios/chrome/browser/ui/content_suggestions/set_up_list/utils.h"
#import "ios/chrome/browser/ui/content_suggestions/tab_resumption/tab_resumption_helper_delegate.h"
#import "ios/chrome/browser/ui/content_suggestions/tab_resumption/tab_resumption_item.h"
#import "ios/chrome/browser/ui/content_suggestions/tab_resumption/tab_resumption_mediator.h"

@interface MagicStackRankingModel () <MostVisitedTilesMediatorDelegate,
                                      ParcelTrackingMediatorDelegate,
                                      SafetyCheckMagicStackMediatorDelegate,
                                      SetUpListMediatorAudience,
                                      ShortcutsMediatorDelegate,
                                      TabResumptionHelperDelegate>
// For testing-only
@property(nonatomic, assign) BOOL hasReceivedMagicStackResponse;
@end

@implementation MagicStackRankingModel {
  segmentation_platform::SegmentationPlatformService* _segmentationService;
  PrefService* _prefService;
  PrefService* _localState;
  // The latest module ranking returned from the SegmentationService.
  NSArray<NSNumber*>* _magicStackOrderFromSegmentation;
  // YES if the module ranking has been received from the SegmentationService.
  BOOL _magicStackOrderFromSegmentationReceived;
  // The latest Magic Stack module order sent up to the consumer. This includes
  // any omissions due to filtering from `_magicStackOrderFromSegmentation` (or
  // `magicStackOrder:` if kSegmentationPlatformIosModuleRanker is disabled) and
  // any additions beyond `_magicStackOrderFromSegmentation` (e.g. Set Up List).
  NSArray<NSNumber*>* _latestMagicStackOrder;
  // Module mediators.
  MostVisitedTilesMediator* _mostVisitedTilesMediator;
  SetUpListMediator* _setUpListMediator;
  TabResumptionMediator* _tabResumptionMediator;
  ParcelTrackingMediator* _parcelTrackingMediator;
  ShortcutsMediator* _shortcutsMediator;
  SafetyCheckMagicStackMediator* _safetyCheckMediator;
}

- (instancetype)initWithSegmentationService:
                    (segmentation_platform::SegmentationPlatformService*)
                        segmentationService
                                prefService:(PrefService*)prefService
                                 localState:(PrefService*)localState
                            moduleMediators:(NSArray*)moduleMediators {
  self = [super init];
  if (self) {
    _segmentationService = segmentationService;
    _prefService = prefService;
    _localState = localState;

    for (id mediator in moduleMediators) {
      if ([mediator isKindOfClass:[MostVisitedTilesMediator class]]) {
        _mostVisitedTilesMediator =
            static_cast<MostVisitedTilesMediator*>(mediator);
        _mostVisitedTilesMediator.delegate = self;
      } else if ([mediator isKindOfClass:[SetUpListMediator class]]) {
        _setUpListMediator = static_cast<SetUpListMediator*>(mediator);
        _setUpListMediator.audience = self;
      } else if ([mediator isKindOfClass:[TabResumptionMediator class]]) {
        _tabResumptionMediator = static_cast<TabResumptionMediator*>(mediator);
        _tabResumptionMediator.delegate = self;
      } else if ([mediator isKindOfClass:[ShortcutsMediator class]]) {
        _shortcutsMediator = static_cast<ShortcutsMediator*>(mediator);
        _shortcutsMediator.delegate = self;
      } else if ([mediator isKindOfClass:[ParcelTrackingMediator class]]) {
        _parcelTrackingMediator =
            static_cast<ParcelTrackingMediator*>(mediator);
        _parcelTrackingMediator.delegate = self;
      } else if ([mediator
                     isKindOfClass:[SafetyCheckMagicStackMediator class]]) {
        _safetyCheckMediator =
            static_cast<SafetyCheckMagicStackMediator*>(mediator);
        _safetyCheckMediator.delegate = self;
      } else {
        // Known module mediators need to be handled.
        NOTREACHED();
      }
    }
  }
  return self;
}

- (void)disconnect {
  _mostVisitedTilesMediator = nil;
  _setUpListMediator = nil;
  _tabResumptionMediator = nil;
  _parcelTrackingMediator = nil;
  _shortcutsMediator = nil;
  _safetyCheckMediator = nil;
}

#pragma mark - Public

- (void)fetchLatestMagicStackRanking {
  if (base::FeatureList::IsEnabled(segmentation_platform::features::
                                       kSegmentationPlatformIosModuleRanker)) {
    [self fetchMagicStackModuleRankingFromSegmentationPlatform];
  } else {
    _latestMagicStackOrder = [self magicStackOrder];
    [self.consumer setMagicStackOrder:_latestMagicStackOrder];
  }
  if (!IsIOSMagicStackCollectionViewEnabled()) {
    if (IsTabResumptionEnabled() && _tabResumptionMediator.itemConfig) {
      [self.consumer
          showTabResumptionWithItem:_tabResumptionMediator.itemConfig];
    }
    if (ShouldPutMostVisitedSitesInMagicStack() &&
        _mostVisitedTilesMediator.mostVisitedConfig) {
      [self.consumer setMostVisitedTilesConfig:_mostVisitedTilesMediator
                                                   .mostVisitedConfig];
    }
    if ([_setUpListMediator shouldShowSetUpList]) {
      [_setUpListMediator showSetUpList];
    }
    if (IsSafetyCheckMagicStackEnabled() &&
        !safety_check_prefs::IsSafetyCheckInMagicStackDisabled(_localState) &&
        _safetyCheckMediator.safetyCheckState.runningState ==
            RunningSafetyCheckState::kDefault) {
      [self.consumer showSafetyCheck:_safetyCheckMediator.safetyCheckState];
    }
    [self.consumer setShortcutTilesConfig:_shortcutsMediator.shortcutsConfig];
  }
}

- (void)logMagicStackEngagementForType:(ContentSuggestionsModuleType)type {
  [self.contentSuggestionsMetricsRecorder
      recordMagicStackModuleEngagementForType:type
                                      atIndex:
                                          [self indexForMagicStackModule:type]];
}

#pragma mark - SetUpListMediatorAudience

- (void)removeSetUpList {
  DCHECK(IsIOSMagicStackCollectionViewEnabled());
  [self.delegate magicStackRankingModel:self
                          didRemoveItem:_setUpListMediator.setUpListConfigs[0]];
}

- (void)replaceSetUpListWithAllSet:(SetUpListConfig*)allSetConfig {
  [self.delegate magicStackRankingModel:self
                         didReplaceItem:_setUpListMediator.setUpListConfigs[0]
                               withItem:allSetConfig];
}

#pragma mark - SafetyCheckMagicStackMediatorDelegate

- (void)removeSafetyCheckModule {
  if (IsIOSMagicStackCollectionViewEnabled()) {
    [self.delegate
        magicStackRankingModel:self
                 didRemoveItem:_safetyCheckMediator.safetyCheckState];
    return;
  }
  MagicStackOrderChange change{MagicStackOrderChange::Type::kRemove};
  change.old_module = ContentSuggestionsModuleType::kSafetyCheck;
  change.index = [self
      indexForMagicStackModule:ContentSuggestionsModuleType::kSafetyCheck];
  CHECK(change.index != NSNotFound);
  [self.consumer updateMagicStackOrder:change];
}

#pragma mark - TabResumptionHelperDelegate

- (void)tabResumptionHelperDidReceiveItem {
  CHECK(IsTabResumptionEnabled());
  if (tab_resumption_prefs::IsTabResumptionDisabled(_localState)) {
    return;
  }

  [self showTabResumptionWithItem:_tabResumptionMediator.itemConfig];
}

- (void)tabResumptionHelperDidReplaceItem:(TabResumptionItem*)oldItem {
  if (tab_resumption_prefs::IsTabResumptionDisabled(_localState)) {
    return;
  }

  if (IsIOSMagicStackCollectionViewEnabled()) {
    TabResumptionItem* item = _tabResumptionMediator.itemConfig;
    [self.delegate magicStackRankingModel:self
                           didReplaceItem:oldItem
                                 withItem:item];
    return;
  }
}

- (void)removeTabResumptionModule {
  if (IsIOSMagicStackCollectionViewEnabled()) {
    [self.delegate magicStackRankingModel:self
                            didRemoveItem:_tabResumptionMediator.itemConfig];
    return;
  }
  [self.consumer hideTabResumption];
}

#pragma mark - ParcelTrackingMediatorDelegate

- (void)newParcelsAvailable {
  if (IsIOSMagicStackCollectionViewEnabled()) {
    MagicStackModule* item = _parcelTrackingMediator.parcelTrackingItemToShow;
    NSArray<MagicStackModule*>* rank = [self latestMagicStackConfigRank];
    NSUInteger index = [rank indexOfObject:item];
    [self.delegate magicStackRankingModel:self
                            didInsertItem:item
                                  atIndex:index];
    return;
  }

  _latestMagicStackOrder =
      base::FeatureList::IsEnabled(
          segmentation_platform::features::kSegmentationPlatformIosModuleRanker)
          ? [self segmentationMagicStackOrder]
          : [self magicStackOrder];
  for (NSUInteger index = 0; index < [_latestMagicStackOrder count]; index++) {
    ContentSuggestionsModuleType type =
        (ContentSuggestionsModuleType)[_latestMagicStackOrder[index] intValue];
    if (type == ContentSuggestionsModuleType::kParcelTracking) {
      MagicStackOrderChange change{MagicStackOrderChange::Type::kInsert};
      change.new_module = type;
      change.index = index;
      [self.consumer updateMagicStackOrder:change];
    }
  }

  [self.consumer showParcelTrackingItem:[_parcelTrackingMediator
                                            parcelTrackingItemToShow]];
}

- (void)parcelTrackingDisabled {
  if (IsIOSMagicStackCollectionViewEnabled()) {
    [self.delegate magicStackRankingModel:self
                            didRemoveItem:_parcelTrackingMediator
                                              .parcelTrackingItemToShow];
    return;
  }

  // Find all parcel tracking modules and remove them.
  for (NSUInteger i = 0; i < [_latestMagicStackOrder count]; i++) {
    ContentSuggestionsModuleType type =
        (ContentSuggestionsModuleType)[_latestMagicStackOrder[i] intValue];
    if (type == ContentSuggestionsModuleType::kParcelTracking) {
      MagicStackOrderChange change{MagicStackOrderChange::Type::kRemove};
      change.old_module = type;
      change.index = [self indexForMagicStackModule:type];
      CHECK(change.index != NSNotFound);
      [self.consumer updateMagicStackOrder:change];
    }
  }
}

#pragma mark - Private

// Adds the correct Set Up List module type to the Magic Stack `order`.
- (void)addSetUpListToMagicStackOrder:(NSMutableArray*)order {
  if ([_setUpListMediator allItemsComplete]) {
    [order addObject:@(int(ContentSuggestionsModuleType::kSetUpListAllSet))];
  } else if (set_up_list_utils::ShouldShowCompactedSetUpListModule()) {
    [order addObject:@(int(ContentSuggestionsModuleType::kCompactedSetUpList))];
  } else {
    for (SetUpListItemViewData* model in _setUpListMediator.setUpListItems) {
      [order addObject:@(int(SetUpListModuleTypeForSetUpListType(model.type)))];
    }
  }
}

// Adds the Safety Check module to `order` based on the current Safety Check
// state.
- (void)addSafetyCheckToMagicStackOrder:(NSMutableArray*)order {
  CHECK(IsSafetyCheckMagicStackEnabled());
  [order addObject:@(int(ContentSuggestionsModuleType::kSafetyCheck))];
}

// Starts a fetch of the Segmentation module ranking.
- (void)fetchMagicStackModuleRankingFromSegmentationPlatform {
  auto inputContext =
      base::MakeRefCounted<segmentation_platform::InputContext>();
  int mvtFreshnessImpressionCount = _localState->GetInteger(
      prefs::kIosMagicStackSegmentationMVTImpressionsSinceFreshness);
  inputContext->metadata_args.emplace(
      segmentation_platform::kMostVisitedTilesFreshness,
      segmentation_platform::processing::ProcessedValue::FromFloat(
          mvtFreshnessImpressionCount));
  int shortcutsFreshnessImpressionCount = _localState->GetInteger(
      prefs::kIosMagicStackSegmentationShortcutsImpressionsSinceFreshness);
  inputContext->metadata_args.emplace(
      segmentation_platform::kShortcutsFreshness,
      segmentation_platform::processing::ProcessedValue::FromFloat(
          shortcutsFreshnessImpressionCount));
  int safetyCheckFreshnessImpressionCount = _localState->GetInteger(
      prefs::kIosMagicStackSegmentationSafetyCheckImpressionsSinceFreshness);
  inputContext->metadata_args.emplace(
      segmentation_platform::kSafetyCheckFreshness,
      segmentation_platform::processing::ProcessedValue::FromFloat(
          safetyCheckFreshnessImpressionCount));
  int tabResumptionFreshnessImpressionCount = _localState->GetInteger(
      prefs::kIosMagicStackSegmentationTabResumptionImpressionsSinceFreshness);
  inputContext->metadata_args.emplace(
      segmentation_platform::kTabResumptionFreshness,
      segmentation_platform::processing::ProcessedValue::FromFloat(
          tabResumptionFreshnessImpressionCount));
  int parcelTrackingFreshnessImpressionCount = _localState->GetInteger(
      prefs::kIosMagicStackSegmentationParcelTrackingImpressionsSinceFreshness);
  inputContext->metadata_args.emplace(
      segmentation_platform::kParcelTrackingFreshness,
      segmentation_platform::processing::ProcessedValue::FromFloat(
          parcelTrackingFreshnessImpressionCount));
  __weak MagicStackRankingModel* weakSelf = self;
  segmentation_platform::PredictionOptions options;
  options.on_demand_execution = true;
  _segmentationService->GetClassificationResult(
      segmentation_platform::kIosModuleRankerKey, options, inputContext,
      base::BindOnce(
          ^(const segmentation_platform::ClassificationResult& result) {
            weakSelf.hasReceivedMagicStackResponse = YES;
            [weakSelf didReceiveSegmentationServiceResult:result];
          }));
}

- (void)didReceiveSegmentationServiceResult:
    (const segmentation_platform::ClassificationResult&)result {
  if (result.status != segmentation_platform::PredictionStatus::kSucceeded) {
    return;
  }

  NSMutableArray* magicStackOrder = [NSMutableArray array];
  for (const std::string& label : result.ordered_labels) {
    if (label == segmentation_platform::kMostVisitedTiles) {
      [magicStackOrder
          addObject:@(int(ContentSuggestionsModuleType::kMostVisited))];
    } else if (label == segmentation_platform::kShortcuts) {
      [magicStackOrder
          addObject:@(int(ContentSuggestionsModuleType::kShortcuts))];
    } else if (label == segmentation_platform::kSafetyCheck) {
      [magicStackOrder
          addObject:@(int(ContentSuggestionsModuleType::kSafetyCheck))];
    } else if (label == segmentation_platform::kTabResumption) {
      [magicStackOrder
          addObject:@(int(ContentSuggestionsModuleType::kTabResumption))];
    } else if (label == segmentation_platform::kParcelTracking) {
      [magicStackOrder
          addObject:@(int(ContentSuggestionsModuleType::kParcelTracking))];
    }
  }
  _magicStackOrderFromSegmentationReceived = YES;
  _magicStackOrderFromSegmentation = magicStackOrder;
  if (IsIOSMagicStackCollectionViewEnabled()) {
    [self.delegate magicStackRankingModel:self
                 didGetLatestRankingOrder:[self latestMagicStackConfigRank]];
  } else {
    _latestMagicStackOrder = [self segmentationMagicStackOrder];
    [self.consumer setMagicStackOrder:_latestMagicStackOrder];
  }
}

- (NSArray<MagicStackModule*>*)latestMagicStackConfigRank {
  NSMutableArray<MagicStackModule*>* magicStackOrder = [NSMutableArray array];
  // Always add Set Up List at the front.
  if ([_setUpListMediator shouldShowSetUpList]) {
    [magicStackOrder addObjectsFromArray:[_setUpListMediator setUpListConfigs]];
  }
  for (NSNumber* moduleNumber in _magicStackOrderFromSegmentation) {
    ContentSuggestionsModuleType moduleType =
        (ContentSuggestionsModuleType)[moduleNumber intValue];
    switch (moduleType) {
      case ContentSuggestionsModuleType::kMostVisited:
        if (ShouldPutMostVisitedSitesInMagicStack() &&
            _mostVisitedTilesMediator.mostVisitedConfig) {
          [magicStackOrder
              addObject:_mostVisitedTilesMediator.mostVisitedConfig];
        }
        break;
      case ContentSuggestionsModuleType::kTabResumption:
        if (!IsTabResumptionEnabled() ||
            tab_resumption_prefs::IsTabResumptionDisabled(_localState) ||
            !_tabResumptionMediator.itemConfig) {
          break;
        }
        // If ShouldHideIrrelevantModules() is enabled and it is not ranked as
        // the first two modules, do not add it to the Magic Stack.
        if (ShouldHideIrrelevantModules() && [magicStackOrder count] > 1) {
          break;
        }
        [magicStackOrder addObject:_tabResumptionMediator.itemConfig];
        break;
      case ContentSuggestionsModuleType::kSafetyCheck:
        if (!IsSafetyCheckMagicStackEnabled() ||
            safety_check_prefs::IsSafetyCheckInMagicStackDisabled(
                _localState)) {
          break;
        }
        // If ShouldHideIrrelevantModules() is enabled and it is not the first
        // ranked module, do not add it to the Magic Stack.
        if (!ShouldHideIrrelevantModules() || [magicStackOrder count] == 0) {
          [magicStackOrder addObject:_safetyCheckMediator.safetyCheckState];
        }
        break;
      case ContentSuggestionsModuleType::kShortcuts:
        [magicStackOrder addObject:_shortcutsMediator.shortcutsConfig];
        break;
      case ContentSuggestionsModuleType::kParcelTracking:
        if (IsIOSParcelTrackingEnabled() &&
            !IsParcelTrackingDisabled(
                GetApplicationContext()->GetLocalState()) &&
            _parcelTrackingMediator.parcelTrackingItemToShow) {
          [magicStackOrder
              addObject:_parcelTrackingMediator.parcelTrackingItemToShow];
        }
        break;
      default:
        // These module types should not have been added by the logic
        // receiving the order list from Segmentation.
        NOTREACHED();
        break;
    }
  }
  return magicStackOrder;
}

// Construct the Magic Stack module order from fetched results from
// Segmentation. This method adds on modules not included on the Segmentation
// side (e.g. Set Up List) and also filters out modules not ready or should not
// be presented.
- (NSArray<NSNumber*>*)segmentationMagicStackOrder {
  NSMutableArray<NSNumber*>* magicStackOrder = [NSMutableArray array];
  // Always add Set Up List at the front.
  if ([_setUpListMediator shouldShowSetUpList]) {
    [self addSetUpListToMagicStackOrder:magicStackOrder];
  }
  for (NSNumber* moduleNumber in _magicStackOrderFromSegmentation) {
    ContentSuggestionsModuleType moduleType =
        (ContentSuggestionsModuleType)[moduleNumber intValue];
    switch (moduleType) {
      case ContentSuggestionsModuleType::kMostVisited:
        if (ShouldPutMostVisitedSitesInMagicStack()) {
          [magicStackOrder addObject:moduleNumber];
        }
        break;
      case ContentSuggestionsModuleType::kTabResumption:
        if (!IsTabResumptionEnabled() ||
            tab_resumption_prefs::IsTabResumptionDisabled(_localState) ||
            !_tabResumptionMediator.itemConfig) {
          break;
        }
        // If ShouldHideIrrelevantModules() is enabled and it is not ranked as
        // the first two modules, do not add it to the Magic Stack.
        if (ShouldHideIrrelevantModules() && [magicStackOrder count] > 1) {
          break;
        }
        [magicStackOrder addObject:moduleNumber];
        break;
      case ContentSuggestionsModuleType::kSafetyCheck:
        if (!IsSafetyCheckMagicStackEnabled() ||
            safety_check_prefs::IsSafetyCheckInMagicStackDisabled(
                _localState)) {
          break;
        }
        // If ShouldHideIrrelevantModules() is enabled and it is not the first
        // ranked module, do not add it to the Magic Stack.
        if (!ShouldHideIrrelevantModules() || [magicStackOrder count] == 0) {
          [self addSafetyCheckToMagicStackOrder:magicStackOrder];
        }
        break;
      case ContentSuggestionsModuleType::kShortcuts:
        [magicStackOrder addObject:moduleNumber];
        break;
      case ContentSuggestionsModuleType::kParcelTracking:
        if (IsIOSParcelTrackingEnabled() &&
            !IsParcelTrackingDisabled(
                GetApplicationContext()->GetLocalState()) &&
            _parcelTrackingMediator.parcelTrackingItemToShow) {
          [magicStackOrder addObject:moduleNumber];
        }
        break;
      default:
        // These module types should not have been added by the logic
        // receiving the order list from Segmentation.
        NOTREACHED();
        break;
    }
  }
  return magicStackOrder;
}

// Returns an array that represents the order of the modules to be shown in the
// Magic Stack. In theory this should not ever be used, but is a fallback list
// in case the segmentation ranking is not turned on.
// TODO(crbug.com/326104853): Remove after MagicStack is launched.
- (NSArray<NSNumber*>*)magicStackOrder {
  NSMutableArray* magicStackModules = [NSMutableArray array];
  if (IsTabResumptionEnabled() &&
      !tab_resumption_prefs::IsTabResumptionDisabled(_localState) &&
      _tabResumptionMediator.itemConfig) {
    [magicStackModules
        addObject:@(int(ContentSuggestionsModuleType::kTabResumption))];
  }
  if ([_setUpListMediator shouldShowSetUpList]) {
    [self addSetUpListToMagicStackOrder:magicStackModules];
  }
  if (ShouldPutMostVisitedSitesInMagicStack()) {
    [magicStackModules
        addObject:@(int(ContentSuggestionsModuleType::kMostVisited))];
  }
  [magicStackModules
      addObject:@(int(ContentSuggestionsModuleType::kShortcuts))];

  if (IsSafetyCheckMagicStackEnabled() &&
      !safety_check_prefs::IsSafetyCheckInMagicStackDisabled(_localState)) {
    [self addSafetyCheckToMagicStackOrder:magicStackModules];
  }

  if (IsIOSParcelTrackingEnabled() &&
      !IsParcelTrackingDisabled(GetApplicationContext()->GetLocalState())) {
      [magicStackModules
          addObject:@(int(ContentSuggestionsModuleType::kParcelTracking))];
  }

  return magicStackModules;
}

// Returns NO if client is expecting the order from Segmentation and it has not
// returned yet.
- (BOOL)isMagicStackOrderReady {
  if (base::FeatureList::IsEnabled(segmentation_platform::features::
                                       kSegmentationPlatformIosModuleRanker)) {
    return _magicStackOrderFromSegmentationReceived;
  }
  return YES;
}

// Shows the tab resumption tile with the given `item` configuration.
- (void)showTabResumptionWithItem:(TabResumptionItem*)item {
  if (tab_resumption_prefs::IsLastOpenedURL(item.tabURL, _prefService)) {
    return;
  }

  if (IsIOSMagicStackCollectionViewEnabled()) {
    NSArray<MagicStackModule*>* rank = [self latestMagicStackConfigRank];
    NSUInteger index = [rank indexOfObject:item];
    [self.delegate magicStackRankingModel:self
                            didInsertItem:item
                                  atIndex:index];
    return;
  }

  if (!self.consumer) {
    return;
  }
  _latestMagicStackOrder =
      base::FeatureList::IsEnabled(
          segmentation_platform::features::kSegmentationPlatformIosModuleRanker)
          ? [self segmentationMagicStackOrder]
          : [self magicStackOrder];
  if ([self isMagicStackOrderReady]) {
    // Only indicate the need for an explicit insertion if the tab resumption
    // item was received after building the initial Magic Stack order or getting
    // the Magic Stack Order from Segmentation.
    NSUInteger insertionIndex = [self
        indexForMagicStackModule:ContentSuggestionsModuleType::kTabResumption];
    if (insertionIndex == NSNotFound) {
      return;
    }
    // Only continue on to insert Tab Resumption after `isMagicStackOrderReady`
    // if it is in the Magic Stack order
    MagicStackOrderChange change{MagicStackOrderChange::Type::kInsert,
                                 ContentSuggestionsModuleType::kTabResumption};
    change.index = insertionIndex;
    [self.consumer updateMagicStackOrder:change];
  }
  [self.consumer showTabResumptionWithItem:item];
}

// Returns the index rank of `moduleType`.
// Callers of this need to handle a NSNotFound return case and do nothing in
// that case.
- (NSUInteger)indexForMagicStackModule:
    (ContentSuggestionsModuleType)moduleType {
  NSUInteger index = [_latestMagicStackOrder indexOfObject:@(int(moduleType))];
  if (index == NSNotFound) {
    // It is possible that a feature is enabled but the segmentation model being
    // used didn't return the feature's module (i.e. first browser session after
    // enabling a feature, so the latest model will not be downloaded until the
    // following session since experiment models are tied via finch). It is also
    // possible that the segmentation result has not returned yet.
    CHECK(base::FeatureList::IsEnabled(
        segmentation_platform::features::kSegmentationPlatformIosModuleRanker));
  }
  return index;
}

@end
