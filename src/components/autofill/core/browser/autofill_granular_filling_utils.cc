// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/autofill_granular_filling_utils.h"

#include "components/autofill/core/browser/autofill_type.h"
#include "components/autofill/core/browser/field_types.h"

namespace autofill {

namespace {

FieldTypeSet GetServerFieldsForFieldGroup(FieldTypeGroup group) {
  switch (group) {
    case FieldTypeGroup::kName:
      return GetFieldTypesOfGroup(FieldTypeGroup::kName);
    case FieldTypeGroup::kAddress:
    case FieldTypeGroup::kCompany:
      return GetAddressFieldsForGroupFilling();
    case FieldTypeGroup::kPhone:
      return GetFieldTypesOfGroup(FieldTypeGroup::kPhone);
    case FieldTypeGroup::kEmail:
      return GetFieldTypesOfGroup(FieldTypeGroup::kEmail);
    case FieldTypeGroup::kNoGroup:
    case FieldTypeGroup::kCreditCard:
    case FieldTypeGroup::kPasswordField:
    case FieldTypeGroup::kTransaction:
    case FieldTypeGroup::kUsernameField:
    case FieldTypeGroup::kUnfillable:
    case FieldTypeGroup::kIban:
      // If `group` is not one of the groups we offer group filling for
      // (name, address and phone field), we default back to fill full form
      // behaviour/pre-granular filling.
      return kAllFieldTypes;
  }
}

}  // namespace

AutofillFillingMethod GetFillingMethodFromTargetedFields(
    const FieldTypeSet& targeted_field_types) {
  if (targeted_field_types == kAllFieldTypes) {
    return AutofillFillingMethod::kFullForm;
  }
  if (AreFieldsGranularFillingGroup(targeted_field_types)) {
    return AutofillFillingMethod::kGroupFilling;
  }
  if (targeted_field_types.size() == 1) {
    return AutofillFillingMethod::kFieldByFieldFilling;
  }
  return AutofillFillingMethod::kNone;
}

FieldTypeSet GetAddressFieldsForGroupFilling() {
  FieldTypeSet fields = GetFieldTypesOfGroup(FieldTypeGroup::kAddress);
  fields.insert_all(GetFieldTypesOfGroup(FieldTypeGroup::kCompany));
  return fields;
}

bool AreFieldsGranularFillingGroup(const FieldTypeSet& field_types) {
  return field_types == GetAddressFieldsForGroupFilling() ||
         field_types == GetFieldTypesOfGroup(FieldTypeGroup::kName) ||
         field_types == GetFieldTypesOfGroup(FieldTypeGroup::kEmail) ||
         field_types == GetFieldTypesOfGroup(FieldTypeGroup::kPhone);
}

FieldTypeSet GetTargetServerFieldsForTypeAndLastTargetedFields(
    const FieldTypeSet& last_targeted_field_types,
    FieldType triggering_field_type) {
  switch (GetFillingMethodFromTargetedFields(last_targeted_field_types)) {
    case AutofillFillingMethod::kGroupFilling:
      return GetServerFieldsForFieldGroup(
          GroupTypeOfFieldType(triggering_field_type));
    case AutofillFillingMethod::kFullForm:
      return kAllFieldTypes;
    case AutofillFillingMethod::kFieldByFieldFilling:
      return {triggering_field_type};
    case AutofillFillingMethod::kNone:
      break;
  }
  NOTREACHED_NORETURN();
}

}  // namespace autofill
