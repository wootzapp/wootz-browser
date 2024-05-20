// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SEARCH_ENGINES_SEARCH_TERMS_DATA_H_
#define COMPONENTS_SEARCH_ENGINES_SEARCH_TERMS_DATA_H_

#include <memory>
#include <string>

#include "base/compiler_specific.h"

// All data needed by TemplateURLRef::ReplaceSearchTerms which typically may
// only be accessed on the UI thread.
class SearchTermsData {
 public:
  // Enumeration of the known search or suggest request sources. These values
  // are not persisted or used in histograms; thus can be freely changed.
  enum class RequestSource {
    SEARCHBOX,                    // Omnibox or the NTP realbox. The default.
    CROS_APP_LIST,                // Chrome OS app list searchbox.
    NTP_MODULE,                   // Suggestions for the NTP modules.
    CONTEXTUAL_SEARCHBOX,         // Contextual searchbox in the lens overlay.
    SEARCH_SIDE_PANEL_SEARCHBOX,  // Search side panel searchbox.
    LENS_SIDE_PANEL_SEARCHBOX,    // Lens side panel searchbox.
  };

  // Utility function that takes a snapshot of a different SearchTermsData
  // instance. This is used to access SearchTermsData off the UI thread, or to
  // copy the SearchTermsData for lifetime reasons.
  static std::unique_ptr<SearchTermsData> MakeSnapshot(
      const SearchTermsData* original_data);

  SearchTermsData();

  SearchTermsData(const SearchTermsData&) = delete;
  SearchTermsData& operator=(const SearchTermsData&) = delete;

  virtual ~SearchTermsData();

  // Returns the value to use for replacements of type GOOGLE_BASE_URL.  This
  // implementation simply returns the default value.
  virtual std::string GoogleBaseURLValue() const;

  // Returns the value to use for the GOOGLE_BASE_SEARCH_BY_IMAGE_URL. Points
  // at Lens if the user is enrolled in the Lens experiment, and defaults to
  // Image Search otherwise.
  virtual std::string GoogleBaseSearchByImageURLValue() const;

  // Returns the value for the GOOGLE_BASE_SUGGEST_URL term.  This
  // implementation simply returns the default value.
  std::string GoogleBaseSuggestURLValue() const;

  // Returns the locale used by the application.  This implementation returns
  // "en" and thus should be overridden where the result is actually meaningful.
  virtual std::string GetApplicationLocale() const;

  // Returns the value for the Chrome Omnibox rlz.  This implementation returns
  // the empty string.
  virtual std::u16string GetRlzParameterValue(bool from_app_list) const;

  // The optional client parameter passed with Google search requests.  This
  // implementation returns the empty string.
  virtual std::string GetSearchClient() const;

  // Returns the value to use for replacements of type
  // GOOGLE_IMAGE_SEARCH_SOURCE.
  virtual std::string GoogleImageSearchSource() const;

  // Returns the optional referral ID to be passed to Yandex when searching from
  // the omnibox (returns the empty string if not supported/applicable).
  virtual std::string GetYandexReferralID() const;

  // Returns the optional referral ID to be passed to @MAIL.RU when searching
  // from the omnibox (returns the empty string if not supported/applicable).
  virtual std::string GetMailRUReferralID() const;

  // Estimates dynamic memory usage.
  // See base/trace_event/memory_usage_estimator.h for more info.
  virtual size_t EstimateMemoryUsage() const;

  // Returns whether the search engine was set in the search engine choice
  // screen.
  bool search_engine_chosen_in_choice_screen() const {
    return search_engine_chosen_in_choice_screen_;
  }

  void set_search_engine_chosen_in_choice_screen(bool chosen_in_choice_screen) {
    search_engine_chosen_in_choice_screen_ = chosen_in_choice_screen;
  }

 private:
  bool search_engine_chosen_in_choice_screen_ = false;
};

#endif  // COMPONENTS_SEARCH_ENGINES_SEARCH_TERMS_DATA_H_
