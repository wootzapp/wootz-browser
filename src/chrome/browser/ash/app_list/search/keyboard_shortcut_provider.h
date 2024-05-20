// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_APP_LIST_SEARCH_KEYBOARD_SHORTCUT_PROVIDER_H_
#define CHROME_BROWSER_ASH_APP_LIST_SEARCH_KEYBOARD_SHORTCUT_PROVIDER_H_

#include "ash/webui/shortcut_customization_ui/backend/search/search_handler.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/sequence_checker.h"
#include "chrome/browser/ash/app_list/search/search_provider.h"

class Profile;

namespace app_list {

class KeyboardShortcutProvider : public SearchProvider {
 public:
  explicit KeyboardShortcutProvider(Profile* profile);
  ~KeyboardShortcutProvider() override;

  KeyboardShortcutProvider(const KeyboardShortcutProvider&) = delete;
  KeyboardShortcutProvider& operator=(const KeyboardShortcutProvider&) = delete;

  // SearchProvider:
  void Start(const std::u16string& query) override;
  void StopQuery() override;
  ash::AppListSearchResultType ResultType() const override;
  // Allow a test class to inject a fake or mock search handler.
  void SetSearchHandlerForTesting(ash::shortcut_ui::SearchHandler* handler) {
    search_handler_ = handler;
  }

  // Overrides `should_apply_query_filtering_` for testing purposes.
  void set_should_apply_query_filtering_for_test(
      bool should_apply_query_filtering) {
    should_apply_query_filtering_ = should_apply_query_filtering;
  }

 private:
  void OnShortcutsSearchComplete(
      std::vector<ash::shortcut_customization::mojom::SearchResultPtr>);

  const raw_ptr<Profile> profile_;

  // A check for whether we should apply query filtering for keyboard shortcut
  // results.
  bool should_apply_query_filtering_ = false;

  // The |search_handler_| is managed by ShortcutsAppManager which is
  // implemented as a KeyedService, active for the lifetime of a logged-in user.
  raw_ptr<ash::shortcut_ui::SearchHandler> search_handler_;

  SEQUENCE_CHECKER(sequence_checker_);
  base::WeakPtrFactory<KeyboardShortcutProvider> weak_factory_{this};
};

}  // namespace app_list

#endif  // CHROME_BROWSER_ASH_APP_LIST_SEARCH_KEYBOARD_SHORTCUT_PROVIDER_H_
