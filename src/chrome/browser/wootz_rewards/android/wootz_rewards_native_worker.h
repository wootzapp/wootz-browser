/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <jni.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/android/jni_weak_ref.h"
#include "base/containers/flat_map.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
// #include "chrome/components/wootz_ads/core/mojom/wootz_ads.mojom.h"
#include "chrome/components/wootz_rewards/browser/rewards_notification_service_observer.h"
#include "chrome/components/wootz_rewards/browser/rewards_service_observer.h"
#include "chrome/components/wootz_rewards/common/mojom/rewards.mojom.h"

namespace wootz_rewards {
class RewardsService;
}

namespace chrome {
namespace android {

typedef std::map<uint64_t, wootz_rewards::mojom::PublisherInfoPtr>
    PublishersInfoMap;

class WootzRewardsNativeWorker
    : public wootz_rewards::RewardsServiceObserver,
      public wootz_rewards::RewardsNotificationServiceObserver {
 public:
  WootzRewardsNativeWorker(JNIEnv* env,
                           const base::android::JavaRef<jobject>& obj);
  ~WootzRewardsNativeWorker() override;

  void Destroy(JNIEnv* env);

  bool IsSupported(JNIEnv* env);
  bool IsSupportedSkipRegionCheck(JNIEnv* env);

  std::string StringifyResult(
      wootz_rewards::mojom::CreateRewardsWalletResult result);

  bool IsRewardsEnabled(JNIEnv* env);

  bool ShouldShowSelfCustodyInvite(JNIEnv* env);

  void CreateRewardsWallet(
      JNIEnv* env,
      const base::android::JavaParamRef<jstring>& country_code);

  void GetRewardsParameters(JNIEnv* env);

  double GetVbatDeadline(JNIEnv* env);

  base::android::ScopedJavaLocalRef<jstring> GetPayoutStatus(JNIEnv* env);

  void GetUserType(JNIEnv* env);

  void FetchBalance(JNIEnv* env);

  void GetPublisherInfo(JNIEnv* env,
                        int tabId,
                        const base::android::JavaParamRef<jstring>& host);

  base::android::ScopedJavaLocalRef<jstring> GetWalletBalance(JNIEnv* env);

  base::android::ScopedJavaLocalRef<jstring> GetExternalWalletType(JNIEnv* env);

//   void GetAdsAccountStatement(JNIEnv* env);

  bool CanConnectAccount(JNIEnv* env);

  base::android::ScopedJavaLocalRef<jdoubleArray> GetTipChoices(JNIEnv* env);

  double GetWalletRate(JNIEnv* env);

  base::android::ScopedJavaLocalRef<jstring> GetPublisherURL(JNIEnv* env,
                                                             uint64_t tabId);

  base::android::ScopedJavaLocalRef<jstring> GetPublisherFavIconURL(
      JNIEnv* env,
      uint64_t tabId);

  base::android::ScopedJavaLocalRef<jstring> GetPublisherName(JNIEnv* env,
                                                              uint64_t tabId);
  base::android::ScopedJavaLocalRef<jstring> GetCaptchaSolutionURL(
      JNIEnv* env,
      const base::android::JavaParamRef<jstring>& paymentId,
      const base::android::JavaParamRef<jstring>& captchaId);
  base::android::ScopedJavaLocalRef<jstring> GetAttestationURL(JNIEnv* env);
  base::android::ScopedJavaLocalRef<jstring> GetAttestationURLWithPaymentId(
      JNIEnv* env,
      const base::android::JavaParamRef<jstring>& paymentId);

  base::android::ScopedJavaLocalRef<jstring> GetPublisherId(JNIEnv* env,
                                                            uint64_t tabId);

  int GetPublisherPercent(JNIEnv* env, uint64_t tabId);

  bool GetPublisherExcluded(JNIEnv* env, uint64_t tabId);

  int GetPublisherStatus(JNIEnv* env, uint64_t tabId);

  void GetCurrentBalanceReport(JNIEnv* env);

  void IncludeInAutoContribution(JNIEnv* env, uint64_t tabId, bool exclude);

  void RemovePublisherFromMap(JNIEnv* env, uint64_t tabId);

  void Donate(JNIEnv* env,
              const base::android::JavaParamRef<jstring>& publisher_key,
              double amount,
              bool recurring);

  void GetAllNotifications(JNIEnv* env);

  void DeleteNotification(
      JNIEnv* env,
      const base::android::JavaParamRef<jstring>& notification_id);

  void GetRecurringDonations(JNIEnv* env);

  bool IsCurrentPublisherInRecurrentDonations(
      JNIEnv* env,
      const base::android::JavaParamRef<jstring>& publisher);

  void GetAutoContributeProperties(JNIEnv* env);

  bool IsAutoContributeEnabled(JNIEnv* env);

  void GetReconcileStamp(JNIEnv* env);

  void ResetTheWholeState(JNIEnv* env);

  double GetPublisherRecurrentDonationAmount(
      JNIEnv* env,
      const base::android::JavaParamRef<jstring>& publisher);

  void RemoveRecurring(JNIEnv* env,
                       const base::android::JavaParamRef<jstring>& publisher);

//   int GetAdsPerHour(JNIEnv* env);

//   void SetAdsPerHour(JNIEnv* env, jint value);

  void SetAutoContributionAmount(JNIEnv* env, jdouble value);

  void GetAutoContributionAmount(JNIEnv* env);

  void GetExternalWallet(JNIEnv* env);

  bool IsTermsOfServiceUpdateRequired(JNIEnv* env);

  void AcceptTermsOfServiceUpdate(JNIEnv* env);

  base::android::ScopedJavaLocalRef<jstring> GetCountryCode(JNIEnv* env);

  void GetAvailableCountries(JNIEnv* env);

  void GetPublisherBanner(
      JNIEnv* env,
      const base::android::JavaParamRef<jstring>& publisher_key);

  void GetPublishersVisitedCount(JNIEnv* env);

  void OnGetPublishersVisitedCount(int count);

  void DisconnectWallet(JNIEnv* env);

  void RefreshPublisher(
      JNIEnv* env,
      const base::android::JavaParamRef<jstring>& publisher_key);

  void RecordPanelTrigger(JNIEnv* env);

  void OnCreateRewardsWallet(
      wootz_rewards::mojom::CreateRewardsWalletResult result);

  void OnCompleteReset(const bool success) override;

  void OnResetTheWholeState(const bool success);

  void OnGetGetReconcileStamp(uint64_t timestamp);

  void OnGetAutoContributeProperties(
      wootz_rewards::mojom::AutoContributePropertiesPtr properties);

  void OnGetAutoContributionAmount(double auto_contribution_amount);

  void OnPanelPublisherInfo(wootz_rewards::RewardsService* rewards_service,
                            const wootz_rewards::mojom::Result result,
                            const wootz_rewards::mojom::PublisherInfo* info,
                            uint64_t tabId) override;

  void OnGetCurrentBalanceReport(
      wootz_rewards::RewardsService* rewards_service,
      const wootz_rewards::mojom::Result result,
      wootz_rewards::mojom::BalanceReportInfoPtr report);

  void OnGetRewardsParameters(
      wootz_rewards::RewardsService* rewards_service,
      wootz_rewards::mojom::RewardsParametersPtr parameters);

  void OnTermsOfServiceUpdateAccepted() override;

  void OnReconcileComplete(
      wootz_rewards::RewardsService* rewards_service,
      const wootz_rewards::mojom::Result result,
      const std::string& contribution_id,
      const double amount,
      const wootz_rewards::mojom::RewardsType type,
      const wootz_rewards::mojom::ContributionProcessor processor) override;

  void OnNotificationAdded(
      wootz_rewards::RewardsNotificationService* rewards_notification_service,
      const wootz_rewards::RewardsNotificationService::RewardsNotification&
          notification) override;

  void OnGetAllNotifications(
      wootz_rewards::RewardsNotificationService* rewards_notification_service,
      const wootz_rewards::RewardsNotificationService::RewardsNotificationsList&
          notifications_list) override;

  void OnNotificationDeleted(
      wootz_rewards::RewardsNotificationService* rewards_notification_service,
      const wootz_rewards::RewardsNotificationService::RewardsNotification&
          notification) override;

  void OnGetRecurringTips(
      std::vector<wootz_rewards::mojom::PublisherInfoPtr> list);

  void OnGetExternalWallet(wootz_rewards::mojom::ExternalWalletPtr wallet);

  void OnGetAvailableCountries(std::vector<std::string> countries);

  void onPublisherBanner(wootz_rewards::mojom::PublisherBannerPtr wallet);

  void OnSendContribution(bool result);

  void OnExternalWalletConnected() override;

  void OnExternalWalletLoggedOut() override;

  void OnExternalWalletReconnected() override;

  void OnRefreshPublisher(const wootz_rewards::mojom::PublisherStatus status,
                          const std::string& publisher_key);
  void SetAutoContributeEnabled(JNIEnv* env, bool isAutoContributeEnabled);

 private:
  std::string StdStrStrMapToJsonString(
      const base::flat_map<std::string, std::string>& args);

  void OnGetUserType(const wootz_rewards::mojom::UserType user_type);

  void OnBalance(wootz_rewards::mojom::BalancePtr balance);

//   void OnGetAdsAccountStatement(wootz_ads::mojom::StatementInfoPtr statement);

  JavaObjectWeakGlobalRef weak_java_wootz_rewards_native_worker_;
  raw_ptr<wootz_rewards::RewardsService> wootz_rewards_service_ = nullptr;
  wootz_rewards::mojom::RewardsParametersPtr parameters_;
  wootz_rewards::mojom::Balance balance_;
  wootz_rewards::mojom::AutoContributePropertiesPtr auto_contrib_properties_;
  PublishersInfoMap map_publishers_info_;
  std::map<std::string, wootz_rewards::mojom::PublisherInfoPtr>
      map_recurrent_publishers_;
  std::map<std::string, std::string> addresses_;
  base::WeakPtrFactory<WootzRewardsNativeWorker> weak_factory_;
};

}  // namespace android
}  // namespace chrome
