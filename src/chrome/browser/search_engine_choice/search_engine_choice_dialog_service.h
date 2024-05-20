// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SEARCH_ENGINE_CHOICE_SEARCH_ENGINE_CHOICE_DIALOG_SERVICE_H_
#define CHROME_BROWSER_SEARCH_ENGINE_CHOICE_SEARCH_ENGINE_CHOICE_DIALOG_SERVICE_H_

#include <string>

#include "base/containers/flat_map.h"
#include "base/functional/callback_forward.h"
#include "base/memory/raw_ref.h"
#include "base/memory/weak_ptr.h"
#include "base/scoped_observation.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/browser_list_observer.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/search_engines/search_engine_choice/search_engine_choice_service.h"
#include "components/search_engines/template_url_data.h"
#include "components/search_engines/template_url_service.h"

class Browser;

namespace search_engines {

class ChoiceScreenData;
enum class SearchEngineChoiceScreenConditions;

// Profile specific data related to the search engine choice.
// `timestamp` is the search engine choice timestamp that's saved in the
// `kDefaultSearchProviderChoiceScreenCompletionTimestamp` pref.
// `chrome_version` is the Chrome version when the user made the choice.
// `default_search_engine` is the profile's default search engine.
struct ChoiceData {
  int64_t timestamp = 0;
  std::string chrome_version;
  TemplateURLData default_search_engine;
};

}  // namespace search_engines

// Service handling the Search Engine Choice dialog.
class SearchEngineChoiceDialogService : public KeyedService {
 public:
  // Specifies the view in which the choice screen UI is rendered.
  enum class EntryPoint {
    kProfileCreation = 0,
    kFirstRunExperience,
    kDialog,
  };

  SearchEngineChoiceDialogService(
      Profile& profile,
      search_engines::SearchEngineChoiceService& search_engine_choice_service,
      TemplateURLService& template_url_service);
  ~SearchEngineChoiceDialogService() override;

  // Informs the service that a Search Engine Choice dialog has been opened
  // for `browser`.
  // Virtual to be able to mock in tests.
  virtual void NotifyDialogOpened(Browser* browser,
                                  base::OnceClosure close_dialog_callback);

  // This function is called when the user makes a search engine choice. It
  // closes the dialogs that are open on other browser windows that
  // have the same profile as the one on which the choice was made and sets the
  // corresponding preferences.
  // `prepopulate_id` is the `prepopulate_id` of the search engine found in
  // `components/search_engines/template_url_data.h`. It will always be > 0.
  // `entry_point` is the view in which the UI is rendered.
  // Virtual to be able to mock in tests.
  virtual void NotifyChoiceMade(int prepopulate_id, EntryPoint entry_point);

  // Informs the service that a Search Engine Choice dialog has been closed for
  // `browser`.
  void NotifyDialogClosed(Browser* browser);

  // Informs the service that the learn more link was clicked. This is used to
  // record histograms. `entry_point` is the view in which the UI is rendered.
  void NotifyLearnMoreLinkClicked(EntryPoint entry_point);

  // Returns whether a Search Engine Choice dialog is currently open or not for
  // `browser`.
  bool IsShowingDialog(Browser* browser);

  // Returns whether the Search Engine Choice dialog can be shown or not.
  // This will return false if the dialog is currently showing.
  bool CanShowDialog(Browser& browser);

  // Returns whether the dialog should be displayed over the passed URL.
  bool IsUrlSuitableForDialog(GURL url);

  // Returns whether the Search Engine Choice dialog is either shown or
  // pending to be shown.
  bool HasPendingDialog(Browser& browser);

  // Checks whether we need to display the Privacy Sandbox dialog
  // in context of the Search Engine Choice.
  // The Privacy Sandbox dialog should be delayed to the next Chrome run if the
  // Search Engine Choice dialog will be or has been displayed in the current
  // run.
  // The Privacy Sandbox dialog should be displayed directly when the
  // browser gets launched in the case where the search engine choice was made
  // in the FRE.
  bool CanSuppressPrivacySandboxPromo() const;

  // Returns the list of search engines.
  // The search engine details returned by this function will be the canonical
  // ones and will not be affected by changes in search engine details from the
  // settings page.
  // Virtual to be able to mock in tests.
  virtual TemplateURL::TemplateURLVector GetSearchEngines();

  // Disables the display of the Search Engine Choice dialog for testing. When
  // `dialog_disabled` is true, `CanShowDialog` will return false.
  // NOTE: This is set to true in InProcessBrowserTest::SetUp, disabling the
  // dialog for those tests. If you set this outside of that context, you should
  // ensure it is reset at the end of your test.
  static void SetDialogDisabledForTests(bool dialog_disabled);

  // Returns a copy of the `ChoiceData` specific to `profile`.
  static search_engines::ChoiceData GetChoiceDataFromProfile(Profile& profile);

  // Updates `profile` with the values from `choice_data`.
  static void UpdateProfileFromChoiceData(
      Profile& profile,
      const search_engines::ChoiceData& choice_data);

 private:
  // Observes the BrowserList to make sure that closed browsers are correctly
  // removed from our set of browser pointers. This ensures that we don't get
  // dangling pointers.
  class BrowserObserver : public BrowserListObserver {
   public:
    explicit BrowserObserver(SearchEngineChoiceDialogService& service);
    ~BrowserObserver() override;

    void OnBrowserRemoved(Browser* browser) override;

   private:
    raw_ref<SearchEngineChoiceDialogService>
        search_engine_choice_dialog_service_;
    base::ScopedObservation<BrowserList, BrowserListObserver> observation_{
        this};
  };
  friend class SearchEngineChoiceDialogServiceFactory;

  search_engines::SearchEngineChoiceScreenConditions ComputeDialogConditions(
      Browser& browser);

  // A map of Browser windows which have an open Search Engine Choice dialog to
  // the callback that will close the browser's dialog.
  base::flat_map<Browser*, base::OnceClosure> browsers_with_open_dialogs_;

  // Observes the browser list for closed browsers.
  BrowserObserver browser_observer_{*this};

  // To know whether the choice was made during the Profile Picker or not.
  bool choice_made_in_profile_picker_ = false;

  // Caches data backing the choice screens, shared across all of the choice
  // screens dialogs shown by this profile.
  // It gets lazily populated the first time something tries to get it.
  std::unique_ptr<search_engines::ChoiceScreenData> choice_screen_data_;

  // The `KeyedService` lifetime is expected to exceed the profile's.
  const raw_ref<Profile> profile_;
  const raw_ref<search_engines::SearchEngineChoiceService>
      search_engine_choice_service_;
  const raw_ref<TemplateURLService> template_url_service_;
  base::WeakPtrFactory<SearchEngineChoiceDialogService> weak_ptr_factory_{this};
};

#endif  // CHROME_BROWSER_SEARCH_ENGINE_CHOICE_SEARCH_ENGINE_CHOICE_DIALOG_SERVICE_H_
