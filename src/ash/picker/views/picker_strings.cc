// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/picker/views/picker_strings.h"

#include <string>

#include "ash/picker/model/picker_search_results_section.h"
#include "ash/picker/views/picker_category_type.h"
#include "ash/public/cpp/picker/picker_category.h"
#include "ash/strings/grit/ash_strings.h"
#include "ui/base/l10n/l10n_util.h"

namespace ash {

std::u16string GetLabelForPickerCategory(PickerCategory category) {
  switch (category) {
    case PickerCategory::kEmojis:
      return l10n_util::GetStringUTF16(IDS_PICKER_EMOJIS_CATEGORY_LABEL);
    case PickerCategory::kSymbols:
      return l10n_util::GetStringUTF16(IDS_PICKER_SYMBOLS_CATEGORY_LABEL);
    case PickerCategory::kEmoticons:
      return l10n_util::GetStringUTF16(IDS_PICKER_EMOTICONS_CATEGORY_LABEL);
    case PickerCategory::kGifs:
      return l10n_util::GetStringUTF16(IDS_PICKER_GIFS_CATEGORY_LABEL);
    case PickerCategory::kOpenTabs:
      return l10n_util::GetStringUTF16(IDS_PICKER_OPEN_TABS_CATEGORY_LABEL);
    case PickerCategory::kBrowsingHistory:
      return l10n_util::GetStringUTF16(
          IDS_PICKER_BROWSING_HISTORY_CATEGORY_LABEL);
    case PickerCategory::kBookmarks:
      return l10n_util::GetStringUTF16(IDS_PICKER_BOOKMARKS_CATEGORY_LABEL);
    case PickerCategory::kDriveFiles:
      return l10n_util::GetStringUTF16(IDS_PICKER_DRIVE_FILES_CATEGORY_LABEL);
    case PickerCategory::kLocalFiles:
      return l10n_util::GetStringUTF16(IDS_PICKER_LOCAL_FILES_CATEGORY_LABEL);
  }
}

std::u16string GetSearchFieldPlaceholderTextForPickerCategory(
    PickerCategory category) {
  switch (category) {
    case PickerCategory::kEmojis:
      return l10n_util::GetStringUTF16(
          IDS_PICKER_EMOJIS_CATEGORY_SEARCH_FIELD_PLACEHOLDER_TEXT);
    case PickerCategory::kSymbols:
      return l10n_util::GetStringUTF16(
          IDS_PICKER_SYMBOLS_CATEGORY_SEARCH_FIELD_PLACEHOLDER_TEXT);
    case PickerCategory::kEmoticons:
      return l10n_util::GetStringUTF16(
          IDS_PICKER_EMOTICONS_CATEGORY_SEARCH_FIELD_PLACEHOLDER_TEXT);
    case PickerCategory::kGifs:
      return l10n_util::GetStringUTF16(
          IDS_PICKER_GIFS_CATEGORY_SEARCH_FIELD_PLACEHOLDER_TEXT);
    case PickerCategory::kOpenTabs:
      return l10n_util::GetStringUTF16(
          IDS_PICKER_OPEN_TABS_CATEGORY_SEARCH_FIELD_PLACEHOLDER_TEXT);
    case PickerCategory::kBrowsingHistory:
      return l10n_util::GetStringUTF16(
          IDS_PICKER_BROWSING_HISTORY_CATEGORY_SEARCH_FIELD_PLACEHOLDER_TEXT);
    case PickerCategory::kBookmarks:
      return l10n_util::GetStringUTF16(
          IDS_PICKER_BOOKMARKS_CATEGORY_SEARCH_FIELD_PLACEHOLDER_TEXT);
    case PickerCategory::kDriveFiles:
      return l10n_util::GetStringUTF16(
          IDS_PICKER_DRIVE_FILES_CATEGORY_SEARCH_FIELD_PLACEHOLDER_TEXT);
    case PickerCategory::kLocalFiles:
      return l10n_util::GetStringUTF16(
          IDS_PICKER_LOCAL_FILES_CATEGORY_SEARCH_FIELD_PLACEHOLDER_TEXT);
  }
}

std::u16string GetSectionTitleForPickerCategoryType(
    PickerCategoryType category_type) {
  switch (category_type) {
    case PickerCategoryType::kExpressions:
      return l10n_util::GetStringUTF16(
          IDS_PICKER_EXPRESSIONS_CATEGORY_TYPE_SECTION_TITLE);
    case PickerCategoryType::kLinks:
      return l10n_util::GetStringUTF16(
          IDS_PICKER_LINKS_CATEGORY_TYPE_SECTION_TITLE);
    case PickerCategoryType::kFiles:
      return l10n_util::GetStringUTF16(
          IDS_PICKER_FILES_CATEGORY_TYPE_SECTION_TITLE);
  }
}

std::u16string GetSectionTitleForPickerSectionType(
    PickerSectionType section_type) {
  // TODO: b/325870358 - Finalize strings and use a GRD file.
  switch (section_type) {
    case PickerSectionType::kCategories:
      return u"Matching categories";
    case PickerSectionType::kSuggestions:
      return u"Suggested";
    case PickerSectionType::kExpressions:
      return u"Matching expressions";
    case PickerSectionType::kLinks:
      return u"Matching links";
    case PickerSectionType::kFiles:
      return u"Matching files";
    case PickerSectionType::kDriveFiles:
      return u"Matching Google Drive files";
    case PickerSectionType::kGifs:
      return u"Other expressions";
    case PickerSectionType::kRecentlyUsed:
      return u"Recently used";
  }
}

}  // namespace ash
