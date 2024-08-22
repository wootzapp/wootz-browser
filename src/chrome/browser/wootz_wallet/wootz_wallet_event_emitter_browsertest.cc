/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <memory>
#include <optional>

#include "base/path_service.h"
#include "base/test/bind.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/thread_test_helper.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_service_factory.h"
#include "components/wootz_wallet/browser/wootz_wallet_constants.h"
#include "components/wootz_wallet/browser/wootz_wallet_service.h"
#include "components/wootz_wallet/browser/wootz_wallet_utils.h"
#include "components/wootz_wallet/browser/json_rpc_service.h"
#include "components/wootz_wallet/browser/keyring_service.h"
#include "components/wootz_wallet/browser/permission_utils.h"
#include "components/wootz_wallet/browser/test_utils.h"
#include "components/wootz_wallet/common/common_utils.h"
#include "components/wootz_wallet/common/features.h"
#include "components/constants/wootz_paths.h"
#include "components/constants/pref_names.h"
#include "components/permissions/contexts/wootz_wallet_permission_context.h"
#include "chrome/browser/content_settings/host_content_settings_map_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/network_session_configurator/common/network_switches.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/content_mock_cert_verifier.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "net/dns/mock_host_resolver.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace {

const char kEmbeddedTestServerDirectory[] = "wootz-wallet";

std::string CheckForEventScript(const std::string& event_var) {
  return base::StringPrintf(R"(
      new Promise(resolve => {
        const timer = setInterval(function () {
          if (%s) {
            clearInterval(timer);
            resolve(true);
          }
        }, 100);
      });
    )",
                            event_var.c_str());
}

}  // namespace

namespace wootz_wallet {

class WootzWalletEventEmitterTest : public InProcessBrowserTest {
 public:
  WootzWalletEventEmitterTest() {
    feature_list_.InitAndEnableFeature(
        wootz_wallet::features::kNativeWootzWalletFeature);
  }

  void SetUpInProcessBrowserTestFixture() override {
    InProcessBrowserTest::SetUpInProcessBrowserTestFixture();
    mock_cert_verifier_.SetUpInProcessBrowserTestFixture();
  }

  void TearDownInProcessBrowserTestFixture() override {
    InProcessBrowserTest::TearDownInProcessBrowserTestFixture();
    mock_cert_verifier_.TearDownInProcessBrowserTestFixture();
  }

  void SetUpOnMainThread() override {
    wootz_wallet::SetDefaultEthereumWallet(
        browser()->profile()->GetPrefs(),
        wootz_wallet::mojom::DefaultWallet::WootzWallet);
    InProcessBrowserTest::SetUpOnMainThread();
    mock_cert_verifier_.mock_cert_verifier()->set_default_result(net::OK);
    host_resolver()->AddRule("*", "127.0.0.1");

    https_server_ = std::make_unique<net::EmbeddedTestServer>(
        net::test_server::EmbeddedTestServer::TYPE_HTTPS);
    https_server_->SetSSLConfig(net::EmbeddedTestServer::CERT_OK);

    base::FilePath test_data_dir;
    base::PathService::Get(wootz::DIR_TEST_DATA, &test_data_dir);
    test_data_dir = test_data_dir.AppendASCII(kEmbeddedTestServerDirectory);
    https_server_->ServeFilesFromDirectory(test_data_dir);

    wootz_wallet_service_ =
        wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(
            browser()->profile());
    keyring_service_ = wootz_wallet_service_->keyring_service();

    ASSERT_TRUE(https_server_->Start());
  }

  void SetUpCommandLine(base::CommandLine* command_line) override {
    InProcessBrowserTest::SetUpCommandLine(command_line);
    mock_cert_verifier_.SetUpCommandLine(command_line);
  }

  net::EmbeddedTestServer* https_server() { return https_server_.get(); }

  mojo::Remote<wootz_wallet::mojom::JsonRpcService> GetJsonRpcService() {
    if (!json_rpc_service_) {
      mojo::PendingRemote<wootz_wallet::mojom::JsonRpcService> remote;
      wootz_wallet_service_->json_rpc_service()->Bind(
          remote.InitWithNewPipeAndPassReceiver());
      json_rpc_service_.Bind(std::move(remote));
    }
    return std::move(json_rpc_service_);
  }

  HostContentSettingsMap* host_content_settings_map() {
    return HostContentSettingsMapFactory::GetForProfile(browser()->profile());
  }

  content::WebContents* web_contents() {
    return browser()->tab_strip_model()->GetActiveWebContents();
  }

  url::Origin GetLastCommitedOrigin() {
    return url::Origin::Create(web_contents()->GetLastCommittedURL());
  }

  AccountUtils GetAccountUtils() { return AccountUtils(keyring_service_); }

  void RestoreWallet() {
    ASSERT_TRUE(keyring_service_->RestoreWalletSync(
        kMnemonicDripCaution, kTestWalletPassword, false));
  }

  void SetSelectedAccount(const mojom::AccountIdPtr& account_id) {
    ASSERT_TRUE(keyring_service_->SetSelectedAccountSync(account_id.Clone()));
  }

 private:
  content::ContentMockCertVerifier mock_cert_verifier_;
  mojo::Remote<wootz_wallet::mojom::JsonRpcService> json_rpc_service_;
  raw_ptr<WootzWalletService> wootz_wallet_service_ = nullptr;
  raw_ptr<KeyringService> keyring_service_ = nullptr;
  std::unique_ptr<net::EmbeddedTestServer> https_server_;
  base::test::ScopedFeatureList feature_list_;
};

IN_PROC_BROWSER_TEST_F(WootzWalletEventEmitterTest, CheckForAConnectEvent) {
  GURL url =
      https_server()->GetURL("a.com", "/wootz_wallet_event_emitter.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));
  content::WebContents* contents =
      browser()->tab_strip_model()->GetActiveWebContents();

  auto result_first =
      EvalJs(contents, CheckForEventScript("received_connect_event"));
  EXPECT_EQ(base::Value(true), result_first.value);
}

IN_PROC_BROWSER_TEST_F(WootzWalletEventEmitterTest,
                       CheckForAChainChangedEvent) {
  GURL url =
      https_server()->GetURL("a.com", "/wootz_wallet_event_emitter.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));
  content::WebContents* contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  auto service = GetJsonRpcService();
  service->SetNetwork(wootz_wallet::mojom::kSepoliaChainId,
                      wootz_wallet::mojom::CoinType::ETH, std::nullopt,
                      base::DoNothing());

  auto result_first =
      EvalJs(contents, CheckForEventScript("received_chain_changed_event"));
  EXPECT_EQ(base::Value(true), result_first.value);
}

IN_PROC_BROWSER_TEST_F(WootzWalletEventEmitterTest,
                       CheckForAnAccountChangedEvent) {
  RestoreWallet();
  auto eth_account = GetAccountUtils().EnsureEthAccount(0);
  GURL url =
      https_server()->GetURL("a.com", "/wootz_wallet_event_emitter.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));
  content::WebContents* contents =
      browser()->tab_strip_model()->GetActiveWebContents();

  url::Origin sub_request_origin;
  ASSERT_TRUE(wootz_wallet::GetSubRequestOrigin(
      permissions::RequestType::kWootzEthereum, GetLastCommitedOrigin(),
      eth_account->address, &sub_request_origin));
  host_content_settings_map()->SetContentSettingDefaultScope(
      sub_request_origin.GetURL(), GetLastCommitedOrigin().GetURL(),
      ContentSettingsType::WOOTZ_ETHEREUM,
      ContentSetting::CONTENT_SETTING_ALLOW);
  SetSelectedAccount(eth_account->account_id);

  auto result_first =
      EvalJs(contents, CheckForEventScript("received_account_changed_event"));
  EXPECT_EQ(base::Value(true), result_first.value);
}

}  // namespace wootz_wallet
