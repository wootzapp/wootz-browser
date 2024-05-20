// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/commerce/core/webui/shopping_service_handler.h"

#include <memory>
#include <vector>

#include "base/check_is_test.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/sequenced_task_runner.h"
#include "base/uuid.h"
#include "components/bookmarks/browser/bookmark_node.h"
#include "components/commerce/core/commerce_constants.h"
#include "components/commerce/core/commerce_feature_list.h"
#include "components/commerce/core/metrics/metrics_utils.h"
#include "components/commerce/core/price_tracking_utils.h"
#include "components/commerce/core/shopping_service.h"
#include "components/commerce/core/subscriptions/commerce_subscription.h"
#include "components/commerce/core/webui/webui_utils.h"
#include "components/feature_engagement/public/tracker.h"
#include "components/payments/core/currency_formatter.h"
#include "components/power_bookmarks/core/power_bookmark_utils.h"
#include "components/power_bookmarks/core/proto/power_bookmark_meta.pb.h"
#include "components/power_bookmarks/core/proto/shopping_specifics.pb.h"
#include "components/prefs/pref_service.h"
#include "components/url_formatter/elide_url.h"
#include "url/gurl.h"

namespace commerce {
namespace {

shopping_service::mojom::BookmarkProductInfoPtr BookmarkNodeToMojoProduct(
    bookmarks::BookmarkModel& model,
    const bookmarks::BookmarkNode* node,
    const std::string& locale) {
  auto bookmark_info = shopping_service::mojom::BookmarkProductInfo::New();
  bookmark_info->bookmark_id = node->id();

  std::unique_ptr<power_bookmarks::PowerBookmarkMeta> meta =
      power_bookmarks::GetNodePowerBookmarkMeta(&model, node);
  const power_bookmarks::ShoppingSpecifics specifics =
      meta->shopping_specifics();

  bookmark_info->info = shopping_service::mojom::ProductInfo::New();
  bookmark_info->info->title = specifics.title();
  bookmark_info->info->domain = base::UTF16ToUTF8(
      url_formatter::FormatUrlForDisplayOmitSchemePathAndTrivialSubdomains(
          GURL(node->url())));

  bookmark_info->info->product_url = node->url();
  bookmark_info->info->image_url = GURL(meta->lead_image().url());
  bookmark_info->info->cluster_id = specifics.product_cluster_id();

  const power_bookmarks::ProductPrice price = specifics.current_price();
  std::string currency_code = price.currency_code();

  std::unique_ptr<payments::CurrencyFormatter> formatter =
      std::make_unique<payments::CurrencyFormatter>(currency_code, locale);
  formatter->SetMaxFractionalDigits(2);

  bookmark_info->info->current_price =
      base::UTF16ToUTF8(formatter->Format(base::NumberToString(
          static_cast<float>(price.amount_micros()) / kToMicroCurrency)));

  // Only send the previous price if it is higher than the current price. This
  // is exclusively used to decide whether to show the price drop chip in the
  // UI.
  if (specifics.has_previous_price() &&
      specifics.previous_price().amount_micros() >
          specifics.current_price().amount_micros()) {
    const power_bookmarks::ProductPrice previous_price =
        specifics.previous_price();
    bookmark_info->info->previous_price =
        base::UTF16ToUTF8(formatter->Format(base::NumberToString(
            static_cast<float>(previous_price.amount_micros()) /
            kToMicroCurrency)));
  }

  return bookmark_info;
}

std::vector<shopping_service::mojom::UrlInfoPtr> UrlInfoToMojo(
    const std::vector<UrlInfo>& url_infos) {
  std::vector<shopping_service::mojom::UrlInfoPtr> url_info_ptr_list;

  for (const UrlInfo& url_info : url_infos) {
    auto url_info_ptr = shopping_service::mojom::UrlInfo::New();
    url_info_ptr->url = url_info.url;
    url_info_ptr->title = base::UTF16ToUTF8(url_info.title);
    url_info_ptr_list.push_back(std::move(url_info_ptr));
  }
  return url_info_ptr_list;
}

shopping_service::mojom::PriceInsightsInfoPtr PriceInsightsInfoToMojoObject(
    const std::optional<PriceInsightsInfo>& info,
    const std::string& locale) {
  auto insights_info = shopping_service::mojom::PriceInsightsInfo::New();

  if (!info.has_value()) {
    return insights_info;
  }

  if (info->product_cluster_id.has_value()) {
    insights_info->cluster_id = info->product_cluster_id.value();
  } else {
    CHECK_IS_TEST();
  }

  std::unique_ptr<payments::CurrencyFormatter> formatter =
      std::make_unique<payments::CurrencyFormatter>(info->currency_code,
                                                    locale);
  formatter->SetMaxFractionalDigits(2);

  if (info->typical_low_price_micros.has_value() &&
      info->typical_high_price_micros.has_value()) {
    insights_info->typical_low_price =
        base::UTF16ToUTF8(formatter->Format(base::NumberToString(
            static_cast<float>(info->typical_low_price_micros.value()) /
            kToMicroCurrency)));

    insights_info->typical_high_price =
        base::UTF16ToUTF8(formatter->Format(base::NumberToString(
            static_cast<float>(info->typical_high_price_micros.value()) /
            kToMicroCurrency)));
  }

  if (info->catalog_attributes.has_value()) {
    insights_info->catalog_attributes = info->catalog_attributes.value();
  }

  if (info->jackpot_url.has_value()) {
    insights_info->jackpot = info->jackpot_url.value();
  }

  shopping_service::mojom::PriceInsightsInfo::PriceBucket bucket;
  switch (info->price_bucket) {
    case PriceBucket::kUnknown:
      bucket =
          shopping_service::mojom::PriceInsightsInfo::PriceBucket::kUnknown;
      break;
    case PriceBucket::kLowPrice:
      bucket = shopping_service::mojom::PriceInsightsInfo::PriceBucket::kLow;
      break;
    case PriceBucket::kTypicalPrice:
      bucket =
          shopping_service::mojom::PriceInsightsInfo::PriceBucket::kTypical;
      break;
    case PriceBucket::kHighPrice:
      bucket = shopping_service::mojom::PriceInsightsInfo::PriceBucket::kHigh;
      break;
  }
  insights_info->bucket = bucket;

  insights_info->has_multiple_catalogs = info->has_multiple_catalogs;

  for (auto history_price : info->catalog_history_prices) {
    auto point = shopping_service::mojom::PricePoint::New();
    point->date = std::get<0>(history_price);

    auto price =
        static_cast<float>(std::get<1>(history_price)) / kToMicroCurrency;
    point->price = price;
    point->formatted_price =
        base::UTF16ToUTF8(formatter->Format(base::NumberToString(price)));
    insights_info->history.push_back(std::move(point));
  }

  insights_info->locale = locale;
  insights_info->currency_code = info->currency_code;

  return insights_info;
}

shopping_service::mojom::ProductSpecificationsPtr ProductSpecsToMojo(
    const ProductSpecifications& specs) {
  auto specs_ptr = shopping_service::mojom::ProductSpecifications::New();

  for (const auto& [id, name] : specs.product_dimension_map) {
    specs_ptr->product_dimension_map[id] = name;
  }

  for (const auto& product : specs.products) {
    auto product_ptr =
        shopping_service::mojom::ProductSpecificationsProduct::New();
    product_ptr->product_cluster_id = product.product_cluster_id;
    product_ptr->title = product.title;
    product_ptr->image_url = product.image_url;
    product_ptr->summary = product.summary;

    for (const auto& [dimen_id, value] : product.product_dimension_values) {
      product_ptr->product_dimension_values[dimen_id] = value;
    }

    specs_ptr->products.push_back(std::move(product_ptr));
  }

  return specs_ptr;
}

shopping_service::mojom::ProductSpecificationsSetPtr ProductSpecsSetToMojo(
    const ProductSpecificationsSet& set) {
  auto set_ptr = shopping_service::mojom::ProductSpecificationsSet::New();

  set_ptr->name = set.name();
  set_ptr->uuid = set.uuid();

  for (const auto& url : set.urls()) {
    set_ptr->urls.push_back(url);
  }

  return set_ptr;
}

}  // namespace

using shopping_service::mojom::BookmarkProductInfo;
using shopping_service::mojom::BookmarkProductInfoPtr;

ShoppingServiceHandler::ShoppingServiceHandler(
    mojo::PendingRemote<shopping_service::mojom::Page> remote_page,
    mojo::PendingReceiver<
        shopping_service::mojom::ShoppingServiceHandler> receiver,
    bookmarks::BookmarkModel* bookmark_model,
    ShoppingService* shopping_service,
    PrefService* prefs,
    feature_engagement::Tracker* tracker,
    std::unique_ptr<Delegate> delegate)
    : remote_page_(std::move(remote_page)),
      receiver_(this, std::move(receiver)),
      bookmark_model_(bookmark_model),
      shopping_service_(shopping_service),
      pref_service_(prefs),
      tracker_(tracker),
      delegate_(std::move(delegate)) {
  scoped_subscriptions_observation_.Observe(shopping_service_);
  scoped_bookmark_model_observation_.Observe(bookmark_model_);
  // It is safe to schedule updates and observe bookmarks. If the feature is
  // disabled, no new information will be fetched or provided to the frontend.
  shopping_service_->ScheduleSavedProductUpdate();

  if (shopping_service_->GetAccountChecker()) {
    locale_ = shopping_service_->GetAccountChecker()->GetLocale();
  }
}

ShoppingServiceHandler::~ShoppingServiceHandler() = default;

void ShoppingServiceHandler::GetAllPriceTrackedBookmarkProductInfo(
    GetAllPriceTrackedBookmarkProductInfoCallback callback) {
  shopping_service_->WaitForReady(base::BindOnce(
      [](base::WeakPtr<ShoppingServiceHandler> handler,
         GetAllPriceTrackedBookmarkProductInfoCallback callback,
         ShoppingService* service) {
        if (!service || !service->IsShoppingListEligible() ||
            handler.WasInvalidated()) {
          base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
              FROM_HERE, base::BindOnce(std::move(callback),
                                        std::vector<BookmarkProductInfoPtr>()));
          return;
        }

        service->GetAllPriceTrackedBookmarks(
            base::BindOnce(
                &ShoppingServiceHandler::OnFetchPriceTrackedBookmarks,
                    handler, std::move(callback)));
      },
      weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
}

void ShoppingServiceHandler::OnFetchPriceTrackedBookmarks(
    GetAllPriceTrackedBookmarkProductInfoCallback callback,
    std::vector<const bookmarks::BookmarkNode*> bookmarks) {
  std::vector<BookmarkProductInfoPtr> info_list =
      BookmarkListToMojoList(*bookmark_model_, bookmarks, locale_);

  if (!info_list.empty()) {
    // Record usage for price tracking promo.
    tracker_->NotifyEvent("price_tracking_side_panel_shown");
  }

  std::move(callback).Run(std::move(info_list));
}

void ShoppingServiceHandler::GetAllShoppingBookmarkProductInfo(
    GetAllShoppingBookmarkProductInfoCallback callback) {
  shopping_service_->WaitForReady(base::BindOnce(
      [](base::WeakPtr<ShoppingServiceHandler> handler,
         GetAllShoppingBookmarkProductInfoCallback callback,
         ShoppingService* service) {
        if (!service || !service->IsShoppingListEligible() ||
            handler.WasInvalidated()) {
          std::move(callback).Run({});
          return;
        }

        std::vector<const bookmarks::BookmarkNode*> bookmarks =
            service->GetAllShoppingBookmarks();

        std::vector<BookmarkProductInfoPtr> info_list = BookmarkListToMojoList(
            *(handler->bookmark_model_), bookmarks, handler->locale_);

        std::move(callback).Run(std::move(info_list));
      },
      weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
}

void ShoppingServiceHandler::TrackPriceForBookmark(int64_t bookmark_id) {
  commerce::SetPriceTrackingStateForBookmark(
      shopping_service_, bookmark_model_,
      bookmarks::GetBookmarkNodeByID(bookmark_model_, bookmark_id), true,
      base::BindOnce(&ShoppingServiceHandler::onPriceTrackResult,
                     weak_ptr_factory_.GetWeakPtr(), bookmark_id,
                     bookmark_model_, true));
}

void ShoppingServiceHandler::UntrackPriceForBookmark(int64_t bookmark_id) {
  commerce::SetPriceTrackingStateForBookmark(
      shopping_service_, bookmark_model_,
      bookmarks::GetBookmarkNodeByID(bookmark_model_, bookmark_id), false,
      base::BindOnce(&ShoppingServiceHandler::onPriceTrackResult,
                     weak_ptr_factory_.GetWeakPtr(), bookmark_id,
                     bookmark_model_, false));
}

void ShoppingServiceHandler::OnSubscribe(
    const CommerceSubscription& subscription,
    bool succeeded) {
  if (succeeded) {
    HandleSubscriptionChange(subscription, true);
  }
}

void ShoppingServiceHandler::OnUnsubscribe(
    const CommerceSubscription& subscription,
    bool succeeded) {
  if (succeeded) {
    HandleSubscriptionChange(subscription, false);
  }
}

void ShoppingServiceHandler::BookmarkModelChanged() {}

void ShoppingServiceHandler::BookmarkNodeMoved(
    const bookmarks::BookmarkNode* old_parent,
    size_t old_index,
    const bookmarks::BookmarkNode* new_parent,
    size_t new_index) {
  const bookmarks::BookmarkNode* node = new_parent->children()[new_index].get();
  if (!node) {
    return;
  }
  std::unique_ptr<power_bookmarks::PowerBookmarkMeta> meta =
      power_bookmarks::GetNodePowerBookmarkMeta(bookmark_model_, node);
  if (!meta || !meta->has_shopping_specifics() ||
      !meta->shopping_specifics().has_product_cluster_id()) {
    return;
  }
  remote_page_->OnProductBookmarkMoved(
      BookmarkNodeToMojoProduct(*bookmark_model_, node, locale_));
}

void ShoppingServiceHandler::HandleSubscriptionChange(
    const CommerceSubscription& sub,
    bool is_tracking) {
  if (sub.id_type != IdentifierType::kProductClusterId) {
    return;
  }

  uint64_t cluster_id;
  if (!base::StringToUint64(sub.id, &cluster_id)) {
    return;
  }

  std::vector<const bookmarks::BookmarkNode*> bookmarks =
      GetBookmarksWithClusterId(bookmark_model_, cluster_id);
  // Special handling when the unsubscription is caused by bookmark deletion and
  // therefore the bookmark can no longer be retrieved.
  // TODO(crbug.com/40066977): Update mojo call to pass cluster ID and make
  // BookmarkProductInfo a nullable parameter.
  if (!bookmarks.size()) {
    auto bookmark_info = shopping_service::mojom::BookmarkProductInfo::New();
    bookmark_info->info = shopping_service::mojom::ProductInfo::New();
    bookmark_info->info->cluster_id = cluster_id;
    remote_page_->PriceUntrackedForBookmark(std::move(bookmark_info));
    return;
  }
  for (auto* node : bookmarks) {
    auto product = BookmarkNodeToMojoProduct(*bookmark_model_, node, locale_);
    if (is_tracking) {
      remote_page_->PriceTrackedForBookmark(std::move(product));
    } else {
      remote_page_->PriceUntrackedForBookmark(std::move(product));
    }
  }
}

std::vector<BookmarkProductInfoPtr>
ShoppingServiceHandler::BookmarkListToMojoList(
    bookmarks::BookmarkModel& model,
    const std::vector<const bookmarks::BookmarkNode*>& bookmarks,
    const std::string& locale) {
  std::vector<BookmarkProductInfoPtr> info_list;

  for (const bookmarks::BookmarkNode* node : bookmarks) {
    info_list.push_back(BookmarkNodeToMojoProduct(model, node, locale));
  }

  return info_list;
}

void ShoppingServiceHandler::onPriceTrackResult(int64_t bookmark_id,
                                             bookmarks::BookmarkModel* model,
                                             bool is_tracking,
                                             bool success) {
  if (success)
    return;

  // We only do work here if price tracking failed. When the UI is interacted
  // with, we assume success. In the event it failed, we switch things back.
  // So in this case, if we were trying to untrack and that action failed, set
  // the UI back to "tracking".
  auto* node = bookmarks::GetBookmarkNodeByID(bookmark_model_, bookmark_id);
  auto product = BookmarkNodeToMojoProduct(*bookmark_model_, node, locale_);

  if (!is_tracking) {
    remote_page_->PriceTrackedForBookmark(std::move(product));
  } else {
    remote_page_->PriceUntrackedForBookmark(std::move(product));
  }
  // Pass in whether the failed operation was to track or untrack price. It
  // should be the reverse of the current tracking status since the operation
  // failed.
  remote_page_->OperationFailedForBookmark(
      BookmarkNodeToMojoProduct(*bookmark_model_, node, locale_), is_tracking);
}

void ShoppingServiceHandler::GetProductInfoForCurrentUrl(
    GetProductInfoForCurrentUrlCallback callback) {
  if (!shopping_service_->IsPriceInsightsEligible() || !delegate_ ||
      !delegate_->GetCurrentTabUrl().has_value()) {
    std::move(callback).Run(shopping_service::mojom::ProductInfo::New());
    return;
  }

  shopping_service_->GetProductInfoForUrl(
      delegate_->GetCurrentTabUrl().value(),
      base::BindOnce(
          [](base::WeakPtr<ShoppingServiceHandler> handler,
             GetProductInfoForCurrentUrlCallback callback, const GURL& url,
             const std::optional<const ProductInfo>& info) {
            if (!handler) {
              std::move(callback).Run(
                  shopping_service::mojom::ProductInfo::New());
              return;
            }

            std::move(callback).Run(
                ProductInfoToMojoProduct(url, info, handler->locale_));
          },
          weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
}

void ShoppingServiceHandler::GetProductInfoForUrl(
    const GURL& url,
    GetProductInfoForUrlCallback callback) {
  shopping_service_->GetProductInfoForUrl(
      url, base::BindOnce(
               [](base::WeakPtr<ShoppingServiceHandler> handler,
                  GetProductInfoForUrlCallback callback, const GURL& url,
                  const std::optional<const ProductInfo>& info) {
                 if (!handler) {
                   std::move(callback).Run(
                       url, shopping_service::mojom::ProductInfo::New());
                   return;
                 }

                 std::move(callback).Run(url, ProductInfoToMojoProduct(
                                                  url, info, handler->locale_));
               },
               weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
}

void ShoppingServiceHandler::IsShoppingListEligible(
    IsShoppingListEligibleCallback callback) {
  std::move(callback).Run(shopping_service_->IsShoppingListEligible());
}

void ShoppingServiceHandler::GetShoppingCollectionBookmarkFolderId(
    GetShoppingCollectionBookmarkFolderIdCallback callback) {
  const bookmarks::BookmarkNode* collection =
      commerce::GetShoppingCollectionBookmarkFolder(bookmark_model_);
  std::move(callback).Run(collection ? collection->id() : -1);
}

void ShoppingServiceHandler::GetPriceTrackingStatusForCurrentUrl(
    GetPriceTrackingStatusForCurrentUrlCallback callback) {
  // The URL may or may not have a bookmark associated with it. Prioritize
  // accessing the product info for the URL before looking at an existing
  // bookmark.
  shopping_service_->GetProductInfoForUrl(
      delegate_->GetCurrentTabUrl().value(),
      base::BindOnce(
          [](base::WeakPtr<ShoppingServiceHandler> handler,
             GetPriceTrackingStatusForCurrentUrlCallback callback,
             const GURL& url, const std::optional<const ProductInfo>& info) {
            if (!info.has_value() || !info->product_cluster_id.has_value() ||
                !handler || !CanTrackPrice(info)) {
              std::move(callback).Run(false);
              return;
            }
            CommerceSubscription sub(
                SubscriptionType::kPriceTrack,
                IdentifierType::kProductClusterId,
                base::NumberToString(info->product_cluster_id.value()),
                ManagementType::kUserManaged);

            handler->shopping_service_->IsSubscribed(
                sub,
                base::BindOnce(
                    [](GetPriceTrackingStatusForCurrentUrlCallback callback,
                       bool subscribed) {
                      std::move(callback).Run(subscribed);
                    },
                    std::move(callback)));
          },
          weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
}

void ShoppingServiceHandler::SetPriceTrackingStatusForCurrentUrl(bool track) {
  if (track) {
    // If the product on the page isn't already tracked, create a bookmark for
    // it and start tracking.
    TrackPriceForBookmark(delegate_->GetOrAddBookmarkForCurrentUrl()->id());
    commerce::metrics::RecordShoppingActionUKM(
        delegate_->GetCurrentTabUkmSourceId(),
        commerce::metrics::ShoppingAction::kPriceTracked);
  } else {
    // If the product is already tracked, there must be a bookmark, but it's not
    // necessarily the page the user is currently on (i.e. multi-merchant
    // tracking). Prioritize accessing the product info for the URL before
    // attempting to access the bookmark.

    base::OnceCallback<void(uint64_t)> unsubscribe = base::BindOnce(
        [](base::WeakPtr<ShoppingServiceHandler> handler, uint64_t id) {
          if (!handler) {
            return;
          }

          commerce::SetPriceTrackingStateForClusterId(
              handler->shopping_service_, handler->bookmark_model_, id, false,
              base::BindOnce([](bool success) {}));
        },
        weak_ptr_factory_.GetWeakPtr());

    shopping_service_->GetProductInfoForUrl(
        delegate_->GetCurrentTabUrl().value(),
        base::BindOnce(
            [](base::WeakPtr<ShoppingServiceHandler> handler,
               base::OnceCallback<void(uint64_t)> unsubscribe, const GURL& url,
               const std::optional<const ProductInfo>& info) {
              if (!handler) {
                return;
              }

              if (!info.has_value() || !info->product_cluster_id.has_value()) {
                std::optional<uint64_t> cluster_id =
                    GetProductClusterIdFromBookmark(url,
                                                    handler->bookmark_model_);

                if (cluster_id.has_value()) {
                  std::move(unsubscribe).Run(cluster_id.value());
                }

                return;
              }

              std::move(unsubscribe).Run(info->product_cluster_id.value());
            },
            weak_ptr_factory_.GetWeakPtr(), std::move(unsubscribe)));
  }
}

void ShoppingServiceHandler::GetParentBookmarkFolderNameForCurrentUrl(
    GetParentBookmarkFolderNameForCurrentUrlCallback callback) {
  const GURL current_url = delegate_->GetCurrentTabUrl().value();
  std::move(callback).Run(
      commerce::GetBookmarkParentName(bookmark_model_, current_url)
          .value_or(std::u16string()));
}

void ShoppingServiceHandler::ShowBookmarkEditorForCurrentUrl() {
  delegate_->ShowBookmarkEditorForCurrentUrl();
}

void ShoppingServiceHandler::GetPriceInsightsInfoForCurrentUrl(
    GetPriceInsightsInfoForCurrentUrlCallback callback) {
  if (!shopping_service_->IsPriceInsightsEligible() || !delegate_ ||
      !delegate_->GetCurrentTabUrl().has_value()) {
    std::move(callback).Run(shopping_service::mojom::PriceInsightsInfo::New());
    return;
  }

  shopping_service_->GetPriceInsightsInfoForUrl(
      delegate_->GetCurrentTabUrl().value(),
      base::BindOnce(
          &ShoppingServiceHandler::OnFetchPriceInsightsInfoForCurrentUrl,
          weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
}

void ShoppingServiceHandler::GetProductSpecificationsForUrls(
    const std::vector<::GURL>& urls,
    GetProductSpecificationsForUrlsCallback callback) {
  if (!shopping_service_ || urls.empty()) {
    std::move(callback).Run(
        shopping_service::mojom::ProductSpecifications::New());
    return;
  }

  shopping_service_->GetProductSpecificationsForUrls(
      urls, base::BindOnce(
                [](base::WeakPtr<ShoppingServiceHandler> handler,
                   GetProductSpecificationsForUrlsCallback callback,
                   std::vector<uint64_t> ids,
                   std::optional<ProductSpecifications> specs) {
                  if (!handler || !specs.has_value()) {
                    std::move(callback).Run(
                        shopping_service::mojom::ProductSpecifications::New());
                    return;
                  }

                  std::move(callback).Run(ProductSpecsToMojo(specs.value()));
                },
                weak_ptr_factory_.GetWeakPtr(), std::move(callback)));
}

void ShoppingServiceHandler::GetUrlInfosForOpenTabs(
    GetUrlInfosForOpenTabsCallback callback) {
  if (!shopping_service_) {
    std::move(callback).Run({});
    return;
  }

  std::move(callback).Run(
      UrlInfoToMojo(shopping_service_->GetUrlInfosForActiveWebWrappers()));
}

void ShoppingServiceHandler::GetUrlInfosForRecentlyViewedTabs(
    GetUrlInfosForRecentlyViewedTabsCallback callback) {
  if (!shopping_service_) {
    std::move(callback).Run({});
    return;
  }

  std::move(callback).Run(UrlInfoToMojo(
      shopping_service_->GetUrlInfosForRecentlyViewedWebWrappers()));
}

void ShoppingServiceHandler::OnFetchPriceInsightsInfoForCurrentUrl(
    GetPriceInsightsInfoForCurrentUrlCallback callback,
    const GURL& url,
    const std::optional<PriceInsightsInfo>& info) {
  std::move(callback).Run(PriceInsightsInfoToMojoObject(info, locale_));
}

void ShoppingServiceHandler::ShowInsightsSidePanelUI() {
  if (delegate_) {
    delegate_->ShowInsightsSidePanelUI();
  }
}

void ShoppingServiceHandler::OnGetPriceTrackingStatusForCurrentUrl(
    GetPriceTrackingStatusForCurrentUrlCallback callback,
    bool tracked) {
  std::move(callback).Run(tracked);
}

void ShoppingServiceHandler::OpenUrlInNewTab(const GURL& url) {
  if (delegate_) {
    delegate_->OpenUrlInNewTab(url);
  }
}

void ShoppingServiceHandler::ShowFeedback() {
  if (delegate_) {
    delegate_->ShowFeedback();
  }
}

void ShoppingServiceHandler::GetAllProductSpecificationsSets(
    GetAllProductSpecificationsSetsCallback callback) {
  if (!shopping_service_ ||
      !shopping_service_->GetProductSpecificationsService()) {
    std::move(callback).Run({});
    return;
  }

  const auto& all_sets = shopping_service_->GetProductSpecificationsService()
                             ->GetAllProductSpecifications();
  std::vector<shopping_service::mojom::ProductSpecificationsSetPtr>
      all_sets_mojo;
  for (const auto& set : all_sets) {
    all_sets_mojo.push_back(ProductSpecsSetToMojo(set));
  }

  std::move(callback).Run(std::move(all_sets_mojo));
}

void ShoppingServiceHandler::GetProductSpecificationsSetByUuid(
    const base::Uuid& uuid,
    GetProductSpecificationsSetByUuidCallback callback) {
  if (!shopping_service_ ||
      !shopping_service_->GetProductSpecificationsService()) {
    std::move(callback).Run(nullptr);
    return;
  }
  const auto& set =
      shopping_service_->GetProductSpecificationsService()->GetSetByUuid(uuid);
  if (set.has_value()) {
    std::move(callback).Run(ProductSpecsSetToMojo(set.value()));
  } else {
    std::move(callback).Run(nullptr);
  }
}

void ShoppingServiceHandler::AddProductSpecificationsSet(
    const std::string& name,
    const std::vector<GURL>& urls,
    AddProductSpecificationsSetCallback callback) {
  if (!shopping_service_ ||
      !shopping_service_->GetProductSpecificationsService()) {
    std::move(callback).Run(nullptr);
    return;
  }

  std::optional<ProductSpecificationsSet> new_set =
      shopping_service_->GetProductSpecificationsService()
          ->AddProductSpecificationsSet(name, urls);

  std::move(callback).Run(
      new_set.has_value() ? ProductSpecsSetToMojo(new_set.value()) : nullptr);
}

void ShoppingServiceHandler::DeleteProductSpecificationsSet(
    const base::Uuid& uuid) {
  if (!shopping_service_ ||
      !shopping_service_->GetProductSpecificationsService()) {
    return;
  }

  shopping_service_->GetProductSpecificationsService()
      ->DeleteProductSpecificationsSet(uuid.AsLowercaseString());
}

void ShoppingServiceHandler::OnProductSpecificationsSetAdded(
    const ProductSpecificationsSet& set) {
  remote_page_->OnProductSpecificationsSetAdded(ProductSpecsSetToMojo(set));
}

void ShoppingServiceHandler::OnProductSpecificationsSetUpdate(
    const ProductSpecificationsSet& before,
    const ProductSpecificationsSet& set) {
  remote_page_->OnProductSpecificationsSetUpdated(ProductSpecsSetToMojo(set));
}

void ShoppingServiceHandler::OnProductSpecificationsSetRemoved(
    const ProductSpecificationsSet& set) {
  remote_page_->OnProductSpecificationsSetRemoved(set.uuid());
}

}  // namespace commerce
