/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "base/strings/stringprintf.h"
#include "chrome/app/wootz_command_ids.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_context_utils.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_service_factory.h"
#include "chrome/browser/ui/wootz_browser.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/sidebar/sidebar_controller.h"
#include "chrome/browser/ui/sidebar/sidebar_model.h"
#include "components/wootz_wallet/browser/pref_names.h"
#include "components/wootz_wallet/common/common_utils.h"
#include "components/wootz_wallet/common/pref_names.h"
#include "components/sidebar/browser/sidebar_item.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_window.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_command_controller.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/policy/core/browser/browser_policy_connector.h"
#include "components/policy/core/common/mock_configuration_policy_provider.h"
#include "components/policy/core/common/policy_map.h"
#include "components/policy/policy_constants.h"
#include "components/prefs/pref_service.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/navigation_handle_observer.h"
#include "extensions/common/constants.h"
#include "url/gurl.h"

namespace policy {

class WootzWalletPolicyTest : public InProcessBrowserTest,
                              public ::testing::WithParamInterface<bool> {
 public:
  WootzWalletPolicyTest() = default;
  ~WootzWalletPolicyTest() override = default;

  void SetUpInProcessBrowserTestFixture() override {
    EXPECT_CALL(provider_, IsInitializationComplete(testing::_))
        .WillRepeatedly(testing::Return(true));
    BrowserPolicyConnector::SetPolicyProviderForTesting(&provider_);
    PolicyMap policies;
    policies.Set(key::kWootzWalletDisabled, POLICY_LEVEL_MANDATORY,
                 POLICY_SCOPE_USER, POLICY_SOURCE_PLATFORM,
                 base::Value(IsWootzWalletDisabledTest()), nullptr);
    provider_.UpdateChromePolicy(policies);
  }

  bool IsWootzWalletDisabledTest() { return GetParam(); }

  content::WebContents* web_contents() const {
    return browser()->tab_strip_model()->GetActiveWebContents();
  }

  content::BrowserContext* browser_context() {
    return web_contents()->GetBrowserContext();
  }

  Profile* profile() { return browser()->profile(); }

  PrefService* prefs() { return user_prefs::UserPrefs::Get(browser_context()); }

 private:
  MockConfigurationPolicyProvider provider_;
};

// Verify that wootz_wallet::IsDisabledByPolicy works correctly based on the
// preference set by the policy.
IN_PROC_BROWSER_TEST_P(WootzWalletPolicyTest, IsWootzWalletDisabled) {
  EXPECT_TRUE(prefs()->FindPreference(wootz_wallet::prefs::kDisabledByPolicy));
  if (IsWootzWalletDisabledTest()) {
    EXPECT_TRUE(prefs()->GetBoolean(wootz_wallet::prefs::kDisabledByPolicy));
    EXPECT_FALSE(wootz_wallet::IsAllowed(prefs()));
    EXPECT_FALSE(wootz_wallet::IsAllowedForContext(profile()));
  } else {
    EXPECT_FALSE(prefs()->GetBoolean(wootz_wallet::prefs::kDisabledByPolicy));
    EXPECT_TRUE(wootz_wallet::IsAllowed(prefs()));
    EXPECT_TRUE(wootz_wallet::IsAllowedForContext(profile()));

    auto* profile = browser()->profile();
    auto* incognito_profile = CreateIncognitoBrowser(profile)->profile();
    ui_test_utils::BrowserChangeObserver browser_creation_observer(
        nullptr, ui_test_utils::BrowserChangeObserver::ChangeType::kAdded);
    profiles::SwitchToGuestProfile();
    Browser* guest_browser = browser_creation_observer.Wait();
    DCHECK(guest_browser);
    auto* guest_profile = guest_browser->profile();
    ASSERT_TRUE(guest_profile->IsGuestSession());

    ui_test_utils::BrowserChangeObserver tor_browser_creation_observer(
        nullptr, ui_test_utils::BrowserChangeObserver::ChangeType::kAdded);
    wootz::NewOffTheRecordWindowTor(browser());
    Browser* tor_browser = tor_browser_creation_observer.Wait();
    DCHECK(tor_browser);
    auto* tor_profile = tor_browser->profile();
    ASSERT_TRUE(tor_profile->IsTor());

    // By default the wallet should not be allowed for private, guest, or tor.
    EXPECT_FALSE(wootz_wallet::IsAllowedForContext(incognito_profile));
    EXPECT_FALSE(wootz_wallet::IsAllowedForContext(tor_profile));
    EXPECT_FALSE(wootz_wallet::IsAllowedForContext(guest_profile));

    // Setting pref should allow it for incognito, but not for guest, or tor.
    prefs()->SetBoolean(kWootzWalletPrivateWindowsEnabled, true);
    EXPECT_TRUE(wootz_wallet::IsAllowedForContext(incognito_profile));
    EXPECT_FALSE(wootz_wallet::IsAllowedForContext(tor_profile));
    EXPECT_FALSE(wootz_wallet::IsAllowedForContext(guest_profile));
  }
}

// Verify that Wallet service doesn't get created when Wootz Wallet is
// disabled by policy.
IN_PROC_BROWSER_TEST_P(WootzWalletPolicyTest, GetWalletService) {
  if (IsWootzWalletDisabledTest()) {
    EXPECT_EQ(wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(
                  profile()),
              nullptr);
  } else {
    EXPECT_NE(wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(
                  profile()),
              nullptr);
  }
}

// Verify that Wallet menu item isn't enabled in the app menu when Wootz
// Wallet is disabled by policy.
IN_PROC_BROWSER_TEST_P(WootzWalletPolicyTest, AppMenuItemDisabled) {
  auto* command_controller = browser()->command_controller();
  if (IsWootzWalletDisabledTest()) {
    EXPECT_FALSE(command_controller->IsCommandEnabled(IDC_SHOW_WOOTZ_WALLET));
  } else {
    EXPECT_TRUE(command_controller->IsCommandEnabled(IDC_SHOW_WOOTZ_WALLET));
  }
}

// Verify that wootz://wallet page isn't reachable when Wootz Wallet is
// disabled by policy.
IN_PROC_BROWSER_TEST_P(WootzWalletPolicyTest, WalletPageAccess) {
  const GURL url("wootzapp://wallet");
  auto* rfh = ui_test_utils::NavigateToURL(browser(), url);
  EXPECT_TRUE(rfh);
  EXPECT_EQ(IsWootzWalletDisabledTest(), rfh->IsErrorDocument());
}

// Verify that the wallet item is not shown in the sidebar when Wootz Wallet is
// disabled by policy.
IN_PROC_BROWSER_TEST_P(WootzWalletPolicyTest, WalletInSidebar) {
  WootzBrowser* wootz_browser = static_cast<WootzBrowser*>(browser());
  sidebar::SidebarController* controller = wootz_browser->sidebar_controller();
  sidebar::SidebarModel* model = controller->model();

  const auto items = model->GetAllSidebarItems();
  EXPECT_LT(0UL, items.size());

  const auto iter = base::ranges::find_if(items, [](const auto& i) {
    return (i.built_in_item_type ==
            sidebar::SidebarItem::BuiltInItemType::kWallet);
  });

  if (IsWootzWalletDisabledTest()) {
    EXPECT_TRUE(iter == items.end());
  } else {
    EXPECT_FALSE(iter == items.end());
  }
}

INSTANTIATE_TEST_SUITE_P(
    WootzWalletPolicyTest,
    WootzWalletPolicyTest,
    ::testing::Bool(),
    [](const testing::TestParamInfo<WootzWalletPolicyTest::ParamType>& info) {
      return base::StringPrintf("WootzWallet_%sByPolicy",
                                info.param ? "Disabled" : "NotDisabled");
    });

}  // namespace policy
