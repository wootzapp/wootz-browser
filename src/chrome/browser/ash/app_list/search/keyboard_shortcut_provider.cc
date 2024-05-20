// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/app_list/search/keyboard_shortcut_provider.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "ash/constants/ash_features.h"
#include "ash/public/cpp/app_list/app_list_types.h"
#include "ash/webui/shortcut_customization_ui/backend/search/search_handler.h"
#include "ash/webui/shortcut_customization_ui/shortcuts_app_manager.h"
#include "ash/webui/shortcut_customization_ui/shortcuts_app_manager_factory.h"
#include "base/check_op.h"
#include "base/feature_list.h"
#include "base/functional/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "chrome/browser/ash/app_list/search/keyboard_shortcut_result.h"
#include "chrome/browser/ash/app_list/search/search_controller.h"
#include "chrome/browser/ash/app_list/search/search_features.h"
#include "chrome/browser/ash/app_list/search/types.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile.h"
#include "chromeos/ash/components/string_matching/tokenized_string.h"
#include "content/public/browser/storage_partition.h"
#include "ui/base/l10n/l10n_util.h"

namespace app_list {

namespace {

using ::ash::string_matching::TokenizedString;

constexpr size_t kMinQueryLength = 3u;
constexpr size_t kMaxResults = 3u;
// The threshold is used to filter the results from the search handler of the
// new shortcuts app.
constexpr double kRelevanceScoreThreshold = 0.52;

// Remove disabled shortcuts and leave enabled ones only.
void RemoveDisabledShortcuts(
    ash::shortcut_customization::mojom::SearchResultPtr& search_result) {
  std::erase_if(search_result->accelerator_infos, [](const auto& x) {
    return x->state != ash::mojom::AcceleratorState::kEnabled;
  });
}

}  // namespace

KeyboardShortcutProvider::KeyboardShortcutProvider(Profile* profile)
    : SearchProvider(SearchCategory::kHelp), profile_(profile) {
  DCHECK(profile_);
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  auto* shortcuts_app_manager_factory =
      ash::shortcut_ui::ShortcutsAppManagerFactory::GetForBrowserContext(
          profile_);
  // The factory is null in unit tests.
  if (shortcuts_app_manager_factory) {
    search_handler_ = shortcuts_app_manager_factory->search_handler();
  }

  const std::string locale = g_browser_process->GetApplicationLocale();
  std::string language_code = l10n_util::GetLanguage(locale);
  // TODO(b/326514738): It's a temporary filtering, and the filtering only
  // applies to English.
  should_apply_query_filtering_ = (language_code == "en");
}

KeyboardShortcutProvider::~KeyboardShortcutProvider() = default;

void KeyboardShortcutProvider::Start(const std::u16string& query) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  // Cancel all previous searches.
  weak_factory_.InvalidateWeakPtrs();

  if (query.size() < kMinQueryLength) {
    return;
  }

  // TODO(b/326514738): Before the over-triggering issue is resolved, display
  // keyboard shortcut results only if the keyword "shortcut" is present in the
  // query. The filtering only applies to English.
  if (should_apply_query_filtering_ &&
      query.find(u"shortcut") == std::u16string::npos) {
    return;
  }

  if (ash::features::IsSearchCustomizableShortcutsInLauncherEnabled()) {
    if (!search_handler_) {
      return;
    }
    search_handler_->Search(
        query, UINT32_MAX,
        base::BindOnce(&KeyboardShortcutProvider::OnShortcutsSearchComplete,
                       weak_factory_.GetWeakPtr()));
  }
}

void KeyboardShortcutProvider::StopQuery() {
  // Cancel all previous searches.
  weak_factory_.InvalidateWeakPtrs();
}

ash::AppListSearchResultType KeyboardShortcutProvider::ResultType() const {
  return ash::AppListSearchResultType::kKeyboardShortcut;
}

void KeyboardShortcutProvider::OnShortcutsSearchComplete(
    std::vector<ash::shortcut_customization::mojom::SearchResultPtr>
        search_results) {
  CHECK(ash::features::IsSearchCustomizableShortcutsInLauncherEnabled());

  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  // Convert final candidates into correct type, and publish.
  SearchProvider::Results results;
  for (auto& search_result : search_results) {
    // Only enabled shortcuts should be displayed.
    RemoveDisabledShortcuts(search_result);
    // The search results are sorted by relevance score in descending order
    // already.
    if (search_result->relevance_score < kRelevanceScoreThreshold) {
      break;
    }
    results.push_back(
        std::make_unique<KeyboardShortcutResult>(profile_, search_result));
    if (results.size() >= kMaxResults) {
      break;
    }
  }
  SwapResults(&results);
}

}  // namespace app_list
