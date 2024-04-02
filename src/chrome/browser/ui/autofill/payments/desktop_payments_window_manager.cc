// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/autofill/payments/desktop_payments_window_manager.h"

#include "base/check_deref.h"
#include "base/functional/callback_helpers.h"
#include "base/types/expected.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_navigator.h"
#include "chrome/browser/ui/browser_navigator_params.h"
#include "components/autofill/content/browser/content_autofill_client.h"
#include "components/autofill/core/browser/autofill_progress_dialog_type.h"
#include "components/autofill/core/browser/payments/payments_autofill_client.h"
#include "components/autofill/core/browser/payments/payments_requests/unmask_card_request.h"
#include "components/autofill/core/browser/payments/payments_util.h"
#include "components/autofill/core/browser/payments/payments_window_manager_util.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/web_contents.h"
#include "url/gurl.h"

namespace autofill::payments {

DesktopPaymentsWindowManager::DesktopPaymentsWindowManager(
    ContentAutofillClient* client)
    : client_(CHECK_DEREF(client)) {
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS_LACROS)
  scoped_observation_.Observe(BrowserList::GetInstance());
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS_LACROS)
}

DesktopPaymentsWindowManager::~DesktopPaymentsWindowManager() = default;

void DesktopPaymentsWindowManager::InitVcn3dsAuthentication(
    Vcn3dsContext context) {
  CHECK_EQ(flow_type_, FlowType::kNoFlow);
  CHECK_EQ(context.card.record_type(), CreditCard::RecordType::kVirtualCard);
  CHECK(!context.completion_callback.is_null());
  flow_type_ = FlowType::kVcn3ds;
  vcn_3ds_context_ = std::move(context);
  CreatePopup(vcn_3ds_context_->challenge_option.url_to_open);
}

void DesktopPaymentsWindowManager::WebContentsDestroyed() {
  if (flow_type_ == FlowType::kVcn3ds) {
    OnWebContentsDestroyedForVcn3ds();
  }
}

#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS_LACROS)
void DesktopPaymentsWindowManager::OnBrowserSetLastActive(Browser* browser) {
  // If there is an ongoing payments window manager pop-up flow, and the
  // original tab's WebContents become active, activate the pop-up's
  // WebContents. This functionality is only required on Linux and LaCros, as on
  // other desktop platforms the pop-up will always be the top-most browser
  // window due to differences in window management on these platforms.
  if (web_contents()) {
    CHECK_NE(flow_type_, FlowType::kNoFlow);
    if (browser->tab_strip_model()->GetActiveWebContents() ==
        &client_->GetWebContents()) {
      web_contents()->GetDelegate()->ActivateContents(web_contents());
    }
  }
}
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS_LACROS)

void DesktopPaymentsWindowManager::CreatePopup(const GURL& url) {
  // Create a pop-up window. The created pop-up will not have any relationship
  // to the underlying tab, because `params.opener` is not set. Ensuring the
  // original tab is not a related site instance to the pop-up is critical for
  // security reasons.
  content::WebContents& source_contents = client_->GetWebContents();
  NavigateParams params(
      Profile::FromBrowserContext(source_contents.GetBrowserContext()), url,
      ui::PAGE_TRANSITION_LINK);
  params.disposition = WindowOpenDisposition::NEW_POPUP;
  params.window_action = NavigateParams::SHOW_WINDOW;
  params.source_contents = &source_contents;
  params.is_tab_modal_popup = true;

  if (base::WeakPtr<content::NavigationHandle> navigation_handle =
          Navigate(&params)) {
    content::WebContentsObserver::Observe(navigation_handle->GetWebContents());
  } else {
    client_->GetPaymentsAutofillClient()->ShowAutofillErrorDialog(
        AutofillErrorDialogContext::WithVirtualCardPermanentOrTemporaryError(
            /*is_permanent_error=*/false));
  }
}

void DesktopPaymentsWindowManager::OnWebContentsDestroyedForVcn3ds() {
  flow_type_ = FlowType::kNoFlow;
  base::expected<RedirectCompletionProof, Vcn3dsAuthenticationPopupErrorType>
      result = ParseFinalUrlForVcn3ds(web_contents()->GetVisibleURL());
  if (result.has_value()) {
    CHECK(!result.value()->empty());
    return client_->GetPaymentsAutofillClient()->LoadRiskData(base::BindOnce(
        &DesktopPaymentsWindowManager::OnDidLoadRiskDataForVcn3ds,
        weak_ptr_factory_.GetWeakPtr(), std::move(result.value())));
  }

  switch (result.error()) {
    case Vcn3dsAuthenticationPopupErrorType::kAuthenticationFailed:
    case Vcn3dsAuthenticationPopupErrorType::kInvalidQueryParams:
      client_->GetPaymentsAutofillClient()->ShowAutofillErrorDialog(
          AutofillErrorDialogContext::WithVirtualCardPermanentOrTemporaryError(
              /*is_permanent_error=*/true));
      break;
    case Vcn3dsAuthenticationPopupErrorType::kAuthenticationNotCompleted:
      break;
  }

  // In the case of an error, we show the user an error dialog but still run the
  // callback to let the caller know failure occurred.
  std::move(vcn_3ds_context_->completion_callback)
      .Run(Vcn3dsAuthenticationResponse());
  vcn_3ds_context_.reset();
}

void DesktopPaymentsWindowManager::OnDidLoadRiskDataForVcn3ds(
    RedirectCompletionProof redirect_completion_proof,
    const std::string& risk_data) {
  client_->GetPaymentsAutofillClient()->ShowAutofillProgressDialog(
      AutofillProgressDialogType::kVirtualCardUnmaskProgressDialog,
      base::BindOnce(&DesktopPaymentsWindowManager::
                         OnVcn3dsAuthenticationProgressDialogCancelled,
                     weak_ptr_factory_.GetWeakPtr()));
  client_->GetPaymentsAutofillClient()
      ->GetPaymentsNetworkInterface()
      ->UnmaskCard(CreateUnmaskRequestDetailsForVcn3ds(
                       *client_, vcn_3ds_context_.value(),
                       std::move(redirect_completion_proof)),
                   base::BindOnce(&DesktopPaymentsWindowManager::
                                      OnVcn3dsAuthenticationResponseReceived,
                                  weak_ptr_factory_.GetWeakPtr()));
}

void DesktopPaymentsWindowManager::OnVcn3dsAuthenticationResponseReceived(
    AutofillClient::PaymentsRpcResult result,
    const PaymentsNetworkInterface::UnmaskResponseDetails& response_details) {
  Vcn3dsAuthenticationResponse response = CreateVcn3dsAuthenticationResponse(
      result, response_details, std::move(vcn_3ds_context_->card));
  client_->GetPaymentsAutofillClient()->CloseAutofillProgressDialog(
      /*show_confirmation_before_closing=*/response.card.has_value(),
      /*no_interactive_authentication_callback=*/base::OnceClosure());
  if (!response.card.has_value()) {
    client_->GetPaymentsAutofillClient()->ShowAutofillErrorDialog(
        AutofillErrorDialogContext::WithVirtualCardPermanentOrTemporaryError(
            /*is_permanent_error=*/true));
  }

  std::move(vcn_3ds_context_->completion_callback).Run(std::move(response));
  vcn_3ds_context_.reset();
}

void DesktopPaymentsWindowManager::
    OnVcn3dsAuthenticationProgressDialogCancelled() {
  client_->GetPaymentsAutofillClient()
      ->GetPaymentsNetworkInterface()
      ->CancelRequest();
  vcn_3ds_context_.reset();
}

}  // namespace autofill::payments
