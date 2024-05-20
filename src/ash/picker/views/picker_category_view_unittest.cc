// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/picker/views/picker_category_view.h"

#include "ash/picker/mock_picker_asset_fetcher.h"
#include "ash/picker/views/picker_search_results_view_delegate.h"
#include "ash/picker/views/picker_skeleton_loader_view.h"
#include "base/test/task_environment.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/compositor/layer.h"
#include "ui/compositor/layer_animator.h"
#include "ui/compositor/scoped_animation_duration_scale_mode.h"
#include "ui/views/test/views_test_base.h"

namespace ash {
namespace {

using ::testing::SizeIs;

constexpr int kPickerWidth = 320;

class PickerCategoryViewTest : public views::ViewsTestBase {
 public:
  PickerCategoryViewTest()
      : views::ViewsTestBase(
            base::test::TaskEnvironment::TimeSource::MOCK_TIME) {}
};

class MockSearchResultsViewDelegate : public PickerSearchResultsViewDelegate {
 public:
  MOCK_METHOD(void, SelectMoreResults, (PickerSectionType), (override));
  MOCK_METHOD(void,
              SelectSearchResult,
              (const PickerSearchResult&),
              (override));
  MOCK_METHOD(void, NotifyPseudoFocusChanged, (views::View*), (override));
};

TEST_F(PickerCategoryViewTest, InitialStateIsEmptyResults) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerCategoryView view(&mock_delegate, kPickerWidth, &asset_fetcher);

  EXPECT_TRUE(view.search_results_view_for_testing().GetVisible());
  EXPECT_FALSE(view.skeleton_loader_view_for_testing().GetVisible());
}

TEST_F(PickerCategoryViewTest, ShowLoadingShowsLoaderView) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerCategoryView view(&mock_delegate, kPickerWidth, &asset_fetcher);

  view.ShowLoadingAnimation();

  EXPECT_FALSE(view.search_results_view_for_testing().GetVisible());
  EXPECT_TRUE(view.skeleton_loader_view_for_testing().GetVisible());
  EXPECT_FALSE(view.skeleton_loader_view_for_testing()
                   .layer()
                   ->GetAnimator()
                   ->is_animating());
}

TEST_F(PickerCategoryViewTest, ShowLoadingAnimatesAfterDelay) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerCategoryView view(&mock_delegate, kPickerWidth, &asset_fetcher);

  view.ShowLoadingAnimation();

  ui::ScopedAnimationDurationScaleMode test_duration_mode(
      ui::ScopedAnimationDurationScaleMode::NON_ZERO_DURATION);
  task_environment()->FastForwardBy(PickerCategoryView::kLoadingAnimationDelay);
  EXPECT_TRUE(view.skeleton_loader_view_for_testing()
                  .layer()
                  ->GetAnimator()
                  ->is_animating());
}

TEST_F(PickerCategoryViewTest, SetResultsShowsResults) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerCategoryView view(&mock_delegate, kPickerWidth, &asset_fetcher);

  view.SetResults({PickerSearchResultsSection(PickerSectionType::kLinks,
                                              {PickerSearchResult::Text(u"1")},
                                              /*has_more_results=*/false)});

  EXPECT_TRUE(view.search_results_view_for_testing().GetVisible());
  EXPECT_FALSE(view.skeleton_loader_view_for_testing().GetVisible());
  EXPECT_THAT(
      view.search_results_view_for_testing().section_views_for_testing(),
      SizeIs(1));
}

TEST_F(PickerCategoryViewTest, SetResultsDuringLoadingStopsAnimation) {
  ui::ScopedAnimationDurationScaleMode test_duration_mode(
      ui::ScopedAnimationDurationScaleMode::NON_ZERO_DURATION);
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerCategoryView view(&mock_delegate, kPickerWidth, &asset_fetcher);
  task_environment()->FastForwardBy(PickerCategoryView::kLoadingAnimationDelay);

  view.SetResults({PickerSearchResultsSection(PickerSectionType::kLinks,
                                              {PickerSearchResult::Text(u"1")},
                                              /*has_more_results=*/false)});

  EXPECT_FALSE(view.skeleton_loader_view_for_testing().GetVisible());
  EXPECT_FALSE(view.skeleton_loader_view_for_testing()
                   .layer()
                   ->GetAnimator()
                   ->is_animating());
}

TEST_F(PickerCategoryViewTest, SetResultsDuringLoadingSetsResults) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerCategoryView view(&mock_delegate, kPickerWidth, &asset_fetcher);
  view.ShowLoadingAnimation();

  view.SetResults({PickerSearchResultsSection(PickerSectionType::kLinks,
                                              {PickerSearchResult::Text(u"1")},
                                              /*has_more_results=*/false)});

  EXPECT_TRUE(view.search_results_view_for_testing().GetVisible());
  EXPECT_FALSE(view.skeleton_loader_view_for_testing().GetVisible());
  EXPECT_THAT(
      view.search_results_view_for_testing().section_views_for_testing(),
      SizeIs(1));
}

TEST_F(PickerCategoryViewTest, ShowsNoResultsFoundWhenResultsAreEmpty) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerCategoryView view(&mock_delegate, kPickerWidth, &asset_fetcher);

  view.SetResults({});

  EXPECT_TRUE(view.search_results_view_for_testing()
                  .no_results_view_for_testing()
                  ->GetVisible());
}

}  // namespace
}  // namespace ash
