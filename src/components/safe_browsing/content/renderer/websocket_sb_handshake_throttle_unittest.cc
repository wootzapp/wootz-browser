// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/safe_browsing/content/renderer/websocket_sb_handshake_throttle.h"

#include <optional>
#include <utility>

#include "base/functional/callback.h"
#include "base/run_loop.h"
#include "base/test/metrics/histogram_tester.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/task_environment.h"
#include "components/safe_browsing/content/common/safe_browsing.mojom.h"
#include "components/safe_browsing/core/common/features.h"
#include "components/safe_browsing/core/common/safe_browsing_url_checker.mojom.h"
#include "ipc/ipc_message.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "net/http/http_request_headers.h"
#include "services/network/public/mojom/fetch_api.mojom.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/platform/web_security_origin.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/platform/web_url.h"

namespace safe_browsing {

namespace {

constexpr char kTestUrl[] = "wss://test/";

#if BUILDFLAG(ENABLE_EXTENSIONS)
constexpr char kTestExtensionId[] = "abcdefghijklmnopabcdefghijklmnop";
constexpr char kTestExtensionUrl[] =
    "chrome-extension://abcdefghijklmnopabcdefghijklmnop/";

class FakeExtensionWebRequestReporter
    : public mojom::ExtensionWebRequestReporter {
 public:
  FakeExtensionWebRequestReporter() {}

  void SendWebRequestData(
      const std::string& origin_extension_id,
      const GURL& telemetry_url,
      mojom::WebRequestProtocolType protocol_type,
      mojom::WebRequestContactInitiatorType contact_initiator_type) override {
    origin_extension_id_ = origin_extension_id;
    telemetry_url_ = telemetry_url;
    protocol_type_ = protocol_type;
    contact_initiator_type_ = contact_initiator_type;
    run_loop_.Quit();
  }

  void Clone(mojo::PendingReceiver<mojom::ExtensionWebRequestReporter> receiver)
      override {
    NOTREACHED_NORETURN();
  }

  void RunUntilCalled() { run_loop_.Run(); }

  std::string origin_extension_id_;
  GURL telemetry_url_;
  mojom::WebRequestProtocolType protocol_type_;
  mojom::WebRequestContactInitiatorType contact_initiator_type_;
  base::RunLoop run_loop_;
};
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)

class FakeSafeBrowsing : public mojom::SafeBrowsing {
 public:
  FakeSafeBrowsing()
      : load_flags_(-1),
        request_destination_(),
        has_user_gesture_(false),
        originated_from_service_worker_(false) {}

  void CreateCheckerAndCheck(
      const std::optional<blink::LocalFrameToken>& frame_token,
      mojo::PendingReceiver<mojom::SafeBrowsingUrlChecker> receiver,
      const GURL& url,
      const std::string& method,
      const net::HttpRequestHeaders& headers,
      int32_t load_flags,
      network::mojom::RequestDestination request_destination,
      bool has_user_gesture,
      bool originated_from_service_worker,
      CreateCheckerAndCheckCallback callback) override {
    frame_token_ = frame_token;
    receiver_ = std::move(receiver);
    url_ = url;
    method_ = method;
    headers_ = headers;
    load_flags_ = load_flags;
    request_destination_ = request_destination;
    has_user_gesture_ = has_user_gesture;
    originated_from_service_worker_ = originated_from_service_worker;
    callback_ = std::move(callback);
    run_loop_.Quit();
  }

  void Clone(mojo::PendingReceiver<mojom::SafeBrowsing> receiver) override {
    NOTREACHED();
  }

  void RunUntilCalled() { run_loop_.Run(); }

  std::optional<blink::LocalFrameToken> frame_token_;
  mojo::PendingReceiver<mojom::SafeBrowsingUrlChecker> receiver_;
  GURL url_;
  std::string method_;
  net::HttpRequestHeaders headers_;
  int32_t load_flags_;
  network::mojom::RequestDestination request_destination_;
  bool has_user_gesture_;
  bool originated_from_service_worker_;
  CreateCheckerAndCheckCallback callback_;
  base::RunLoop run_loop_;
};

class FakeCallback {
 public:
  enum Result { RESULT_NOT_CALLED, RESULT_SUCCESS, RESULT_ERROR };

  FakeCallback() : result_(RESULT_NOT_CALLED) {}

  void OnCompletion(const std::optional<blink::WebString>& message) {
    if (message) {
      result_ = RESULT_ERROR;
      message_ = *message;
      run_loop_.Quit();
      return;
    }

    result_ = RESULT_SUCCESS;
    run_loop_.Quit();
  }

  void RunUntilCalled() { run_loop_.Run(); }

  void RunUntilIdle() { base::RunLoop().RunUntilIdle(); }

  Result result_;
  blink::WebString message_;
  base::RunLoop run_loop_;
};

class WebSocketSBHandshakeThrottleTest : public ::testing::Test {
 protected:
#if BUILDFLAG(ENABLE_EXTENSIONS)
  WebSocketSBHandshakeThrottleTest()
      : safe_browsing_receiver_(&safe_browsing_),
        extension_web_request_reporter_receiver_(
            &extension_web_request_reporter_) {
    safe_browsing_receiver_.Bind(
        safe_browsing_remote_.BindNewPipeAndPassReceiver());
    extension_web_request_reporter_receiver_.Bind(
        extension_web_request_reporter_remote_.BindNewPipeAndPassReceiver());
    throttle_ = std::make_unique<WebSocketSBHandshakeThrottle>(
        safe_browsing_remote_.get(), std::nullopt,
        extension_web_request_reporter_remote_.get());
  }
#else
  WebSocketSBHandshakeThrottleTest()
      : safe_browsing_receiver_(&safe_browsing_) {
    safe_browsing_receiver_.Bind(
        safe_browsing_remote_.BindNewPipeAndPassReceiver());
    throttle_ = std::make_unique<WebSocketSBHandshakeThrottle>(
        safe_browsing_remote_.get(), std::nullopt);
  }
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)

  void SetUp() override {
    feature_list_.InitAndDisableFeature(kSafeBrowsingSkipSubresources2);
  }

  base::test::TaskEnvironment message_loop_;
  FakeSafeBrowsing safe_browsing_;
  mojo::Receiver<mojom::SafeBrowsing> safe_browsing_receiver_;
  mojo::Remote<mojom::SafeBrowsing> safe_browsing_remote_;
#if BUILDFLAG(ENABLE_EXTENSIONS)
  FakeExtensionWebRequestReporter extension_web_request_reporter_;
  mojo::Receiver<mojom::ExtensionWebRequestReporter>
      extension_web_request_reporter_receiver_;
  mojo::Remote<mojom::ExtensionWebRequestReporter>
      extension_web_request_reporter_remote_;
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)
  std::unique_ptr<WebSocketSBHandshakeThrottle> throttle_;
  FakeCallback fake_callback_;
  base::test::ScopedFeatureList feature_list_;
};

TEST_F(WebSocketSBHandshakeThrottleTest, Construction) {}

TEST_F(WebSocketSBHandshakeThrottleTest, CheckArguments) {
  base::HistogramTester histogram_tester;
  throttle_->ThrottleHandshake(
      GURL(kTestUrl), blink::WebSecurityOrigin::CreateFromString(kTestUrl),
      blink::WebSecurityOrigin(),
      base::BindOnce(&FakeCallback::OnCompletion,
                     base::Unretained(&fake_callback_)));
  safe_browsing_.RunUntilCalled();
  EXPECT_FALSE(safe_browsing_.frame_token_);
  EXPECT_EQ(GURL(kTestUrl), safe_browsing_.url_);
  EXPECT_EQ("GET", safe_browsing_.method_);
  EXPECT_TRUE(safe_browsing_.headers_.GetHeaderVector().empty());
  EXPECT_EQ(0, safe_browsing_.load_flags_);
  EXPECT_EQ(network::mojom::RequestDestination::kEmpty,
            safe_browsing_.request_destination_);
  EXPECT_FALSE(safe_browsing_.has_user_gesture_);
  EXPECT_FALSE(safe_browsing_.originated_from_service_worker_);
  EXPECT_TRUE(safe_browsing_.callback_);
  histogram_tester.ExpectUniqueSample("SafeBrowsing.WebSocketCheck.Skipped",
                                      false, 1);
}

TEST_F(WebSocketSBHandshakeThrottleTest, Safe) {
  base::HistogramTester histogram_tester;
  throttle_->ThrottleHandshake(
      GURL(kTestUrl), blink::WebSecurityOrigin::CreateFromString(kTestUrl),
      blink::WebSecurityOrigin(),
      base::BindOnce(&FakeCallback::OnCompletion,
                     base::Unretained(&fake_callback_)));
  safe_browsing_.RunUntilCalled();
  std::move(safe_browsing_.callback_).Run(mojo::NullReceiver(), true, false);
  fake_callback_.RunUntilCalled();
  EXPECT_EQ(FakeCallback::RESULT_SUCCESS, fake_callback_.result_);
  histogram_tester.ExpectUniqueSample("SafeBrowsing.WebSocketCheck.Skipped",
                                      false, 1);
}

TEST_F(WebSocketSBHandshakeThrottleTest, Unsafe) {
  base::HistogramTester histogram_tester;
  throttle_->ThrottleHandshake(
      GURL(kTestUrl), blink::WebSecurityOrigin::CreateFromString(kTestUrl),
      blink::WebSecurityOrigin(),
      base::BindOnce(&FakeCallback::OnCompletion,
                     base::Unretained(&fake_callback_)));
  safe_browsing_.RunUntilCalled();
  std::move(safe_browsing_.callback_).Run(mojo::NullReceiver(), false, false);
  fake_callback_.RunUntilCalled();
  EXPECT_EQ(FakeCallback::RESULT_ERROR, fake_callback_.result_);
  EXPECT_EQ(
      blink::WebString(
          "WebSocket connection to wss://test/ failed safe browsing check"),
      fake_callback_.message_);
  histogram_tester.ExpectUniqueSample("SafeBrowsing.WebSocketCheck.Skipped",
                                      false, 1);
}

TEST_F(WebSocketSBHandshakeThrottleTest, SlowCheckNotifier) {
  base::HistogramTester histogram_tester;
  throttle_->ThrottleHandshake(
      GURL(kTestUrl), blink::WebSecurityOrigin::CreateFromString(kTestUrl),
      blink::WebSecurityOrigin(),
      base::BindOnce(&FakeCallback::OnCompletion,
                     base::Unretained(&fake_callback_)));
  safe_browsing_.RunUntilCalled();

  mojo::Remote<mojom::UrlCheckNotifier> slow_check_notifier;
  std::move(safe_browsing_.callback_)
      .Run(slow_check_notifier.BindNewPipeAndPassReceiver(), false, false);
  fake_callback_.RunUntilIdle();
  EXPECT_EQ(FakeCallback::RESULT_NOT_CALLED, fake_callback_.result_);

  slow_check_notifier->OnCompleteCheck(true, false);
  fake_callback_.RunUntilCalled();
  EXPECT_EQ(FakeCallback::RESULT_SUCCESS, fake_callback_.result_);
  histogram_tester.ExpectUniqueSample("SafeBrowsing.WebSocketCheck.Skipped",
                                      false, 1);
}

TEST_F(WebSocketSBHandshakeThrottleTest, MojoServiceNotThere) {
  base::HistogramTester histogram_tester;
  safe_browsing_receiver_.reset();
  throttle_->ThrottleHandshake(
      GURL(kTestUrl), blink::WebSecurityOrigin::CreateFromString(kTestUrl),
      blink::WebSecurityOrigin(),
      base::BindOnce(&FakeCallback::OnCompletion,
                     base::Unretained(&fake_callback_)));
  fake_callback_.RunUntilCalled();
  EXPECT_EQ(FakeCallback::RESULT_SUCCESS, fake_callback_.result_);
  histogram_tester.ExpectUniqueSample("SafeBrowsing.WebSocketCheck.Skipped",
                                      false, 1);
}

#if BUILDFLAG(ENABLE_EXTENSIONS)
TEST_F(WebSocketSBHandshakeThrottleTest, SendExtensionWebRequestData) {
  base::HistogramTester histogram_tester;
  throttle_->ThrottleHandshake(
      GURL(kTestUrl),
      blink::WebSecurityOrigin::CreateFromString(kTestExtensionUrl),
      blink::WebSecurityOrigin(),
      base::BindOnce(&FakeCallback::OnCompletion,
                     base::Unretained(&fake_callback_)));
  extension_web_request_reporter_.RunUntilCalled();

  EXPECT_EQ(extension_web_request_reporter_.origin_extension_id_,
            kTestExtensionId);
  EXPECT_EQ(extension_web_request_reporter_.telemetry_url_, GURL(kTestUrl));
  EXPECT_EQ(extension_web_request_reporter_.protocol_type_,
            mojom::WebRequestProtocolType::kWebSocket);
  EXPECT_EQ(extension_web_request_reporter_.contact_initiator_type_,
            mojom::WebRequestContactInitiatorType::kExtension);

  // A log of "false" represents the data being sent.
  histogram_tester.ExpectBucketCount(
      "SafeBrowsing.ExtensionTelemetry.WebSocketRequestDataSentOrReceived",
      false, 1);
}

TEST_F(WebSocketSBHandshakeThrottleTest,
       SendExtensionWebRequestData_ContentScript) {
  base::HistogramTester histogram_tester;
  throttle_->ThrottleHandshake(
      GURL(kTestUrl), blink::WebSecurityOrigin(),
      blink::WebSecurityOrigin::CreateFromString(kTestExtensionUrl),
      base::BindOnce(&FakeCallback::OnCompletion,
                     base::Unretained(&fake_callback_)));
  extension_web_request_reporter_.RunUntilCalled();

  EXPECT_EQ(extension_web_request_reporter_.origin_extension_id_,
            kTestExtensionId);
  EXPECT_EQ(extension_web_request_reporter_.telemetry_url_, GURL(kTestUrl));
  EXPECT_EQ(extension_web_request_reporter_.protocol_type_,
            mojom::WebRequestProtocolType::kWebSocket);
  EXPECT_EQ(extension_web_request_reporter_.contact_initiator_type_,
            mojom::WebRequestContactInitiatorType::kContentScript);

  // A log of "false" represents the data being sent.
  histogram_tester.ExpectBucketCount(
      "SafeBrowsing.ExtensionTelemetry.WebSocketRequestDataSentOrReceived",
      false, 1);
}
#endif  // BUILDFLAG(ENABLE_EXTENSIONS)

class WebSocketSBHandshakeThrottleNoSafeBrowsingCheckTest
    : public WebSocketSBHandshakeThrottleTest {
  void SetUp() override {
    feature_list_.InitAndEnableFeature(kSafeBrowsingSkipSubresources2);
  }
};

TEST_F(WebSocketSBHandshakeThrottleNoSafeBrowsingCheckTest, DoesNotRunCheck) {
  base::HistogramTester histogram_tester;
  throttle_->ThrottleHandshake(
      GURL(kTestUrl), blink::WebSecurityOrigin::CreateFromString(kTestUrl),
      blink::WebSecurityOrigin(),
      base::BindOnce(&FakeCallback::OnCompletion,
                     base::Unretained(&fake_callback_)));
  EXPECT_EQ(FakeCallback::RESULT_SUCCESS, fake_callback_.result_);
  histogram_tester.ExpectUniqueSample("SafeBrowsing.WebSocketCheck.Skipped",
                                      true, 1);
}

}  // namespace

}  // namespace safe_browsing
