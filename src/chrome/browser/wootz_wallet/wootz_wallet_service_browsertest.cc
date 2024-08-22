/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "base/memory/raw_ptr.h"
#include "base/path_service.h"
#include "base/test/bind.h"
#include "base/test/mock_callback.h"
#include "base/test/scoped_feature_list.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_service_factory.h"
#include "components/wootz_wallet/browser/wootz_wallet_service.h"
#include "components/wootz_wallet/browser/wootz_wallet_service_observer_base.h"
#include "components/wootz_wallet/browser/wootz_wallet_utils.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/wootz_wallet/common/features.h"
#include "components/constants/wootz_paths.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "net/dns/mock_host_resolver.h"
#include "url/origin.h"

namespace wootz_wallet {

namespace {

void OnGetActiveOrigin(bool* callback_called,
                       mojom::OriginInfoPtr expected_active_origin,
                       mojom::OriginInfoPtr active_origin) {
  EXPECT_EQ(expected_active_origin, active_origin);
  *callback_called = true;
}

}  // namespace

class TestWootzWalletServiceObserver
    : public wootz_wallet::WootzWalletServiceObserverBase {
 public:
  TestWootzWalletServiceObserver() = default;

  void OnActiveOriginChanged(mojom::OriginInfoPtr origin_info) override {
    active_origin_info_ = origin_info->Clone();
  }

  const mojom::OriginInfoPtr& active_origin_info() const {
    return active_origin_info_;
  }

  mojo::PendingRemote<wootz_wallet::mojom::WootzWalletServiceObserver>
  GetReceiver() {
    return observer_receiver_.BindNewPipeAndPassRemote();
  }

  void Reset() { active_origin_info_ = {}; }

 private:
  mojom::OriginInfoPtr active_origin_info_;
  mojo::Receiver<wootz_wallet::mojom::WootzWalletServiceObserver>
      observer_receiver_{this};
};

class WootzWalletServiceTest : public InProcessBrowserTest {
 public:
  WootzWalletServiceTest()
      : https_server_(net::EmbeddedTestServer::TYPE_HTTPS) {}

  void SetUp() override {
    InProcessBrowserTest::SetUp();
    feature_list_.InitAndEnableFeature(
        wootz_wallet::features::kNativeWootzWalletFeature);
  }

  void SetUpOnMainThread() override {
    InProcessBrowserTest::SetUpOnMainThread();
    base::FilePath test_data_dir;
    base::PathService::Get(wootz::DIR_TEST_DATA, &test_data_dir);
    https_server_.SetSSLConfig(net::EmbeddedTestServer::CERT_TEST_NAMES);
    https_server_.ServeFilesFromDirectory(test_data_dir);
    EXPECT_TRUE(https_server_.Start());
    host_resolver()->AddRule("*", "127.0.0.1");
    wallet_service_ = wootz_wallet::WootzWalletServiceFactory::GetInstance()
                          ->GetServiceForContext(browser()->profile());
  }

  void TestIsPrivateWindow(WootzWalletService* wallet_service,
                           bool expected_result) {
    base::MockCallback<base::OnceCallback<void(bool)>> callback;
    EXPECT_CALL(callback, Run(expected_result)).Times(1);

    wallet_service->IsPrivateWindow(callback.Get());
  }

  WootzWalletService* wallet_service() { return wallet_service_; }
  WootzWalletService* incognito_wallet_service() {
    return wootz_wallet::WootzWalletServiceFactory::GetInstance()
        ->GetServiceForContext(
            CreateIncognitoBrowser(browser()->profile())->profile());
  }
  const net::EmbeddedTestServer* https_server() const { return &https_server_; }

 private:
  raw_ptr<WootzWalletService> wallet_service_ = nullptr;
  net::EmbeddedTestServer https_server_;
  base::test::ScopedFeatureList feature_list_;
};

IN_PROC_BROWSER_TEST_F(WootzWalletServiceTest, ActiveOrigin) {
  GURL url = https_server()->GetURL("a.test", "/simple.html");
  auto expected_origin_info = MakeOriginInfo(url::Origin::Create(url));
  TestWootzWalletServiceObserver observer;
  wallet_service()->AddObserver(observer.GetReceiver());
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));

  bool callback_called = false;
  wallet_service()->GetActiveOrigin(base::BindOnce(
      &OnGetActiveOrigin, &callback_called, expected_origin_info->Clone()));
  base::RunLoop().RunUntilIdle();
  EXPECT_TRUE(callback_called);
  EXPECT_EQ(observer.active_origin_info(), expected_origin_info);

  url = https_server()->GetURL("b.test", "/simple.html");
  expected_origin_info = MakeOriginInfo(url::Origin::Create(url));
  callback_called = false;
  observer.Reset();
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));
  wallet_service()->GetActiveOrigin(base::BindOnce(
      &OnGetActiveOrigin, &callback_called, expected_origin_info->Clone()));
  base::RunLoop().RunUntilIdle();
  EXPECT_TRUE(callback_called);
  EXPECT_EQ(observer.active_origin_info(), expected_origin_info);

  url = https_server()->GetURL("c.test", "/simple.html");
  expected_origin_info = MakeOriginInfo(url::Origin::Create(url));
  observer.Reset();
  ui_test_utils::NavigateToURLWithDisposition(
      browser(), url, WindowOpenDisposition::NEW_FOREGROUND_TAB,
      ui_test_utils::BROWSER_TEST_WAIT_FOR_LOAD_STOP);
  wallet_service()->GetActiveOrigin(base::BindOnce(
      &OnGetActiveOrigin, &callback_called, expected_origin_info->Clone()));
  base::RunLoop().RunUntilIdle();
  EXPECT_TRUE(callback_called);
  EXPECT_EQ(observer.active_origin_info(), expected_origin_info);

  url = https_server()->GetURL("d.test", "/simple.html");
  expected_origin_info = MakeOriginInfo(url::Origin::Create(url));
  observer.Reset();
  ui_test_utils::NavigateToURLWithDisposition(
      browser(), url, WindowOpenDisposition::NEW_WINDOW,
      ui_test_utils::BROWSER_TEST_WAIT_FOR_LOAD_STOP);
  wallet_service()->GetActiveOrigin(base::BindOnce(
      &OnGetActiveOrigin, &callback_called, expected_origin_info->Clone()));
  base::RunLoop().RunUntilIdle();
  EXPECT_TRUE(callback_called);
  EXPECT_EQ(observer.active_origin_info(), expected_origin_info);
}

IN_PROC_BROWSER_TEST_F(WootzWalletServiceTest, IsPrivateWindow) {
  TestIsPrivateWindow(wallet_service(), false);
  wallet_service()->SetPrivateWindowsEnabled(true);
  TestIsPrivateWindow(incognito_wallet_service(), true);
  TestIsPrivateWindow(wallet_service(), false);
}

}  // namespace wootz_wallet
