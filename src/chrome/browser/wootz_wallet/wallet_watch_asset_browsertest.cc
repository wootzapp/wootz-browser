/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <optional>

#include "base/command_line.h"
#include "base/memory/raw_ptr.h"
#include "base/path_service.h"
#include "base/test/bind.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_service_factory.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_tab_helper.h"
#include "components/wootz_wallet/browser/wootz_wallet_service.h"
#include "components/wootz_wallet/browser/wootz_wallet_utils.h"
#include "components/wootz_wallet/browser/keyring_service.h"
#include "components/wootz_wallet/browser/test_utils.h"
#include "components/constants/wootz_paths.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/grit/wootz_components_strings.h"
#include "components/network_session_configurator/common/network_switches.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/content_mock_cert_verifier.h"
#include "content/public/test/test_utils.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "ui/base/l10n/l10n_util.h"
#include "url/gurl.h"

namespace wootz_wallet {

class WalletWatchAssetBrowserTest : public InProcessBrowserTest {
 public:
  WalletWatchAssetBrowserTest()
      : https_server_(net::EmbeddedTestServer::TYPE_HTTPS) {}

  ~WalletWatchAssetBrowserTest() override = default;

  void SetUpCommandLine(base::CommandLine* command_line) override {
    InProcessBrowserTest::SetUpCommandLine(command_line);
    mock_cert_verifier_.SetUpCommandLine(command_line);
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
    mock_cert_verifier_.mock_cert_verifier()->set_default_result(net::OK);
    host_resolver()->AddRule("*", "127.0.0.1");

    base::FilePath test_data_dir;
    base::PathService::Get(wootz::DIR_TEST_DATA, &test_data_dir);
    test_data_dir = test_data_dir.AppendASCII("wootz-wallet");
    https_server_.ServeFilesFromDirectory(test_data_dir);
    https_server_.SetSSLConfig(net::EmbeddedTestServer::CERT_TEST_NAMES);
    ASSERT_TRUE(https_server()->Start());

    wootz_wallet_service_ =
        wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(
            browser()->profile());
  }

  content::WebContents* web_contents() {
    return browser()->tab_strip_model()->GetActiveWebContents();
  }

  net::EmbeddedTestServer* https_server() { return &https_server_; }

  void RestoreWallet() {
    ASSERT_TRUE(wootz_wallet_service_->keyring_service()->RestoreWalletSync(
        kMnemonicDripCaution, kTestWalletPassword, false));
  }

  std::vector<mojom::BlockchainTokenPtr> GetUserAssets() const {
    base::RunLoop run_loop;
    std::vector<mojom::BlockchainTokenPtr> tokens_out;
    wootz_wallet_service_->GetUserAssets(
        wootz_wallet_service_->network_manager()->GetCurrentChainId(
            mojom::CoinType::ETH, std::nullopt),
        mojom::CoinType::ETH,
        base::BindLambdaForTesting(
            [&](std::vector<mojom::BlockchainTokenPtr> tokens) {
              for (const auto& token : tokens) {
                tokens_out.push_back(token.Clone());
              }
              run_loop.Quit();
            }));
    run_loop.Run();
    return tokens_out;
  }

 protected:
  raw_ptr<WootzWalletService> wootz_wallet_service_ = nullptr;
  std::vector<std::string> methods_{"request", "send1", "send2", "sendAsync"};
  std::vector<std::string> addresses_{
      "0x6B175474E89094C44Da98b954EedeAC495271d0F",
      "0xdAC17F958D2ee523a2206206994597C13D831ec7",
      "0xc00e94Cb662C3520282E6f5717214004A7f26888",
      "0x4Fabb145d64652a948d72533023f6E7A623C7C53"};
  std::vector<std::string> symbols_{"USDC", "USDT", "GUSD", "BUSD"};
  std::vector<int> decimals_{6, 6, 2, 18};

 private:
  content::ContentMockCertVerifier mock_cert_verifier_;
  net::test_server::EmbeddedTestServer https_server_;
};

IN_PROC_BROWSER_TEST_F(WalletWatchAssetBrowserTest, UserApprovedRequest) {
  RestoreWallet();
  GURL url = https_server()->GetURL("a.com", "/wallet_watch_asset.html");

  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));

  size_t asset_num = GetUserAssets().size();
  for (size_t i = 0; i < methods_.size(); i++) {
    ASSERT_TRUE(ExecJs(
        web_contents(),
        base::StringPrintf("wallet_watchAsset('%s', 'ERC20', '%s', '%s', %d)",
                           methods_[i].c_str(), addresses_[i].c_str(),
                           symbols_[i].c_str(), decimals_[i]),
        content::EXECUTE_SCRIPT_NO_RESOLVE_PROMISES));
    base::RunLoop().RunUntilIdle();
    EXPECT_TRUE(
        wootz_wallet::WootzWalletTabHelper::FromWebContents(web_contents())
            ->IsShowingBubble());
    wootz_wallet_service_->NotifyAddSuggestTokenRequestsProcessed(
        true, {addresses_[i]});
  }
  EXPECT_EQ(asset_num + methods_.size(), GetUserAssets().size());
}

IN_PROC_BROWSER_TEST_F(WalletWatchAssetBrowserTest, UserRejectedRequest) {
  RestoreWallet();
  GURL url = https_server()->GetURL("a.com", "/wallet_watch_asset.html");

  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));

  size_t asset_num = GetUserAssets().size();
  for (size_t i = 0; i < methods_.size(); i++) {
    ASSERT_TRUE(ExecJs(
        web_contents(),
        base::StringPrintf("wallet_watchAsset('%s', 'ERC20', '%s', '%s', %d)",
                           methods_[i].c_str(), addresses_[i].c_str(),
                           symbols_[i].c_str(), decimals_[i]),
        content::EXECUTE_SCRIPT_NO_RESOLVE_PROMISES));
    base::RunLoop().RunUntilIdle();
    EXPECT_TRUE(
        wootz_wallet::WootzWalletTabHelper::FromWebContents(web_contents())
            ->IsShowingBubble());
    wootz_wallet_service_->NotifyAddSuggestTokenRequestsProcessed(
        false, {addresses_[i]});
  }
  EXPECT_EQ(asset_num, GetUserAssets().size());
}

IN_PROC_BROWSER_TEST_F(WalletWatchAssetBrowserTest, InvalidTypeParam) {
  RestoreWallet();
  GURL url = https_server()->GetURL("a.com", "/wallet_watch_asset.html");

  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));

  for (size_t i = 0; i < methods_.size(); i++) {
    EXPECT_EQ(EvalJs(web_contents(),
                     base::StringPrintf(
                         "wallet_watchAsset('%s', 'ERC721', '%s', '%s', %d)",
                         methods_[i].c_str(), addresses_[i].c_str(),
                         symbols_[i].c_str(), decimals_[i]))
                  .ExtractString(),
              "Asset of type 'ERC721' not supported");
  }
}

}  // namespace wootz_wallet
