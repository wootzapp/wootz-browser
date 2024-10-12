// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/metrics/profile_import_metrics.h"

#include "base/containers/contains.h"
#include "base/metrics/histogram_functions.h"
#include "base/strings/strcat.h"
#include "components/autofill/core/browser/metrics/autofill_metrics_utils.h"
#include "components/autofill/core/browser/profile_requirement_utils.h"
#include "services/metrics/public/cpp/ukm_builders.h"

namespace autofill::autofill_metrics {

namespace {

const char* GetAddressPromptDecisionMetricsSuffix(
    AutofillClient::AddressPromptUserDecision decision) {
  switch (decision) {
    case AutofillClient::AddressPromptUserDecision::kUndefined:
      return ".Undefined";
    case AutofillClient::AddressPromptUserDecision::kUserNotAsked:
      return ".UserNotAsked";
    case AutofillClient::AddressPromptUserDecision::kAccepted:
      return ".Accepted";
    case AutofillClient::AddressPromptUserDecision::kDeclined:
      return ".Declined";
    case AutofillClient::AddressPromptUserDecision::kEditAccepted:
      return ".EditAccepted";
    case AutofillClient::AddressPromptUserDecision::kEditDeclined:
      return ".EditDeclined";
    case AutofillClient::AddressPromptUserDecision::kNever:
      return ".Never";
    case AutofillClient::AddressPromptUserDecision::kIgnored:
      return ".Ignored";
    case AutofillClient::AddressPromptUserDecision::kMessageTimeout:
      return ".MessageTimeout";
    case AutofillClient::AddressPromptUserDecision::kMessageDeclined:
      return ".MessageDeclined";
    case AutofillClient::AddressPromptUserDecision::kAutoDeclined:
      return ".AutoDeclined";
  }
  NOTREACHED_IN_MIGRATION();
  return "";
}

}  // namespace

void LogAddressProfileImportUkm(
    ukm::UkmRecorder* ukm_recorder,
    ukm::SourceId source_id,
    AutofillProfileImportType import_type,
    AutofillClient::AddressPromptUserDecision user_decision,
    const ProfileImportMetadata& profile_import_metadata,
    size_t num_edited_fields) {
  ukm::builders::Autofill_AddressProfileImport(source_id)
      .SetAutocompleteUnrecognizedImport(
          profile_import_metadata
              .did_import_from_unrecognized_autocomplete_field)
      .SetImportType(static_cast<int64_t>(import_type))
      .SetNumberOfEditedFields(num_edited_fields)
      .SetPhoneNumberStatus(
          static_cast<int64_t>(profile_import_metadata.phone_import_status))
      .SetUserDecision(static_cast<int64_t>(user_decision))
      .Record(ukm_recorder);
}

void LogAddressFormImportRequirementMetric(
    AddressProfileImportRequirementMetric metric) {
  base::UmaHistogramEnumeration("Autofill.AddressProfileImportRequirements",
                                metric);
}

void LogAddressFormImportRequirementMetric(const AutofillProfile& profile) {
  std::vector<AddressProfileImportRequirementMetric> requirements =
      ValidateProfileImportRequirements(profile);
  for (AddressProfileImportRequirementMetric& requirement : requirements) {
    LogAddressFormImportRequirementMetric(requirement);
  }

  bool is_zip_missing = base::Contains(
      requirements,
      AddressProfileImportRequirementMetric::kZipRequirementViolated);
  bool is_state_missing = base::Contains(
      requirements,
      AddressProfileImportRequirementMetric::kStateRequirementViolated);
  bool is_city_missing = base::Contains(
      requirements,
      AddressProfileImportRequirementMetric::kCityRequirementViolated);
  bool is_line1_missing = base::Contains(
      requirements,
      AddressProfileImportRequirementMetric::kLine1RequirementViolated);
  const auto metric =
      static_cast<AddressProfileImportCountrySpecificFieldRequirementsMetric>(
          (is_zip_missing ? 0b1 : 0) | (is_state_missing ? 0b10 : 0) |
          (is_city_missing ? 0b100 : 0) | (is_line1_missing ? 0b1000 : 0));
  base::UmaHistogramEnumeration(
      "Autofill.AddressProfileImportCountrySpecificFieldRequirements", metric);
}

void LogAddressFormImportStatusMetric(AddressProfileImportStatusMetric metric) {
  base::UmaHistogramEnumeration("Autofill.AddressProfileImportStatus", metric);
}

void LogProfileImportType(AutofillProfileImportType import_type) {
  base::UmaHistogramEnumeration("Autofill.ProfileImport.ProfileImportType",
                                import_type);
}

void LogSilentUpdatesProfileImportType(AutofillProfileImportType import_type) {
  base::UmaHistogramEnumeration(
      "Autofill.ProfileImport.SilentUpdatesProfileImportType", import_type);
}

void LogNewProfileImportDecision(
    AutofillClient::AddressPromptUserDecision decision) {
  base::UmaHistogramEnumeration("Autofill.ProfileImport.NewProfileDecision",
                                decision);
}

void LogProfileUpdateImportDecision(
    AutofillClient::AddressPromptUserDecision decision) {
  base::UmaHistogramEnumeration("Autofill.ProfileImport.UpdateProfileDecision",
                                decision);
}

// static
void LogRemovedSettingInaccessibleFields(bool did_remove) {
  base::UmaHistogramBoolean(
      "Autofill.ProfileImport.InaccessibleFieldsRemoved.Total", did_remove);
}

// static
void LogRemovedSettingInaccessibleField(FieldType field) {
  base::UmaHistogramEnumeration(
      "Autofill.ProfileImport.InaccessibleFieldsRemoved.ByFieldType",
      ConvertSettingsVisibleFieldTypeForMetrics(field));
}

// static
void LogPhoneNumberImportParsingResult(bool parsed_successfully) {
  base::UmaHistogramBoolean("Autofill.ProfileImport.PhoneNumberParsed",
                            parsed_successfully);
}

void LogNewProfileEditedType(FieldType edited_type) {
  base::UmaHistogramEnumeration(
      "Autofill.ProfileImport.NewProfileEditedType",
      ConvertSettingsVisibleFieldTypeForMetrics(edited_type));
}

void LogProfileUpdateAffectedType(
    FieldType affected_type,
    AutofillClient::AddressPromptUserDecision decision) {
  // Record the decision-specific metric.
  base::UmaHistogramEnumeration(
      base::StrCat({"Autofill.ProfileImport.UpdateProfileAffectedType",
                    GetAddressPromptDecisionMetricsSuffix(decision)}),
      ConvertSettingsVisibleFieldTypeForMetrics(affected_type));

  // But also collect an histogram for any decision.
  base::UmaHistogramEnumeration(
      "Autofill.ProfileImport.UpdateProfileAffectedType.Any",
      ConvertSettingsVisibleFieldTypeForMetrics(affected_type));
}

void LogProfileUpdateEditedType(FieldType edited_type) {
  base::UmaHistogramEnumeration(
      "Autofill.ProfileImport.UpdateProfileEditedType",
      ConvertSettingsVisibleFieldTypeForMetrics(edited_type));
}

void LogUpdateProfileNumberOfAffectedFields(
    int number_of_edited_fields,
    AutofillClient::AddressPromptUserDecision decision) {
  // Record the decision-specific metric.
  base::UmaHistogramExactLinear(
      base::StrCat(
          {"Autofill.ProfileImport.UpdateProfileNumberOfAffectedFields",
           GetAddressPromptDecisionMetricsSuffix(decision)}),
      number_of_edited_fields, /*exclusive_max=*/15);

  // But also collect an histogram for any decision.
  base::UmaHistogramExactLinear(
      "Autofill.ProfileImport.UpdateProfileNumberOfAffectedFields.Any",
      number_of_edited_fields, /*exclusive_max=*/15);
}

void LogProfileMigrationImportDecision(
    AutofillClient::AddressPromptUserDecision decision) {
  base::UmaHistogramEnumeration("Autofill.ProfileImport.MigrateProfileDecision",
                                decision);
}

void LogProfileMigrationEditedType(FieldType edited_type) {
  base::UmaHistogramEnumeration(
      "Autofill.ProfileImport.MigrateProfileEditedType",
      ConvertSettingsVisibleFieldTypeForMetrics(edited_type));
}

}  // namespace autofill::autofill_metrics
