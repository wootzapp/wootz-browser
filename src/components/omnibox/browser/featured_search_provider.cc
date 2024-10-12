// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/omnibox/browser/featured_search_provider.h"

#include <stddef.h>

#include <climits>
#include <string>
#include <vector>

#include "base/strings/string_util.h"
#include "build/build_config.h"
#include "components/omnibox/browser/autocomplete_input.h"
#include "components/omnibox/browser/autocomplete_match.h"
#include "components/omnibox/browser/autocomplete_match_classification.h"
#include "components/omnibox/browser/autocomplete_match_type.h"
#include "components/omnibox/browser/autocomplete_provider_client.h"
#include "components/omnibox/browser/in_memory_url_index_types.h"
#include "components/omnibox/browser/omnibox_field_trial.h"
#include "components/omnibox/browser/omnibox_prefs.h"
#include "components/prefs/pref_service.h"
#include "components/search_engines/template_url.h"
#include "components/search_engines/template_url_data.h"
#include "components/search_engines/template_url_service.h"
#include "components/search_engines/template_url_starter_pack_data.h"
#include "components/strings/grit/components_strings.h"
#include "third_party/metrics_proto/omnibox_focus_type.pb.h"
#include "third_party/metrics_proto/omnibox_input_type.pb.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/page_transition_types.h"

constexpr bool kIsDesktop = !BUILDFLAG(IS_ANDROID) && !BUILDFLAG(IS_IOS);

// Scored higher than history URL provider suggestions since inputs like '@b'
// would default 'bing.com' instead (history URL provider seems to ignore '@'
// prefix in the input). Featured Enterprise search ranks higher than "ask
// google" suggestions, which ranks higher than the other starter pack
// suggestions.
const int FeaturedSearchProvider::kAskGoogleRelevance = 1460;
const int FeaturedSearchProvider::kFeaturedEnterpriseSearchRelevance = 1470;
const int FeaturedSearchProvider::kStarterPackRelevance = 1450;

FeaturedSearchProvider::FeaturedSearchProvider(
    AutocompleteProviderClient* client)
    : AutocompleteProvider(AutocompleteProvider::TYPE_FEATURED_SEARCH),
      client_(client) {
  template_url_service_ = client->GetTemplateURLService();
}

void FeaturedSearchProvider::Start(const AutocompleteInput& input,
                                   bool minimal_changes) {
  matches_.clear();

  if (ShouldShowIPHMatch(input)) {
    AddIPHMatch();
  }

  if (input.focus_type() != metrics::OmniboxFocusType::INTERACTION_DEFAULT ||
      (input.type() == metrics::OmniboxInputType::EMPTY)) {
    return;
  }

  DoStarterPackAutocompletion(input);
}

void FeaturedSearchProvider::DeleteMatch(const AutocompleteMatch& match) {
  // Only `NULL_RESULT_MESSAGE` types from this provider are deletable.
  CHECK(match.deletable);
  CHECK(match.type == AutocompleteMatchType::NULL_RESULT_MESSAGE);

  // Set the pref so this provider doesn't continue to offer the suggestion.
  PrefService* prefs = client_->GetPrefs();
  prefs->SetBoolean(omnibox::kShowGeminiIPH, false);

  // Delete `match` from `matches_`.
  std::erase_if(matches_, [&match](const auto& i) {
    return i.contents == match.contents;
  });
}

FeaturedSearchProvider::~FeaturedSearchProvider() = default;

void FeaturedSearchProvider::DoStarterPackAutocompletion(
    const AutocompleteInput& input) {
  // When the user's input begins with '@', we want to prioritize providing
  // suggestions for all active starter pack search engines.
  bool starts_with_starter_pack_symbol = base::StartsWith(
      input.text(), u"@", base::CompareCase::INSENSITIVE_ASCII);

  if (starts_with_starter_pack_symbol) {
    TemplateURLService::TemplateURLVector matches;
    template_url_service_->AddMatchingKeywords(input.text(), false, &matches);
    for (TemplateURL* match : matches) {
      if (match->starter_pack_id() > 0 &&
          match->is_active() == TemplateURLData::ActiveStatus::kTrue) {
        // Don't add the expanded set of starter pack engines unless the feature
        // is enabled.
        if (!OmniboxFieldTrial::IsStarterPackExpansionEnabled() &&
            match->starter_pack_id() > TemplateURLStarterPackData::kTabs) {
          continue;
        }

        AddStarterPackMatch(*match, input);
      } else if (base::FeatureList::IsEnabled(
                     omnibox::kShowFeaturedEnterpriseSiteSearch) &&
                 match->featured_by_policy()) {
        AddFeaturedEnterpriseSearchMatch(*match, input);
      }
    }
  }
}

void FeaturedSearchProvider::AddStarterPackMatch(
    const TemplateURL& template_url,
    const AutocompleteInput& input) {
  // The history starter pack engine is disabled in incognito mode.
  if (client_->IsOffTheRecord() &&
      template_url.starter_pack_id() == TemplateURLStarterPackData::kHistory) {
    return;
  }

  // The starter pack relevance score is currently ranked above
  // search-what-you-typed suggestion to avoid the keyword mode chip attaching
  // to the search suggestion instead of Builtin suggestions.
  // TODO(yoangela): This should be updated so the keyword chip only attaches to
  //  STARTER_PACK type suggestions rather than rely on out-scoring all other
  //  suggestions.
  AutocompleteMatch match(this, kStarterPackRelevance, false,
                          AutocompleteMatchType::STARTER_PACK);

  const std::u16string destination_url =
      TemplateURLStarterPackData::GetDestinationUrlForStarterPackID(
          template_url.starter_pack_id());
  match.fill_into_edit = template_url.keyword();
  match.inline_autocompletion =
      match.fill_into_edit.substr(input.text().length());
  match.destination_url = GURL(destination_url);
  match.transition = ui::PAGE_TRANSITION_GENERATED;
  if (kIsDesktop &&
      input.current_page_classification() !=
          metrics::OmniboxEventProto::NTP_REALBOX &&
      template_url.keyword().starts_with(u'@')) {
    // The AskGoogle provider doesn't follow the "Search X" pattern and should
    // also be ranked first.
    // TODO(b/41494524): Currently templateurlservice returns the keywords in
    //  alphabetical order, which is the order we rank them. There should be a
    //  more sustainable way for specifying the order they should appear in the
    //  omnibox.
    if (OmniboxFieldTrial::IsStarterPackExpansionEnabled() &&
        template_url.starter_pack_id() ==
            TemplateURLStarterPackData::kAskGoogle) {
      match.description = l10n_util::GetStringFUTF16(
          IDS_OMNIBOX_INSTANT_KEYWORD_CHAT_TEXT, template_url.keyword(),
          template_url.short_name());
      match.relevance = kAskGoogleRelevance;
    } else {
      std::u16string short_name = template_url.short_name();
      if (template_url.short_name() == u"Tabs") {
        // Very special request from UX to sentence-case "Tabs" -> "tabs" only
        // in this context. It needs to stay capitalized elsewhere since it's
        // treated like a proper engine name.
        match.description = short_name = u"tabs";
      }
      match.description =
          l10n_util::GetStringFUTF16(IDS_OMNIBOX_INSTANT_KEYWORD_SEARCH_TEXT,
                                     template_url.keyword(), short_name);
    }
    match.description_class = {
        {0, ACMatchClassification::NONE},
        {template_url.keyword().size(), ACMatchClassification::DIM}};
    match.contents.clear();
    match.contents_class = {{}};
    match.allowed_to_be_default_match = false;
    match.keyword = template_url.keyword();
  } else {
    match.description = template_url.short_name();
    match.description_class.emplace_back(0, ACMatchClassification::NONE);
    match.contents = destination_url;
    match.contents_class.emplace_back(0, ACMatchClassification::URL);
    match.SetAllowedToBeDefault(input);
  }
  matches_.push_back(match);
}

void FeaturedSearchProvider::AddIPHMatch() {
  // This value doesn't really matter as this suggestion is grouped after all
  // other suggestions. Use an arbitrary constant.
  constexpr int kRelevanceScore = 1000;
  AutocompleteMatch match(this, kRelevanceScore, /*deletable=*/false,
                          AutocompleteMatchType::NULL_RESULT_MESSAGE);

  // Use this suggestion's contents field to display a message to the user that
  // cannot be acted upon.
  match.contents = l10n_util::GetStringUTF16(IDS_OMNIBOX_GEMINI_IPH);
  match.deletable = true;

  // Bolds just the "@gemini" portion of the IPH string. The rest of the string
  // is dimmed.
  TermMatches term_matches = MatchTermInString(u"@gemini", match.contents, 0);
  match.contents_class = ClassifyTermMatches(
      term_matches, match.contents.size(), ACMatchClassification::MATCH,
      ACMatchClassification::DIM);

  matches_.push_back(match);
  iph_shown_count_++;
}

void FeaturedSearchProvider::AddFeaturedEnterpriseSearchMatch(
    const TemplateURL& template_url,
    const AutocompleteInput& input) {
  if (!kIsDesktop || input.current_page_classification() ==
                         metrics::OmniboxEventProto::NTP_REALBOX) {
    return;
  }

  AutocompleteMatch match(this, kFeaturedEnterpriseSearchRelevance, false,
                          AutocompleteMatchType::FEATURED_ENTERPRISE_SEARCH);

  match.fill_into_edit = template_url.keyword();
  match.inline_autocompletion =
      match.fill_into_edit.substr(input.text().length());
  match.destination_url = GURL(template_url.url());
  match.transition = ui::PAGE_TRANSITION_GENERATED;
  match.description = l10n_util::GetStringFUTF16(
      IDS_OMNIBOX_INSTANT_KEYWORD_SEARCH_TEXT, template_url.keyword(),
      template_url.short_name());
  match.description_class = {
      {0, ACMatchClassification::NONE},
      {template_url.keyword().size(), ACMatchClassification::DIM}};
  match.contents.clear();
  match.contents_class = {{}};
  match.allowed_to_be_default_match = false;
  match.keyword = template_url.keyword();

  matches_.push_back(match);
}

bool FeaturedSearchProvider::ShouldShowIPHMatch(
    const AutocompleteInput& input) {
  // The IPH suggestion should only be shown in Zero prefix state.
  if (!OmniboxFieldTrial::IsStarterPackIPHEnabled() || !input.IsZeroSuggest()) {
    return false;
  }

  // If the IPH suggestion has been shown more than the limited number of times
  // this session, or has been manually deleted by the user, do not continue to
  // offer it. If the limit is set to INT_MAX, do not limit.
  PrefService* prefs = client_->GetPrefs();
  size_t iph_shown_limit =
      OmniboxFieldTrial::kStarterPackIPHPerSessionLimit.Get();
  if (!prefs->GetBoolean(omnibox::kShowGeminiIPH) ||
      ((iph_shown_limit != INT_MAX) && (iph_shown_count_ >= iph_shown_limit))) {
    return false;
  }

  // The @gemini IPH should no longer be shown once a user has successfully
  // used @gemini.
  TemplateURL* gemini_turl = template_url_service_->FindStarterPackTemplateURL(
      TemplateURLStarterPackData::kAskGoogle);
  if (gemini_turl && gemini_turl->usage_count() > 0) {
    return false;
  }

  return true;
}
