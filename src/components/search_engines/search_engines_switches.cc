// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/search_engines/search_engines_switches.h"

namespace switches {

// Additional query params to insert in the search and instant URLs.  Useful for
// testing.
COMPONENT_EXPORT(SEARCH_ENGINES_SWITCHES)
const char kExtraSearchQueryParams[] = "extra-search-query-params";

// Override the country used for search engine choice region checks.
// Intended for testing. Expects 2-letter country codes.
COMPONENT_EXPORT(SEARCH_ENGINES_SWITCHES)
const char kSearchEngineChoiceCountry[] = "search-engine-choice-country";

// Disable the search engine choice screen for testing / autmation.
COMPONENT_EXPORT(SEARCH_ENGINES_SWITCHES)
const char kDisableSearchEngineChoiceScreen[] =
    "disable-search-engine-choice-screen";

// Force-enable showing the search engine choice screen for testing regardless
// of region or choice already having been made.
COMPONENT_EXPORT(SEARCH_ENGINES_SWITCHES)
const char kForceSearchEngineChoiceScreen[] =
    "force-search-engine-choice-screen";

// Enables the search engine choice screen. Feature parameters below can
// affect the actual triggering logic.
// The default feature state is split by platform to ease potential merges
// that could be needed if we need to change the state while waterfalling this
// feature.
COMPONENT_EXPORT(SEARCH_ENGINES_SWITCHES)
BASE_FEATURE(kSearchEngineChoiceTrigger,
             "SearchEngineChoiceTrigger",
#if BUILDFLAG(IS_ANDROID)
             base::FEATURE_DISABLED_BY_DEFAULT
#elif BUILDFLAG(IS_IOS)
             base::FEATURE_DISABLED_BY_DEFAULT
#elif BUILDFLAG(IS_WIN)
             base::FEATURE_DISABLED_BY_DEFAULT
#elif BUILDFLAG(IS_CHROMEOS)
             base::FEATURE_DISABLED_BY_DEFAULT
#else
             base::FEATURE_DISABLED_BY_DEFAULT
#endif

);

COMPONENT_EXPORT(SEARCH_ENGINES_SWITCHES)
const base::FeatureParam<bool> kSearchEngineChoiceTriggerForTaggedProfilesOnly{
    &kSearchEngineChoiceTrigger, /*name=*/"for_tagged_profiles_only", true};

COMPONENT_EXPORT(SEARCH_ENGINES_SWITCHES)
const base::FeatureParam<bool> kSearchEngineChoiceTriggerWithForceEeaCountry{
    &kSearchEngineChoiceTrigger, /*name=*/"with_force_eea_country", false};

COMPONENT_EXPORT(SEARCH_ENGINES_SWITCHES)
const base::FeatureParam<std::string> kSearchEngineChoiceTriggerRepromptParams{
    &kSearchEngineChoiceTrigger,
    /*name=*/"reprompt",
    /*default_value=*/"{}"};

COMPONENT_EXPORT(SEARCH_ENGINES_SWITCHES)
const base::FeatureParam<bool> kSearchEngineChoiceTriggerSkipFor3p{
    &kSearchEngineChoiceTrigger,
    /*name=*/"skip_for_3p",
    /*default_value=*/true};

}  // namespace switches
