// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/browser_state/model/browser_state_keyed_service_factories.h"

#import "base/feature_list.h"
#import "components/page_content_annotations/core/page_content_annotations_features.h"
#import "ios/chrome/browser/affiliations/model/ios_chrome_affiliation_service_factory.h"
#import "ios/chrome/browser/autocomplete/model/autocomplete_classifier_factory.h"
#import "ios/chrome/browser/autocomplete/model/in_memory_url_index_factory.h"
#import "ios/chrome/browser/autocomplete/model/shortcuts_backend_factory.h"
#import "ios/chrome/browser/autofill/model/personal_data_manager_factory.h"
#import "ios/chrome/browser/bookmarks/model/account_bookmark_model_factory.h"
#import "ios/chrome/browser/bookmarks/model/account_bookmark_sync_service_factory.h"
#import "ios/chrome/browser/bookmarks/model/bookmark_model_factory.h"
#import "ios/chrome/browser/bookmarks/model/bookmark_undo_service_factory.h"
#import "ios/chrome/browser/bookmarks/model/local_or_syncable_bookmark_model_factory.h"
#import "ios/chrome/browser/bookmarks/model/local_or_syncable_bookmark_sync_service_factory.h"
#import "ios/chrome/browser/bookmarks/model/managed_bookmark_service_factory.h"
#import "ios/chrome/browser/bring_android_tabs/model/bring_android_tabs_to_ios_service_factory.h"
#import "ios/chrome/browser/browsing_data/model/browsing_data_remover_factory.h"
#import "ios/chrome/browser/commerce/model/shopping_service_factory.h"
#import "ios/chrome/browser/consent_auditor/model/consent_auditor_factory.h"
#import "ios/chrome/browser/content_notification/model/content_notification_service_factory.h"
#import "ios/chrome/browser/content_settings/model/cookie_settings_factory.h"
#import "ios/chrome/browser/contextual_panel/model/contextual_panel_model_service_factory.h"
#import "ios/chrome/browser/contextual_panel/sample/model/sample_panel_model_factory.h"
#import "ios/chrome/browser/crash_report/model/breadcrumbs/breadcrumb_manager_keyed_service_factory.h"
#import "ios/chrome/browser/credential_provider/model/credential_provider_buildflags.h"
#import "ios/chrome/browser/device_sharing/model/device_sharing_manager_factory.h"
#import "ios/chrome/browser/discover_feed/model/discover_feed_service_factory.h"
#import "ios/chrome/browser/dom_distiller/model/dom_distiller_service_factory.h"
#import "ios/chrome/browser/download/model/background_service/background_download_service_factory.h"
#import "ios/chrome/browser/download/model/browser_download_service_factory.h"
#import "ios/chrome/browser/drive/model/drive_service_factory.h"
#import "ios/chrome/browser/enterprise/model/idle/idle_service_factory.h"
#import "ios/chrome/browser/favicon/model/favicon_service_factory.h"
#import "ios/chrome/browser/favicon/model/ios_chrome_favicon_loader_factory.h"
#import "ios/chrome/browser/favicon/model/ios_chrome_large_icon_cache_factory.h"
#import "ios/chrome/browser/favicon/model/ios_chrome_large_icon_service_factory.h"
#import "ios/chrome/browser/feature_engagement/model/tracker_factory.h"
#import "ios/chrome/browser/follow/model/follow_service_factory.h"
#import "ios/chrome/browser/gcm/model/ios_chrome_gcm_profile_service_factory.h"
#import "ios/chrome/browser/google/model/google_logo_service_factory.h"
#import "ios/chrome/browser/history/model/domain_diversity_reporter_factory.h"
#import "ios/chrome/browser/history/model/history_service_factory.h"
#import "ios/chrome/browser/history/model/top_sites_factory.h"
#import "ios/chrome/browser/history/model/web_history_service_factory.h"
#import "ios/chrome/browser/invalidation/model/ios_chrome_profile_invalidation_provider_factory.h"
#import "ios/chrome/browser/language/model/accept_languages_service_factory.h"
#import "ios/chrome/browser/language/model/language_model_manager_factory.h"
#import "ios/chrome/browser/language/model/url_language_histogram_factory.h"
#import "ios/chrome/browser/mailto_handler/model/mailto_handler_service_factory.h"
#import "ios/chrome/browser/metrics/model/google_groups_updater_service_factory.h"
#import "ios/chrome/browser/metrics/model/ios_profile_session_durations_service_factory.h"
#import "ios/chrome/browser/optimization_guide/model/optimization_guide_service_factory.h"
#import "ios/chrome/browser/page_content_annotations/model/page_content_annotations_service_factory.h"
#import "ios/chrome/browser/page_info/about_this_site_service_factory.h"
#import "ios/chrome/browser/passwords/model/ios_chrome_account_password_store_factory.h"
#import "ios/chrome/browser/passwords/model/ios_chrome_password_check_manager_factory.h"
#import "ios/chrome/browser/passwords/model/ios_chrome_profile_password_store_factory.h"
#import "ios/chrome/browser/photos/model/photos_service_factory.h"
#import "ios/chrome/browser/plus_addresses/model/plus_address_service_factory.h"
#import "ios/chrome/browser/policy/model/cloud/user_policy_signin_service_factory.h"
#import "ios/chrome/browser/policy_url_blocking/model/policy_url_blocking_service.h"
#import "ios/chrome/browser/price_insights/model/price_insights_model_factory.h"
#import "ios/chrome/browser/promos_manager/model/promos_manager_factory.h"
#import "ios/chrome/browser/push_notification/model/push_notification_browser_state_service_factory.h"
#import "ios/chrome/browser/reading_list/model/reading_list_model_factory.h"
#import "ios/chrome/browser/safe_browsing/model/chrome_password_protection_service_factory.h"
#import "ios/chrome/browser/safe_browsing/model/real_time_url_lookup_service_factory.h"
#import "ios/chrome/browser/safe_browsing/model/safe_browsing_client_factory.h"
#import "ios/chrome/browser/safe_browsing/model/safe_browsing_metrics_collector_factory.h"
#import "ios/chrome/browser/safe_browsing/model/verdict_cache_manager_factory.h"
#import "ios/chrome/browser/safety_check/model/ios_chrome_safety_check_manager_factory.h"
#import "ios/chrome/browser/screen_time/model/screen_time_buildflags.h"
#import "ios/chrome/browser/search_engines/model/search_engine_choice_service_factory.h"
#import "ios/chrome/browser/search_engines/model/template_url_service_factory.h"
#import "ios/chrome/browser/segmentation_platform/model/segmentation_platform_service_factory.h"
#import "ios/chrome/browser/sessions/session_restoration_service_factory.h"
#import "ios/chrome/browser/shared/model/browser/browser_list_factory.h"
#import "ios/chrome/browser/shared/public/features/features.h"
#import "ios/chrome/browser/signin/model/about_signin_internals_factory.h"
#import "ios/chrome/browser/signin/model/account_consistency_service_factory.h"
#import "ios/chrome/browser/signin/model/account_reconcilor_factory.h"
#import "ios/chrome/browser/signin/model/authentication_service_factory.h"
#import "ios/chrome/browser/signin/model/chrome_account_manager_service_factory.h"
#import "ios/chrome/browser/signin/model/identity_manager_factory.h"
#import "ios/chrome/browser/signin/model/signin_browser_state_info_updater_factory.h"
#import "ios/chrome/browser/signin/model/signin_client_factory.h"
#import "ios/chrome/browser/signin/model/signin_error_controller_factory.h"
#import "ios/chrome/browser/signin/model/trusted_vault_client_backend_factory.h"
#import "ios/chrome/browser/supervised_user/model/child_account_service_factory.h"
#import "ios/chrome/browser/supervised_user/model/list_family_members_service_factory.h"
#import "ios/chrome/browser/supervised_user/model/supervised_user_metrics_service_factory.h"
#import "ios/chrome/browser/supervised_user/model/supervised_user_service_factory.h"
#import "ios/chrome/browser/supervised_user/model/supervised_user_settings_service_factory.h"
#import "ios/chrome/browser/sync/model/ios_user_event_service_factory.h"
#import "ios/chrome/browser/sync/model/model_type_store_service_factory.h"
#import "ios/chrome/browser/sync/model/sync_service_factory.h"
#import "ios/chrome/browser/tabs_search/model/tabs_search_service_factory.h"
#import "ios/chrome/browser/text_selection/model/text_classifier_model_service_factory.h"
#import "ios/chrome/browser/translate/model/translate_ranker_factory.h"
#import "ios/chrome/browser/trusted_vault/model/ios_trusted_vault_service_factory.h"
#import "ios/chrome/browser/unified_consent/model/unified_consent_service_factory.h"
#import "ios/chrome/browser/unit_conversion/unit_conversion_service_factory.h"
#import "ios/chrome/browser/voice/ui_bundled/text_to_speech_playback_controller_factory.h"
#import "ios/chrome/browser/webdata_services/model/web_data_service_factory.h"

#if BUILDFLAG(IOS_CREDENTIAL_PROVIDER_ENABLED)
#import "ios/chrome/browser/credential_provider/model/credential_provider_service_factory.h"
#endif

#if BUILDFLAG(IOS_SCREEN_TIME_ENABLED)
#import "ios/chrome/browser/screen_time/model/features.h"
#import "ios/chrome/browser/screen_time/model/screen_time_history_deleter_factory.h"
#endif

// This method gets the instance of each ServiceFactory. We do this so that
// each ServiceFactory initializes itself and registers its dependencies with
// the global PreferenceDependencyManager. We need to have a complete
// dependency graph when we create a browser state so we can dispatch the
// browser state creation message to the services that want to create their
// services at browser state creation time.
//
// TODO(erg): This needs to be something else. I don't think putting every
// FooServiceFactory here will scale or is desirable long term.
void EnsureBrowserStateKeyedServiceFactoriesBuilt() {
  // Keep this list alphabetized -- namespaced factories first, followed by
  // non-namespaced factories.
  autofill::PersonalDataManagerFactory::GetInstance();
  commerce::ShoppingServiceFactory::GetInstance();
  dom_distiller::DomDistillerServiceFactory::GetInstance();
  enterprise_idle::IdleServiceFactory::GetInstance();
  feature_engagement::TrackerFactory::GetInstance();
  ios::AboutSigninInternalsFactory::GetInstance();
  ios::AccountBookmarkSyncServiceFactory::GetInstance();
  ios::AccountConsistencyServiceFactory::GetInstance();
  ios::AccountReconcilorFactory::GetInstance();
  ios::AutocompleteClassifierFactory::GetInstance();
  ios::LocalOrSyncableBookmarkModelFactory::GetInstance();
  ios::AccountBookmarkModelFactory::GetInstance();
  ios::BookmarkModelFactory::GetInstance();
  ios::BookmarkUndoServiceFactory::GetInstance();
  ios::CookieSettingsFactory::GetInstance();
  ios::FaviconServiceFactory::GetInstance();
  ios::HistoryServiceFactory::GetInstance();
  ios::LocalOrSyncableBookmarkSyncServiceFactory::GetInstance();
  ios::InMemoryURLIndexFactory::GetInstance();
  ios::SearchEngineChoiceServiceFactory::GetInstance();
  ios::ShortcutsBackendFactory::GetInstance();
  ios::SigninErrorControllerFactory::GetInstance();
  ios::TemplateURLServiceFactory::GetInstance();
  ios::TopSitesFactory::GetInstance();
  ios::WebDataServiceFactory::GetInstance();
  ios::WebHistoryServiceFactory::GetInstance();
  translate::TranslateRankerFactory::GetInstance();
  AboutThisSiteServiceFactory::GetInstance();
  AcceptLanguagesServiceFactory::GetInstance();
  AuthenticationServiceFactory::GetInstance();
  BreadcrumbManagerKeyedServiceFactory::GetInstance();
  BringAndroidTabsToIOSServiceFactory::GetInstance();
  BrowserDownloadServiceFactory::GetInstance();
  BrowserListFactory::GetInstance();
  BrowsingDataRemoverFactory::GetInstance();
  ChildAccountServiceFactory::GetInstance();
  ChromeAccountManagerServiceFactory::GetInstance();
  ChromePasswordProtectionServiceFactory::GetInstance();
  ConsentAuditorFactory::GetInstance();
  ContentNotificationServiceFactory::GetInstance();
  ContextualPanelModelServiceFactory::GetInstance();
  DeviceSharingManagerFactory::GetInstance();
  DiscoverFeedServiceFactory::GetInstance();
  DomainDiversityReporterFactory::GetInstance();
  BackgroundDownloadServiceFactory::GetInstance();
  FollowServiceFactory::GetInstance();
  GoogleGroupsUpdaterServiceFactory::GetInstance();
  GoogleLogoServiceFactory::GetInstance();
  IdentityManagerFactory::GetInstance();
  IOSChromeAccountPasswordStoreFactory::GetInstance();
  IOSChromeAffiliationServiceFactory::GetInstance();
  IOSChromeFaviconLoaderFactory::GetInstance();
  IOSChromeGCMProfileServiceFactory::GetInstance();
  IOSChromeLargeIconCacheFactory::GetInstance();
  IOSChromeLargeIconServiceFactory::GetInstance();
  IOSChromePasswordCheckManagerFactory::GetInstance();
  IOSChromeProfilePasswordStoreFactory::GetInstance();
  IOSChromeProfileInvalidationProviderFactory::GetInstance();
  IOSChromeSafetyCheckManagerFactory::GetInstance();
  IOSProfileSessionDurationsServiceFactory::GetInstance();
  IOSTrustedVaultServiceFactory::GetInstance();
  IOSUserEventServiceFactory::GetInstance();
  LanguageModelManagerFactory::GetInstance();
  ListFamilyMembersServiceFactory::GetInstance();
  MailtoHandlerServiceFactory::GetInstance();
  ManagedBookmarkServiceFactory::GetInstance();
  ModelTypeStoreServiceFactory::GetInstance();
  OptimizationGuideServiceFactory::GetInstance();
  PhotosServiceFactory::GetInstance();
  PlusAddressServiceFactory::GetInstance();
  policy::UserPolicySigninServiceFactory::GetInstance();
  PolicyBlocklistServiceFactory::GetInstance();
  PromosManagerFactory::GetInstance();
  PushNotificationBrowserStateServiceFactory::GetInstance();
  ReadingListModelFactory::GetInstance();
  RealTimeUrlLookupServiceFactory::GetInstance();
  SafeBrowsingClientFactory::GetInstance();
  SafeBrowsingMetricsCollectorFactory::GetInstance();
  SamplePanelModelFactory::GetInstance();
  PriceInsightsModelFactory::GetInstance();
  segmentation_platform::SegmentationPlatformServiceFactory::GetInstance();
  SessionRestorationServiceFactory::GetInstance();
  SigninBrowserStateInfoUpdaterFactory::GetInstance();
  SigninClientFactory::GetInstance();
  SupervisedUserMetricsServiceFactory::GetInstance();
  SupervisedUserServiceFactory::GetInstance();
  SupervisedUserSettingsServiceFactory::GetInstance();
  SyncServiceFactory::GetInstance();
  TabsSearchServiceFactory::GetInstance();
  TextClassifierModelServiceFactory::GetInstance();
  TextToSpeechPlaybackControllerFactory::GetInstance();
  TrustedVaultClientBackendFactory::GetInstance();
  UnifiedConsentServiceFactory::GetInstance();
  UnitConversionServiceFactory::GetInstance();
  UrlLanguageHistogramFactory::GetInstance();
  VerdictCacheManagerFactory::GetInstance();
  // Keep the above list alphabetized! Don't just add new entries at the end.

  // All factories that are only conditionally added go below here.
  if (base::FeatureList::IsEnabled(kIOSSaveToDrive)) {
    drive::DriveServiceFactory::GetInstance();
  }

  if (page_content_annotations::features::
          ShouldEnablePageContentAnnotations()) {
    PageContentAnnotationsServiceFactory::GetInstance();
  }

#if BUILDFLAG(IOS_CREDENTIAL_PROVIDER_ENABLED)
  CredentialProviderServiceFactory::GetInstance();
#endif

#if BUILDFLAG(IOS_SCREEN_TIME_ENABLED)
  if (IsScreenTimeIntegrationEnabled()) {
    ScreenTimeHistoryDeleterFactory::GetInstance();
  }
#endif
}
