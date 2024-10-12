// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/price_notifications/price_notifications_price_tracking_mediator.h"

#import <string_view>

#import "base/apple/foundation_util.h"
#import "base/memory/raw_ptr.h"
#import "base/strings/sys_string_conversions.h"
#import "base/test/ios/wait_util.h"
#import "base/test/scoped_feature_list.h"
#import "components/bookmarks/test/bookmark_test_helpers.h"
#import "components/commerce/core/mock_shopping_service.h"
#import "components/commerce/core/price_tracking_utils.h"
#import "components/commerce/core/test_utils.h"
#import "components/image_fetcher/core/cached_image_fetcher.h"
#import "components/image_fetcher/core/image_data_fetcher.h"
#import "components/power_bookmarks/core/power_bookmark_utils.h"
#import "components/sync/base/features.h"
#import "ios/chrome/browser/bookmarks/model/account_bookmark_model_factory.h"
#import "ios/chrome/browser/bookmarks/model/bookmark_model_factory.h"
#import "ios/chrome/browser/bookmarks/model/legacy_bookmark_model.h"
#import "ios/chrome/browser/bookmarks/model/local_or_syncable_bookmark_model_factory.h"
#import "ios/chrome/browser/commerce/model/shopping_service_factory.h"
#import "ios/chrome/browser/price_insights/coordinator/test_price_insights_consumer.h"
#import "ios/chrome/browser/price_insights/ui/price_insights_item.h"
#import "ios/chrome/browser/push_notification/model/push_notification_service.h"
#import "ios/chrome/browser/shared/model/browser/browser.h"
#import "ios/chrome/browser/shared/model/browser/browser_list.h"
#import "ios/chrome/browser/shared/model/browser/browser_list_factory.h"
#import "ios/chrome/browser/shared/model/browser/test/test_browser.h"
#import "ios/chrome/browser/shared/model/browser_state/chrome_browser_state.h"
#import "ios/chrome/browser/shared/model/browser_state/test_chrome_browser_state.h"
#import "ios/chrome/browser/shared/model/browser_state/test_chrome_browser_state_manager.h"
#import "ios/chrome/browser/shared/model/web_state_list/web_state_list.h"
#import "ios/chrome/browser/shared/ui/table_view/table_view_utils.h"
#import "ios/chrome/browser/ui/price_notifications/cells/price_notifications_table_view_item.h"
#import "ios/chrome/browser/ui/price_notifications/price_notifications_mutator.h"
#import "ios/chrome/browser/ui/price_notifications/price_notifications_table_view_controller.h"
#import "ios/chrome/browser/ui/price_notifications/test_price_notifications_consumer.h"
#import "ios/chrome/test/testing_application_context.h"
#import "ios/public/provider/chrome/browser/push_notification/push_notification_api.h"
#import "ios/web/public/test/fakes/fake_navigation_manager.h"
#import "ios/web/public/test/fakes/fake_web_state.h"
#import "ios/web/public/test/web_task_environment.h"
#import "ios/web/public/web_state.h"
#import "testing/platform_test.h"

namespace {

const char kTestUrl[] = "https://www.merchant.com/price_drop_product";
const char kBookmarkTitle[] = "My product title";
uint64_t kClusterId = 12345L;

PriceInsightsItem* GetPriceInsightsItem() {
  PriceInsightsItem* item = [[PriceInsightsItem alloc] init];
  item.title = base::SysUTF8ToNSString(kBookmarkTitle);
  item.productURL = GURL(kTestUrl);
  item.buyingOptionsURL = GURL(kTestUrl);
  return item;
}

void TrackBookmark(commerce::ShoppingService* shopping_service,
                   bookmarks::BookmarkModel* bookmark_model,
                   const bookmarks::BookmarkNode* product) {
  base::RunLoop run_loop;
  SetPriceTrackingStateForBookmark(
      shopping_service, bookmark_model, product, true,
      base::BindOnce(
          [](base::RunLoop* run_loop, bool success) {
            EXPECT_TRUE(success);
            run_loop->Quit();
          },
          &run_loop));
  run_loop.Run();
}

const bookmarks::BookmarkNode* PrepareSubscription(
    commerce::MockShoppingService* shopping_service,
    BOOL unsubscribe_callback) {
  bookmarks::BookmarkModel* bookmark_model =
      shopping_service->GetBookmarkModelUsedForSync();
  std::string_view title = kBookmarkTitle;
  const bookmarks::BookmarkNode* product =
      commerce::AddProductBookmark(bookmark_model, base::UTF8ToUTF16(title),
                                   GURL(kTestUrl), kClusterId, true);
  const bookmarks::BookmarkNode* default_folder =
      base::FeatureList::IsEnabled(
          syncer::kEnableBookmarkFoldersForAccountStorage)
          ? bookmark_model->account_mobile_node()
          : bookmark_model->mobile_node();
  bookmark_model->AddURL(default_folder, default_folder->children().size(),
                         base::UTF8ToUTF16(title), GURL(kTestUrl));
  shopping_service->SetSubscribeCallbackValue(true);
  shopping_service->SetIsSubscribedCallbackValue(true);
  shopping_service->SetUnsubscribeCallbackValue(unsubscribe_callback);
  TrackBookmark(shopping_service, bookmark_model, product);

  commerce::ProductInfo product_info;
  product_info.title = kBookmarkTitle;
  std::optional<commerce::ProductInfo> optional_product_info;
  optional_product_info.emplace(product_info);
  shopping_service->SetResponseForGetProductInfoForUrl(optional_product_info);

  std::vector<commerce::CommerceSubscription> subscriptions;
  std::unique_ptr<power_bookmarks::PowerBookmarkMeta> meta =
      power_bookmarks::GetNodePowerBookmarkMeta(bookmark_model, product);
  commerce::CommerceSubscription sub(
      commerce::SubscriptionType::kPriceTrack,
      commerce::IdentifierType::kProductClusterId,
      base::NumberToString(meta->shopping_specifics().product_cluster_id()),
      commerce::ManagementType::kUserManaged);
  subscriptions.push_back(sub);
  shopping_service->SetGetAllSubscriptionsCallbackValue(subscriptions);
  return product;
}

}  // namespace

class PriceNotificationsPriceTrackingMediatorTest
    : public PlatformTest,
      public testing::WithParamInterface<bool> {
 public:
  PriceNotificationsPriceTrackingMediatorTest() {
    if (GetParam()) {
      feature_list_.InitAndEnableFeature(
          syncer::kEnableBookmarkFoldersForAccountStorage);
    } else {
      feature_list_.InitAndDisableFeature(
          syncer::kEnableBookmarkFoldersForAccountStorage);
    }

    TestChromeBrowserState::Builder builder;
    builder.AddTestingFactory(ios::BookmarkModelFactory::GetInstance(),
                              ios::BookmarkModelFactory::GetDefaultFactory());
    builder.AddTestingFactory(
        ios::LocalOrSyncableBookmarkModelFactory::GetInstance(),
        ios::LocalOrSyncableBookmarkModelFactory::GetDefaultFactory());
    builder.AddTestingFactory(
        ios::AccountBookmarkModelFactory::GetInstance(),
        ios::AccountBookmarkModelFactory::GetDefaultFactory());
    builder.AddTestingFactory(
        commerce::ShoppingServiceFactory::GetInstance(),
        base::BindRepeating(
            [](web::BrowserState*) -> std::unique_ptr<KeyedService> {
              return commerce::MockShoppingService::Build();
            }));
    std::unique_ptr<TestChromeBrowserState> test_chrome_browser_state =
        builder.Build();

    browser_list_ =
        BrowserListFactory::GetForBrowserState(test_chrome_browser_state.get());
    browser_ = std::make_unique<TestBrowser>(test_chrome_browser_state.get());
    browser_list_->AddBrowser(browser_.get());
    web_state_ = std::make_unique<web::FakeWebState>();
    std::unique_ptr<web::FakeNavigationManager> navigation_manager =
        std::make_unique<web::FakeNavigationManager>();
    navigation_manager->AddItem(GURL(kTestUrl), ui::PAGE_TRANSITION_LINK);
    navigation_manager->SetLastCommittedItem(
        navigation_manager->GetItemAtIndex(0));
    web_state_->SetNavigationManager(std::move(navigation_manager));
    web_state_->SetBrowserState(test_chrome_browser_state.get());
    web_state_->SetNavigationItemCount(1);
    web_state_->SetCurrentURL(GURL(kTestUrl));
    image_fetcher_ = std::make_unique<image_fetcher::ImageDataFetcher>(
        test_chrome_browser_state->GetSharedURLLoaderFactory());

    if (base::FeatureList::IsEnabled(
            syncer::kEnableBookmarkFoldersForAccountStorage)) {
      bookmark_model_ = ios::BookmarkModelFactory::
          GetModelForBrowserStateIfUnificationEnabledOrDie(
              test_chrome_browser_state.get());
      bookmarks::test::WaitForBookmarkModelToLoad(bookmark_model_);
      bookmark_model_->CreateAccountPermanentFolders();
    } else {
      bookmark_model_ = ios::AccountBookmarkModelFactory::
          GetDedicatedUnderlyingModelForBrowserStateIfUnificationDisabledOrDie(
              test_chrome_browser_state.get());
      bookmarks::test::WaitForBookmarkModelToLoad(bookmark_model_);
    }

    shopping_service_ = static_cast<commerce::MockShoppingService*>(
        commerce::ShoppingServiceFactory::GetForBrowserState(
            test_chrome_browser_state.get()));
    shopping_service_->SetupPermissiveMock();
    shopping_service_->SetBookmarkModelUsedForSync(bookmark_model_);
    test_manager_ = std::make_unique<TestChromeBrowserStateManager>(
        std::move(test_chrome_browser_state));
    TestingApplicationContext::GetGlobal()->SetChromeBrowserStateManager(
        test_manager_.get());
    push_notification_service_ = ios::provider::CreatePushNotificationService();
    mediator_ = [[PriceNotificationsPriceTrackingMediator alloc]
        initWithShoppingService:(commerce::ShoppingService*)shopping_service_
                   imageFetcher:std::move(image_fetcher_)
                       webState:(web::WebState*)web_state_.get()
        pushNotificationService:(PushNotificationService*)
                                    push_notification_service_.get()];
  }

 protected:
  base::test::ScopedFeatureList feature_list_;
  web::WebTaskEnvironment task_environment_;
  std::unique_ptr<Browser> browser_;
  PriceNotificationsPriceTrackingMediator* mediator_;
  std::unique_ptr<ios::ChromeBrowserStateManager> test_manager_;
  std::unique_ptr<web::FakeWebState> web_state_;
  raw_ptr<commerce::MockShoppingService> shopping_service_;
  raw_ptr<bookmarks::BookmarkModel> bookmark_model_;
  raw_ptr<BrowserList> browser_list_;
  std::unique_ptr<image_fetcher::ImageDataFetcher> image_fetcher_;
  std::unique_ptr<PushNotificationService> push_notification_service_;
  TestPriceNotificationsConsumer* consumer_ =
      [[TestPriceNotificationsConsumer alloc] init];
  TestPriceInsightsConsumer* price_insights_consumer_ =
      [[TestPriceInsightsConsumer alloc] init];
};

TEST_P(PriceNotificationsPriceTrackingMediatorTest,
       TrackableItemIsEmptyWhenUserIsViewingProductWebpageAndProduct) {
  PrepareSubscription(shopping_service_, true);
  mediator_.consumer = consumer_;

  ASSERT_TRUE(base::test::ios::WaitUntilConditionOrTimeout(
      base::test::ios::kWaitForActionTimeout, ^bool {
        base::RunLoop().RunUntilIdle();
        return consumer_.didExecuteAction;
      }));

  EXPECT_EQ(consumer_.trackableItem, nil);
  EXPECT_EQ(consumer_.isCurrentlyTrackingVisibleProduct, YES);
}

TEST_P(
    PriceNotificationsPriceTrackingMediatorTest,
    TrackableItemExistsWhenUserUntracksProductFromWebpageIsCurrentlyViewing) {
  commerce::ProductInfo product_info;
  product_info.title = kBookmarkTitle;
  product_info.product_cluster_id.emplace(12345L);
  std::optional<commerce::ProductInfo> optional_product_info;
  optional_product_info.emplace(product_info);
  shopping_service_->SetResponseForGetProductInfoForUrl(optional_product_info);
  mediator_.consumer = consumer_;

  ASSERT_TRUE(base::test::ios::WaitUntilConditionOrTimeout(
      base::test::ios::kWaitForActionTimeout, ^bool {
        base::RunLoop().RunUntilIdle();
        return consumer_.didExecuteAction;
      }));

  consumer_.didExecuteAction = NO;
  const bookmarks::BookmarkNode* default_folder =
      base::FeatureList::IsEnabled(
          syncer::kEnableBookmarkFoldersForAccountStorage)
          ? bookmark_model_->account_mobile_node()
          : bookmark_model_->mobile_node();
  bookmark_model_->AddURL(default_folder, default_folder->children().size(),
                          base::UTF8ToUTF16(product_info.title),
                          GURL(kTestUrl));
  shopping_service_->SetUnsubscribeCallbackValue(true);

  PriceNotificationsTableViewItem* product =
      [[PriceNotificationsTableViewItem alloc] init];
  product.title = base::SysUTF8ToNSString(kBookmarkTitle);
  product.entryURL = GURL(kTestUrl);
  id<PriceNotificationsMutator> mutator = mediator_;
  [mutator stopTrackingItem:product];
  ASSERT_TRUE(base::test::ios::WaitUntilConditionOrTimeout(
      base::test::ios::kWaitForActionTimeout, ^bool {
        base::RunLoop().RunUntilIdle();
        return consumer_.didExecuteAction;
      }));

  EXPECT_EQ(consumer_.trackableItem.title, product.title);
}

TEST_P(PriceNotificationsPriceTrackingMediatorTest,
       SuccessfullyTrackedProductURLFromPriceInsights) {
  commerce::ProductInfo product_info;
  product_info.title = kBookmarkTitle;
  product_info.product_cluster_id = std::make_optional(kClusterId);
  std::optional<commerce::ProductInfo> optional_product_info;
  optional_product_info.emplace(product_info);
  shopping_service_->SetResponseForGetProductInfoForUrl(optional_product_info);

  price_insights_consumer_.didPriceTrack = NO;
  mediator_.priceInsightsConsumer = price_insights_consumer_;
  [mediator_ priceInsightsTrackItem:GetPriceInsightsItem()];

  ASSERT_TRUE(base::test::ios::WaitUntilConditionOrTimeout(
      base::test::ios::kWaitForActionTimeout, ^bool {
        base::RunLoop().RunUntilIdle();
        return price_insights_consumer_.didPriceTrack;
      }));
}

TEST_P(PriceNotificationsPriceTrackingMediatorTest,
       PresentAlertWhenTrackingIsUnsuccessfulFromPriceInsights) {
  commerce::ProductInfo product_info;
  product_info.title = kBookmarkTitle;
  product_info.product_cluster_id = std::make_optional(kClusterId);
  std::optional<commerce::ProductInfo> optional_product_info;
  optional_product_info.emplace(product_info);
  shopping_service_->SetResponseForGetProductInfoForUrl(optional_product_info);
  shopping_service_->SetSubscribeCallbackValue(false);

  price_insights_consumer_.didPresentStartPriceTrackingErrorAlertForItem = NO;
  mediator_.priceInsightsConsumer = price_insights_consumer_;
  [mediator_ priceInsightsTrackItem:GetPriceInsightsItem()];

  ASSERT_TRUE(base::test::ios::WaitUntilConditionOrTimeout(
      base::test::ios::kWaitForActionTimeout, ^bool {
        base::RunLoop().RunUntilIdle();
        return price_insights_consumer_
            .didPresentStartPriceTrackingErrorAlertForItem;
      }));
}

TEST_P(PriceNotificationsPriceTrackingMediatorTest,
       SuccessfullyUntrackedProductURLFromPriceInsights) {
  commerce::ProductInfo product_info;
  product_info.title = kBookmarkTitle;
  product_info.product_cluster_id = std::make_optional(kClusterId);
  std::optional<commerce::ProductInfo> optional_product_info;
  optional_product_info.emplace(product_info);

  const bookmarks::BookmarkNode* default_folder =
      base::FeatureList::IsEnabled(
          syncer::kEnableBookmarkFoldersForAccountStorage)
          ? bookmark_model_->account_mobile_node()
          : bookmark_model_->mobile_node();
  bookmark_model_->AddURL(default_folder, default_folder->children().size(),
                          base::UTF8ToUTF16(product_info.title),
                          GURL(kTestUrl));
  shopping_service_->SetUnsubscribeCallbackValue(true);
  shopping_service_->SetResponseForGetProductInfoForUrl(optional_product_info);

  price_insights_consumer_.didPriceUntrack = NO;
  mediator_.priceInsightsConsumer = price_insights_consumer_;
  [mediator_ priceInsightsStopTrackingItem:GetPriceInsightsItem()];

  ASSERT_TRUE(base::test::ios::WaitUntilConditionOrTimeout(
      base::test::ios::kWaitForActionTimeout, ^bool {
        base::RunLoop().RunUntilIdle();
        return price_insights_consumer_.didPriceUntrack;
      }));
}

TEST_P(PriceNotificationsPriceTrackingMediatorTest,
       PresentAlertWhenUntrackingIsUnsuccessfulFromPriceInsights) {
  commerce::ProductInfo product_info;
  product_info.title = kBookmarkTitle;
  product_info.product_cluster_id = std::make_optional(kClusterId);
  std::optional<commerce::ProductInfo> optional_product_info;
  optional_product_info.emplace(product_info);

  const bookmarks::BookmarkNode* default_folder =
      base::FeatureList::IsEnabled(
          syncer::kEnableBookmarkFoldersForAccountStorage)
          ? bookmark_model_->account_mobile_node()
          : bookmark_model_->mobile_node();
  bookmark_model_->AddURL(default_folder, default_folder->children().size(),
                          base::UTF8ToUTF16(product_info.title),
                          GURL(kTestUrl));
  shopping_service_->SetUnsubscribeCallbackValue(false);
  shopping_service_->SetResponseForGetProductInfoForUrl(optional_product_info);

  price_insights_consumer_.didPresentStopPriceTrackingErrorAlertForItem = NO;
  mediator_.priceInsightsConsumer = price_insights_consumer_;
  [mediator_ priceInsightsStopTrackingItem:GetPriceInsightsItem()];

  ASSERT_TRUE(base::test::ios::WaitUntilConditionOrTimeout(
      base::test::ios::kWaitForActionTimeout, ^bool {
        base::RunLoop().RunUntilIdle();
        return price_insights_consumer_
            .didPresentStopPriceTrackingErrorAlertForItem;
      }));
}

TEST_P(PriceNotificationsPriceTrackingMediatorTest,
       NavigateToWebPageUponUserRequestsFromPriceInsights) {
  price_insights_consumer_.didNavigateToWebpage = NO;
  mediator_.priceInsightsConsumer = price_insights_consumer_;
  [mediator_ priceInsightsNavigateToWebpageForItem:GetPriceInsightsItem()];

  ASSERT_TRUE(base::test::ios::WaitUntilConditionOrTimeout(
      base::test::ios::kWaitForActionTimeout, ^bool {
        base::RunLoop().RunUntilIdle();
        return price_insights_consumer_.didNavigateToWebpage;
      }));
}

INSTANTIATE_TEST_SUITE_P(UnifiedBookmarkModel,
                         PriceNotificationsPriceTrackingMediatorTest,
                         testing::Bool());
