// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/search_engines/template_url_prepopulate_data.h"

#include <algorithm>
#include <random>

#include "base/check_deref.h"
#include "base/check_is_test.h"
#include "base/containers/contains.h"
#include "base/feature_list.h"
#include "base/logging.h"
#include "base/metrics/histogram_functions.h"
#include "base/rand_util.h"
#include "base/ranges/algorithm.h"
#include "build/build_config.h"
#include "components/country_codes/country_codes.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"
#include "components/search_engines/prepopulated_engines.h"
#include "components/search_engines/search_engine_choice/search_engine_choice_service.h"
#include "components/search_engines/search_engine_choice_utils.h"
#include "components/search_engines/search_engines_pref_names.h"
#include "components/search_engines/template_url_data.h"
#include "components/search_engines/template_url_data_util.h"
#include "components/search_engines/template_url_service.h"
#include "components/version_info/version_info.h"

namespace TemplateURLPrepopulateData {

// Helpers --------------------------------------------------------------------

namespace {

#include "components/search_engines/search_engine_countries-inc.cc"

std::vector<std::unique_ptr<TemplateURLData>>
GetPrepopulatedEnginesForEeaRegionCountries(int country_id,
                                            PrefService* prefs) {
  std::vector<std::unique_ptr<TemplateURLData>> t_urls;
  std::vector<const PrepopulatedEngine*> top_engines;
  std::vector<const PrepopulatedEngine*> tying_engines;
  std::vector<const PrepopulatedEngine*> remaining_engines;

  CHECK(search_engines::IsEeaChoiceCountry(country_id) &&
        search_engines::IsChoiceScreenFlagEnabled(
            search_engines::ChoicePromo::kAny));

  const std::vector<EngineAndTier> country_engines =
      GetPrepopulationSetFromCountryID(country_id);

  for (const EngineAndTier& country_engine : country_engines) {
    switch (country_engine.tier) {
      case SearchEngineTier::kTopEngines:
        top_engines.push_back(country_engine.search_engine);
        break;
      case SearchEngineTier::kTyingEngines:
        tying_engines.push_back(country_engine.search_engine);
        break;
      case SearchEngineTier::kRemainingEngines:
        remaining_engines.push_back(country_engine.search_engine);
        break;
    }
  }

  uint64_t profile_seed;
  if (prefs) {
    profile_seed = prefs->GetInt64(
        prefs::kDefaultSearchProviderChoiceScreenRandomShuffleSeed);
    int seed_version_number = prefs->GetInteger(
        prefs::kDefaultSearchProviderChoiceScreenShuffleMilestone);
    int current_version_number = version_info::GetMajorVersionNumberAsInt();
    // Ensure that the generated seed is not 0 to avoid accidental re-seeding
    // and re-shuffle on every chrome update.
    while (profile_seed == 0 || current_version_number != seed_version_number) {
      profile_seed = base::RandUint64();
      prefs->SetInt64(
          prefs::kDefaultSearchProviderChoiceScreenRandomShuffleSeed,
          profile_seed);
      prefs->SetInteger(
          prefs::kDefaultSearchProviderChoiceScreenShuffleMilestone,
          current_version_number);
      seed_version_number = current_version_number;
    }
  } else {
    // TODO(crbug.com/1499181): Avoid passing null prefs and unbranch the code.
    CHECK_IS_TEST();
    // Choosing a fixed magic number to ensure a stable shuffle in tests too.
    profile_seed = 42;
  }

  // Randomize all vectors using the generated seed.
  std::default_random_engine generator;
  generator.seed(profile_seed);
  std::shuffle(top_engines.begin(), top_engines.end(), generator);
  std::shuffle(tying_engines.begin(), tying_engines.end(), generator);
  std::shuffle(remaining_engines.begin(), remaining_engines.end(), generator);

  size_t current_number_of_engines = 0;
  for (const PrepopulatedEngine* engine : top_engines) {
    if (current_number_of_engines == kMaxEeaPrepopulatedEngines) {
      break;
    }
    t_urls.push_back(TemplateURLDataFromPrepopulatedEngine(*engine));
    current_number_of_engines++;
  }
  for (const PrepopulatedEngine* engine : tying_engines) {
    if (current_number_of_engines == kMaxEeaPrepopulatedEngines) {
      break;
    }
    t_urls.push_back(TemplateURLDataFromPrepopulatedEngine(*engine));
    current_number_of_engines++;
  }
  for (const PrepopulatedEngine* engine : remaining_engines) {
    if (current_number_of_engines == kMaxEeaPrepopulatedEngines) {
      break;
    }
    t_urls.push_back(TemplateURLDataFromPrepopulatedEngine(*engine));
    current_number_of_engines++;
  }

  return t_urls;
}

std::vector<std::unique_ptr<TemplateURLData>> GetPrepopulatedTemplateURLData(
    int country_id,
    PrefService* prefs) {
  std::vector<std::unique_ptr<TemplateURLData>> t_urls;

  if (search_engines::IsEeaChoiceCountry(country_id) &&
      search_engines::IsChoiceScreenFlagEnabled(
          search_engines::ChoicePromo::kAny)) {
    CHECK(prefs);
    return GetPrepopulatedEnginesForEeaRegionCountries(country_id, prefs);
  }

  std::vector<EngineAndTier> engines =
      GetPrepopulationSetFromCountryID(country_id);
  for (const EngineAndTier& engine : engines) {
    if (engine.tier == SearchEngineTier::kTopEngines) {
      t_urls.push_back(
          TemplateURLDataFromPrepopulatedEngine(*engine.search_engine));
    }
  }
  return t_urls;
}

// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum class SearchProviderOverrideStatus {
  // No preferences are available for `prefs::kSearchProviderOverrides`.
  kNoPref = 0,

  // The preferences for `prefs::kSearchProviderOverrides` do not contain valid
  // template URLs.
  kEmptyPref = 1,

  // The preferences for `prefs::kSearchProviderOverrides` contain valid
  // template URL(s).
  kPrefHasValidUrls = 2,

  kMaxValue = kPrefHasValidUrls
};

std::vector<std::unique_ptr<TemplateURLData>> GetOverriddenTemplateURLData(
    PrefService* prefs) {
  std::vector<std::unique_ptr<TemplateURLData>> t_urls;
  if (!prefs)
    return t_urls;

  const base::Value::List& list =
      prefs->GetList(prefs::kSearchProviderOverrides);

  for (const base::Value& engine : list) {
    if (engine.is_dict()) {
      auto t_url = TemplateURLDataFromOverrideDictionary(engine.GetDict());
      if (t_url) {
        t_urls.push_back(std::move(t_url));
      }
    }
  }

  base::UmaHistogramEnumeration(
      "Search.SearchProviderOverrideStatus",
      !t_urls.empty() ? SearchProviderOverrideStatus::kPrefHasValidUrls
                      : (prefs->HasPrefPath(prefs::kSearchProviderOverrides)
                             ? SearchProviderOverrideStatus::kEmptyPref
                             : SearchProviderOverrideStatus::kNoPref));

  return t_urls;
}

}  // namespace

// Global functions -----------------------------------------------------------

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  country_codes::RegisterProfilePrefs(registry);
  registry->RegisterListPref(prefs::kSearchProviderOverrides);
  registry->RegisterIntegerPref(prefs::kSearchProviderOverridesVersion, -1);
  registry->RegisterInt64Pref(
      prefs::kDefaultSearchProviderChoiceScreenRandomShuffleSeed, 0);
  registry->RegisterIntegerPref(
      prefs::kDefaultSearchProviderChoiceScreenShuffleMilestone, 0);
  registry->RegisterBooleanPref(
      prefs::kDefaultSearchProviderKeywordsUseExtendedList, false);
  registry->RegisterBooleanPref(prefs::kDefaultSearchProviderChoicePending,
                                false);
}

int GetDataVersion(PrefService* prefs) {
  // Allow tests to override the local version.
  return (prefs && prefs->HasPrefPath(prefs::kSearchProviderOverridesVersion)) ?
      prefs->GetInteger(prefs::kSearchProviderOverridesVersion) :
      kCurrentDataVersion;
}

std::vector<std::unique_ptr<TemplateURLData>> GetPrepopulatedEngines(
    PrefService* prefs,
    search_engines::SearchEngineChoiceService* search_engine_choice_service,
    size_t* default_search_provider_index,
    bool include_current_default,
    TemplateURLService* template_url_service) {
  // If there is a set of search engines in the preferences file, it overrides
  // the built-in set.
  std::vector<std::unique_ptr<TemplateURLData>> t_urls =
      GetOverriddenTemplateURLData(prefs);
  if (t_urls.empty()) {
    // `search_engine_choice_service` (and `prefs`) can be null in tests.
    // TODO(b/318801987): Make sure `prefs` and `search_engine_choice_service`
    //                    are always not null.
    int country_id = search_engine_choice_service
                         ? search_engine_choice_service->GetCountryId()
                         : country_codes::GetCurrentCountryID();
    t_urls = GetPrepopulatedTemplateURLData(country_id, prefs);

    if (include_current_default && template_url_service) {
      CHECK(search_engines::IsChoiceScreenFlagEnabled(
          search_engines::ChoicePromo::kAny));
      // This would add the current default search engine to the top of the
      // returned list if it's not already there.
      const TemplateURL* default_search_engine =
          template_url_service->GetDefaultSearchProvider();
      bool inserted_default = false;
      if (default_search_engine &&
          !base::Contains(t_urls, default_search_engine->prepopulate_id(),
                          [](const std::unique_ptr<TemplateURLData>& engine) {
                            return engine->prepopulate_id;
                          })) {
        t_urls.insert(t_urls.begin(), std::make_unique<TemplateURLData>(
                                          default_search_engine->data()));
        inserted_default = true;
      }
      search_engines::RecordIsDefaultProviderAddedToChoices(inserted_default);
    }
  }
  if (default_search_provider_index) {
    const auto itr =
        base::ranges::find(t_urls, google.id, &TemplateURLData::prepopulate_id);
    *default_search_provider_index =
        itr == t_urls.end() ? 0 : std::distance(t_urls.begin(), itr);
  }
  return t_urls;
}

std::unique_ptr<TemplateURLData> GetPrepopulatedEngine(
    PrefService* prefs,
    search_engines::SearchEngineChoiceService* search_engine_choice_service,
    int prepopulated_id) {
  auto engines = TemplateURLPrepopulateData::GetPrepopulatedEngines(
      prefs, search_engine_choice_service, nullptr);
  for (auto& engine : engines) {
    if (engine->prepopulate_id == prepopulated_id)
      return std::move(engine);
  }
  return nullptr;
}

#if BUILDFLAG(IS_ANDROID)

std::vector<std::unique_ptr<TemplateURLData>> GetLocalPrepopulatedEngines(
    const std::string& country_code,
    PrefService& prefs) {
  int country_id = country_codes::CountryStringToCountryID(country_code);
  if (country_id == country_codes::kCountryIDUnknown) {
    LOG(ERROR) << "Unknown country code specified: " << country_code;
    return std::vector<std::unique_ptr<TemplateURLData>>();
  }

  return GetPrepopulatedTemplateURLData(country_id, &prefs);
}

#endif

std::vector<const PrepopulatedEngine*> GetAllPrepopulatedEngines() {
  return std::vector<const PrepopulatedEngine*>(
      &kAllEngines[0], &kAllEngines[0] + kAllEnginesLength);
}

std::unique_ptr<TemplateURLData> GetPrepopulatedEngineFromFullList(
    PrefService* prefs,
    search_engines::SearchEngineChoiceService* search_engine_choice_service,
    int prepopulated_id) {
  // TODO(crbug.com/1500526): Refactor to better share code with
  // `GetPrepopulatedEngine()`.

  // If there is a set of search engines in the preferences file, we look for
  // the ID there first.
  for (std::unique_ptr<TemplateURLData>& data :
       GetOverriddenTemplateURLData(prefs)) {
    if (data->prepopulate_id == prepopulated_id) {
      return std::move(data);
    }
  }

  // We look in the profile country's prepopulated set first. This is intended
  // to help using the right entry for the case where we have multiple ones in
  // the full list that share a same prepopulated id.
  const int country = search_engine_choice_service->GetCountryId();
  for (const EngineAndTier& engine_and_tier :
       GetPrepopulationSetFromCountryID(country)) {
    if (engine_and_tier.search_engine->id == prepopulated_id) {
      return TemplateURLDataFromPrepopulatedEngine(
          *engine_and_tier.search_engine);
    }
  }

  // Fallback: just grab the first matching entry from the complete list. In
  // case of IDs shared across multiple entries, we might be returning the
  // wrong one for the profile country. We can look into better heuristics in
  // future work.
  for (size_t i = 0; i < kAllEnginesLength; ++i) {
    const PrepopulatedEngine* engine = kAllEngines[i];
    if (engine->id == prepopulated_id) {
      return TemplateURLDataFromPrepopulatedEngine(*engine);
    }
  }

  return nullptr;
}

void ClearPrepopulatedEnginesInPrefs(PrefService* prefs) {
  if (!prefs)
    return;

  prefs->ClearPref(prefs::kSearchProviderOverrides);
  prefs->ClearPref(prefs::kSearchProviderOverridesVersion);
}

std::unique_ptr<TemplateURLData> GetPrepopulatedDefaultSearch(
    PrefService* prefs,
    search_engines::SearchEngineChoiceService* search_engine_choice_service) {
  size_t default_search_index;
  // This could be more efficient.  We load all URLs but keep only the default.
  std::vector<std::unique_ptr<TemplateURLData>> loaded_urls =
      GetPrepopulatedEngines(prefs, search_engine_choice_service,
                             &default_search_index);

  return (default_search_index < loaded_urls.size())
             ? std::move(loaded_urls[default_search_index])
             : nullptr;
}

}  // namespace TemplateURLPrepopulateData
