// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/webid/fedcm_account_selection_view_desktop.h"

#include <memory>
#include <string>

#include "base/test/metrics/histogram_tester.h"
#include "chrome/browser/ui/views/chrome_constrained_window_views_client.h"
#include "chrome/browser/ui/views/webid/account_selection_bubble_view.h"
#include "chrome/test/base/browser_with_test_window_test.h"
#include "chrome/test/base/testing_profile.h"
#include "chrome/test/views/chrome_views_test_base.h"
#include "components/constrained_window/constrained_window_views.h"
#include "content/public/browser/identity_request_dialog_controller.h"
#include "content/public/test/test_renderer_host.h"
#include "content/public/test/web_contents_tester.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/views/test/mock_input_event_activation_protector.h"
#include "url/gurl.h"

using LoginState = content::IdentityRequestAccount::LoginState;
using SignInMode = content::IdentityRequestAccount::SignInMode;
using TokenError = content::IdentityCredentialTokenError;
using DismissReason = content::IdentityRequestDialogController::DismissReason;

namespace {

constexpr char kTopFrameEtldPlusOne[] = "top-frame-example.com";
constexpr char kIframeEtldPlusOne[] = "iframe-example.com";
constexpr char kIdpEtldPlusOne[] = "idp-example.com";
constexpr char kConfigUrl[] = "https://idp-example.com/fedcm.json";
constexpr char kLoginUrl[] = "https://idp-example.com/login";

// Mock AccountSelectionViewBase which tracks state.
class TestAccountSelectionView : public AccountSelectionViewBase {
 public:
  enum class SheetType {
    kAccountPicker,
    kConfirmAccount,
    kVerifying,
    kFailure,
    kError,
    kRequestPermission,
    kLoading
  };

  explicit TestAccountSelectionView(views::Widget* dialog_widget)
      : dialog_widget_(dialog_widget) {}
  ~TestAccountSelectionView() override = default;

  TestAccountSelectionView(const TestAccountSelectionView&) = delete;
  TestAccountSelectionView& operator=(const TestAccountSelectionView&) = delete;

  void ShowMultiAccountPicker(
      const std::vector<IdentityProviderDisplayData>& idp_data_list) override {
    show_back_button_ = false;
    sheet_type_ = SheetType::kAccountPicker;

    account_ids_.clear();
    for (content::IdentityRequestAccount account : idp_data_list[0].accounts) {
      account_ids_.push_back(account.id);
    }
  }

  void ShowVerifyingSheet(const content::IdentityRequestAccount& account,
                          const IdentityProviderDisplayData& idp_data,
                          const std::u16string& title) override {
    sheet_type_ = SheetType::kVerifying;
    account_ids_ = {account.id};
  }

  void ShowSingleAccountConfirmDialog(
      const std::u16string& top_frame_for_display,
      const std::optional<std::u16string>& iframe_for_display,
      const content::IdentityRequestAccount& account,
      const IdentityProviderDisplayData& idp_data,
      bool show_back_button) override {
    show_back_button_ = show_back_button;
    sheet_type_ = SheetType::kConfirmAccount;
    account_ids_ = {account.id};
  }

  void ShowFailureDialog(
      const std::u16string& top_frame_for_display,
      const std::optional<std::u16string>& iframe_for_display,
      const std::u16string& idp_for_display,
      const content::IdentityProviderMetadata& idp_metadata) override {
    sheet_type_ = SheetType::kFailure;
    account_ids_ = {};
  }

  void ShowErrorDialog(const std::u16string& top_frame_for_display,
                       const std::optional<std::u16string>& iframe_for_display,
                       const std::u16string& idp_for_display,
                       const content::IdentityProviderMetadata& idp_metadata,
                       const std::optional<TokenError>& error) override {
    sheet_type_ = SheetType::kError;
    account_ids_ = {};
  }

  void ShowRequestPermissionDialog(
      const std::u16string& top_frame_for_display,
      const content::IdentityRequestAccount& account,
      const IdentityProviderDisplayData& idp_display_data) override {
    sheet_type_ = SheetType::kRequestPermission;
    account_ids_ = {account.id};
  }

  void ShowLoadingDialog() override {
    sheet_type_ = SheetType::kLoading;
    account_ids_ = {};
  }

  std::string GetDialogTitle() const override { return std::string(); }
  std::optional<std::string> GetDialogSubtitle() const override {
    return std::nullopt;
  }

  void InitDialogWidget() override {}
  void CloseDialog() override {}
  base::WeakPtr<views::Widget> GetDialogWidget() override {
    return dialog_widget_->GetWeakPtr();
  }

  bool show_back_button_{false};
  std::optional<SheetType> sheet_type_;
  std::vector<std::string> account_ids_;
  raw_ptr<views::Widget> dialog_widget_;
};

// Mock version of FedCmModalDialogView for injection during tests.
class MockFedCmModalDialogView : public FedCmModalDialogView {
 public:
  explicit MockFedCmModalDialogView(content::WebContents* web_contents,
                                    FedCmModalDialogView::Observer* observer)
      : FedCmModalDialogView(web_contents, observer) {}
  ~MockFedCmModalDialogView() override = default;

  MockFedCmModalDialogView(const MockFedCmModalDialogView&) = delete;
  MockFedCmModalDialogView& operator=(const MockFedCmModalDialogView&) = delete;

  MOCK_METHOD(content::WebContents*,
              ShowPopupWindow,
              (const GURL& url),
              (override));

  void ClosePopupWindow() override {
    FedCmModalDialogView::Observer* observer = GetObserverForTesting();
    if (observer) {
      observer->OnPopupWindowDestroyed();
    }
  }
};

// Test FedCmAccountSelectionView which uses TestAccountSelectionView.
class TestFedCmAccountSelectionView : public FedCmAccountSelectionView {
 public:
  TestFedCmAccountSelectionView(
      Delegate* delegate,
      TestAccountSelectionView* account_selection_view)
      : FedCmAccountSelectionView(delegate),
        account_selection_view_(account_selection_view) {
    auto input_protector =
        std::make_unique<views::MockInputEventActivationProtector>();
    ON_CALL(*input_protector, IsPossiblyUnintendedInteraction)
        .WillByDefault(testing::Return(false));
    SetInputEventActivationProtectorForTesting(std::move(input_protector));
  }

  TestFedCmAccountSelectionView(const TestFedCmAccountSelectionView&) = delete;
  TestFedCmAccountSelectionView& operator=(
      const TestFedCmAccountSelectionView&) = delete;

  blink::mojom::RpContext GetRpContext() { return rp_context_; }
  size_t num_dialogs_{0u};

 protected:
  AccountSelectionViewBase* CreateAccountSelectionView(
      const std::u16string& top_frame_etld_plus_one,
      const std::optional<std::u16string>& iframe_etld_plus_one,
      const std::optional<std::u16string>& idp_title,
      blink::mojom::RpContext rp_context,
      blink::mojom::RpMode rp_mode,
      bool has_modal_support) override {
    ++num_dialogs_;
    rp_context_ = rp_context;
    if (rp_mode == blink::mojom::RpMode::kButton && has_modal_support) {
      dialog_type_ = FedCmAccountSelectionView::DialogType::MODAL;
    }
    return account_selection_view_;
  }

  FedCmAccountSelectionView::DialogType GetDialogType() override {
    return dialog_type_;
  }

 private:
  raw_ptr<TestAccountSelectionView> account_selection_view_;
  blink::mojom::RpContext rp_context_;
  FedCmAccountSelectionView::DialogType dialog_type_{
      FedCmAccountSelectionView::DialogType::BUBBLE};
};

// Stub AccountSelectionView::Delegate.
class StubAccountSelectionViewDelegate : public AccountSelectionView::Delegate {
 public:
  explicit StubAccountSelectionViewDelegate(content::WebContents* web_contents)
      : web_contents_(web_contents) {}
  ~StubAccountSelectionViewDelegate() override = default;

  StubAccountSelectionViewDelegate(const StubAccountSelectionViewDelegate&) =
      delete;
  StubAccountSelectionViewDelegate& operator=(
      const StubAccountSelectionViewDelegate&) = delete;

  void OnAccountSelected(const GURL&,
                         const content::IdentityRequestAccount&) override {}
  void OnDismiss(DismissReason dismiss_reason) override {
    dismiss_reason_ = dismiss_reason;
  }
  void OnLoginToIdP(const GURL& idp_config_url,
                    const GURL& idp_login_url) override {}
  void OnMoreDetails() override {}
  void OnAccountsDisplayed() override {}
  gfx::NativeView GetNativeView() override { return gfx::NativeView(); }

  content::WebContents* GetWebContents() override { return web_contents_; }
  const DismissReason& GetDismissReason() { return dismiss_reason_; }

 private:
  raw_ptr<content::WebContents> web_contents_;
  DismissReason dismiss_reason_;
};

}  // namespace

class FedCmAccountSelectionViewDesktopTest : public ChromeViewsTestBase {
 public:
  void SetUp() override {
    ChromeViewsTestBase::SetUp();

    test_web_contents_ =
        content::WebContentsTester::CreateTestWebContents(&profile_, nullptr);
    delegate_ = std::make_unique<StubAccountSelectionViewDelegate>(
        test_web_contents_.get());

    dialog_widget_.reset(CreateTestWidget().release());
    account_selection_view_ =
        std::make_unique<TestAccountSelectionView>(dialog_widget_.get());
  }

  IdentityProviderDisplayData CreateIdentityProviderDisplayData(
      const std::vector<std::pair<std::string, LoginState>>& account_infos,
      bool has_login_status_mismatch = false) {
    std::vector<content::IdentityRequestAccount> accounts;
    for (const auto& account_info : account_infos) {
      accounts.emplace_back(account_info.first, "", "", "", GURL(),
                            /*login_hints=*/std::vector<std::string>(),
                            /*domain_hints=*/std::vector<std::string>(),
                            account_info.second);
    }
    return IdentityProviderDisplayData(
        u"", content::IdentityProviderMetadata(),
        content::ClientMetadata(GURL(), GURL()), std::move(accounts),
        /*request_permission=*/true, has_login_status_mismatch);
  }

  std::unique_ptr<TestFedCmAccountSelectionView> CreateAndShow(
      const std::vector<content::IdentityRequestAccount>& accounts,
      SignInMode sign_in_mode,
      blink::mojom::RpMode rp_mode = blink::mojom::RpMode::kWidget) {
    auto controller = std::make_unique<TestFedCmAccountSelectionView>(
        delegate_.get(), account_selection_view_.get());
    Show(*controller, accounts, sign_in_mode, rp_mode);
    return controller;
  }

  void Show(TestFedCmAccountSelectionView& controller,
            const std::vector<content::IdentityRequestAccount>& accounts,
            SignInMode sign_in_mode,
            blink::mojom::RpMode rp_mode,
            const std::optional<content::IdentityProviderData>&
                new_account_idp = std::nullopt) {
    controller.Show(
        kTopFrameEtldPlusOne,
        std::make_optional<std::string>(kIframeEtldPlusOne),
        {{kIdpEtldPlusOne, accounts, content::IdentityProviderMetadata(),
          content::ClientMetadata(GURL(), GURL()),
          blink::mojom::RpContext::kSignIn, /*request_permission=*/true,
          /*has_login_status_mismatch=*/false}},
        sign_in_mode, rp_mode, new_account_idp);
  }

  std::unique_ptr<TestFedCmAccountSelectionView> CreateAndShowMismatchDialog(
      blink::mojom::RpContext rp_context = blink::mojom::RpContext::kSignIn,
      blink::mojom::RpMode rp_mode = blink::mojom::RpMode::kWidget) {
    auto controller = std::make_unique<TestFedCmAccountSelectionView>(
        delegate_.get(), account_selection_view_.get());
    controller->ShowFailureDialog(kTopFrameEtldPlusOne, kIframeEtldPlusOne,
                                  kIdpEtldPlusOne, rp_context, rp_mode,
                                  content::IdentityProviderMetadata());
    EXPECT_EQ(TestAccountSelectionView::SheetType::kFailure,
              account_selection_view_->sheet_type_);
    return controller;
  }

  std::unique_ptr<TestFedCmAccountSelectionView> CreateAndShowErrorDialog(
      blink::mojom::RpContext rp_context = blink::mojom::RpContext::kSignIn,
      blink::mojom::RpMode rp_mode = blink::mojom::RpMode::kWidget) {
    auto controller = std::make_unique<TestFedCmAccountSelectionView>(
        delegate_.get(), account_selection_view_.get());
    controller->ShowErrorDialog(
        kTopFrameEtldPlusOne, kIframeEtldPlusOne, kIdpEtldPlusOne, rp_context,
        rp_mode, content::IdentityProviderMetadata(), /*error=*/std::nullopt);
    EXPECT_EQ(TestAccountSelectionView::SheetType::kError,
              account_selection_view_->sheet_type_);
    return controller;
  }

  std::unique_ptr<TestFedCmAccountSelectionView> CreateAndShowLoadingDialog(
      blink::mojom::RpContext rp_context = blink::mojom::RpContext::kSignIn,
      blink::mojom::RpMode rp_mode = blink::mojom::RpMode::kButton) {
    auto controller = std::make_unique<TestFedCmAccountSelectionView>(
        delegate_.get(), account_selection_view_.get());
    controller->ShowLoadingDialog(kTopFrameEtldPlusOne, kIdpEtldPlusOne,
                                  rp_context, rp_mode);
    EXPECT_EQ(TestAccountSelectionView::SheetType::kLoading,
              account_selection_view_->sheet_type_);
    return controller;
  }

  void CreateAndShowPopupWindow(TestFedCmAccountSelectionView& controller) {
    auto idp_signin_popup_window = std::make_unique<MockFedCmModalDialogView>(
        test_web_contents_.get(), &controller);
    EXPECT_CALL(*idp_signin_popup_window, ShowPopupWindow).Times(1);
    controller.SetIdpSigninPopupWindowForTesting(
        std::move(idp_signin_popup_window));

    controller.ShowModalDialog(GURL(u"https://example.com"));
  }

  std::unique_ptr<TestFedCmAccountSelectionView> CreateAndShowMultiIdp(
      const std::vector<IdentityProviderDisplayData>& idp_list,
      SignInMode sign_in_mode,
      blink::mojom::RpMode rp_mode) {
    auto controller = std::make_unique<TestFedCmAccountSelectionView>(
        delegate_.get(), account_selection_view_.get());
    std::vector<content::IdentityProviderData> idp_data;
    for (const auto& idp : idp_list) {
      idp_data.emplace_back(
          reinterpret_cast<const char*>(idp.idp_etld_plus_one.data()),
          idp.accounts, idp.idp_metadata, idp.client_metadata,
          blink::mojom::RpContext::kSignIn, idp.request_permission,
          idp.has_login_status_mismatch);
    }
    controller->Show(kTopFrameEtldPlusOne,
                     std::make_optional<std::string>(kIframeEtldPlusOne),
                     idp_data, sign_in_mode, rp_mode,
                     /*new_account_idp=*/std::nullopt);
    return controller;
  }

  ui::MouseEvent CreateMouseEvent() {
    return ui::MouseEvent(ui::ET_MOUSE_PRESSED, gfx::Point(), gfx::Point(),
                          base::TimeTicks(), ui::EF_LEFT_MOUSE_BUTTON, 0);
  }

 protected:
  TestingProfile profile_;

  // This enables uses of TestWebContents.
  content::RenderViewHostTestEnabler test_render_host_factories_;

  std::unique_ptr<content::WebContents> test_web_contents_;
  views::ViewsTestBase::WidgetAutoclosePtr dialog_widget_;
  std::unique_ptr<TestAccountSelectionView> account_selection_view_;
  std::unique_ptr<AccountSelectionView::Delegate> delegate_;

  base::HistogramTester histogram_tester_;
};

TEST_F(FedCmAccountSelectionViewDesktopTest, SingleAccountFlow) {
  const char kAccountId[] = "account_id";
  IdentityProviderDisplayData idp_data =
      CreateIdentityProviderDisplayData({{kAccountId, LoginState::kSignUp}});
  const std::vector<Account>& accounts = idp_data.accounts;
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShow(accounts, SignInMode::kExplicit);
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());

  EXPECT_FALSE(account_selection_view_->show_back_button_);
  EXPECT_EQ(TestAccountSelectionView::SheetType::kConfirmAccount,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId));

  observer->OnAccountSelected(accounts[0], idp_data, CreateMouseEvent());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kVerifying,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId));
}

TEST_F(FedCmAccountSelectionViewDesktopTest, MultipleAccountFlowReturning) {
  const char kAccountId1[] = "account_id1";
  const char kAccountId2[] = "account_id2";
  IdentityProviderDisplayData idp_data = CreateIdentityProviderDisplayData(
      {{kAccountId1, LoginState::kSignIn}, {kAccountId2, LoginState::kSignIn}});
  const std::vector<Account>& accounts = idp_data.accounts;
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShow(accounts, SignInMode::kExplicit);
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());

  EXPECT_FALSE(account_selection_view_->show_back_button_);
  EXPECT_EQ(TestAccountSelectionView::SheetType::kAccountPicker,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId1, kAccountId2));

  observer->OnAccountSelected(accounts[0], idp_data, CreateMouseEvent());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kVerifying,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId1));
}

TEST_F(FedCmAccountSelectionViewDesktopTest, MultipleAccountFlowBack) {
  const char kAccountId1[] = "account_id1";
  const char kAccountId2[] = "account_id2";
  IdentityProviderDisplayData idp_data = CreateIdentityProviderDisplayData({
      {kAccountId1, LoginState::kSignUp},
      {kAccountId2, LoginState::kSignUp},
  });
  const std::vector<Account>& accounts = idp_data.accounts;
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShow(accounts, SignInMode::kExplicit);
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());

  EXPECT_FALSE(account_selection_view_->show_back_button_);
  EXPECT_EQ(TestAccountSelectionView::SheetType::kAccountPicker,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId1, kAccountId2));

  observer->OnAccountSelected(accounts[0], idp_data, CreateMouseEvent());
  EXPECT_TRUE(account_selection_view_->show_back_button_);
  EXPECT_EQ(TestAccountSelectionView::SheetType::kConfirmAccount,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId1));

  observer->OnBackButtonClicked();
  EXPECT_FALSE(account_selection_view_->show_back_button_);
  EXPECT_EQ(TestAccountSelectionView::SheetType::kAccountPicker,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId1, kAccountId2));

  observer->OnAccountSelected(accounts[1], idp_data, CreateMouseEvent());
  EXPECT_TRUE(account_selection_view_->show_back_button_);
  EXPECT_EQ(TestAccountSelectionView::SheetType::kConfirmAccount,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId2));

  observer->OnAccountSelected(accounts[1], idp_data, CreateMouseEvent());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kVerifying,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId2));
}

// Test transitioning from IdP sign-in status mismatch failure dialog to regular
// sign-in dialog.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       IdpSigninStatusMismatchDialogToSigninFlow) {
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowMismatchDialog();
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());

  EXPECT_EQ(TestAccountSelectionView::SheetType::kFailure,
            account_selection_view_->sheet_type_);

  const char kAccountId[] = "account_id";
  IdentityProviderDisplayData idp_data = CreateIdentityProviderDisplayData({
      {kAccountId, LoginState::kSignUp},
  });
  Show(*controller, idp_data.accounts, SignInMode::kExplicit,
       blink::mojom::RpMode::kWidget);

  EXPECT_EQ(TestAccountSelectionView::SheetType::kConfirmAccount,
            account_selection_view_->sheet_type_);
  observer->OnAccountSelected(idp_data.accounts[0], idp_data,
                              CreateMouseEvent());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kVerifying,
            account_selection_view_->sheet_type_);

  // Failure bubble should have been re-used for sign-in dialog.
  EXPECT_EQ(1u, controller->num_dialogs_);
}

// Test transitioning from IdP sign-in status mismatch failure dialog to regular
// sign-in dialog while the dialog is hidden. This emulates a user signing
// into the IdP in a different tab.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       IdpSigninStatusMismatchDialogToSigninFlowHidden) {
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowMismatchDialog();
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());

  EXPECT_EQ(TestAccountSelectionView::SheetType::kFailure,
            account_selection_view_->sheet_type_);

  const char kAccountId[] = "account_id";
  IdentityProviderDisplayData idp_data = CreateIdentityProviderDisplayData({
      {kAccountId, LoginState::kSignUp},
  });

  // If the user switched tabs to sign-into the IdP, Show() may be called while
  // the associated FedCM tab is inactive. Show() should not show the
  // views::Widget in this case.
  controller->OnVisibilityChanged(content::Visibility::HIDDEN);
  Show(*controller, idp_data.accounts, SignInMode::kExplicit,
       blink::mojom::RpMode::kWidget);
  EXPECT_FALSE(dialog_widget_->IsVisible());

  controller->OnVisibilityChanged(content::Visibility::VISIBLE);
  EXPECT_TRUE(dialog_widget_->IsVisible());

  EXPECT_EQ(TestAccountSelectionView::SheetType::kConfirmAccount,
            account_selection_view_->sheet_type_);
  observer->OnAccountSelected(idp_data.accounts[0], idp_data,
                              CreateMouseEvent());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kVerifying,
            account_selection_view_->sheet_type_);

  // Failure bubble should have been re-used for sign-in dialog.
  EXPECT_EQ(1u, controller->num_dialogs_);
}

TEST_F(FedCmAccountSelectionViewDesktopTest, AutoReauthnSingleAccountFlow) {
  const char kAccountId[] = "account_id";
  IdentityProviderDisplayData idp_data =
      CreateIdentityProviderDisplayData({{kAccountId, LoginState::kSignIn}});
  const std::vector<Account>& accounts = idp_data.accounts;
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShow(accounts, SignInMode::kAuto);

  EXPECT_FALSE(account_selection_view_->show_back_button_);
  EXPECT_EQ(TestAccountSelectionView::SheetType::kVerifying,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId));
}

namespace {

// AccountSelectionViewDelegate which deletes the FedCmAccountSelectionView in
// OnAccountSelected().
class ViewDeletingAccountSelectionViewDelegate
    : public StubAccountSelectionViewDelegate {
 public:
  explicit ViewDeletingAccountSelectionViewDelegate(
      content::WebContents* web_contents)
      : StubAccountSelectionViewDelegate(web_contents) {}
  ~ViewDeletingAccountSelectionViewDelegate() override = default;

  ViewDeletingAccountSelectionViewDelegate(
      const ViewDeletingAccountSelectionViewDelegate&) = delete;
  ViewDeletingAccountSelectionViewDelegate& operator=(
      const ViewDeletingAccountSelectionViewDelegate&) = delete;

  void SetView(std::unique_ptr<FedCmAccountSelectionView> view) {
    view_ = std::move(view);
  }

  void OnAccountSelected(const GURL&,
                         const content::IdentityRequestAccount&) override {
    view_.reset();
  }

 private:
  std::unique_ptr<FedCmAccountSelectionView> view_;
};

}  // namespace

TEST_F(FedCmAccountSelectionViewDesktopTest, AccountSelectedDeletesView) {
  delegate_ = std::make_unique<ViewDeletingAccountSelectionViewDelegate>(
      test_web_contents_.get());
  ViewDeletingAccountSelectionViewDelegate* view_deleting_delegate =
      static_cast<ViewDeletingAccountSelectionViewDelegate*>(delegate_.get());

  const char kAccountId1[] = "account_id1";
  IdentityProviderDisplayData idp_data = CreateIdentityProviderDisplayData({
      {kAccountId1, LoginState::kSignIn},
  });
  const std::vector<Account>& accounts = idp_data.accounts;

  AccountSelectionViewBase::Observer* observer = nullptr;
  {
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShow(accounts, SignInMode::kExplicit);
    observer =
        static_cast<AccountSelectionViewBase::Observer*>(controller.get());
    view_deleting_delegate->SetView(std::move(controller));
  }

  // Destroys FedCmAccountSelectionView. Should not cause crash.
  observer->OnAccountSelected(accounts[0], idp_data, CreateMouseEvent());
}

TEST_F(FedCmAccountSelectionViewDesktopTest, ClickProtection) {
  const char kAccountId[] = "account_id";
  IdentityProviderDisplayData idp_data =
      CreateIdentityProviderDisplayData({{kAccountId, LoginState::kSignUp}});
  const std::vector<Account>& accounts = idp_data.accounts;
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShow(accounts, SignInMode::kExplicit);
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());

  // Use a mock input protector to more easily test. The protector rejects the
  // first input and accepts any subsequent input.
  auto input_protector =
      std::make_unique<views::MockInputEventActivationProtector>();
  EXPECT_CALL(*input_protector, IsPossiblyUnintendedInteraction)
      .WillOnce(testing::Return(true))
      .WillRepeatedly(testing::Return(false));
  controller->SetInputEventActivationProtectorForTesting(
      std::move(input_protector));

  observer->OnAccountSelected(accounts[0], idp_data, CreateMouseEvent());
  // Nothing should change after first account selected.
  EXPECT_FALSE(account_selection_view_->show_back_button_);
  EXPECT_EQ(TestAccountSelectionView::SheetType::kConfirmAccount,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId));

  observer->OnAccountSelected(accounts[0], idp_data, CreateMouseEvent());
  // Should show verifying sheet after first account selected.
  EXPECT_EQ(TestAccountSelectionView::SheetType::kVerifying,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId));
}

// Tests that when the auth re-authn dialog is closed, the relevant metric is
// recorded.
TEST_F(FedCmAccountSelectionViewDesktopTest, CloseAutoReauthnSheetMetric) {
  const char kAccountId[] = "account_id";
  IdentityProviderDisplayData idp_data =
      CreateIdentityProviderDisplayData({{kAccountId, LoginState::kSignIn}});
  const std::vector<Account>& accounts = idp_data.accounts;
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShow(accounts, SignInMode::kAuto);
  histogram_tester_.ExpectTotalCount("Blink.FedCm.ClosedSheetType.Desktop", 0);

  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());
  observer->OnCloseButtonClicked(CreateMouseEvent());
  histogram_tester_.ExpectUniqueSample(
      "Blink.FedCm.ClosedSheetType.Desktop",
      static_cast<int>(FedCmAccountSelectionView::SheetType::AUTO_REAUTHN), 1);
}

// Tests that when the mismatch dialog is closed through the close icon, the
// relevant metric is recorded.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       MismatchDialogDismissedByCloseIconMetric) {
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowMismatchDialog();
  histogram_tester_.ExpectTotalCount(
      "Blink.FedCm.IdpSigninStatus.MismatchDialogResult", 0);

  // Emulate user clicking the close icon.
  dialog_widget_->CloseWithReason(
      views::Widget::ClosedReason::kCloseButtonClicked);
  controller->OnWidgetDestroying(dialog_widget_.get());

  histogram_tester_.ExpectUniqueSample(
      "Blink.FedCm.IdpSigninStatus.MismatchDialogResult",
      static_cast<int>(FedCmAccountSelectionView::MismatchDialogResult::
                           kDismissedByCloseIcon),
      1);
}

// Tests that when the mismatch dialog is closed through means other than the
// close icon, the relevant metric is recorded.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       MismatchDialogDismissedForOtherReasonsMetric) {
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowMismatchDialog();
  histogram_tester_.ExpectTotalCount(
      "Blink.FedCm.IdpSigninStatus.MismatchDialogResult", 0);

  // Emulate user closing the mismatch dialog for an unspecified reason.
  dialog_widget_->CloseWithReason(views::Widget::ClosedReason::kUnspecified);
  controller->OnWidgetDestroying(dialog_widget_.get());

  histogram_tester_.ExpectUniqueSample(
      "Blink.FedCm.IdpSigninStatus.MismatchDialogResult",
      static_cast<int>(FedCmAccountSelectionView::MismatchDialogResult::
                           kDismissedForOtherReasons),
      1);
}

// Tests that when FedCmAccountSelectionView is destroyed while the mismatch
// dialog is open, the relevant metric is recorded.
TEST_F(FedCmAccountSelectionViewDesktopTest, MismatchDialogDestroyedMetric) {
  {
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowMismatchDialog();
    histogram_tester_.ExpectTotalCount(
        "Blink.FedCm.IdpSigninStatus.MismatchDialogResult", 0);
  }

  histogram_tester_.ExpectUniqueSample(
      "Blink.FedCm.IdpSigninStatus.MismatchDialogResult",
      static_cast<int>(FedCmAccountSelectionView::MismatchDialogResult::
                           kDismissedForOtherReasons),
      1);
}

// Tests that when the continue button on the mismatch dialog is clicked, the
// relevant metric is recorded.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       MismatchDialogContinueClickedMetric) {
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowMismatchDialog();
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());
  histogram_tester_.ExpectTotalCount(
      "Blink.FedCm.IdpSigninStatus.MismatchDialogResult", 0);

  // Emulate user clicking on "Continue" button in the mismatch dialog.
  observer->OnLoginToIdP(GURL(kConfigUrl), GURL(kLoginUrl), CreateMouseEvent());
  CreateAndShowPopupWindow(*controller);

  histogram_tester_.ExpectUniqueSample(
      "Blink.FedCm.IdpSigninStatus.MismatchDialogResult",
      static_cast<int>(
          FedCmAccountSelectionView::MismatchDialogResult::kContinued),
      1);
}

// Tests that when the continue button on the mismatch dialog is clicked and
// then FedCmAccountSelectionView is destroyed, we record only the metric for
// the continue button being clicked.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       MismatchDialogContinueClickedThenDestroyedMetric) {
  {
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowMismatchDialog();
    AccountSelectionViewBase::Observer* observer =
        static_cast<AccountSelectionViewBase::Observer*>(controller.get());
    histogram_tester_.ExpectTotalCount(
        "Blink.FedCm.IdpSigninStatus.MismatchDialogResult", 0);

    // Emulate user clicking on "Continue" button in the mismatch dialog.
    observer->OnLoginToIdP(GURL(kConfigUrl), GURL(kLoginUrl),
                           CreateMouseEvent());
    CreateAndShowPopupWindow(*controller);
  }

  histogram_tester_.ExpectUniqueSample(
      "Blink.FedCm.IdpSigninStatus.MismatchDialogResult",
      static_cast<int>(
          FedCmAccountSelectionView::MismatchDialogResult::kContinued),
      1);
}

// Test transitioning from IdP sign-in status mismatch dialog to regular sign-in
// dialog. This emulates a user signing into the IdP in a pop-up window and the
// pop-up window closes PRIOR to the mismatch dialog being updated to a regular
// sign-in dialog.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       IdpSigninStatusPopupClosedBeforeAccountsPopulated) {
  {
    // Trigger IdP sign-in status mismatch dialog.
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowMismatchDialog();
    AccountSelectionViewBase::Observer* observer =
        static_cast<AccountSelectionViewBase::Observer*>(controller.get());

    // Emulate user clicking on "Continue" button in the mismatch dialog.
    observer->OnLoginToIdP(GURL(kConfigUrl), GURL(kLoginUrl),
                           CreateMouseEvent());
    CreateAndShowPopupWindow(*controller);

    // When pop-up window is shown, mismatch dialog should be hidden.
    EXPECT_FALSE(dialog_widget_->IsVisible());

    // Emulate user completing the sign-in flow and IdP prompts closing the
    // pop-up window.
    controller->CloseModalDialog();

    // Mismatch dialog should remain hidden because it has not been updated to
    // an accounts dialog yet.
    EXPECT_FALSE(dialog_widget_->IsVisible());

    histogram_tester_.ExpectTotalCount(
        "Blink.FedCm.IdpSigninStatus."
        "IdpClosePopupToBrowserShowAccountsDuration",
        0);
    histogram_tester_.ExpectTotalCount(
        "Blink.FedCm.IdpSigninStatus.PopupWindowResult", 0);

    // Emulate IdP sending the IdP sign-in status header which updates the
    // mismatch dialog to an accounts dialog.
    const char kAccountId[] = "account_id";
    IdentityProviderDisplayData idp_data = CreateIdentityProviderDisplayData({
        {kAccountId, LoginState::kSignUp},
    });
    Show(*controller, idp_data.accounts, SignInMode::kExplicit,
         blink::mojom::RpMode::kWidget);

    // Accounts dialog should now be visible. One account is logged in, so no
    // back button is shown.
    EXPECT_TRUE(dialog_widget_->IsVisible());
    EXPECT_FALSE(account_selection_view_->show_back_button_);
  }

  histogram_tester_.ExpectTotalCount(
      "Blink.FedCm.IdpSigninStatus.IdpClosePopupToBrowserShowAccountsDuration",
      1);
  histogram_tester_.ExpectUniqueSample(
      "Blink.FedCm.IdpSigninStatus.PopupWindowResult",
      static_cast<int>(FedCmAccountSelectionView::PopupWindowResult::
                           kAccountsReceivedAndPopupClosedByIdp),
      1);
}

// Test transitioning from IdP sign-in status mismatch dialog to regular sign-in
// dialog. This emulates a user signing into the IdP in a pop-up window and the
// pop-up window closes AFTER the mismatch dialog has been updated to a regular
// sign-in dialog.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       IdpSigninStatusPopupClosedAfterAccountsPopulated) {
  {
    // Trigger IdP sign-in status mismatch dialog.
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowMismatchDialog();
    AccountSelectionViewBase::Observer* observer =
        static_cast<AccountSelectionViewBase::Observer*>(controller.get());

    // Emulate user clicking on "Continue" button in the mismatch dialog.
    observer->OnLoginToIdP(GURL(kConfigUrl), GURL(kLoginUrl),
                           CreateMouseEvent());
    CreateAndShowPopupWindow(*controller);

    // When pop-up window is shown, mismatch dialog should be hidden.
    EXPECT_FALSE(dialog_widget_->IsVisible());

    // Emulate IdP sending the IdP sign-in status header which updates the
    // mismatch dialog to an accounts dialog.
    const char kAccountId[] = "account_id";
    IdentityProviderDisplayData idp_data = CreateIdentityProviderDisplayData({
        {kAccountId, LoginState::kSignUp},
    });
    Show(*controller, idp_data.accounts, SignInMode::kExplicit,
         blink::mojom::RpMode::kWidget);

    // Accounts dialog should remain hidden because the pop-up window has not
    // been closed yet.
    EXPECT_FALSE(dialog_widget_->IsVisible());

    histogram_tester_.ExpectTotalCount(
        "Blink.FedCm.IdpSigninStatus."
        "IdpClosePopupToBrowserShowAccountsDuration",
        0);
    histogram_tester_.ExpectTotalCount(
        "Blink.FedCm.IdpSigninStatus.PopupWindowResult", 0);

    // Emulate IdP closing the pop-up window.
    controller->CloseModalDialog();

    // Accounts dialog should now be visible.
    EXPECT_TRUE(dialog_widget_->IsVisible());
  }

  histogram_tester_.ExpectTotalCount(
      "Blink.FedCm.IdpSigninStatus."
      "IdpClosePopupToBrowserShowAccountsDuration",
      1);
  histogram_tester_.ExpectUniqueSample(
      "Blink.FedCm.IdpSigninStatus.PopupWindowResult",
      static_cast<int>(FedCmAccountSelectionView::PopupWindowResult::
                           kAccountsReceivedAndPopupClosedByIdp),
      1);
}

// Test that when user opens a pop-up window to complete the IDP sign-in flow,
// we record the appropriate metric when accounts are received but
// IdentityProvider.close() is not called.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       IdpSigninStatusAccountsReceivedAndNoPopupClosedByIdpMetric) {
  {
    // Trigger IdP sign-in status mismatch dialog.
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowMismatchDialog();
    AccountSelectionViewBase::Observer* observer =
        static_cast<AccountSelectionViewBase::Observer*>(controller.get());

    // Emulate user clicking on "Continue" button in the mismatch dialog.
    observer->OnLoginToIdP(GURL(kConfigUrl), GURL(kLoginUrl),
                           CreateMouseEvent());
    CreateAndShowPopupWindow(*controller);

    // Emulate IdP sending the IdP sign-in status header which updates the
    // failure dialog to an accounts dialog.
    const char kAccountId[] = "account_id";
    IdentityProviderDisplayData idp_data = CreateIdentityProviderDisplayData({
        {kAccountId, LoginState::kSignUp},
    });
    Show(*controller, idp_data.accounts, SignInMode::kExplicit,
         blink::mojom::RpMode::kWidget);

    histogram_tester_.ExpectTotalCount(
        "Blink.FedCm.IdpSigninStatus.PopupWindowResult", 0);
  }

  histogram_tester_.ExpectUniqueSample(
      "Blink.FedCm.IdpSigninStatus.PopupWindowResult",
      static_cast<int>(FedCmAccountSelectionView::PopupWindowResult::
                           kAccountsReceivedAndPopupNotClosedByIdp),
      1);
}

// Test that when user opens a pop-up window to complete the IDP sign-in flow,
// we record the appropriate metric when accounts are not received but
// IdentityProvider.close() is called.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       IdpSigninStatusAccountsNotReceivedAndPopupClosedByIdpMetric) {
  {
    // Trigger IdP sign-in status mismatch dialog.
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowMismatchDialog();
    AccountSelectionViewBase::Observer* observer =
        static_cast<AccountSelectionViewBase::Observer*>(controller.get());

    // Emulate user clicking on "Continue" button in the mismatch dialog.
    observer->OnLoginToIdP(GURL(kConfigUrl), GURL(kLoginUrl),
                           CreateMouseEvent());
    CreateAndShowPopupWindow(*controller);

    // Emulate IdentityProvider.close() being called in the pop-up window.
    controller->CloseModalDialog();

    histogram_tester_.ExpectTotalCount(
        "Blink.FedCm.IdpSigninStatus.PopupWindowResult", 0);
  }

  histogram_tester_.ExpectUniqueSample(
      "Blink.FedCm.IdpSigninStatus.PopupWindowResult",
      static_cast<int>(FedCmAccountSelectionView::PopupWindowResult::
                           kAccountsNotReceivedAndPopupClosedByIdp),
      1);
}

// Test that when user opens a pop-up window to complete the IDP sign-in flow,
// we record the appropriate metric when accounts are not received and
// IdentityProvider.close() is not called.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       IdpSigninStatusAccountsNotReceivedAndNoPopupClosedByIdpMetric) {
  {
    // Trigger IdP sign-in status mismatch dialog.
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowMismatchDialog();
    AccountSelectionViewBase::Observer* observer =
        static_cast<AccountSelectionViewBase::Observer*>(controller.get());

    // Emulate user clicking on "Continue" button in the mismatch dialog.
    observer->OnLoginToIdP(GURL(kConfigUrl), GURL(kLoginUrl),
                           CreateMouseEvent());
    CreateAndShowPopupWindow(*controller);

    histogram_tester_.ExpectTotalCount(
        "Blink.FedCm.IdpSigninStatus.PopupWindowResult", 0);
  }

  histogram_tester_.ExpectUniqueSample(
      "Blink.FedCm.IdpSigninStatus.PopupWindowResult",
      static_cast<int>(FedCmAccountSelectionView::PopupWindowResult::
                           kAccountsNotReceivedAndPopupNotClosedByIdp),
      1);
}

// Test closing the IdP sign-in pop-up window through IdentityProvider.close()
// should not close the widget.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       IdpSigninStatusPopupClosedViaIdentityProviderClose) {
  // Trigger IdP sign-in status mismatch dialog.
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowMismatchDialog();

  // Emulate user clicking on "Continue" button in the mismatch dialog.
  CreateAndShowPopupWindow(*controller);

  // Emulate IdentityProvider.close() being called in the pop-up window.
  controller->CloseModalDialog();

  // Widget should not be closed.
  EXPECT_FALSE(dialog_widget_->IsClosed());
}

// Test closing the IdP sign-in pop-up window through means other than
// IdentityProvider.close() should also close the widget.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       IdpSigninStatusPopupClosedViaPopupDestroyed) {
  // Trigger IdP sign-in status mismatch dialog.
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowMismatchDialog();

  // Emulate user clicking on "Continue" button in the mismatch dialog.
  CreateAndShowPopupWindow(*controller);

  // Emulate user closing the pop-up window.
  controller->OnPopupWindowDestroyed();

  // Widget should be closed.
  EXPECT_TRUE(dialog_widget_->IsClosed());
}

// Test that the mismatch dialog can be shown again after the pop-up window is
// closed.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       IdpSigninStatusMismatchDialogReshown) {
  // Trigger IdP sign-in status mismatch dialog.
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowMismatchDialog();

  // Emulate user clicking on "Continue" button in the mismatch dialog.
  CreateAndShowPopupWindow(*controller);

  // Mismatch dialog should be hidden because pop-up window is open.
  EXPECT_FALSE(dialog_widget_->IsVisible());

  // Emulate IdentityProvider.close() being called in the pop-up window.
  controller->CloseModalDialog();

  // Mismatch dialog should remain hidden because it has not been updated to an
  // accounts dialog yet.
  EXPECT_FALSE(dialog_widget_->IsVisible());

  // Emulate another mismatch so we need to show the mismatch dialog again.
  controller->ShowFailureDialog(
      kTopFrameEtldPlusOne, kIframeEtldPlusOne, kIdpEtldPlusOne,
      blink::mojom::RpContext::kSignIn, blink::mojom::RpMode::kWidget,
      content::IdentityProviderMetadata());

  // Mismatch dialog is visible again.
  EXPECT_TRUE(dialog_widget_->IsVisible());
}

// Tests that RP context is properly set for the mismatch UI.
TEST_F(FedCmAccountSelectionViewDesktopTest, MismatchDialogWithRpContext) {
  {
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowMismatchDialog();
    EXPECT_EQ(controller->GetRpContext(), blink::mojom::RpContext::kSignIn);
  }
  {
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowMismatchDialog(blink::mojom::RpContext::kSignUp);
    EXPECT_EQ(controller->GetRpContext(), blink::mojom::RpContext::kSignUp);
  }
  {
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowMismatchDialog(blink::mojom::RpContext::kContinue);
    EXPECT_EQ(controller->GetRpContext(), blink::mojom::RpContext::kContinue);
  }
  {
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowMismatchDialog(blink::mojom::RpContext::kUse);
    EXPECT_EQ(controller->GetRpContext(), blink::mojom::RpContext::kUse);
  }
}

// Tests the following
// 1. pop-up window is closed
// 2. visibility changes to hidden e.g. user navigates to different tab
// 3. Show() is invoked
// 4. widget should remain hidden
// 5. visibility changes to visible e.g. user navigates back to same tab
// 6. widget should now be visible
TEST_F(FedCmAccountSelectionViewDesktopTest,
       BubbleWidgetAfterPopupRemainsHiddenAfterAccountsFetched) {
  // Trigger IdP sign-in status mismatch dialog.
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowMismatchDialog();
  // Emulate user clicking on "Continue" button in the mismatch dialog.
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());
  observer->OnLoginToIdP(GURL(kConfigUrl), GURL(kLoginUrl), CreateMouseEvent());
  CreateAndShowPopupWindow(*controller);

  // Emulate IdP closing the pop-up window.
  controller->CloseModalDialog();

  controller->OnVisibilityChanged(content::Visibility::HIDDEN);

  // Emulate IdP sending the IdP sign-in status header which updates the
  // mismatch dialog to an accounts dialog.
  const char kAccountId[] = "account_id";
  IdentityProviderDisplayData idp_data = CreateIdentityProviderDisplayData({
      {kAccountId, LoginState::kSignUp},
  });
  Show(*controller, idp_data.accounts, SignInMode::kExplicit,
       blink::mojom::RpMode::kWidget);
  EXPECT_FALSE(dialog_widget_->IsVisible());

  controller->OnVisibilityChanged(content::Visibility::VISIBLE);
  EXPECT_TRUE(dialog_widget_->IsVisible());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kConfirmAccount,
            account_selection_view_->sheet_type_);
}

// Tests the following
// 1. pop-up window is closed
// 2. visibility changes to hidden e.g. user navigates to different tab
// 3. visibility changes to visible e.g. user navigates back to same tab
// 4. widget should remain hidden
// 5. Show() is invoked
// 6. widget should now be visible
TEST_F(FedCmAccountSelectionViewDesktopTest,
       BubbleWidgetAfterPopupRemainsHiddenBeforeAccountsFetched) {
  // Trigger IdP sign-in status mismatch dialog.
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowMismatchDialog();
  // Emulate user clicking on "Continue" button in the mismatch dialog.
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());
  observer->OnLoginToIdP(GURL(kConfigUrl), GURL(kLoginUrl), CreateMouseEvent());
  CreateAndShowPopupWindow(*controller);

  // Emulate IdP closing the pop-up window.
  controller->CloseModalDialog();

  // Switch to a different tab and then switch back to the same tab. The widget
  // should remain hidden because the mismatch dialog has not been updated into
  // an accounts dialog yet.
  controller->OnVisibilityChanged(content::Visibility::HIDDEN);
  EXPECT_FALSE(dialog_widget_->IsVisible());
  controller->OnVisibilityChanged(content::Visibility::VISIBLE);
  EXPECT_FALSE(dialog_widget_->IsVisible());

  // Emulate IdP sending the IdP sign-in status header which updates the
  // mismatch dialog to an accounts dialog.
  const char kAccountId[] = "account_id";
  IdentityProviderDisplayData idp_data = CreateIdentityProviderDisplayData({
      {kAccountId, LoginState::kSignUp},
  });
  Show(*controller, idp_data.accounts, SignInMode::kExplicit,
       blink::mojom::RpMode::kWidget);

  // The widget should now be visible.
  EXPECT_TRUE(dialog_widget_->IsVisible());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kConfirmAccount,
            account_selection_view_->sheet_type_);
}

// Test transitioning from IdP sign-in status mismatch failure dialog to regular
// sign-in dialog where two accounts are logged in at once.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       IdpSigninStatusMismatchMultiAccount) {
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowMismatchDialog();
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());

  const char kAccountId1[] = "account_id1";
  const char kAccountId2[] = "account_id2";
  IdentityProviderDisplayData idp_data = CreateIdentityProviderDisplayData(
      {{kAccountId1, LoginState::kSignUp}, {kAccountId2, LoginState::kSignUp}});
  std::vector<content::IdentityRequestAccount> new_accounts = {
      {kAccountId1, "", "", "", GURL(),
       /*login_hints=*/std::vector<std::string>(),
       /*domain_hints=*/std::vector<std::string>(), LoginState::kSignUp}};
  content::IdentityProviderData new_idp_data = {
      kIdpEtldPlusOne,
      new_accounts,
      content::IdentityProviderMetadata(),
      content::ClientMetadata(GURL(), GURL()),
      blink::mojom::RpContext::kSignIn,
      /*request_permission=*/true,
      /*has_login_status_mismatch=*/false};

  Show(*controller, idp_data.accounts, SignInMode::kExplicit,
       blink::mojom::RpMode::kWidget, new_idp_data);

  EXPECT_EQ(TestAccountSelectionView::SheetType::kConfirmAccount,
            account_selection_view_->sheet_type_);
  observer->OnAccountSelected(new_idp_data.accounts[0], idp_data,
                              CreateMouseEvent());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kVerifying,
            account_selection_view_->sheet_type_);
  // Should have only shown one account.
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId1));

  // Failure bubble should have been re-used for sign-in dialog.
  EXPECT_EQ(1u, controller->num_dialogs_);
}

// Test the use another account flow, resulting in the new account being shown
// after logging in.
TEST_F(FedCmAccountSelectionViewDesktopTest, UseAnotherAccount) {
  const char kAccountId[] = "account_id";
  IdentityProviderDisplayData idp_data =
      CreateIdentityProviderDisplayData({{kAccountId, LoginState::kSignUp}});
  const std::vector<Account>& accounts = idp_data.accounts;
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShow(accounts, SignInMode::kExplicit);
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());

  EXPECT_FALSE(account_selection_view_->show_back_button_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId));

  // Emulate the user clicking "use another account button".
  observer->OnLoginToIdP(GURL(kConfigUrl), GURL(kLoginUrl), CreateMouseEvent());
  CreateAndShowPopupWindow(*controller);

  // Emulate user completing the sign-in flow and IdP prompts closing the
  // pop-up window and sending new accounts.
  controller->CloseModalDialog();

  const char kAccountId2[] = "account_id2";
  IdentityProviderDisplayData idp_data2 = CreateIdentityProviderDisplayData(
      {{kAccountId, LoginState::kSignUp}, {kAccountId2, LoginState::kSignUp}});
  // The new account would be kAccountId2.
  std::vector<content::IdentityRequestAccount> new_accounts = {
      {kAccountId2, "", "", "", GURL(),
       /*login_hints=*/std::vector<std::string>(),
       /*domain_hints=*/std::vector<std::string>(), LoginState::kSignUp}};
  content::IdentityProviderData new_idp_data = {
      kIdpEtldPlusOne,
      new_accounts,
      content::IdentityProviderMetadata(),
      content::ClientMetadata(GURL(), GURL()),
      blink::mojom::RpContext::kSignIn,
      /*request_permission=*/true,
      /*has_login_status_mismatch=*/false};

  Show(*controller, idp_data2.accounts, SignInMode::kExplicit,
       blink::mojom::RpMode::kWidget, new_idp_data);

  // Only the newly logged in account is shown in the latest dialog.
  EXPECT_EQ(TestAccountSelectionView::SheetType::kConfirmAccount,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId2));

  // Back button must be showing since there are multiple accounts.
  EXPECT_TRUE(account_selection_view_->show_back_button_);

  // Clicking the back button shows all the accounts.
  observer->OnBackButtonClicked();
  EXPECT_FALSE(account_selection_view_->show_back_button_);
  EXPECT_EQ(TestAccountSelectionView::SheetType::kAccountPicker,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId, kAccountId2));
}

// Test the use another account flow in a modal, resulting in the new account
// being shown after logging in.
TEST_F(FedCmAccountSelectionViewDesktopTest, UseAnotherAccountModal) {
  const char kAccountId[] = "account_id";
  IdentityProviderDisplayData idp_data =
      CreateIdentityProviderDisplayData({{kAccountId, LoginState::kSignUp}});
  const std::vector<Account>& accounts = idp_data.accounts;
  std::unique_ptr<TestFedCmAccountSelectionView> controller = CreateAndShow(
      accounts, SignInMode::kExplicit, blink::mojom::RpMode::kButton);
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());

  EXPECT_FALSE(account_selection_view_->show_back_button_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId));

  // Emulate the user clicking "use another account button".
  observer->OnLoginToIdP(GURL(kConfigUrl), GURL(kLoginUrl), CreateMouseEvent());
  CreateAndShowPopupWindow(*controller);

  // Emulate user completing the sign-in flow and IdP prompts closing the
  // pop-up window and sending new accounts.
  controller->CloseModalDialog();

  const char kAccountId2[] = "account_id2";
  IdentityProviderDisplayData idp_data2 = CreateIdentityProviderDisplayData(
      {{kAccountId, LoginState::kSignUp}, {kAccountId2, LoginState::kSignUp}});
  // The new account would be kAccountId2.
  std::vector<content::IdentityRequestAccount> new_accounts = {
      {kAccountId2, "", "", "", GURL(),
       /*login_hints=*/std::vector<std::string>(),
       /*domain_hints=*/std::vector<std::string>(), LoginState::kSignUp}};
  content::IdentityProviderData new_idp_data = {
      kIdpEtldPlusOne,
      new_accounts,
      content::IdentityProviderMetadata(),
      content::ClientMetadata(GURL(), GURL()),
      blink::mojom::RpContext::kSignIn,
      /*request_permission=*/true,
      /*has_login_status_mismatch=*/false};

  Show(*controller, idp_data2.accounts, SignInMode::kExplicit,
       blink::mojom::RpMode::kButton, new_idp_data);

  // Only the newly logged in account is shown in the latest dialog.
  EXPECT_EQ(TestAccountSelectionView::SheetType::kRequestPermission,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId2));
}

// Tests that the error dialog can be shown.
TEST_F(FedCmAccountSelectionViewDesktopTest, ErrorDialogShown) {
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowErrorDialog();
  EXPECT_TRUE(dialog_widget_->IsVisible());
}

// Tests that RP context is properly set for the error dialog.
TEST_F(FedCmAccountSelectionViewDesktopTest, ErrorDialogWithRpContext) {
  {
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowErrorDialog();
    EXPECT_EQ(controller->GetRpContext(), blink::mojom::RpContext::kSignIn);
  }
  {
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowErrorDialog(blink::mojom::RpContext::kSignUp);
    EXPECT_EQ(controller->GetRpContext(), blink::mojom::RpContext::kSignUp);
  }
  {
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowErrorDialog(blink::mojom::RpContext::kContinue);
    EXPECT_EQ(controller->GetRpContext(), blink::mojom::RpContext::kContinue);
  }
  {
    std::unique_ptr<TestFedCmAccountSelectionView> controller =
        CreateAndShowErrorDialog(blink::mojom::RpContext::kUse);
    EXPECT_EQ(controller->GetRpContext(), blink::mojom::RpContext::kUse);
  }
}

// Tests the flow for when the "got it" button on the error dialog is clicked.
TEST_F(FedCmAccountSelectionViewDesktopTest, ErrorDialogGotItClicked) {
  // Trigger error dialog.
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowErrorDialog();
  EXPECT_TRUE(dialog_widget_->IsVisible());

  // Emulate user clicking on "got it" button in the error dialog.
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());
  observer->OnGotIt(CreateMouseEvent());

  // Widget should be dismissed.
  StubAccountSelectionViewDelegate* delegate =
      static_cast<StubAccountSelectionViewDelegate*>(delegate_.get());
  EXPECT_EQ(delegate->GetDismissReason(), DismissReason::kGotItButton);
}

// Tests the flow for when the "more details" button on the error dialog is
// clicked.
TEST_F(FedCmAccountSelectionViewDesktopTest, ErrorDialogMoreDetailsClicked) {
  // Trigger error dialog.
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowErrorDialog();
  EXPECT_TRUE(dialog_widget_->IsVisible());

  // Emulate user clicking on "more details" button in the error dialog.
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());
  observer->OnMoreDetails(CreateMouseEvent());
  CreateAndShowPopupWindow(*controller);

  // Widget should be dismissed.
  StubAccountSelectionViewDelegate* delegate =
      static_cast<StubAccountSelectionViewDelegate*>(delegate_.get());
  EXPECT_EQ(delegate->GetDismissReason(), DismissReason::kMoreDetailsButton);
}

TEST_F(FedCmAccountSelectionViewDesktopTest, MultiIdpWithOneIdpMismatch) {
  const char kAccountId[] = "account_id";
  std::vector<IdentityProviderDisplayData> idp_list = {
      CreateIdentityProviderDisplayData({{kAccountId, LoginState::kSignUp}}),
      CreateIdentityProviderDisplayData(/*account_infos=*/{},
                                        /*has_login_status_mismatch*/ true)};
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowMultiIdp(idp_list, SignInMode::kExplicit,
                            blink::mojom::RpMode::kWidget);

  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());

  EXPECT_FALSE(account_selection_view_->show_back_button_);
  EXPECT_EQ(TestAccountSelectionView::SheetType::kAccountPicker,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId));

  observer->OnAccountSelected(idp_list[0].accounts[0], idp_list[0],
                              CreateMouseEvent());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kConfirmAccount,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId));
}

// Tests that if a pop-up window is opened in button flow mode, closing the
// pop-up window triggers the dismiss callback.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       ButtonFlowPopupCloseTriggersDismissCallback) {
  // Initialize a controller but do not trigger any dialogs.
  auto controller = std::make_unique<TestFedCmAccountSelectionView>(
      delegate_.get(), account_selection_view_.get());
  EXPECT_FALSE(dialog_widget_->IsVisible());

  // Emulate user clicking on a button to sign in with an IDP via button flow.
  auto popup_window = std::make_unique<MockFedCmModalDialogView>(
      test_web_contents_.get(), controller.get());
  EXPECT_CALL(*popup_window, ShowPopupWindow).Times(1);
  controller->SetIdpSigninPopupWindowForTesting(std::move(popup_window));
  controller->ShowModalDialog(GURL(u"https://example.com"));

  // Emulate user closing the pop-up window.
  controller->OnPopupWindowDestroyed();

  // Widget should be dismissed.
  StubAccountSelectionViewDelegate* delegate =
      static_cast<StubAccountSelectionViewDelegate*>(delegate_.get());
  EXPECT_EQ(delegate->GetDismissReason(), DismissReason::kOther);

  // Reset the widget explicitly since no widget was shown. Otherwise, the test
  // will complain that a widget is still open.
  dialog_widget_.reset();
}

// Tests that if a single account chooser is opened in button flow mode,
// selecting an account shows the request permission sheet. Then, confirming the
// account on the request permission sheet shows the verifying sheet.
TEST_F(FedCmAccountSelectionViewDesktopTest, SingleAccountFlowModal) {
  const char kAccountId[] = "account_id";
  IdentityProviderDisplayData idp_data =
      CreateIdentityProviderDisplayData({{kAccountId, LoginState::kSignUp}});
  const std::vector<Account>& accounts = idp_data.accounts;
  std::unique_ptr<TestFedCmAccountSelectionView> controller = CreateAndShow(
      accounts, SignInMode::kExplicit, blink::mojom::RpMode::kButton);
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());

  EXPECT_FALSE(account_selection_view_->show_back_button_);
  EXPECT_EQ(TestAccountSelectionView::SheetType::kConfirmAccount,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId));

  observer->OnAccountSelected(accounts[0], idp_data, CreateMouseEvent());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kRequestPermission,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId));

  observer->OnAccountSelected(accounts[0], idp_data, CreateMouseEvent());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kVerifying,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId));
}

// Tests that if a multiple account chooser is opened in button flow mode,
// selecting an account shows the request permission sheet. Then, confirming the
// account on the request permission sheet shows the verifying sheet.
TEST_F(FedCmAccountSelectionViewDesktopTest, MultipleAccountFlowModal) {
  const char kAccountId1[] = "account_id1";
  const char kAccountId2[] = "account_id2";
  IdentityProviderDisplayData idp_data = CreateIdentityProviderDisplayData(
      {{kAccountId1, LoginState::kSignUp}, {kAccountId2, LoginState::kSignUp}});
  const std::vector<Account>& accounts = idp_data.accounts;
  std::unique_ptr<TestFedCmAccountSelectionView> controller = CreateAndShow(
      accounts, SignInMode::kExplicit, blink::mojom::RpMode::kButton);
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());

  EXPECT_FALSE(account_selection_view_->show_back_button_);
  EXPECT_EQ(TestAccountSelectionView::SheetType::kAccountPicker,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId1, kAccountId2));

  observer->OnAccountSelected(accounts[0], idp_data, CreateMouseEvent());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kRequestPermission,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId1));

  observer->OnAccountSelected(accounts[0], idp_data, CreateMouseEvent());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kVerifying,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId1));
}

// Tests that if a single account chooser is opened in button flow mode,
// selecting a returning account shows the verifying sheet.
TEST_F(FedCmAccountSelectionViewDesktopTest, SingleAccountFlowReturningModal) {
  const char kAccountId[] = "account_id";
  IdentityProviderDisplayData idp_data =
      CreateIdentityProviderDisplayData({{kAccountId, LoginState::kSignIn}});
  const std::vector<Account>& accounts = idp_data.accounts;
  std::unique_ptr<TestFedCmAccountSelectionView> controller = CreateAndShow(
      accounts, SignInMode::kExplicit, blink::mojom::RpMode::kButton);
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());

  EXPECT_FALSE(account_selection_view_->show_back_button_);
  EXPECT_EQ(TestAccountSelectionView::SheetType::kConfirmAccount,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId));

  observer->OnAccountSelected(accounts[0], idp_data, CreateMouseEvent());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kVerifying,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId));
}

// Tests that if a multiple account chooser is opened in button flow mode,
// selecting a returning account shows the verifying sheet.
TEST_F(FedCmAccountSelectionViewDesktopTest,
       MultipleAccountFlowReturningModal) {
  const char kAccountId1[] = "account_id1";
  const char kAccountId2[] = "account_id2";
  IdentityProviderDisplayData idp_data = CreateIdentityProviderDisplayData(
      {{kAccountId1, LoginState::kSignIn}, {kAccountId2, LoginState::kSignUp}});
  const std::vector<Account>& accounts = idp_data.accounts;
  std::unique_ptr<TestFedCmAccountSelectionView> controller = CreateAndShow(
      accounts, SignInMode::kExplicit, blink::mojom::RpMode::kButton);
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());

  EXPECT_FALSE(account_selection_view_->show_back_button_);
  EXPECT_EQ(TestAccountSelectionView::SheetType::kAccountPicker,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId1, kAccountId2));

  observer->OnAccountSelected(accounts[0], idp_data, CreateMouseEvent());
  EXPECT_EQ(TestAccountSelectionView::SheetType::kVerifying,
            account_selection_view_->sheet_type_);
  EXPECT_THAT(account_selection_view_->account_ids_,
              testing::ElementsAre(kAccountId1));
}

// Tests that the user can dismiss the loading modal.
TEST_F(FedCmAccountSelectionViewDesktopTest, DismissLoadingModal) {
  std::unique_ptr<TestFedCmAccountSelectionView> controller =
      CreateAndShowLoadingDialog();
  AccountSelectionViewBase::Observer* observer =
      static_cast<AccountSelectionViewBase::Observer*>(controller.get());
  observer->OnCloseButtonClicked(CreateMouseEvent());
}
