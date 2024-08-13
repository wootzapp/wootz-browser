/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/browser/wootz_rewards/android/wootz_rewards_native_worker.h"

#include <iomanip>
#include <string>
#include <vector>
#include <utility>

#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/containers/contains.h"
#include "base/containers/flat_map.h"
#include "base/feature_list.h"
#include "base/json/json_writer.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"

#include "chrome/browser/wootz_rewards/rewards_service_factory.h"
#include "chrome/build/android/jni_headers/WootzRewardsNativeWorker_jni.h"
#include "chrome/components/wootz_adaptive_captcha/wootz_adaptive_captcha_service.h"
#include "chrome/components/wootz_adaptive_captcha/server_util.h"

#include "chrome/components/wootz_rewards/browser/rewards_p3a.h"
#include "chrome/components/wootz_rewards/browser/rewards_service.h"
#include "chrome/components/wootz_rewards/common/features.h"
#include "chrome/components/wootz_rewards/common/pref_names.h"
#include "chrome/components/wootz_rewards/common/rewards_util.h"
#include "chrome/components/wootz_rewards/core/global_constants.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/url_data_source.h"

// #define DEFAULT_ADS_PER_HOUR 2

namespace chrome {
namespace android {

WootzRewardsNativeWorker::WootzRewardsNativeWorker(JNIEnv* env,
    const base::android::JavaRef<jobject>& obj):
    weak_java_wootz_rewards_native_worker_(env, obj),
    wootz_rewards_service_(nullptr),
    weak_factory_(this) {
  Java_WootzRewardsNativeWorker_setNativePtr(env, obj,
    reinterpret_cast<intptr_t>(this));

  wootz_rewards_service_ = wootz_rewards::RewardsServiceFactory::GetForProfile(
      ProfileManager::GetActiveUserProfile()->GetOriginalProfile());
  if (wootz_rewards_service_) {
    wootz_rewards_service_->AddObserver(this);
    wootz_rewards::RewardsNotificationService* notification_service =
      wootz_rewards_service_->GetNotificationService();
    if (notification_service) {
      notification_service->AddObserver(this);
    }
  }
}

WootzRewardsNativeWorker::~WootzRewardsNativeWorker() {
}

void WootzRewardsNativeWorker::Destroy(JNIEnv* env) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->RemoveObserver(this);
    wootz_rewards::RewardsNotificationService* notification_service =
      wootz_rewards_service_->GetNotificationService();
    if (notification_service) {
      notification_service->RemoveObserver(this);
    }
  }
  delete this;
}

bool WootzRewardsNativeWorker::IsSupported(JNIEnv* env) {
  return wootz_rewards::IsSupported(
      ProfileManager::GetActiveUserProfile()->GetOriginalProfile()->GetPrefs(),
      wootz_rewards::IsSupportedOptions::kNone);
}

bool WootzRewardsNativeWorker::IsSupportedSkipRegionCheck(JNIEnv* env) {
  return wootz_rewards::IsSupported(
      ProfileManager::GetActiveUserProfile()->GetOriginalProfile()->GetPrefs(),
      wootz_rewards::IsSupportedOptions::kSkipRegionCheck);
}

std::string WootzRewardsNativeWorker::StringifyResult(
    wootz_rewards::mojom::CreateRewardsWalletResult result) {
  switch (result) {
    case wootz_rewards::mojom::CreateRewardsWalletResult::kSuccess:
      return "success";
    case wootz_rewards::mojom::CreateRewardsWalletResult::
        kWalletGenerationDisabled:
      return "wallet-generation-disabled";
    case wootz_rewards::mojom::CreateRewardsWalletResult::
        kGeoCountryAlreadyDeclared:
      return "country-already-declared";
    case wootz_rewards::mojom::CreateRewardsWalletResult::kUnexpected:
      return "unexpected-error";
  }
}

bool WootzRewardsNativeWorker::IsRewardsEnabled(JNIEnv* env) {
  return ProfileManager::GetActiveUserProfile()
      ->GetOriginalProfile()
      ->GetPrefs()
      ->GetBoolean(wootz_rewards::prefs::kEnabled);
}

bool WootzRewardsNativeWorker::ShouldShowSelfCustodyInvite(JNIEnv* env) {
  bool is_self_custody_invite_dismissed =
      ProfileManager::GetActiveUserProfile()
          ->GetOriginalProfile()
          ->GetPrefs()
          ->GetBoolean(wootz_rewards::prefs::kSelfCustodyInviteDismissed);

  if (is_self_custody_invite_dismissed) {
    return false;
  }

  std::string country_code = wootz_rewards_service_->GetCountryCode();
  const std::vector<std::string> providers =
      wootz_rewards_service_->GetExternalWalletProviders();
  if (!base::Contains(providers,
                      wootz_rewards::internal::constant::kWalletSolana)) {
    return false;
  }

  const auto& regions = parameters_->wallet_provider_regions.at(
      wootz_rewards::internal::constant::kWalletSolana);
  if (!regions) {
    return true;
  }

  const auto& [allow, block] = *regions;
  if (allow.empty() && block.empty()) {
    return true;
  }

  return base::Contains(allow, country_code) ||
         (!block.empty() && !base::Contains(block, country_code));
}

void WootzRewardsNativeWorker::CreateRewardsWallet(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& country_code) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->CreateRewardsWallet(
        base::android::ConvertJavaStringToUTF8(env, country_code),
        base::BindOnce(&WootzRewardsNativeWorker::OnCreateRewardsWallet,
                       weak_factory_.GetWeakPtr()));
  }
}

void WootzRewardsNativeWorker::OnCreateRewardsWallet(
    wootz_rewards::mojom::CreateRewardsWalletResult result) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onCreateRewardsWallet(
      env, weak_java_wootz_rewards_native_worker_.get(env),
      base::android::ConvertUTF8ToJavaString(env, StringifyResult(result)));
}

void WootzRewardsNativeWorker::GetRewardsParameters(JNIEnv* env) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->GetRewardsParameters(
        base::BindOnce(&WootzRewardsNativeWorker::OnGetRewardsParameters,
                       weak_factory_.GetWeakPtr(), wootz_rewards_service_));
  }
}

void WootzRewardsNativeWorker::OnGetRewardsParameters(
    wootz_rewards::RewardsService* rewards_service,
    wootz_rewards::mojom::RewardsParametersPtr parameters) {
  if (parameters) {
    parameters_ = std::move(parameters);
  }

  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onRewardsParameters(
      env, weak_java_wootz_rewards_native_worker_.get(env));
}

void WootzRewardsNativeWorker::OnTermsOfServiceUpdateAccepted() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onTermsOfServiceUpdateAccepted(
      env, weak_java_wootz_rewards_native_worker_.get(env));
}

double WootzRewardsNativeWorker::GetVbatDeadline(JNIEnv* env) {
  if (parameters_) {
    if (!parameters_->vbat_deadline.is_null()) {
      return floor(parameters_->vbat_deadline.InSecondsFSinceUnixEpoch() *
                   base::Time::kMillisecondsPerSecond);
    }
  }
  return 0.0;
}

base::android::ScopedJavaLocalRef<jstring>
WootzRewardsNativeWorker::GetPayoutStatus(JNIEnv* env) {
  std::string wallet_type;
  std::string payout_status;
  if (wootz_rewards_service_) {
    wallet_type = wootz_rewards_service_->GetExternalWalletType();
    if (parameters_) {
      if (!parameters_->payout_status.empty()) {
        payout_status = parameters_->payout_status.at(wallet_type);
      }
    }
  }
  return base::android::ConvertUTF8ToJavaString(env, payout_status);
}

void WootzRewardsNativeWorker::GetUserType(JNIEnv* env) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->GetUserType(base::BindOnce(
        &WootzRewardsNativeWorker::OnGetUserType, weak_factory_.GetWeakPtr()));
  }
}

void WootzRewardsNativeWorker::OnGetUserType(
    const wootz_rewards::mojom::UserType user_type) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onGetUserType(
      env, weak_java_wootz_rewards_native_worker_.get(env),
      static_cast<int>(user_type));
}

void WootzRewardsNativeWorker::FetchBalance(JNIEnv* env) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->FetchBalance(base::BindOnce(
        &WootzRewardsNativeWorker::OnBalance, weak_factory_.GetWeakPtr()));
  }
}

void WootzRewardsNativeWorker::OnBalance(
    wootz_rewards::mojom::BalancePtr balance) {
  if (balance) {
    balance_ = *balance;
  }

  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onBalance(
      env, weak_java_wootz_rewards_native_worker_.get(env), !!balance);
}

void WootzRewardsNativeWorker::GetPublisherInfo(
    JNIEnv* env,
    int tabId,
    const base::android::JavaParamRef<jstring>& host) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->GetPublisherActivityFromUrl(tabId,
      base::android::ConvertJavaStringToUTF8(env, host), "", "");
  }
}

void WootzRewardsNativeWorker::OnPanelPublisherInfo(
    wootz_rewards::RewardsService* rewards_service,
    const wootz_rewards::mojom::Result result,
    const wootz_rewards::mojom::PublisherInfo* info,
    uint64_t tabId) {
  if (!info) {
    return;
  }
  wootz_rewards::mojom::PublisherInfoPtr pi = info->Clone();
  map_publishers_info_[tabId] = std::move(pi);
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onPublisherInfo(
      env, weak_java_wootz_rewards_native_worker_.get(env), tabId);
}

void WootzRewardsNativeWorker::OnReconcileComplete(
    wootz_rewards::RewardsService* rewards_service,
    const wootz_rewards::mojom::Result result,
    const std::string& contribution_id,
    const double amount,
    const wootz_rewards::mojom::RewardsType type,
    const wootz_rewards::mojom::ContributionProcessor processor) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onReconcileComplete(
      env, weak_java_wootz_rewards_native_worker_.get(env),
      static_cast<int>(result), static_cast<int>(type), amount);
}

base::android::ScopedJavaLocalRef<jstring>
WootzRewardsNativeWorker::GetPublisherURL(JNIEnv* env, uint64_t tabId) {
  base::android::ScopedJavaLocalRef<jstring> res =
    base::android::ConvertUTF8ToJavaString(env, "");

  PublishersInfoMap::const_iterator iter(map_publishers_info_.find(tabId));
  if (iter != map_publishers_info_.end()) {
    res = base::android::ConvertUTF8ToJavaString(env, iter->second->url);
  }

  return res;
}

base::android::ScopedJavaLocalRef<jstring>
WootzRewardsNativeWorker::GetPublisherFavIconURL(JNIEnv* env, uint64_t tabId) {
  base::android::ScopedJavaLocalRef<jstring> res =
    base::android::ConvertUTF8ToJavaString(env, "");

  PublishersInfoMap::const_iterator iter(map_publishers_info_.find(tabId));
  if (iter != map_publishers_info_.end()) {
    res = base::android::ConvertUTF8ToJavaString(env,
      iter->second->favicon_url);
  }

  return res;
}

base::android::ScopedJavaLocalRef<jstring>
WootzRewardsNativeWorker::GetCaptchaSolutionURL(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& paymentId,
    const base::android::JavaParamRef<jstring>& captchaId) {
  const std::string path = base::StringPrintf(
      "/v3/captcha/solution/%s/%s",
      base::android::ConvertJavaStringToUTF8(env, paymentId).c_str(),
      base::android::ConvertJavaStringToUTF8(env, captchaId).c_str());
  std::string captcha_solution_url =
      wootz_adaptive_captcha::ServerUtil::GetInstance()->GetServerUrl(path);

  return base::android::ConvertUTF8ToJavaString(env, captcha_solution_url);
}

base::android::ScopedJavaLocalRef<jstring>
WootzRewardsNativeWorker::GetAttestationURL(JNIEnv* env) {
  const std::string path = "/v1/attestations/android";
  std::string captcha_solution_url =
      wootz_adaptive_captcha::ServerUtil::GetInstance()->GetServerUrl(path);

  return base::android::ConvertUTF8ToJavaString(env, captcha_solution_url);
}

base::android::ScopedJavaLocalRef<jstring>
WootzRewardsNativeWorker::GetAttestationURLWithPaymentId(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& paymentId) {
  const std::string path = base::StringPrintf(
      "/v1/attestations/android/%s",
      base::android::ConvertJavaStringToUTF8(env, paymentId).c_str());
  std::string captcha_solution_url =
      wootz_adaptive_captcha::ServerUtil::GetInstance()->GetServerUrl(path);

  return base::android::ConvertUTF8ToJavaString(env, captcha_solution_url);
}

base::android::ScopedJavaLocalRef<jstring>
WootzRewardsNativeWorker::GetPublisherName(JNIEnv* env, uint64_t tabId) {
  base::android::ScopedJavaLocalRef<jstring> res =
    base::android::ConvertUTF8ToJavaString(env, "");

  PublishersInfoMap::const_iterator iter(map_publishers_info_.find(tabId));
  if (iter != map_publishers_info_.end()) {
    res = base::android::ConvertUTF8ToJavaString(env, iter->second->name);
  }

  return res;
}

base::android::ScopedJavaLocalRef<jstring>
WootzRewardsNativeWorker::GetPublisherId(JNIEnv* env, uint64_t tabId) {
  base::android::ScopedJavaLocalRef<jstring> res =
    base::android::ConvertUTF8ToJavaString(env, "");

  PublishersInfoMap::const_iterator iter(map_publishers_info_.find(tabId));
  if (iter != map_publishers_info_.end()) {
    res = base::android::ConvertUTF8ToJavaString(env, iter->second->id);
  }

  return res;
}

int WootzRewardsNativeWorker::GetPublisherPercent(JNIEnv* env, uint64_t tabId) {
  int res = 0;

  PublishersInfoMap::const_iterator iter(map_publishers_info_.find(tabId));
  if (iter != map_publishers_info_.end()) {
    res = iter->second->percent;
  }

  return res;
}

bool WootzRewardsNativeWorker::GetPublisherExcluded(JNIEnv* env,
                                                    uint64_t tabId) {
  bool res = false;

  PublishersInfoMap::const_iterator iter(map_publishers_info_.find(tabId));
  if (iter != map_publishers_info_.end()) {
    res = iter->second->excluded ==
          wootz_rewards::mojom::PublisherExclude::EXCLUDED;
  }

  return res;
}

int WootzRewardsNativeWorker::GetPublisherStatus(JNIEnv* env, uint64_t tabId) {
  int res =
      static_cast<int>(wootz_rewards::mojom::PublisherStatus::NOT_VERIFIED);
  PublishersInfoMap::const_iterator iter = map_publishers_info_.find(tabId);
  if (iter != map_publishers_info_.end()) {
    res = static_cast<int>(iter->second->status);
  }
  return res;
}

void WootzRewardsNativeWorker::IncludeInAutoContribution(JNIEnv* env,
                                                         uint64_t tabId,
                                                         bool exclude) {
  PublishersInfoMap::iterator iter(map_publishers_info_.find(tabId));
  if (iter != map_publishers_info_.end()) {
    if (exclude) {
      iter->second->excluded = wootz_rewards::mojom::PublisherExclude::EXCLUDED;
    } else {
      iter->second->excluded = wootz_rewards::mojom::PublisherExclude::INCLUDED;
    }
    if (wootz_rewards_service_) {
      wootz_rewards_service_->SetPublisherExclude(iter->second->id, exclude);
    }
  }
}

void WootzRewardsNativeWorker::RemovePublisherFromMap(JNIEnv* env,
                                                      uint64_t tabId) {
  PublishersInfoMap::const_iterator iter(map_publishers_info_.find(tabId));
  if (iter != map_publishers_info_.end()) {
    map_publishers_info_.erase(iter);
  }
}

base::android::ScopedJavaLocalRef<jstring>
WootzRewardsNativeWorker::GetWalletBalance(JNIEnv* env) {
  std::string json_balance;
  base::Value::Dict root;
  root.Set("total", balance_.total);

  base::Value::Dict json_wallets;
  for (const auto & item : balance_.wallets) {
    json_wallets.Set(item.first, item.second);
  }
  root.SetByDottedPath("wallets", std::move(json_wallets));
  base::JSONWriter::Write(std::move(root), &json_balance);

  return base::android::ConvertUTF8ToJavaString(env, json_balance);
}

base::android::ScopedJavaLocalRef<jstring>
WootzRewardsNativeWorker::GetExternalWalletType(JNIEnv* env) {
  std::string wallet_type;
  if (wootz_rewards_service_) {
    wallet_type = wootz_rewards_service_->GetExternalWalletType();
  }

  return base::android::ConvertUTF8ToJavaString(env, wallet_type);
}

// void WootzRewardsNativeWorker::GetAdsAccountStatement(JNIEnv* env) {
//   auto* ads_service = wootz_ads::AdsServiceFactory::GetForProfile(
//       ProfileManager::GetActiveUserProfile()->GetOriginalProfile());
//   if (!ads_service) {
//     return;
//   }
//   ads_service->GetStatementOfAccounts(
//       base::BindOnce(&WootzRewardsNativeWorker::OnGetAdsAccountStatement,
//                      weak_factory_.GetWeakPtr()));
// }

// void WootzRewardsNativeWorker::OnGetAdsAccountStatement(
//     wootz_ads::mojom::StatementInfoPtr statement) {
//   JNIEnv* env = base::android::AttachCurrentThread();
//   if (!statement) {
//     Java_WootzRewardsNativeWorker_onGetAdsAccountStatement(
//         env, weak_java_wootz_rewards_native_worker_.get(env),
//         /* success */ false, 0.0, 0, 0.0, 0.0, 0.0, 0.0);
//     return;
//   }

//   Java_WootzRewardsNativeWorker_onGetAdsAccountStatement(
//       env, weak_java_wootz_rewards_native_worker_.get(env),
//       /* success */ true,
//       statement->next_payment_date.InSecondsFSinceUnixEpoch() * 1000,
//       statement->ads_received_this_month, statement->min_earnings_this_month,
//       statement->max_earnings_this_month, statement->min_earnings_last_month,
//       statement->max_earnings_last_month);
// }

bool WootzRewardsNativeWorker::CanConnectAccount(JNIEnv* env) {
  if (!parameters_ || !wootz_rewards_service_) {
    return true;
  }
  std::string country_code = wootz_rewards_service_->GetCountryCode();
  return base::ranges::any_of(
      wootz_rewards_service_->GetExternalWalletProviders(),
      [this, &country_code](const std::string& provider) {
        if (!parameters_->wallet_provider_regions.count(provider)) {
          return true;
        }

        const auto& regions = parameters_->wallet_provider_regions.at(provider);
        if (!regions) {
          return true;
        }

        const auto& [allow, block] = *regions;
        if (allow.empty() && block.empty()) {
          return true;
        }

        return base::Contains(allow, country_code) ||
               (!block.empty() && !base::Contains(block, country_code));
      });
}

base::android::ScopedJavaLocalRef<jdoubleArray>
WootzRewardsNativeWorker::GetTipChoices(JNIEnv* env) {
  return base::android::ToJavaDoubleArray(
      env, parameters_ ? parameters_->tip_choices : std::vector<double>());
}

double WootzRewardsNativeWorker::GetWalletRate(JNIEnv* env) {
  return parameters_ ? parameters_->rate : 0.0;
}

void WootzRewardsNativeWorker::GetCurrentBalanceReport(JNIEnv* env) {
  if (wootz_rewards_service_) {
    auto now = base::Time::Now();
    base::Time::Exploded exploded;
    now.LocalExplode(&exploded);

    wootz_rewards_service_->GetBalanceReport(
        exploded.month, exploded.year,
        base::BindOnce(&WootzRewardsNativeWorker::OnGetCurrentBalanceReport,
                       weak_factory_.GetWeakPtr(), wootz_rewards_service_));
  }
}

void WootzRewardsNativeWorker::OnGetCurrentBalanceReport(
    wootz_rewards::RewardsService* rewards_service,
    const wootz_rewards::mojom::Result result,
    wootz_rewards::mojom::BalanceReportInfoPtr report) {
  base::android::ScopedJavaLocalRef<jdoubleArray> java_array;
  JNIEnv* env = base::android::AttachCurrentThread();
  if (report) {
    std::vector<double> values;
    // values.push_back(report->earning_from_ads);
    values.push_back(report->auto_contribute);
    values.push_back(report->recurring_donation);
    values.push_back(report->one_time_donation);
    java_array = base::android::ToJavaDoubleArray(env, values);
  }
  Java_WootzRewardsNativeWorker_onGetCurrentBalanceReport(
      env, weak_java_wootz_rewards_native_worker_.get(env), java_array);
}

void WootzRewardsNativeWorker::Donate(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& publisher_key,
    double amount,
    bool recurring) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->SendContribution(
        base::android::ConvertJavaStringToUTF8(env, publisher_key), amount,
        recurring,
        base::BindOnce(&WootzRewardsNativeWorker::OnSendContribution,
                       weak_factory_.GetWeakPtr()));
  }
}

void WootzRewardsNativeWorker::OnSendContribution(bool result) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onSendContribution(
      env, weak_java_wootz_rewards_native_worker_.get(env), result);
}

void WootzRewardsNativeWorker::GetAllNotifications(JNIEnv* env) {
  if (!wootz_rewards_service_) {
    return;
  }
  wootz_rewards::RewardsNotificationService* notification_service =
    wootz_rewards_service_->GetNotificationService();
  if (notification_service) {
    notification_service->GetNotifications();
  }
}

void WootzRewardsNativeWorker::DeleteNotification(JNIEnv* env,
        const base::android::JavaParamRef<jstring>& notification_id) {
  if (!wootz_rewards_service_) {
    return;
  }
  wootz_rewards::RewardsNotificationService* notification_service =
    wootz_rewards_service_->GetNotificationService();
  if (notification_service) {
    notification_service->DeleteNotification(
      base::android::ConvertJavaStringToUTF8(env, notification_id));
  }
}

void WootzRewardsNativeWorker::GetRecurringDonations(JNIEnv* env) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->GetRecurringTips(
        base::BindOnce(&WootzRewardsNativeWorker::OnGetRecurringTips,
                       weak_factory_.GetWeakPtr()));
  }
}

void WootzRewardsNativeWorker::OnGetRecurringTips(
    std::vector<wootz_rewards::mojom::PublisherInfoPtr> list) {
  map_recurrent_publishers_.clear();
  for (const auto& item : list) {
    map_recurrent_publishers_[item->id] = item->Clone();
  }

  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onRecurringDonationUpdated(
      env, weak_java_wootz_rewards_native_worker_.get(env));
}

bool WootzRewardsNativeWorker::IsCurrentPublisherInRecurrentDonations(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& publisher) {
  return map_recurrent_publishers_.find(
    base::android::ConvertJavaStringToUTF8(env, publisher)) !=
      map_recurrent_publishers_.end();
}

void WootzRewardsNativeWorker::GetAutoContributeProperties(JNIEnv* env) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->GetAutoContributeProperties(
        base::BindOnce(&WootzRewardsNativeWorker::OnGetAutoContributeProperties,
                       weak_factory_.GetWeakPtr()));
  }
}

void WootzRewardsNativeWorker::OnGetAutoContributeProperties(
    wootz_rewards::mojom::AutoContributePropertiesPtr properties) {
  if (properties) {
    auto_contrib_properties_ = std::move(properties);
  }

  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onGetAutoContributeProperties(
      env, weak_java_wootz_rewards_native_worker_.get(env));
}

bool WootzRewardsNativeWorker::IsAutoContributeEnabled(JNIEnv* env) {
  if (!auto_contrib_properties_) {
    return false;
  }

  return auto_contrib_properties_->enabled_contribute;
}

void WootzRewardsNativeWorker::GetReconcileStamp(JNIEnv* env) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->GetReconcileStamp(
        base::BindOnce(&WootzRewardsNativeWorker::OnGetGetReconcileStamp,
                       weak_factory_.GetWeakPtr()));
  }
}

void WootzRewardsNativeWorker::ResetTheWholeState(JNIEnv* env) {
  if (!wootz_rewards_service_) {
    OnResetTheWholeState(false);
    return;
  }
  wootz_rewards_service_->CompleteReset(
      base::BindOnce(&WootzRewardsNativeWorker::OnResetTheWholeState,
                     weak_factory_.GetWeakPtr()));
}

void WootzRewardsNativeWorker::OnCompleteReset(const bool success) {
  JNIEnv* env = base::android::AttachCurrentThread();

  Java_WootzRewardsNativeWorker_onCompleteReset(
      env, weak_java_wootz_rewards_native_worker_.get(env), success);
}

void WootzRewardsNativeWorker::OnResetTheWholeState(const bool success) {
  JNIEnv* env = base::android::AttachCurrentThread();

  Java_WootzRewardsNativeWorker_onResetTheWholeState(
      env, weak_java_wootz_rewards_native_worker_.get(env), success);
}

double WootzRewardsNativeWorker::GetPublisherRecurrentDonationAmount(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& publisher) {
  double amount(0.0);
  auto it = map_recurrent_publishers_.find(
    base::android::ConvertJavaStringToUTF8(env, publisher));

  if (it != map_recurrent_publishers_.end()) {
    amount = it->second->weight;
  }
  return  amount;
}

void WootzRewardsNativeWorker::RemoveRecurring(JNIEnv* env,
    const base::android::JavaParamRef<jstring>& publisher) {
  if (wootz_rewards_service_) {
      wootz_rewards_service_->RemoveRecurringTip(
        base::android::ConvertJavaStringToUTF8(env, publisher));
      auto it = map_recurrent_publishers_.find(
          base::android::ConvertJavaStringToUTF8(env, publisher));

      if (it != map_recurrent_publishers_.end()) {
        map_recurrent_publishers_.erase(it);
      }
  }
}

void WootzRewardsNativeWorker::OnGetGetReconcileStamp(uint64_t timestamp) {
  JNIEnv* env = base::android::AttachCurrentThread();

  Java_WootzRewardsNativeWorker_onGetReconcileStamp(
      env, weak_java_wootz_rewards_native_worker_.get(env), timestamp);
}

void WootzRewardsNativeWorker::OnNotificationAdded(
    wootz_rewards::RewardsNotificationService* rewards_notification_service,
    const wootz_rewards::RewardsNotificationService::RewardsNotification&
      notification) {
  JNIEnv* env = base::android::AttachCurrentThread();

  Java_WootzRewardsNativeWorker_onNotificationAdded(
      env, weak_java_wootz_rewards_native_worker_.get(env),
      base::android::ConvertUTF8ToJavaString(env, notification.id_),
      notification.type_, notification.timestamp_,
      base::android::ToJavaArrayOfStrings(env, notification.args_));
}

void WootzRewardsNativeWorker::OnGetAllNotifications(
    wootz_rewards::RewardsNotificationService* rewards_notification_service,
    const wootz_rewards::RewardsNotificationService::RewardsNotificationsList&
      notifications_list) {
  JNIEnv* env = base::android::AttachCurrentThread();

  // Notify about notifications count
  Java_WootzRewardsNativeWorker_onNotificationsCount(
      env, weak_java_wootz_rewards_native_worker_.get(env),
      notifications_list.size());

  wootz_rewards::RewardsNotificationService::RewardsNotificationsList::
    const_iterator iter =
      std::max_element(notifications_list.begin(), notifications_list.end(),
        [](const wootz_rewards::RewardsNotificationService::
            RewardsNotification& notification_a,
          const wootz_rewards::RewardsNotificationService::
            RewardsNotification& notification_b) {
        return notification_a.timestamp_ > notification_b.timestamp_;
      });

  if (iter != notifications_list.end()) {
    Java_WootzRewardsNativeWorker_onGetLatestNotification(
        env, weak_java_wootz_rewards_native_worker_.get(env),
        base::android::ConvertUTF8ToJavaString(env, iter->id_), iter->type_,
        iter->timestamp_,
        base::android::ToJavaArrayOfStrings(env, iter->args_));
  }
}

void WootzRewardsNativeWorker::OnNotificationDeleted(
      wootz_rewards::RewardsNotificationService* rewards_notification_service,
      const wootz_rewards::RewardsNotificationService::RewardsNotification&
        notification) {
  JNIEnv* env = base::android::AttachCurrentThread();

  Java_WootzRewardsNativeWorker_onNotificationDeleted(
      env, weak_java_wootz_rewards_native_worker_.get(env),
      base::android::ConvertUTF8ToJavaString(env, notification.id_));
}

// int WootzRewardsNativeWorker::GetAdsPerHour(JNIEnv* env) {
//   auto* ads_service_ = wootz_ads::AdsServiceFactory::GetForProfile(
//       ProfileManager::GetActiveUserProfile()->GetOriginalProfile());
//   if (!ads_service_) {
//     return DEFAULT_ADS_PER_HOUR;
//   }
//   return ads_service_->GetMaximumNotificationAdsPerHour();
// }

// void WootzRewardsNativeWorker::SetAdsPerHour(JNIEnv* env, jint value) {
//   ProfileManager::GetActiveUserProfile()
//       ->GetOriginalProfile()
//       ->GetPrefs()
//       ->SetInt64(wootz_ads::prefs::kMaximumNotificationAdsPerHour,
//                  static_cast<int64_t>(value));
// }

void WootzRewardsNativeWorker::SetAutoContributionAmount(JNIEnv* env,
                                                         jdouble value) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->SetAutoContributionAmount(value);
  }
}

void WootzRewardsNativeWorker::GetAutoContributionAmount(JNIEnv* env) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->GetAutoContributionAmount(
        base::BindOnce(&WootzRewardsNativeWorker::OnGetAutoContributionAmount,
                       weak_factory_.GetWeakPtr()));
  }
}

void WootzRewardsNativeWorker::OnGetAutoContributionAmount(
    double auto_contribution_amount) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onGetAutoContributionAmount(
      env, weak_java_wootz_rewards_native_worker_.get(env),
      auto_contribution_amount);
}

void WootzRewardsNativeWorker::GetExternalWallet(JNIEnv* env) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->GetExternalWallet(
        base::BindOnce(&WootzRewardsNativeWorker::OnGetExternalWallet,
                       weak_factory_.GetWeakPtr()));
  }
}

bool WootzRewardsNativeWorker::IsTermsOfServiceUpdateRequired(JNIEnv* env) {
  if (!wootz_rewards_service_) {
    return false;
  }
  return wootz_rewards_service_->IsTermsOfServiceUpdateRequired();
}

void WootzRewardsNativeWorker::AcceptTermsOfServiceUpdate(JNIEnv* env) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->AcceptTermsOfServiceUpdate();
  }
}

base::android::ScopedJavaLocalRef<jstring>
WootzRewardsNativeWorker::GetCountryCode(JNIEnv* env) {
  std::string country_code;
  if (wootz_rewards_service_) {
    country_code = wootz_rewards_service_->GetCountryCode();
  }

  return base::android::ConvertUTF8ToJavaString(env, country_code);
}

void WootzRewardsNativeWorker::GetAvailableCountries(JNIEnv* env) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->GetAvailableCountries(
        base::BindOnce(&WootzRewardsNativeWorker::OnGetAvailableCountries,
                       weak_factory_.GetWeakPtr()));
  }
}

void WootzRewardsNativeWorker::OnGetAvailableCountries(
    std::vector<std::string> countries) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onGetAvailableCountries(
      env, weak_java_wootz_rewards_native_worker_.get(env),
      base::android::ToJavaArrayOfStrings(env, countries));
}

void WootzRewardsNativeWorker::GetPublishersVisitedCount(JNIEnv* env) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->GetPublishersVisitedCount(
        base::BindOnce(&WootzRewardsNativeWorker::OnGetPublishersVisitedCount,
                       weak_factory_.GetWeakPtr()));
  }
}

void WootzRewardsNativeWorker::OnGetPublishersVisitedCount(int count) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onGetPublishersVisitedCount(
      env, weak_java_wootz_rewards_native_worker_.get(env), count);
}

void WootzRewardsNativeWorker::GetPublisherBanner(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& publisher_key) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->GetPublisherBanner(
        base::android::ConvertJavaStringToUTF8(env, publisher_key),
        base::BindOnce(&WootzRewardsNativeWorker::onPublisherBanner,
                       weak_factory_.GetWeakPtr()));
  }
}

void WootzRewardsNativeWorker::onPublisherBanner(
    wootz_rewards::mojom::PublisherBannerPtr banner) {
  std::string json_banner_info;
  if (!banner) {
    json_banner_info = "";
  } else {
    base::Value::Dict dict;
    dict.Set("publisher_key", banner->publisher_key);
    dict.Set("title", banner->title);

    dict.Set("name", banner->name);
    dict.Set("description", banner->description);
    dict.Set("background", banner->background);
    dict.Set("logo", banner->logo);
    dict.Set("provider", banner->provider);
    dict.Set("web3_url", banner->web3_url);

    base::Value::Dict links;
    for (auto const& link : banner->links) {
      links.Set(link.first, link.second);
    }
    dict.Set("links", std::move(links));

    dict.Set("status", static_cast<int32_t>(banner->status));
    base::JSONWriter::Write(dict, &json_banner_info);
  }
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onPublisherBanner(
      env, weak_java_wootz_rewards_native_worker_.get(env),
      base::android::ConvertUTF8ToJavaString(env, json_banner_info));
}

void WootzRewardsNativeWorker::OnGetExternalWallet(
    wootz_rewards::mojom::ExternalWalletPtr wallet) {
  std::string json_wallet;
  if (!wallet) {
    // If the user does not have an external wallet, expose a default/empty
    // wallet for backward compatibility with Android code that expects an
    // external wallet structure with a NOT_CONNECTED status.
    base::Value::Dict dict;
    dict.Set("token", "");
    dict.Set("address", "");
    dict.Set("status", static_cast<int32_t>(
                           wootz_rewards::mojom::WalletStatus::kNotConnected));
    dict.Set("type", wootz_rewards_service_->GetExternalWalletType());
    dict.Set("user_name", "");
    dict.Set("account_url", "");
    base::JSONWriter::Write(dict, &json_wallet);
  } else {
    base::Value::Dict dict;
    dict.Set("token", wallet->token);
    dict.Set("address", wallet->address);

    // enum class WalletStatus : int32_t
    dict.Set("status", static_cast<int32_t>(wallet->status));
    dict.Set("type", wallet->type);
    dict.Set("user_name", wallet->user_name);
    dict.Set("account_url", wallet->account_url);
    base::JSONWriter::Write(dict, &json_wallet);
  }
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onGetExternalWallet(
      env, weak_java_wootz_rewards_native_worker_.get(env),
      base::android::ConvertUTF8ToJavaString(env, json_wallet));
}

void WootzRewardsNativeWorker::DisconnectWallet(JNIEnv* env) {
  // TODO(zenparsing): Remove disconnect ability from Android UI.
}

void WootzRewardsNativeWorker::OnExternalWalletConnected() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onExternalWalletConnected(
      env, weak_java_wootz_rewards_native_worker_.get(env));
}

void WootzRewardsNativeWorker::OnExternalWalletLoggedOut() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onExternalWalletLoggedOut(
      env, weak_java_wootz_rewards_native_worker_.get(env));
}

void WootzRewardsNativeWorker::OnExternalWalletReconnected() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onExternalWalletReconnected(
      env, weak_java_wootz_rewards_native_worker_.get(env));
}

std::string WootzRewardsNativeWorker::StdStrStrMapToJsonString(
    const base::flat_map<std::string, std::string>& args) {
    std::string json_args;
    base::Value::Dict dict;
    for (const auto & item : args) {
      dict.Set(item.first, item.second);
    }
    base::JSONWriter::Write(dict, &json_args);
    return json_args;
}

void WootzRewardsNativeWorker::RefreshPublisher(
    JNIEnv* env,
    const base::android::JavaParamRef<jstring>& publisher_key) {
  if (!wootz_rewards_service_) {
    NOTREACHED_IN_MIGRATION();
    return;
  }
  wootz_rewards_service_->RefreshPublisher(
      base::android::ConvertJavaStringToUTF8(env, publisher_key),
      base::BindOnce(&WootzRewardsNativeWorker::OnRefreshPublisher,
                     weak_factory_.GetWeakPtr()));
}

void WootzRewardsNativeWorker::OnRefreshPublisher(
    const wootz_rewards::mojom::PublisherStatus status,
    const std::string& publisher_key) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzRewardsNativeWorker_onRefreshPublisher(
      env, weak_java_wootz_rewards_native_worker_.get(env),
      static_cast<int>(status),
      base::android::ConvertUTF8ToJavaString(env, publisher_key));
}

void WootzRewardsNativeWorker::SetAutoContributeEnabled(
    JNIEnv* env,
    bool isAutoContributeEnabled) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->SetAutoContributeEnabled(isAutoContributeEnabled);
  }
}

void WootzRewardsNativeWorker::RecordPanelTrigger(JNIEnv* env) {
  if (wootz_rewards_service_) {
    wootz_rewards_service_->GetP3AConversionMonitor()->RecordPanelTrigger(
        wootz_rewards::p3a::PanelTrigger::kToolbarButton);
  }
}

static void JNI_WootzRewardsNativeWorker_Init(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& jcaller) {
  new WootzRewardsNativeWorker(env, jcaller);
}

}  // namespace android
}  // namespace chrome