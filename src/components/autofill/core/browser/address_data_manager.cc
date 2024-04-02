// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/address_data_manager.h"

#include <memory>

#include "base/check_op.h"
#include "base/containers/contains.h"
#include "base/functional/callback.h"
#include "base/notreached.h"
#include "base/ranges/algorithm.h"
#include "base/strings/utf_string_conversions.h"
#include "components/autofill/core/browser/geo/country_data.h"
#include "components/autofill/core/browser/metrics/profile_token_quality_metrics.h"
#include "components/autofill/core/browser/metrics/stored_profile_metrics.h"
#include "components/autofill/core/common/autofill_clock.h"
#include "components/autofill/core/common/autofill_prefs.h"
#include "components/prefs/pref_service.h"
#include "components/sync/base/model_type.h"
#include "components/webdata/common/web_data_results.h"

namespace autofill {

namespace {

// Orders all `profiles` by the specified `order` rule.
void OrderProfiles(std::vector<AutofillProfile*>& profiles,
                   AddressDataManager::ProfileOrder order) {
  switch (order) {
    case AddressDataManager::ProfileOrder::kNone:
      break;
    case AddressDataManager::ProfileOrder::kHighestFrecencyDesc:
      base::ranges::sort(profiles, [comparison_time = AutofillClock::Now()](
                                       AutofillProfile* a, AutofillProfile* b) {
        return a->HasGreaterRankingThan(b, comparison_time);
      });
      break;
    case AddressDataManager::ProfileOrder::kMostRecentlyModifiedDesc:
      base::ranges::sort(profiles, [](AutofillProfile* a, AutofillProfile* b) {
        return a->modification_date() > b->modification_date();
      });
      break;
    case AddressDataManager::ProfileOrder::kMostRecentlyUsedFirstDesc:
      base::ranges::sort(profiles, [](AutofillProfile* a, AutofillProfile* b) {
        return a->use_date() > b->use_date();
      });
      break;
  }
}

}  // namespace

AddressDataManager::AddressDataManager(
    scoped_refptr<AutofillWebDataService> webdata_service,
    PrefService* pref_service,
    StrikeDatabaseBase* strike_database,
    base::RepeatingClosure notify_pdm_observers,
    const std::string& app_locale)
    : notify_pdm_observers_(notify_pdm_observers),
      webdata_service_(webdata_service),
      app_locale_(app_locale) {
  if (webdata_service_) {
    // The `webdata_service_` is null when the TestPDM is used.
    webdata_service_->SetAutofillProfileChangedCallback(
        base::BindRepeating(&AddressDataManager::OnAutofillProfileChanged,
                            weak_factory_.GetWeakPtr()));
    webdata_service_observer_.Observe(webdata_service_.get());
  }
  SetPrefService(pref_service);
  SetStrikeDatabase(strike_database);
}

AddressDataManager::~AddressDataManager() {
  CancelAllPendingQueries();
}

void AddressDataManager::OnAutofillChangedBySync(syncer::ModelType model_type) {
  if (model_type == syncer::ModelType::AUTOFILL_PROFILE ||
      model_type == syncer::ModelType::CONTACT_INFO) {
    LoadProfiles();
  }
}

void AddressDataManager::OnWebDataServiceRequestDone(
    WebDataServiceBase::Handle handle,
    std::unique_ptr<WDTypedResult> result) {
  CHECK(handle == pending_synced_local_profiles_query_ ||
        handle == pending_account_profiles_query_);

  if (!result) {
    // Error from the database.
    if (handle == pending_synced_local_profiles_query_) {
      pending_synced_local_profiles_query_ = 0;
    } else {
      pending_account_profiles_query_ = 0;
    }
  } else {
    CHECK_EQ(result->GetType(), AUTOFILL_PROFILES_RESULT);
    std::vector<std::unique_ptr<AutofillProfile>> profiles_from_db =
        static_cast<WDResult<std::vector<std::unique_ptr<AutofillProfile>>>*>(
            result.get())
            ->GetValue();
    if (handle == pending_synced_local_profiles_query_) {
      synced_local_profiles_ = std::move(profiles_from_db);
      pending_synced_local_profiles_query_ = 0;
    } else {
      account_profiles_ = std::move(profiles_from_db);
      pending_account_profiles_query_ = 0;
    }
  }

  if (HasPendingQueries()) {
    return;
  }
  if (!has_initial_load_finished_) {
    has_initial_load_finished_ = true;
    LogStoredDataMetrics();
  }
  notify_pdm_observers_.Run();
}

std::vector<AutofillProfile*> AddressDataManager::GetProfiles(
    ProfileOrder order) const {
  std::vector<AutofillProfile*> a = GetProfilesFromSource(
      AutofillProfile::Source::kLocalOrSyncable, ProfileOrder::kNone);
  std::vector<AutofillProfile*> b = GetProfilesFromSource(
      AutofillProfile::Source::kAccount, ProfileOrder::kNone);
  a.reserve(a.size() + b.size());
  base::ranges::move(b, std::back_inserter(a));
  OrderProfiles(a, order);
  return a;
}

std::vector<AutofillProfile*> AddressDataManager::GetProfilesFromSource(
    AutofillProfile::Source profile_source,
    ProfileOrder order) const {
  const std::vector<std::unique_ptr<AutofillProfile>>& profiles =
      GetProfileStorage(profile_source);
  std::vector<AutofillProfile*> result;
  result.reserve(profiles.size());
  for (const std::unique_ptr<AutofillProfile>& profile : profiles) {
    result.push_back(profile.get());
  }
  OrderProfiles(result, order);
  return result;
}

AutofillProfile* AddressDataManager::GetProfileByGUID(
    const std::string& guid) const {
  std::vector<AutofillProfile*> profiles = GetProfiles();
  auto it = base::ranges::find(
      profiles, guid,
      [](const AutofillProfile* profile) { return profile->guid(); });
  return it != profiles.end() ? *it : nullptr;
}

void AddressDataManager::AddProfile(const AutofillProfile& profile) {
  if (!webdata_service_ || !IsAutofillProfileEnabled()) {
    return;
  }
  if (profile.IsEmpty(app_locale_)) {
    // TODO(crbug.com/1007974): This call is only used to notify tests to stop
    // waiting. Since no profile is added, this case shouldn't trigger
    // `OnPersonalDataChanged()`.
    notify_pdm_observers_.Run();
    return;
  }
  ongoing_profile_changes_[profile.guid()].emplace_back(
      AutofillProfileChange(AutofillProfileChange::ADD, profile.guid(),
                            profile),
      /*is_ongoing=*/false);
  HandleNextProfileChange(profile.guid());
}

void AddressDataManager::UpdateProfile(const AutofillProfile& profile) {
  if (!webdata_service_) {
    return;
  }

  // If the profile is empty, remove it unconditionally.
  if (profile.IsEmpty(app_locale_)) {
    RemoveProfile(profile.guid());
    return;
  }

  // The profile is a duplicate of an existing profile if it has a distinct GUID
  // but the same content.
  // Duplicates can exist across profile sources.
  const std::vector<std::unique_ptr<AutofillProfile>>& profiles =
      GetProfileStorage(profile.source());
  auto duplicate_profile_iter =
      base::ranges::find_if(profiles, [&profile](const auto& other_profile) {
        return profile.guid() != other_profile->guid() &&
               other_profile->Compare(profile) == 0;
      });

  // Remove the profile if it is a duplicate of another already existing
  // profile.
  if (duplicate_profile_iter != profiles.end()) {
    // Keep the more recently used version of the profile.
    if (profile.use_date() > duplicate_profile_iter->get()->use_date()) {
      UpdateProfileInDB(profile);
      RemoveProfile(duplicate_profile_iter->get()->guid());
    } else {
      RemoveProfile(profile.guid());
    }
    return;
  }

  UpdateProfileInDB(profile);
}

void AddressDataManager::RemoveProfile(const std::string& guid) {
  if (!webdata_service_) {
    return;
  }

  // Find the profile to remove.
  // TODO(crbug.com/1420547): This shouldn't be necessary. Providing a `guid`
  // to the `AutofillProfileChange()` should suffice for removals.
  const AutofillProfile* profile =
      ProfileChangesAreOngoing(guid)
          ? &ongoing_profile_changes_[guid].back().first.data_model()
          : GetProfileByGUID(guid);
  if (!profile) {
    notify_pdm_observers_.Run();
    return;
  }

  ongoing_profile_changes_[guid].emplace_back(
      AutofillProfileChange(AutofillProfileChange::REMOVE, guid, *profile),
      /*is_ongoing=*/false);
  HandleNextProfileChange(guid);
}

void AddressDataManager::MigrateProfileToAccount(
    const AutofillProfile& profile) {
  CHECK_EQ(profile.source(), AutofillProfile::Source::kLocalOrSyncable);
  AutofillProfile account_profile = profile.ConvertToAccountProfile();
  DCHECK_NE(profile.guid(), account_profile.guid());
  // Update the database (and this way indirectly Sync).
  RemoveProfile(profile.guid());
  AddProfile(account_profile);
}

void AddressDataManager::LoadProfiles() {
  if (!webdata_service_) {
    return;
  }

  CancelPendingQuery(pending_synced_local_profiles_query_);
  CancelPendingQuery(pending_account_profiles_query_);
  pending_synced_local_profiles_query_ = webdata_service_->GetAutofillProfiles(
      AutofillProfile::Source::kLocalOrSyncable, this);
  pending_account_profiles_query_ = webdata_service_->GetAutofillProfiles(
      AutofillProfile::Source::kAccount, this);
}

void AddressDataManager::RecordUseOf(const AutofillProfile& profile) {
  AutofillProfile* adm_profile = GetProfileByGUID(profile.guid());
  if (!adm_profile) {
    return;
  }
  AutofillProfile updated_profile = *adm_profile;
  updated_profile.RecordAndLogUse();
  UpdateProfile(updated_profile);
}

bool AddressDataManager::IsProfileMigrationBlocked(
    const std::string& guid) const {
  AutofillProfile* profile = GetProfileByGUID(guid);
  DCHECK(profile == nullptr ||
         profile->source() == AutofillProfile::Source::kLocalOrSyncable);
  if (!GetProfileMigrationStrikeDatabase()) {
    return false;
  }
  return GetProfileMigrationStrikeDatabase()->ShouldBlockFeature(guid);
}

void AddressDataManager::AddStrikeToBlockProfileMigration(
    const std::string& guid) {
  if (!GetProfileMigrationStrikeDatabase()) {
    return;
  }
  GetProfileMigrationStrikeDatabase()->AddStrike(guid);
}

void AddressDataManager::AddMaxStrikesToBlockProfileMigration(
    const std::string& guid) {
  if (AutofillProfileMigrationStrikeDatabase* db =
          GetProfileMigrationStrikeDatabase()) {
    db->AddStrikes(db->GetMaxStrikesLimit() - db->GetStrikes(guid), guid);
  }
}

void AddressDataManager::RemoveStrikesToBlockProfileMigration(
    const std::string& guid) {
  if (!GetProfileMigrationStrikeDatabase()) {
    return;
  }
  GetProfileMigrationStrikeDatabase()->ClearStrikes(guid);
}

bool AddressDataManager::IsNewProfileImportBlockedForDomain(
    const GURL& url) const {
  if (!GetProfileSaveStrikeDatabase() || !url.is_valid() || !url.has_host()) {
    return false;
  }

  return GetProfileSaveStrikeDatabase()->ShouldBlockFeature(url.host());
}

void AddressDataManager::AddStrikeToBlockNewProfileImportForDomain(
    const GURL& url) {
  if (!GetProfileSaveStrikeDatabase() || !url.is_valid() || !url.has_host()) {
    return;
  }
  GetProfileSaveStrikeDatabase()->AddStrike(url.host());
}

void AddressDataManager::RemoveStrikesToBlockNewProfileImportForDomain(
    const GURL& url) {
  if (!GetProfileSaveStrikeDatabase() || !url.is_valid() || !url.has_host()) {
    return;
  }
  GetProfileSaveStrikeDatabase()->ClearStrikes(url.host());
}

bool AddressDataManager::IsProfileUpdateBlocked(const std::string& guid) const {
  if (!GetProfileUpdateStrikeDatabase()) {
    return false;
  }
  return GetProfileUpdateStrikeDatabase()->ShouldBlockFeature(guid);
}

void AddressDataManager::AddStrikeToBlockProfileUpdate(
    const std::string& guid) {
  if (!GetProfileUpdateStrikeDatabase()) {
    return;
  }
  GetProfileUpdateStrikeDatabase()->AddStrike(guid);
}

void AddressDataManager::RemoveStrikesToBlockProfileUpdate(
    const std::string& guid) {
  if (!GetProfileUpdateStrikeDatabase()) {
    return;
  }
  GetProfileUpdateStrikeDatabase()->ClearStrikes(guid);
}

bool AddressDataManager::AreAddressSuggestionsBlocked(
    FormSignature form_signature,
    FieldSignature field_signature,
    const GURL& gurl) const {
  if (!GetAddressSuggestionStrikeDatabase()) {
    return false;
  }
  return GetAddressSuggestionStrikeDatabase()->ShouldBlockFeature(
      AddressSuggestionStrikeDatabase::GetId(form_signature, field_signature,
                                             gurl));
}

void AddressDataManager::AddStrikeToBlockAddressSuggestions(
    FormSignature form_signature,
    FieldSignature field_signature,
    const GURL& gurl) {
  if (!GetAddressSuggestionStrikeDatabase()) {
    return;
  }
  GetAddressSuggestionStrikeDatabase()->AddStrike(
      AddressSuggestionStrikeDatabase::GetId(form_signature, field_signature,
                                             gurl));
}

void AddressDataManager::ClearStrikesToBlockAddressSuggestions(
    FormSignature form_signature,
    FieldSignature field_signature,
    const GURL& gurl) {
  if (!GetAddressSuggestionStrikeDatabase()) {
    return;
  }
  GetAddressSuggestionStrikeDatabase()->ClearStrikes(
      AddressSuggestionStrikeDatabase::GetId(form_signature, field_signature,
                                             gurl));
}

void AddressDataManager::SetPrefService(PrefService* pref_service) {
  pref_service_ = pref_service;
  profile_enabled_pref_ = std::make_unique<BooleanPrefMember>();
  // `pref_service_` can be nullptr in tests. Using base::Unretained(this) is
  // safe because observer instances are destroyed once `this` is destroyed.
  if (pref_service_) {
    profile_enabled_pref_->Init(prefs::kAutofillProfileEnabled, pref_service_,
                                base::BindRepeating(
                                    [](AddressDataManager* self) {
                                      self->most_common_country_code_.clear();
                                      self->LoadProfiles();
                                    },
                                    this));
  }
}

void AddressDataManager::SetStrikeDatabase(
    StrikeDatabaseBase* strike_database) {
  if (!strike_database) {
    return;
  }
  profile_migration_strike_database_ =
      std::make_unique<AutofillProfileMigrationStrikeDatabase>(strike_database);
  profile_save_strike_database_ =
      std::make_unique<AutofillProfileSaveStrikeDatabase>(strike_database);
  profile_update_strike_database_ =
      std::make_unique<AutofillProfileUpdateStrikeDatabase>(strike_database);
  address_suggestion_strike_database_ =
      std::make_unique<AddressSuggestionStrikeDatabase>(strike_database);
}

AutofillProfileMigrationStrikeDatabase*
AddressDataManager::GetProfileMigrationStrikeDatabase() {
  return const_cast<AutofillProfileMigrationStrikeDatabase*>(
      std::as_const(*this).GetProfileMigrationStrikeDatabase());
}

const AutofillProfileMigrationStrikeDatabase*
AddressDataManager::GetProfileMigrationStrikeDatabase() const {
  return profile_migration_strike_database_.get();
}

AutofillProfileSaveStrikeDatabase*
AddressDataManager::GetProfileSaveStrikeDatabase() {
  return const_cast<AutofillProfileSaveStrikeDatabase*>(
      std::as_const(*this).GetProfileSaveStrikeDatabase());
}

const AutofillProfileSaveStrikeDatabase*
AddressDataManager::GetProfileSaveStrikeDatabase() const {
  return profile_save_strike_database_.get();
}

AutofillProfileUpdateStrikeDatabase*
AddressDataManager::GetProfileUpdateStrikeDatabase() {
  return const_cast<AutofillProfileUpdateStrikeDatabase*>(
      std::as_const(*this).GetProfileUpdateStrikeDatabase());
}

const AutofillProfileUpdateStrikeDatabase*
AddressDataManager::GetProfileUpdateStrikeDatabase() const {
  return profile_update_strike_database_.get();
}

AddressSuggestionStrikeDatabase*
AddressDataManager::GetAddressSuggestionStrikeDatabase() {
  return const_cast<AddressSuggestionStrikeDatabase*>(
      std::as_const(*this).GetAddressSuggestionStrikeDatabase());
}

const AddressSuggestionStrikeDatabase*
AddressDataManager::GetAddressSuggestionStrikeDatabase() const {
  return address_suggestion_strike_database_.get();
}

bool AddressDataManager::IsAutofillProfileEnabled() const {
  return prefs::IsAutofillProfileEnabled(pref_service_);
}

void AddressDataManager::CancelPendingQuery(
    WebDataServiceBase::Handle& handle) {
  if (!webdata_service_ || !handle) {
    return;
  }
  webdata_service_->CancelRequest(handle);
  handle = 0;
}

const std::vector<std::unique_ptr<AutofillProfile>>&
AddressDataManager::GetProfileStorage(AutofillProfile::Source source) const {
  switch (source) {
    case AutofillProfile::Source::kLocalOrSyncable:
      return synced_local_profiles_;
    case AutofillProfile::Source::kAccount:
      return account_profiles_;
  }
  NOTREACHED();
}

void AddressDataManager::OnAutofillProfileChanged(
    const AutofillProfileChange& change) {
  const std::string& guid = change.key();
  const AutofillProfile& profile = change.data_model();
  DCHECK(guid == profile.guid());
  if (!ProfileChangesAreOngoing(guid)) {
    return;
  }

  std::vector<std::unique_ptr<AutofillProfile>>& profiles =
      GetProfileStorage(profile.source());
  const AutofillProfile* existing_profile = GetProfileByGUID(guid);
  switch (change.type()) {
    case AutofillProfileChange::ADD:
      if (!existing_profile &&
          base::ranges::none_of(profiles, [&](const auto& o) {
            return o->Compare(profile) == 0;
          })) {
        profiles.push_back(std::make_unique<AutofillProfile>(profile));
      }
      break;
    case AutofillProfileChange::UPDATE:
      if (existing_profile &&
          !existing_profile->EqualsForUpdatePurposes(profile)) {
        profiles.erase(
            base::ranges::find(profiles, existing_profile,
                               &std::unique_ptr<AutofillProfile>::get));
        profiles.push_back(std::make_unique<AutofillProfile>(profile));
      }
      break;
    case AutofillProfileChange::REMOVE:
      if (existing_profile) {
        profiles.erase(
            base::ranges::find(profiles, existing_profile,
                               &std::unique_ptr<AutofillProfile>::get));
      }
      break;
  }

  OnProfileChangeDone(guid);
}

void AddressDataManager::UpdateProfileInDB(const AutofillProfile& profile) {
  if (!ProfileChangesAreOngoing(profile.guid())) {
    const AutofillProfile* existing_profile = GetProfileByGUID(profile.guid());
    if (!existing_profile ||
        existing_profile->EqualsForUpdatePurposes(profile)) {
      notify_pdm_observers_.Run();
      return;
    }
  }

  ongoing_profile_changes_[profile.guid()].emplace_back(
      AutofillProfileChange(AutofillProfileChange::UPDATE, profile.guid(),
                            profile),
      /*is_ongoing=*/false);
  HandleNextProfileChange(profile.guid());
}

void AddressDataManager::HandleNextProfileChange(const std::string& guid) {
  if (!ProfileChangesAreOngoing(guid)) {
    return;
  }

  auto& [change, is_ongoing] = ongoing_profile_changes_[guid].front();
  if (is_ongoing) {
    return;
  }

  const AutofillProfile* existing_profile = GetProfileByGUID(guid);
  const AutofillProfile& profile = change.data_model();
  DCHECK(guid == profile.guid());

  switch (change.type()) {
    case AutofillProfileChange::REMOVE: {
      if (!existing_profile) {
        OnProfileChangeDone(guid);
        return;
      }
      webdata_service_->RemoveAutofillProfile(guid, existing_profile->source());
      break;
    }
    case AutofillProfileChange::ADD: {
      if (existing_profile ||
          base::ranges::any_of(
              GetProfileStorage(profile.source()),
              [&](const auto& o) { return o->Compare(profile) == 0; })) {
        OnProfileChangeDone(guid);
        return;
      }
      webdata_service_->AddAutofillProfile(profile);
      break;
    }
    case AutofillProfileChange::UPDATE: {
      if (!existing_profile ||
          existing_profile->EqualsForUpdatePurposes(profile)) {
        OnProfileChangeDone(guid);
        return;
      }
      // At this point, the `existing_profile` is consistent with
      // AutofillTable's state. Reset observations for all types that change due
      // to this update.
      AutofillProfile updated_profile = profile;
      updated_profile.token_quality().ResetObservationsForDifferingTokens(
          *existing_profile);
      // Unless only metadata has changed, which operator== ignores, update the
      // modification date. This happens e.g. when increasing the use count.
      if (*existing_profile != updated_profile) {
        updated_profile.set_modification_date(AutofillClock::Now());
      }
      webdata_service_->UpdateAutofillProfile(updated_profile);
      break;
    }
  }
  is_ongoing = true;
}

bool AddressDataManager::ProfileChangesAreOngoing(
    const std::string& guid) const {
  auto it = ongoing_profile_changes_.find(guid);
  return it != ongoing_profile_changes_.end() && !it->second.empty();
}

bool AddressDataManager::ProfileChangesAreOngoing() const {
  for (const auto& [guid, change] : ongoing_profile_changes_) {
    if (ProfileChangesAreOngoing(guid)) {
      return true;
    }
  }
  return false;
}

void AddressDataManager::OnProfileChangeDone(const std::string& guid) {
  ongoing_profile_changes_[guid].pop_front();
  notify_pdm_observers_.Run();
  HandleNextProfileChange(guid);
}

const std::string& AddressDataManager::MostCommonCountryCodeFromProfiles()
    const {
  // When `!IsAutofillProfileEnabled()`, `most_common_country_code_` is empty,
  // since it is reset by a pref observer. See `SetPrefService()`.
  if (!most_common_country_code_.empty() || !IsAutofillProfileEnabled()) {
    return most_common_country_code_;
  }
  // Count up country codes from existing profiles.
  std::map<std::string, int> votes;
  const std::vector<AutofillProfile*>& profiles = GetProfiles();
  const std::vector<std::string>& country_codes =
      CountryDataMap::GetInstance()->country_codes();
  for (const AutofillProfile* profile : profiles) {
    std::string country_code = base::ToUpperASCII(
        base::UTF16ToASCII(profile->GetRawInfo(ADDRESS_HOME_COUNTRY)));
    if (base::Contains(country_codes, country_code)) {
      votes[country_code]++;
    }
  }

  // Take the most common country code.
  if (!votes.empty()) {
    most_common_country_code_ =
        base::ranges::max_element(votes, [](auto& a, auto& b) {
          return a.second < b.second;
        })->first;
  }

  return most_common_country_code_;
}

void AddressDataManager::LogStoredDataMetrics() const {
  const std::vector<AutofillProfile*> profiles = GetProfiles();
  autofill_metrics::LogStoredProfileMetrics(profiles);
  autofill_metrics::LogStoredProfileTokenQualityMetrics(profiles);
  autofill_metrics::LogLocalProfileSupersetMetrics(std::move(profiles),
                                                   app_locale_);
}

}  // namespace autofill
