// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/picker/views/picker_search_results_view.h"

#include <string>

#include "ash/picker/mock_picker_asset_fetcher.h"
#include "ash/picker/model/picker_search_results_section.h"
#include "ash/picker/picker_test_util.h"
#include "ash/picker/views/picker_item_view.h"
#include "ash/picker/views/picker_list_item_view.h"
#include "ash/picker/views/picker_search_results_view_delegate.h"
#include "ash/picker/views/picker_section_list_view.h"
#include "ash/picker/views/picker_section_view.h"
#include "ash/picker/views/picker_strings.h"
#include "ash/public/cpp/picker/picker_search_result.h"
#include "ash/style/ash_color_provider.h"
#include "ash/test/view_drawn_waiter.h"
#include "base/files/file_path.h"
#include "base/functional/callback_helpers.h"
#include "base/test/test_future.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/gfx/geometry/size.h"
#include "ui/views/controls/label.h"
#include "ui/views/test/views_test_base.h"
#include "ui/views/view_utils.h"
#include "ui/views/widget/widget.h"
#include "url/gurl.h"

namespace ash {
namespace {

using ::testing::_;
using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::IsNull;
using ::testing::Not;
using ::testing::NotNull;
using ::testing::Pointee;
using ::testing::Property;
using ::testing::ResultOf;
using ::testing::SizeIs;

constexpr int kPickerWidth = 320;

using PickerSearchResultsViewTest = views::ViewsTestBase;

template <class V, class Matcher>
auto AsView(Matcher matcher) {
  return ResultOf(
      "AsViewClass",
      [](views::View* view) { return views::AsViewClass<V>(view); },
      Pointee(matcher));
}

auto MatchesResultSection(PickerSectionType section_type, int num_items) {
  return AllOf(
      Property(&PickerSectionView::title_label_for_testing,
               Property(&views::Label::GetText,
                        GetSectionTitleForPickerSectionType(section_type))),
      Property(&PickerSectionView::item_views_for_testing, SizeIs(num_items)));
}

template <class Matcher>
auto MatchesResultSectionWithOneItem(PickerSectionType section_type,
                                     Matcher item_matcher) {
  return AllOf(
      Property(&PickerSectionView::title_label_for_testing,
               Property(&views::Label::GetText,
                        GetSectionTitleForPickerSectionType(section_type))),
      Property(&PickerSectionView::item_views_for_testing,
               ElementsAre(item_matcher)));
}

class MockSearchResultsViewDelegate : public PickerSearchResultsViewDelegate {
 public:
  MOCK_METHOD(void, SelectMoreResults, (PickerSectionType), (override));
  MOCK_METHOD(void,
              SelectSearchResult,
              (const PickerSearchResult&),
              (override));
  MOCK_METHOD(void, NotifyPseudoFocusChanged, (views::View*), (override));
};

TEST_F(PickerSearchResultsViewTest, CreatesResultsSections) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerSearchResultsView view(&mock_delegate, kPickerWidth, &asset_fetcher);

  view.AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kExpressions,
      {{PickerSearchResult::Text(u"Result A")}}, /*has_more_results=*/false));
  view.AppendSearchResults(
      PickerSearchResultsSection(PickerSectionType::kLinks,
                                 {{PickerSearchResult::Text(u"Result B"),
                                   PickerSearchResult::Text(u"Result C")}},
                                 /*has_more_results=*/false));

  EXPECT_THAT(view.section_list_view_for_testing()->children(), SizeIs(2));
  EXPECT_THAT(
      view.section_views_for_testing(),
      ElementsAre(
          Pointee(MatchesResultSection(PickerSectionType::kExpressions, 1)),
          Pointee(MatchesResultSection(PickerSectionType::kLinks, 2))));
}

TEST_F(PickerSearchResultsViewTest, ClearSearchResultsClearsView) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerSearchResultsView view(&mock_delegate, kPickerWidth, &asset_fetcher);
  view.AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kExpressions, {{PickerSearchResult::Text(u"Result")}},
      /*has_more_results=*/false));

  view.ClearSearchResults();

  EXPECT_THAT(view.section_list_view_for_testing()->children(), IsEmpty());
}

TEST_F(PickerSearchResultsViewTest, ClearSearchResultsClearsPseudoFocus) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerSearchResultsView view(&mock_delegate, kPickerWidth, &asset_fetcher);
  view.AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kExpressions, {{PickerSearchResult::Text(u"Result")}},
      /*has_more_results=*/false));

  EXPECT_CALL(mock_delegate, NotifyPseudoFocusChanged(nullptr));

  view.ClearSearchResults();
}

TEST_F(PickerSearchResultsViewTest, CreatesResultsSectionWithGif) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerSearchResultsView view(&mock_delegate, kPickerWidth, &asset_fetcher);

  view.AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kGifs,
      {{PickerSearchResult::Gif(
          /*preview_url=*/GURL(), /*preview_image_url=*/GURL(), gfx::Size(),
          /*full_url=*/GURL(), gfx::Size(),
          /*content_description=*/u"")}},
      /*has_more_results=*/false));

  EXPECT_THAT(view.section_list_view_for_testing()->children(), SizeIs(1));
  EXPECT_THAT(
      view.section_views_for_testing(),
      ElementsAre(Pointee(MatchesResultSection(PickerSectionType::kGifs, 1))));
}

TEST_F(PickerSearchResultsViewTest, CreatesResultsSectionWithCategories) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerSearchResultsView view(&mock_delegate, kPickerWidth, &asset_fetcher);

  view.AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kCategories,
      {{PickerSearchResult::Category(PickerCategory::kExpressions)}},
      /*has_more_results=*/false));

  EXPECT_THAT(view.section_list_view_for_testing()->children(), SizeIs(1));
  EXPECT_THAT(view.section_views_for_testing(),
              ElementsAre(Pointee(
                  MatchesResultSection(PickerSectionType::kCategories, 1))));
}

TEST_F(PickerSearchResultsViewTest, CreatesResultsSectionWithLocalFiles) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerSearchResultsView view(&mock_delegate, kPickerWidth, &asset_fetcher);

  view.AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kFiles,
      {{PickerSearchResult::LocalFile(u"local", base::FilePath())}},
      /*has_more_results=*/false));

  EXPECT_THAT(view.section_list_view_for_testing()->children(), SizeIs(1));
  EXPECT_THAT(
      view.section_views_for_testing(),
      ElementsAre(Pointee(MatchesResultSectionWithOneItem(
          PickerSectionType::kFiles,
          AsView<PickerListItemView>(Property(
              &PickerListItemView::GetPrimaryTextForTesting, u"local"))))));
}

TEST_F(PickerSearchResultsViewTest, CreatesResultsSectionWithDriveFiles) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerSearchResultsView view(&mock_delegate, kPickerWidth, &asset_fetcher);

  view.AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kFiles,
      {{PickerSearchResult::DriveFile(u"drive", GURL(), /*icon=*/{})}},
      /*has_more_results=*/false));

  EXPECT_THAT(view.section_list_view_for_testing()->children(), SizeIs(1));
  EXPECT_THAT(
      view.section_views_for_testing(),
      ElementsAre(Pointee(MatchesResultSectionWithOneItem(
          PickerSectionType::kFiles,
          AsView<PickerListItemView>(Property(
              &PickerListItemView::GetPrimaryTextForTesting, u"drive"))))));
}

TEST_F(PickerSearchResultsViewTest, UpdatesResultsSections) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerSearchResultsView view(&mock_delegate, kPickerWidth, &asset_fetcher);

  view.AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kExpressions, {{PickerSearchResult::Text(u"Result")}},
      /*has_more_results=*/false));
  view.AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kLinks,
      {{PickerSearchResult::Text(u"Updated Result")}},
      /*has_more_results=*/false));

  EXPECT_THAT(view.section_list_view_for_testing()->children(), SizeIs(2));
  EXPECT_THAT(
      view.section_views_for_testing(),
      ElementsAre(
          Pointee(MatchesResultSection(PickerSectionType::kExpressions, 1)),
          Pointee(MatchesResultSection(PickerSectionType::kLinks, 1))));
}

TEST_F(PickerSearchResultsViewTest,
       NoPseudoFocusedActionForEmptySearchResults) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerSearchResultsView view(&mock_delegate, kPickerWidth, &asset_fetcher);

  EXPECT_FALSE(view.DoPseudoFocusedAction());
}

TEST_F(PickerSearchResultsViewTest, PseudoFocusedActionDefaultsToFirstResult) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerSearchResultsView view(&mock_delegate, kPickerWidth, &asset_fetcher);

  EXPECT_CALL(mock_delegate,
              SelectSearchResult(PickerSearchResult::Emoji(u"😊")));
  EXPECT_CALL(mock_delegate, NotifyPseudoFocusChanged(_));

  view.AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kExpressions,
      {{PickerSearchResult::Emoji(u"😊"), PickerSearchResult::Symbol(u"♬")}},
      /*has_more_results=*/false));

  EXPECT_TRUE(view.DoPseudoFocusedAction());
}

TEST_F(PickerSearchResultsViewTest, MovesPseudoFocusRight) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerSearchResultsView view(&mock_delegate, kPickerWidth, &asset_fetcher);
  view.AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kExpressions,
      {{PickerSearchResult::Emoji(u"😊"), PickerSearchResult::Symbol(u"♬")}},
      /*has_more_results=*/false));

  EXPECT_CALL(mock_delegate,
              SelectSearchResult(PickerSearchResult::Symbol(u"♬")));
  EXPECT_CALL(mock_delegate, NotifyPseudoFocusChanged(_));

  EXPECT_TRUE(view.MovePseudoFocusRight());
  EXPECT_TRUE(view.DoPseudoFocusedAction());
}

TEST_F(PickerSearchResultsViewTest, MovesPseudoFocusDown) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerSearchResultsView view(&mock_delegate, kPickerWidth, &asset_fetcher);
  view.AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kCategories,
      {{PickerSearchResult::Category(PickerCategory::kExpressions),
        PickerSearchResult::Category(PickerCategory::kClipboard)}},
      /*has_more_results=*/false));

  EXPECT_CALL(mock_delegate, SelectSearchResult(PickerSearchResult::Category(
                                 PickerCategory::kClipboard)));
  EXPECT_CALL(mock_delegate, NotifyPseudoFocusChanged(_));

  EXPECT_TRUE(view.MovePseudoFocusDown());
  EXPECT_TRUE(view.DoPseudoFocusedAction());
}

TEST_F(PickerSearchResultsViewTest, AdvancesPseudoFocusForward) {
  std::unique_ptr<views::Widget> widget = CreateTestWidget();
  widget->SetFullscreen(true);
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  auto* view =
      widget->SetContentsView(std::make_unique<PickerSearchResultsView>(
          &mock_delegate, kPickerWidth, &asset_fetcher));
  view->AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kCategories,
      {{PickerSearchResult::Category(PickerCategory::kExpressions),
        PickerSearchResult::Category(PickerCategory::kClipboard),
        PickerSearchResult::Category(PickerCategory::kDriveFiles)}},
      /*has_more_results=*/false));
  ViewDrawnWaiter().Wait(view->section_list_view_for_testing()->GetTopItem());

  EXPECT_CALL(mock_delegate, SelectSearchResult(PickerSearchResult::Category(
                                 PickerCategory::kClipboard)));
  EXPECT_CALL(mock_delegate, NotifyPseudoFocusChanged(_));

  view->AdvancePseudoFocus(
      PickerPseudoFocusHandler::PseudoFocusDirection::kForward);
  ASSERT_TRUE(view->DoPseudoFocusedAction());
}

TEST_F(PickerSearchResultsViewTest, AdvancesPseudoFocusBackward) {
  std::unique_ptr<views::Widget> widget = CreateTestWidget();
  widget->SetFullscreen(true);
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  auto* view =
      widget->SetContentsView(std::make_unique<PickerSearchResultsView>(
          &mock_delegate, kPickerWidth, &asset_fetcher));
  view->AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kCategories,
      {{PickerSearchResult::Category(PickerCategory::kExpressions),
        PickerSearchResult::Category(PickerCategory::kClipboard),
        PickerSearchResult::Category(PickerCategory::kDriveFiles)}},
      /*has_more_results=*/false));
  ViewDrawnWaiter().Wait(view->section_list_view_for_testing()->GetTopItem());

  EXPECT_CALL(mock_delegate, SelectSearchResult(PickerSearchResult::Category(
                                 PickerCategory::kExpressions)));
  EXPECT_CALL(mock_delegate, NotifyPseudoFocusChanged(_)).Times(2);

  view->AdvancePseudoFocus(
      PickerPseudoFocusHandler::PseudoFocusDirection::kForward);
  view->AdvancePseudoFocus(
      PickerPseudoFocusHandler::PseudoFocusDirection::kBackward);
  ASSERT_TRUE(view->DoPseudoFocusedAction());
}

TEST_F(PickerSearchResultsViewTest, ShowsSeeMoreLinkWhenThereAreMoreResults) {
  std::unique_ptr<views::Widget> widget = CreateTestWidget();
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  auto* view =
      widget->SetContentsView(std::make_unique<PickerSearchResultsView>(
          &mock_delegate, kPickerWidth, &asset_fetcher));

  view->AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kGifs, {}, /*has_more_results=*/true));

  ASSERT_THAT(
      view->section_views_for_testing(),
      ElementsAre(Pointee(Property(
          "title_trailing_link_for_testing",
          &PickerSectionView::title_trailing_link_for_testing, NotNull()))));
}

TEST_F(PickerSearchResultsViewTest,
       DoesNotShowSeeMoreLinkWhenThereAreNoMoreResults) {
  std::unique_ptr<views::Widget> widget = CreateTestWidget();
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  auto* view =
      widget->SetContentsView(std::make_unique<PickerSearchResultsView>(
          &mock_delegate, kPickerWidth, &asset_fetcher));

  view->AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kGifs, {}, /*has_more_results=*/false));

  ASSERT_THAT(
      view->section_views_for_testing(),
      ElementsAre(Pointee(Property(
          "title_trailing_link_for_testing",
          &PickerSectionView::title_trailing_link_for_testing, IsNull()))));
}

TEST_F(PickerSearchResultsViewTest, ClickingSeeMoreLinkCallsCallback) {
  std::unique_ptr<views::Widget> widget = CreateTestWidget();
  widget->SetFullscreen(true);
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  auto* view =
      widget->SetContentsView(std::make_unique<PickerSearchResultsView>(
          &mock_delegate, kPickerWidth, &asset_fetcher));
  widget->Show();
  view->AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kGifs, {}, /*has_more_results=*/true));

  EXPECT_CALL(mock_delegate, SelectMoreResults(PickerSectionType::kGifs));

  views::View* trailing_link =
      view->section_views_for_testing()[0]->title_trailing_link_for_testing();
  ViewDrawnWaiter().Wait(trailing_link);
  LeftClickOn(*trailing_link);
}

TEST_F(PickerSearchResultsViewTest, ShowNoResultsFoundShowsView) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerSearchResultsView view(&mock_delegate, kPickerWidth, &asset_fetcher);

  view.ShowNoResultsFound();

  EXPECT_FALSE(view.section_list_view_for_testing()->GetVisible());
  EXPECT_TRUE(view.no_results_view_for_testing()->GetVisible());
}

TEST_F(PickerSearchResultsViewTest, ClearSearchResultsShowsSearchResults) {
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  PickerSearchResultsView view(&mock_delegate, kPickerWidth, &asset_fetcher);
  view.ShowNoResultsFound();

  view.ClearSearchResults();

  EXPECT_TRUE(view.section_list_view_for_testing()->GetVisible());
  EXPECT_FALSE(view.no_results_view_for_testing()->GetVisible());
}

struct PickerSearchResultTestCase {
  std::string test_name;
  PickerSearchResult result;
};

class PickerSearchResultsViewResultSelectionTest
    : public PickerSearchResultsViewTest,
      public testing::WithParamInterface<PickerSearchResultTestCase> {
 private:
  AshColorProvider ash_color_provider_;
};

TEST_P(PickerSearchResultsViewResultSelectionTest, LeftClickSelectsResult) {
  const PickerSearchResultTestCase& test_case = GetParam();
  std::unique_ptr<views::Widget> widget = CreateTestWidget();
  widget->SetFullscreen(true);
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  auto* view =
      widget->SetContentsView(std::make_unique<PickerSearchResultsView>(
          &mock_delegate, kPickerWidth, &asset_fetcher));
  widget->Show();
  view->AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kExpressions, {{test_case.result}},
      /*has_more_results=*/false));
  ASSERT_THAT(view->section_views_for_testing(), Not(IsEmpty()));
  ASSERT_THAT(view->section_views_for_testing()[0]->item_views_for_testing(),
              Not(IsEmpty()));

  EXPECT_CALL(mock_delegate, SelectSearchResult(test_case.result));

  PickerItemView* result_view =
      view->section_views_for_testing()[0]->item_views_for_testing()[0];
  ViewDrawnWaiter().Wait(result_view);
  LeftClickOn(*result_view);
}

TEST_P(PickerSearchResultsViewResultSelectionTest,
       PseudoFocusedActionSelectsResult) {
  const PickerSearchResultTestCase& test_case = GetParam();
  std::unique_ptr<views::Widget> widget = CreateTestWidget();
  widget->SetFullscreen(true);
  MockSearchResultsViewDelegate mock_delegate;
  MockPickerAssetFetcher asset_fetcher;
  auto* view =
      widget->SetContentsView(std::make_unique<PickerSearchResultsView>(
          &mock_delegate, kPickerWidth, &asset_fetcher));
  view->AppendSearchResults(PickerSearchResultsSection(
      PickerSectionType::kExpressions, {{test_case.result}},
      /*has_more_results=*/false));

  EXPECT_CALL(mock_delegate, SelectSearchResult(test_case.result));

  EXPECT_TRUE(view->DoPseudoFocusedAction());
}

INSTANTIATE_TEST_SUITE_P(
    All,
    PickerSearchResultsViewResultSelectionTest,
    testing::ValuesIn<PickerSearchResultTestCase>({
        {"Text", PickerSearchResult::Text(u"result")},
        {"Emoji", PickerSearchResult::Emoji(u"😊")},
        {"Symbol", PickerSearchResult::Symbol(u"♬")},
        {"Emoticon", PickerSearchResult::Emoticon(u"¯\\_(ツ)_/¯")},
        {"Gif", PickerSearchResult::Gif(/*preview_url=*/GURL(),
                                        /*preview_image_url=*/GURL(),
                                        gfx::Size(10, 10),
                                        /*full_url=*/GURL(),
                                        gfx::Size(20, 20),
                                        u"cat gif")},
        {"Category",
         PickerSearchResult::Category(PickerCategory::kExpressions)},
        {"LocalFile",
         PickerSearchResult::LocalFile(u"local", base::FilePath())},
        {"DriveFile",
         PickerSearchResult::DriveFile(u"drive", GURL(), /*icon=*/{})},
    }),
    [](const testing::TestParamInfo<
        PickerSearchResultsViewResultSelectionTest::ParamType>& info) {
      return info.param.test_name;
    });

}  // namespace
}  // namespace ash
