// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/performance_manager/policies/memory_saver_mode_policy.h"

#include "base/test/scoped_feature_list.h"
#include "base/test/task_environment.h"
#include "base/time/time.h"
#include "chrome/browser/performance_manager/test_support/page_discarding_utils.h"
#include "components/performance_manager/public/decorators/tab_connectedness_decorator.h"
#include "components/performance_manager/public/decorators/tab_page_decorator.h"
#include "components/performance_manager/public/features.h"
#include "components/performance_manager/public/user_tuning/prefs.h"
#include "components/performance_manager/public/user_tuning/tab_revisit_tracker.h"
#include "components/prefs/testing_pref_service.h"

namespace performance_manager::policies {

class TestTabRevisitTracker : public TabRevisitTracker {
 public:
  void SetStateBundle(const TabPageDecorator::TabHandle* tab_handle,
                      StateBundle bundle) {
    state_bundles_[tab_handle] = bundle;
  }

 private:
  StateBundle GetStateForTabHandle(
      const TabPageDecorator::TabHandle* tab_handle) override {
    auto it = state_bundles_.find(tab_handle);
    // Some of these tests don't exercise behavior around the
    // TabRevisitTrackerState. Instead of requiring all of them to set up proper
    // state explicitly, just return a default constructed bundle. The only
    // field that is being used from MemorySaverModePolicy is `num_revisits`,
    // and it being default initialized to 0 is what we'd want anyway.
    if (it != state_bundles_.end()) {
      return it->second;
    }
    return {};
  }

  std::map<const TabPageDecorator::TabHandle*, TabRevisitTracker::StateBundle>
      state_bundles_;
};

class MemorySaverModeTest : public testing::GraphTestHarnessWithMockDiscarder {
 public:
  void SetUp() override {
    testing::GraphTestHarnessWithMockDiscarder::SetUp();

    graph()->PassToGraph(
        std::make_unique<performance_manager::TabPageDecorator>());
    graph()->PassToGraph(std::make_unique<TabConnectednessDecorator>());
    std::unique_ptr<TestTabRevisitTracker> tab_revisit_tracker =
        std::make_unique<TestTabRevisitTracker>();
    tab_revisit_tracker_ = tab_revisit_tracker.get();
    graph()->PassToGraph(std::move(tab_revisit_tracker));

    // This is usually called when the profile is created. Fake it here since it
    // doesn't happen in tests.
    PageDiscardingHelper::GetFromGraph(graph())->SetNoDiscardPatternsForProfile(
        static_cast<PageNode*>(page_node())->GetBrowserContextID(), {});

    auto policy = std::make_unique<MemorySaverModePolicy>();
    policy->SetTimeBeforeDiscard(base::Hours(2));
    policy_ = policy.get();
    graph()->PassToGraph(std::move(policy));
  }

  void TearDown() override {
    graph()->TakeFromGraph(policy_);
    testing::GraphTestHarnessWithMockDiscarder::TearDown();
  }

  MemorySaverModePolicy* policy() { return policy_; }

 protected:
  PageNodeImpl* CreateOtherPageNode() {
    other_process_node_ = CreateNode<performance_manager::ProcessNodeImpl>();
    other_page_node_ = CreateNode<performance_manager::PageNodeImpl>();
    other_main_frame_node_ = CreateFrameNodeAutoId(other_process_node_.get(),
                                                   other_page_node_.get());
    testing::MakePageNodeDiscardable(other_page_node_.get(), task_env());

    return other_page_node_.get();
  }

  void ResetOtherPage() {
    other_main_frame_node_.reset();
    other_page_node_.reset();
    other_page_node_.reset();
  }

  TestTabRevisitTracker* tab_revisit_tracker() { return tab_revisit_tracker_; }

 private:
  raw_ptr<MemorySaverModePolicy, DanglingUntriaged> policy_;

  performance_manager::TestNodeWrapper<performance_manager::PageNodeImpl>
      other_page_node_;
  performance_manager::TestNodeWrapper<performance_manager::ProcessNodeImpl>
      other_process_node_;
  performance_manager::TestNodeWrapper<performance_manager::FrameNodeImpl>
      other_main_frame_node_;

  raw_ptr<TestTabRevisitTracker> tab_revisit_tracker_;
};

TEST_F(MemorySaverModeTest, NoDiscardIfMemorySaverOff) {
  page_node()->SetType(PageType::kTab);
  page_node()->SetIsVisible(true);
  page_node()->SetIsVisible(false);
  task_env().FastForwardUntilNoTasksRemain();
  ::testing::Mock::VerifyAndClearExpectations(discarder());
}

TEST_F(MemorySaverModeTest, DiscardAfterBackgrounded) {
  page_node()->SetType(PageType::kTab);
  page_node()->SetIsVisible(true);
  policy()->OnMemorySaverModeChanged(true);

  EXPECT_CALL(*discarder(), DiscardPageNodeImpl(page_node()))
      .WillOnce(::testing::Return(true));
  page_node()->SetIsVisible(false);

  task_env().FastForwardBy(policy()->GetTimeBeforeDiscardForTesting());
  ::testing::Mock::VerifyAndClearExpectations(discarder());
}

TEST_F(MemorySaverModeTest, DontDiscardAfterBackgroundedIfSuspended) {
  // TODO(crbug/40925329): When cleaning up this feature, consider also
  // cleaning up the "user managed time" code, or making it controllable
  // via a mechanism other than `modal_memory_saver_mode` if it should
  // be kept.
  base::test::ScopedFeatureList feature_list_;
  feature_list_.InitWithFeaturesAndParameters(
      {
          {features::kModalMemorySaver, {{"modal_memory_saver_mode", "0"}}},
      },
      /*disabled_features=*/{});
  policy()->SetTimeBeforeDiscard(base::Hours(2));
  page_node()->SetType(PageType::kTab);
  page_node()->SetIsVisible(true);
  policy()->OnMemorySaverModeChanged(true);
  page_node()->SetIsVisible(false);

  EXPECT_EQ(policy()->GetTimeBeforeDiscardForTesting(), base::Hours(2));

  // The tab isn't discarded if the elapsed time was spent with the device
  // suspended.
  task_env().SuspendedFastForwardBy(base::Hours(10));
  ::testing::Mock::VerifyAndClearExpectations(discarder());

  // Advance only one hour, there should still not be a discard.
  task_env().FastForwardBy(base::Hours(1));
  ::testing::Mock::VerifyAndClearExpectations(discarder());

  // Suspend again for more than the expected time, no discard should happen.
  task_env().SuspendedFastForwardBy(base::Hours(10));
  ::testing::Mock::VerifyAndClearExpectations(discarder());

  EXPECT_CALL(*discarder(), DiscardPageNodeImpl(page_node()))
      .WillOnce(::testing::Return(true));

  // Finally advance un-suspended until the time is elapsed, the tab should be
  // discarded.
  task_env().FastForwardBy(base::Hours(1));
  ::testing::Mock::VerifyAndClearExpectations(discarder());
}

TEST_F(MemorySaverModeTest, DontDiscardIfPageIsNotATab) {
  page_node()->SetType(PageType::kUnknown);
  policy()->OnMemorySaverModeChanged(true);
  page_node()->SetIsVisible(true);
  page_node()->SetIsVisible(false);

  task_env().FastForwardUntilNoTasksRemain();
  ::testing::Mock::VerifyAndClearExpectations(discarder());
}

// The tab shouldn't be discarded if it's playing audio. There are many other
// conditions that prevent discarding, but they're implemented in
// `PageDiscardingHelper` and therefore tested there.
TEST_F(MemorySaverModeTest, DontDiscardIfPlayingAudio) {
  page_node()->SetType(PageType::kTab);
  page_node()->SetIsVisible(true);
  policy()->OnMemorySaverModeChanged(true);

  page_node()->SetIsAudible(true);

  page_node()->SetIsVisible(false);
  task_env().FastForwardUntilNoTasksRemain();
  ::testing::Mock::VerifyAndClearExpectations(discarder());
}

TEST_F(MemorySaverModeTest, TimeBeforeDiscardChangedBeforeTimerStarted) {
  // TODO(crbug/40925329): When cleaning up this feature, consider also
  // cleaning up the "user managed time" code, or making it controllable
  // via a mechanism other than `modal_memory_saver_mode` if it should
  // be kept.
  base::test::ScopedFeatureList feature_list_;
  feature_list_.InitWithFeaturesAndParameters(
      {
          {features::kModalMemorySaver, {{"modal_memory_saver_mode", "0"}}},
      },
      /*disabled_features=*/{});
  base::TimeDelta original_time_before_discard =
      policy()->GetTimeBeforeDiscardForTesting();
  base::TimeDelta increased_time_before_discard = base::Seconds(10);
  policy()->SetTimeBeforeDiscard(original_time_before_discard +
                                 increased_time_before_discard);

  page_node()->SetType(PageType::kTab);
  page_node()->SetIsVisible(true);
  page_node()->SetIsVisible(false);
  policy()->OnMemorySaverModeChanged(true);

  task_env().FastForwardBy(original_time_before_discard);
  ::testing::Mock::VerifyAndClearExpectations(discarder());

  EXPECT_CALL(*discarder(), DiscardPageNodeImpl(page_node()))
      .WillOnce(::testing::Return(true));

  task_env().FastForwardBy(increased_time_before_discard);
  ::testing::Mock::VerifyAndClearExpectations(discarder());
}

TEST_F(MemorySaverModeTest, TimeBeforeDiscardReduced) {
  // TODO(crbug/40925329): When cleaning up this feature, consider also
  // cleaning up the "user managed time" code, or making it controllable
  // via a mechanism other than `modal_memory_saver_mode` if it should
  // be kept.
  base::test::ScopedFeatureList feature_list_;
  feature_list_.InitWithFeaturesAndParameters(
      {
          {features::kModalMemorySaver, {{"modal_memory_saver_mode", "0"}}},
      },
      /*disabled_features=*/{});
  base::TimeDelta original_time_before_discard =
      policy()->GetTimeBeforeDiscardForTesting();
  constexpr base::TimeDelta kNewTimeBeforeDiscard = base::Minutes(20);
  constexpr base::TimeDelta kInitialBackgroundTime = base::Minutes(10);
  EXPECT_GE(original_time_before_discard, kNewTimeBeforeDiscard);
  EXPECT_GE(kNewTimeBeforeDiscard, kInitialBackgroundTime);

  page_node()->SetType(PageType::kTab);
  page_node()->SetIsVisible(true);
  page_node()->SetIsVisible(false);
  policy()->OnMemorySaverModeChanged(true);

  task_env().FastForwardBy(kInitialBackgroundTime);
  ::testing::Mock::VerifyAndClearExpectations(discarder());

  policy()->SetTimeBeforeDiscard(kNewTimeBeforeDiscard);

  // Expect tab to not take into account time spent in the background prior
  // to the time before discard changing.
  task_env().FastForwardBy(kNewTimeBeforeDiscard - kInitialBackgroundTime);
  ::testing::Mock::VerifyAndClearExpectations(discarder());

  // Expect tab to be discarded after the new time before discard has elapsed
  // since the last change to it.
  EXPECT_CALL(*discarder(), DiscardPageNodeImpl(page_node()))
      .WillOnce(::testing::Return(true));

  task_env().FastForwardBy(kInitialBackgroundTime);
  ::testing::Mock::VerifyAndClearExpectations(discarder());
}

TEST_F(MemorySaverModeTest, TimeBeforeDiscardReducedBelowBackgroundedTime) {
  // TODO(crbug/40925329): When cleaning up this feature, consider also
  // cleaning up the "user managed time" code, or making it controllable
  // via a mechanism other than `modal_memory_saver_mode` if it should
  // be kept.
  base::test::ScopedFeatureList feature_list_;
  feature_list_.InitWithFeaturesAndParameters(
      {
          {features::kModalMemorySaver, {{"modal_memory_saver_mode", "0"}}},
      },
      /*disabled_features=*/{});
  base::TimeDelta original_time_before_discard =
      policy()->GetTimeBeforeDiscardForTesting();
  constexpr base::TimeDelta kNewTimeBeforeDiscard = base::Minutes(5);
  constexpr base::TimeDelta kInitialBackgroundTime = base::Minutes(10);
  EXPECT_GE(original_time_before_discard, kInitialBackgroundTime);
  EXPECT_GE(kInitialBackgroundTime, kNewTimeBeforeDiscard);

  page_node()->SetType(PageType::kTab);
  page_node()->SetIsVisible(true);
  page_node()->SetIsVisible(false);
  policy()->OnMemorySaverModeChanged(true);

  task_env().FastForwardBy(kInitialBackgroundTime);
  ::testing::Mock::VerifyAndClearExpectations(discarder());

  // Expect tab to not be immediately discarded if time to discard is changed
  // to something smaller than the already elapsed time in the background.
  policy()->SetTimeBeforeDiscard(kNewTimeBeforeDiscard);
  ::testing::Mock::VerifyAndClearExpectations(discarder());

  // Expect tab to be discarded after the new time before discard has elapsed
  // since the last change to it.
  EXPECT_CALL(*discarder(), DiscardPageNodeImpl(page_node()))
      .WillOnce(::testing::Return(true));

  task_env().FastForwardBy(kNewTimeBeforeDiscard);
  ::testing::Mock::VerifyAndClearExpectations(discarder());
}

TEST_F(MemorySaverModeTest, TimeBeforeDiscardIncreased) {
  // TODO(crbug/40925329): When cleaning up this feature, consider also
  // cleaning up the "user managed time" code, or making it controllable
  // via a mechanism other than `modal_memory_saver_mode` if it should
  // be kept.
  base::test::ScopedFeatureList feature_list_;
  feature_list_.InitWithFeaturesAndParameters(
      {
          {features::kModalMemorySaver, {{"modal_memory_saver_mode", "0"}}},
      },
      /*disabled_features=*/{});
  base::TimeDelta original_time_before_discard =
      policy()->GetTimeBeforeDiscardForTesting();
  constexpr base::TimeDelta kNewTimeBeforeDiscard = base::Hours(3);
  constexpr base::TimeDelta kInitialBackgroundTime = base::Minutes(10);
  EXPECT_GE(kNewTimeBeforeDiscard, original_time_before_discard);
  EXPECT_GE(original_time_before_discard, kInitialBackgroundTime);

  page_node()->SetType(PageType::kTab);
  page_node()->SetIsVisible(true);
  page_node()->SetIsVisible(false);
  policy()->OnMemorySaverModeChanged(true);

  task_env().FastForwardBy(kInitialBackgroundTime);
  ::testing::Mock::VerifyAndClearExpectations(discarder());
  // Time elapsed since beginning of test = kInitialBackgroundTime

  policy()->SetTimeBeforeDiscard(kNewTimeBeforeDiscard);

  // Expect original timer to not be in effect
  task_env().FastForwardBy(original_time_before_discard -
                           kInitialBackgroundTime);
  ::testing::Mock::VerifyAndClearExpectations(discarder());
  // Time elapsed since beginning of test = original_time_before_discard

  // Expect tab to not take into account time spent in the background prior to
  // the time before discard changing.
  task_env().FastForwardBy(kNewTimeBeforeDiscard -
                           original_time_before_discard);
  ::testing::Mock::VerifyAndClearExpectations(discarder());
  // Time elapsed since beginning of test = kNewTimeBeforeDiscard

  // Expect tab to be discarded after the new time before discard has elapsed
  // since the last change to it.
  EXPECT_CALL(*discarder(), DiscardPageNodeImpl(page_node()))
      .WillOnce(::testing::Return(true));
  task_env().FastForwardBy(kInitialBackgroundTime);
  ::testing::Mock::VerifyAndClearExpectations(discarder());
  // Time elapsed since beginning of test = kInitialBackgroundTime +
  //                                        kNewTimeBeforeDiscard
}

TEST_F(MemorySaverModeTest, DontDiscardIfAlreadyNotVisibleWhenModeEnabled) {
  page_node()->SetType(PageType::kTab);
  page_node()->SetIsVisible(true);
  page_node()->SetIsVisible(false);

  // Shouldn't be discarded yet
  task_env().FastForwardUntilNoTasksRemain();
  ::testing::Mock::VerifyAndClearExpectations(discarder());

  // Advance time by the usual discard interval, minus 10 seconds.
  task_env().FastForwardBy(policy()->GetTimeBeforeDiscardForTesting() -
                           base::Seconds(10));
  ::testing::Mock::VerifyAndClearExpectations(discarder());

  policy()->OnMemorySaverModeChanged(true);

  // The page should not be discarded 10 seconds after the mode is changed.
  task_env().FastForwardBy(base::Seconds(10));
  ::testing::Mock::VerifyAndClearExpectations(discarder());

  // Instead, it should be discarded after the usual discard interval.
  EXPECT_CALL(*discarder(), DiscardPageNodeImpl(page_node()))
      .WillOnce(::testing::Return(true));
  task_env().FastForwardBy(policy()->GetTimeBeforeDiscardForTesting() -
                           base::Seconds(10));
  ::testing::Mock::VerifyAndClearExpectations(discarder());
}

TEST_F(MemorySaverModeTest, NoDiscardIfPageNodeRemoved) {
  // This case will be using a different page node, so make the default one
  // visible so it's not discarded.
  page_node()->SetIsVisible(true);
  policy()->OnMemorySaverModeChanged(true);

  PageNodeImpl* page_node = CreateOtherPageNode();
  EXPECT_EQ(PageType::kUnknown, page_node->GetType());

  page_node->SetType(PageType::kTab);

  page_node->SetIsVisible(false);
  ResetOtherPage();

  task_env().FastForwardUntilNoTasksRemain();
  ::testing::Mock::VerifyAndClearExpectations(discarder());
}

TEST_F(MemorySaverModeTest, UnknownPageNodeNeverAddedToMap) {
  // This case will be using a different page node, so make the default one
  // visible so it's not discarded.
  page_node()->SetIsVisible(true);
  policy()->OnMemorySaverModeChanged(true);

  PageNodeImpl* page_node = CreateOtherPageNode();
  EXPECT_EQ(PageType::kUnknown, page_node->GetType());

  page_node->SetIsVisible(false);
  ResetOtherPage();

  task_env().FastForwardUntilNoTasksRemain();
  ::testing::Mock::VerifyAndClearExpectations(discarder());
}

TEST_F(MemorySaverModeTest, PageNodeDiscardedIfTypeChanges) {
  // This case will be using a different page node, so make the default one
  // visible so it's not discarded.
  page_node()->SetIsVisible(true);
  policy()->OnMemorySaverModeChanged(true);

  PageNodeImpl* page_node = CreateOtherPageNode();
  EXPECT_EQ(PageType::kUnknown, page_node->GetType());

  page_node->SetType(PageType::kTab);

  EXPECT_CALL(*discarder(), DiscardPageNodeImpl(page_node))
      .WillOnce(::testing::Return(true));
  page_node->SetIsVisible(false);

  task_env().FastForwardBy(policy()->GetTimeBeforeDiscardForTesting());
  ::testing::Mock::VerifyAndClearExpectations(discarder());
}

TEST_F(MemorySaverModeTest,
       DiscardAfterTimeForCurrentModeIfNumRevisitsUnderMax) {
  page_node()->SetType(PageType::kTab);
  page_node()->SetIsVisible(true);
  policy()->OnMemorySaverModeChanged(true);

  base::test::ScopedFeatureList feature_list;
  // 1 is "conservative, so 6 hours and max_num_revisits == 5"
  feature_list.InitAndEnableFeatureWithParameters(
      features::kModalMemorySaver, {{"modal_memory_saver_mode", "1"}});

  page_node()->SetIsVisible(false);
  EXPECT_EQ(policy()->GetTimeBeforeDiscardForTesting(), base::Hours(6));

  // Advancing by less than 6 hours shouldn't discard.
  task_env().FastForwardBy(policy()->GetTimeBeforeDiscardForTesting() -
                           base::Seconds(10));
  ::testing::Mock::VerifyAndClearExpectations(discarder());

  EXPECT_CALL(*discarder(), DiscardPageNodeImpl(page_node()))
      .WillOnce(::testing::Return(true));
  task_env().FastForwardBy(base::Seconds(10));
  ::testing::Mock::VerifyAndClearExpectations(discarder());
}

TEST_F(MemorySaverModeTest, DontDiscardIfAboveMaxNumRevisits) {
  page_node()->SetType(PageType::kTab);
  page_node()->SetIsVisible(true);
  policy()->OnMemorySaverModeChanged(true);

  base::test::ScopedFeatureList feature_list;
  // 1 is "conservative, so 6 hours and max_num_revisits == 5"
  feature_list.InitAndEnableFeatureWithParameters(
      features::kModalMemorySaver, {{"modal_memory_saver_mode", "1"}});

  TabRevisitTracker::StateBundle state;
  state.num_revisits =
      100;  // needs to be > 5 because the mode is set to "conservative".
  tab_revisit_tracker()->SetStateBundle(
      TabPageDecorator::FromPageNode(page_node()), state);

  page_node()->SetIsVisible(false);
  EXPECT_EQ(policy()->GetTimeBeforeDiscardForTesting(), base::Hours(6));

  // Advancing by 6 hours shouldn't discard because the tab has been revisited
  // too many times.
  task_env().FastForwardBy(policy()->GetTimeBeforeDiscardForTesting());
  ::testing::Mock::VerifyAndClearExpectations(discarder());
}

}  // namespace performance_manager::policies
