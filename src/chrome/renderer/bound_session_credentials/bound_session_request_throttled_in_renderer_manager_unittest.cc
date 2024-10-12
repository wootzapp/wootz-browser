// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/renderer/bound_session_credentials/bound_session_request_throttled_in_renderer_manager.h"

#include <memory>

#include "base/memory/ptr_util.h"
#include "base/memory/scoped_refptr.h"
#include "base/test/task_environment.h"
#include "base/test/test_future.h"
#include "chrome/common/bound_session_request_throttled_handler.h"
#include "chrome/common/renderer_configuration.mojom.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {
using testing::FieldsAre;
using UnblockAction = BoundSessionRequestThrottledHandler::UnblockAction;
using ResumeBlockedRequestsTrigger =
    chrome::mojom::ResumeBlockedRequestsTrigger;

class FakeBoundSessionRequestThrottledHandler
    : public chrome::mojom::BoundSessionRequestThrottledHandler {
 public:
  FakeBoundSessionRequestThrottledHandler(
      mojo::PendingReceiver<chrome::mojom::BoundSessionRequestThrottledHandler>
          receiver)
      : receiver_(this, std::move(receiver)) {}

  void HandleRequestBlockedOnCookie(
      HandleRequestBlockedOnCookieCallback callback) override {
    // There shouldn't be more than one notification at a time of requests being
    // blocked.
    EXPECT_FALSE(callback_);
    callback_ = std::move(callback);
  }

  void SimulateHandleRequestBlockedOnCookieCompleted() {
    EXPECT_TRUE(callback_);
    std::move(callback_).Run(ResumeBlockedRequestsTrigger::kCookieAlreadyFresh);
  }

  bool IsRequestBlocked() { return !callback_.is_null(); }

 private:
  HandleRequestBlockedOnCookieCallback callback_;
  mojo::Receiver<chrome::mojom::BoundSessionRequestThrottledHandler> receiver_;
};
}  // namespace

class BoundSessionRequestThrottledInRendererManagerTest
    : public ::testing::Test {
 public:
  BoundSessionRequestThrottledInRendererManagerTest() {
    mojo::PendingRemote<chrome::mojom::BoundSessionRequestThrottledHandler>
        remote;
    handler_ = std::make_unique<FakeBoundSessionRequestThrottledHandler>(
        remote.InitWithNewPipeAndPassReceiver());
    manager_ = new BoundSessionRequestThrottledInRendererManager();
    manager_->Initialize(std::move(remote));
  }

  ~BoundSessionRequestThrottledInRendererManagerTest() override = default;

  void RunUntilIdle() { task_environment_.RunUntilIdle(); }

  BoundSessionRequestThrottledInRendererManager* manager() const {
    return manager_.get();
  }

  FakeBoundSessionRequestThrottledHandler* handler() { return handler_.get(); }

  void ResetHandler() { handler_.reset(); }

 private:
  base::test::TaskEnvironment task_environment_;
  std::unique_ptr<FakeBoundSessionRequestThrottledHandler> handler_;
  scoped_refptr<BoundSessionRequestThrottledInRendererManager> manager_;
};

TEST_F(BoundSessionRequestThrottledInRendererManagerTest, SingleRequest) {
  base::test::TestFuture<UnblockAction, ResumeBlockedRequestsTrigger> future;
  manager()->HandleRequestBlockedOnCookie(future.GetCallback());

  RunUntilIdle();
  EXPECT_TRUE(handler()->IsRequestBlocked());

  handler()->SimulateHandleRequestBlockedOnCookieCompleted();
  EXPECT_THAT(future.Get(),
              FieldsAre(UnblockAction::kResume,
                        ResumeBlockedRequestsTrigger::kCookieAlreadyFresh));
}

TEST_F(BoundSessionRequestThrottledInRendererManagerTest, MultipleRequests) {
  constexpr size_t kBlockedRequests = 5;
  std::array<
      base::test::TestFuture<UnblockAction, ResumeBlockedRequestsTrigger>,
      kBlockedRequests>
      futures;
  for (auto& future : futures) {
    manager()->HandleRequestBlockedOnCookie(future.GetCallback());
  }

  // Allow mojo message posting to complete.
  RunUntilIdle();
  EXPECT_TRUE(handler()->IsRequestBlocked());
  for (auto& future : futures) {
    EXPECT_FALSE(future.IsReady());
  }

  handler()->SimulateHandleRequestBlockedOnCookieCompleted();
  for (auto& future : futures) {
    EXPECT_THAT(future.Get(),
                FieldsAre(UnblockAction::kResume,
                          ResumeBlockedRequestsTrigger::kCookieAlreadyFresh));
  }
}

TEST_F(BoundSessionRequestThrottledInRendererManagerTest,
       RemoteDisconnectedPendingBlockedRequestsAreCancelled) {
  constexpr size_t kBlockedRequests = 5;
  std::array<
      base::test::TestFuture<UnblockAction, ResumeBlockedRequestsTrigger>,
      kBlockedRequests>
      futures;
  for (auto& future : futures) {
    manager()->HandleRequestBlockedOnCookie(future.GetCallback());
  }

  // Allow mojo message posting to complete.
  RunUntilIdle();
  EXPECT_TRUE(handler()->IsRequestBlocked());

  ResetHandler();
  for (auto& future : futures) {
    EXPECT_THAT(future.Get(),
                FieldsAre(UnblockAction::kCancel,
                          ResumeBlockedRequestsTrigger::kRendererDisconnected));
  }
}

TEST_F(BoundSessionRequestThrottledInRendererManagerTest,
       RemoteDisconnectedNewBlockedRequestsAreCancelled) {
  ResetHandler();

  base::test::TestFuture<UnblockAction, ResumeBlockedRequestsTrigger> future;
  manager()->HandleRequestBlockedOnCookie(future.GetCallback());

  EXPECT_THAT(future.Get(),
              FieldsAre(UnblockAction::kCancel,
                        ResumeBlockedRequestsTrigger::kRendererDisconnected));
}
