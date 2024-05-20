// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_FACILITATED_PAYMENTS_UI_CHROME_FACILITATED_PAYMENTS_CLIENT_H_
#define CHROME_BROWSER_FACILITATED_PAYMENTS_UI_CHROME_FACILITATED_PAYMENTS_CLIENT_H_

#include "base/gtest_prod_util.h"
#include "chrome/browser/facilitated_payments/ui/android/facilitated_payments_controller.h"
#include "components/facilitated_payments/content/browser/content_facilitated_payments_driver_factory.h"
#include "components/facilitated_payments/core/browser/facilitated_payments_client.h"
#include "content/public/browser/web_contents_user_data.h"

namespace content {
class WebContents;
}  // namespace content

namespace optimization_guide {
class OptimizationGuideDecider;
}  // namespace optimization_guide

// Chrome implementation of `FacilitatedPaymentsClient`. `WebContents` owns 1
// instance of this class. Creates and owns
// `ContentFacilitatedPaymentsDriverFactory`.
class ChromeFacilitatedPaymentsClient
    : public payments::facilitated::FacilitatedPaymentsClient,
      public content::WebContentsUserData<ChromeFacilitatedPaymentsClient> {
 public:
  ChromeFacilitatedPaymentsClient(
      content::WebContents* web_contents,
      optimization_guide::OptimizationGuideDecider* optimization_guide_decider);
  ChromeFacilitatedPaymentsClient(const ChromeFacilitatedPaymentsClient&) =
      delete;
  ChromeFacilitatedPaymentsClient& operator=(
      const ChromeFacilitatedPaymentsClient&) = delete;
  ~ChromeFacilitatedPaymentsClient() override;

  // RiskDataLoader:
  void LoadRiskData(base::OnceCallback<void(const std::string&)>
                        on_risk_data_loaded_callback) override;

 private:
  friend class content::WebContentsUserData<ChromeFacilitatedPaymentsClient>;

  FRIEND_TEST_ALL_PREFIXES(ChromeFacilitatedPaymentsClientTest,
                           GetPaymentsDataManager);
  FRIEND_TEST_ALL_PREFIXES(ChromeFacilitatedPaymentsClientTest,
                           GetFacilitatedPaymentsNetworkInterface);

  // FacilitatedPaymentsClient:
  // This returns nullptr if the `Profile` associated is null.
  autofill::PaymentsDataManager* GetPaymentsDataManager() override;
  // This returns nullptr if the `Profile` associated is null.
  payments::facilitated::FacilitatedPaymentsNetworkInterface*
  GetFacilitatedPaymentsNetworkInterface() override;
  // This returns std::nullopt if the `Profile` associated is null.
  std::optional<CoreAccountInfo> GetCoreAccountInfo() override;
  bool ShowPixPaymentPrompt(
      base::span<autofill::BankAccount> bank_account_suggestions,
      base::OnceCallback<void(bool, int64_t)> on_user_decision_callback)
      override;

  payments::facilitated::ContentFacilitatedPaymentsDriverFactory
      driver_factory_;

  std::unique_ptr<payments::facilitated::FacilitatedPaymentsNetworkInterface>
      facilitated_payments_network_interface_;

#if BUILDFLAG(IS_ANDROID)
  FacilitatedPaymentsController facilitated_payments_controller_;
#endif

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

#endif  // CHROME_BROWSER_FACILITATED_PAYMENTS_UI_CHROME_FACILITATED_PAYMENTS_CLIENT_H_
