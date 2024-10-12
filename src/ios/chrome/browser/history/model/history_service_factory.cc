// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/history/model/history_service_factory.h"

#include <utility>

#include "base/feature_list.h"
#include "base/no_destructor.h"
#include "components/history/core/browser/history_database_params.h"
#include "components/history/core/browser/history_service.h"
#include "components/history/core/browser/visit_delegate.h"
#include "components/history/core/common/pref_names.h"
#include "components/history/ios/browser/history_database_helper.h"
#include "components/keyed_service/core/service_access_type.h"
#include "components/keyed_service/ios/browser_state_dependency_manager.h"
#include "components/prefs/pref_service.h"
#include "components/sync/base/features.h"
#include "ios/chrome/browser/bookmarks/model/account_bookmark_model_factory.h"
#include "ios/chrome/browser/bookmarks/model/bookmark_model_factory.h"
#include "ios/chrome/browser/bookmarks/model/local_or_syncable_bookmark_model_factory.h"
#include "ios/chrome/browser/history/model/history_client_impl.h"
#include "ios/chrome/browser/shared/model/browser_state/browser_state_otr_helper.h"
#include "ios/chrome/browser/shared/model/browser_state/chrome_browser_state.h"
#include "ios/chrome/common/channel_info.h"

namespace ios {

namespace {

std::unique_ptr<HistoryClientImpl> BuildHistoryClientWithUnifiedBookmarkModel(
    ChromeBrowserState* browser_state) {
  CHECK(base::FeatureList::IsEnabled(
      syncer::kEnableBookmarkFoldersForAccountStorage));
  return std::make_unique<HistoryClientImpl>(
      BookmarkModelFactory::GetModelForBrowserStateIfUnificationEnabledOrDie(
          browser_state),
      /*account_bookmark_model=*/nullptr);
}

std::unique_ptr<HistoryClientImpl> BuildHistoryClientWithTwoBookmarkModels(
    ChromeBrowserState* browser_state) {
  CHECK(!base::FeatureList::IsEnabled(
      syncer::kEnableBookmarkFoldersForAccountStorage));
  return std::make_unique<HistoryClientImpl>(
      LocalOrSyncableBookmarkModelFactory::
          GetDedicatedUnderlyingModelForBrowserStateIfUnificationDisabledOrDie(
              browser_state),
      AccountBookmarkModelFactory::
          GetDedicatedUnderlyingModelForBrowserStateIfUnificationDisabledOrDie(
              browser_state));
}

std::unique_ptr<HistoryClientImpl> BuildHistoryClient(
    ChromeBrowserState* browser_state) {
  if (base::FeatureList::IsEnabled(
          syncer::kEnableBookmarkFoldersForAccountStorage)) {
    return BuildHistoryClientWithUnifiedBookmarkModel(browser_state);
  }

  return BuildHistoryClientWithTwoBookmarkModels(browser_state);
}

std::unique_ptr<KeyedService> BuildHistoryService(web::BrowserState* context) {
  ChromeBrowserState* browser_state =
      ChromeBrowserState::FromBrowserState(context);
  std::unique_ptr<history::HistoryService> history_service(
      new history::HistoryService(BuildHistoryClient(browser_state), nullptr));
  if (!history_service->Init(history::HistoryDatabaseParamsForPath(
          browser_state->GetStatePath(), GetChannel()))) {
    return nullptr;
  }
  return history_service;
}

}  // namespace

// static
history::HistoryService* HistoryServiceFactory::GetForBrowserState(
    ChromeBrowserState* browser_state,
    ServiceAccessType access_type) {
  // If saving history is disabled, only allow explicit access.
  if (access_type != ServiceAccessType::EXPLICIT_ACCESS &&
      browser_state->GetPrefs()->GetBoolean(
          prefs::kSavingBrowserHistoryDisabled)) {
    return nullptr;
  }

  return static_cast<history::HistoryService*>(
      GetInstance()->GetServiceForBrowserState(browser_state, true));
}

// static
history::HistoryService* HistoryServiceFactory::GetForBrowserStateIfExists(
    ChromeBrowserState* browser_state,
    ServiceAccessType access_type) {
  // If saving history is disabled, only allow explicit access.
  if (access_type != ServiceAccessType::EXPLICIT_ACCESS &&
      browser_state->GetPrefs()->GetBoolean(
          prefs::kSavingBrowserHistoryDisabled)) {
    return nullptr;
  }

  return static_cast<history::HistoryService*>(
      GetInstance()->GetServiceForBrowserState(browser_state, true));
}

// static
HistoryServiceFactory* HistoryServiceFactory::GetInstance() {
  static base::NoDestructor<HistoryServiceFactory> instance;
  return instance.get();
}

// static
HistoryServiceFactory::TestingFactory
HistoryServiceFactory::GetDefaultFactory() {
  return base::BindRepeating(&BuildHistoryService);
}

HistoryServiceFactory::HistoryServiceFactory()
    : BrowserStateKeyedServiceFactory(
          "HistoryService",
          BrowserStateDependencyManager::GetInstance()) {
  if (base::FeatureList::IsEnabled(
          syncer::kEnableBookmarkFoldersForAccountStorage)) {
    DependsOn(BookmarkModelFactory::GetInstance());
  } else {
    DependsOn(AccountBookmarkModelFactory::GetInstance());
    DependsOn(LocalOrSyncableBookmarkModelFactory::GetInstance());
  }
}

HistoryServiceFactory::~HistoryServiceFactory() {
}

std::unique_ptr<KeyedService> HistoryServiceFactory::BuildServiceInstanceFor(
    web::BrowserState* context) const {
  return BuildHistoryService(context);
}

web::BrowserState* HistoryServiceFactory::GetBrowserStateToUse(
    web::BrowserState* context) const {
  return GetBrowserStateRedirectedInIncognito(context);
}

bool HistoryServiceFactory::ServiceIsNULLWhileTesting() const {
  return true;
}

}  // namespace ios
