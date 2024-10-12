// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_PICKER_VIEWS_PICKER_CATEGORY_VIEW_H_
#define ASH_PICKER_VIEWS_PICKER_CATEGORY_VIEW_H_

#include "ash/ash_export.h"
#include "ash/picker/views/picker_page_view.h"
#include "ash/picker/views/picker_search_results_view.h"
#include "ui/base/metadata/metadata_header_macros.h"

namespace ash {

class PickerAssetFetcher;
class PickerSearchResultsSection;
class PickerSearchResultsViewDelegate;
class PickerSkeletonLoaderView;

// View to show Picker results for a specific category.
class ASH_EXPORT PickerCategoryView : public PickerPageView {
  METADATA_HEADER(PickerCategoryView, PickerPageView)

 public:
  explicit PickerCategoryView(PickerSearchResultsViewDelegate* delegate,
                              int picker_view_width,
                              PickerAssetFetcher* asset_fetcher);
  PickerCategoryView(const PickerCategoryView&) = delete;
  PickerCategoryView& operator=(const PickerCategoryView&) = delete;
  ~PickerCategoryView() override;

  // The skeleton loader should not be used for short loading times.
  // Wait for a delay before showing the animation.
  static constexpr base::TimeDelta kLoadingAnimationDelay =
      base::Milliseconds(400);

  // PickerPageView:
  bool DoPseudoFocusedAction() override;
  bool MovePseudoFocusUp() override;
  bool MovePseudoFocusDown() override;
  bool MovePseudoFocusLeft() override;
  bool MovePseudoFocusRight() override;
  void AdvancePseudoFocus(PseudoFocusDirection direction) override;

  // Show a loading animation while results are not ready yet.
  void ShowLoadingAnimation();

  // Replaces the current results with `sections`.
  void SetResults(std::vector<PickerSearchResultsSection> sections);

  PickerSearchResultsView& search_results_view_for_testing() {
    return *search_results_view_;
  }
  PickerSkeletonLoaderView& skeleton_loader_view_for_testing() {
    return *skeleton_loader_view_;
  }

 private:
  // Default view for displaying category results.
  // TODO: b/316936620 - Replace this with specific category pages.
  raw_ptr<PickerSearchResultsView> search_results_view_ = nullptr;

  // The skeleton loader view, shown when the results are pending.
  // TODO: b/333729037 - Consider moving this to PickerSearchResultsView.
  raw_ptr<PickerSkeletonLoaderView> skeleton_loader_view_ = nullptr;
};

}  // namespace ash

#endif  // ASH_PICKER_VIEWS_PICKER_CATEGORY_VIEW_H_
