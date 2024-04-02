// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_BROWSER_UI_MOCK_AUTOFILL_POPUP_DELEGATE_H_
#define COMPONENTS_AUTOFILL_CORE_BROWSER_UI_MOCK_AUTOFILL_POPUP_DELEGATE_H_

#include <optional>

#include "base/functional/callback.h"
#include "base/memory/weak_ptr.h"
#include "components/autofill/core/browser/ui/autofill_popup_delegate.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace autofill {

// Mock version of AutofillPopupDelegate.
class MockAutofillPopupDelegate : public AutofillPopupDelegate {
 public:
  MockAutofillPopupDelegate();
  ~MockAutofillPopupDelegate() override;

  MOCK_METHOD((absl::variant<AutofillDriver*,
                             password_manager::PasswordManagerDriver*>),
              GetDriver,
              (),
              (override));
  MOCK_METHOD(void, OnPopupShown, (), (override));
  MOCK_METHOD(void, OnPopupHidden, (), (override));
  MOCK_METHOD(void,
              DidSelectSuggestion,
              (const Suggestion& suggestion),
              (override));
  MOCK_METHOD(void,
              DidAcceptSuggestion,
              (const Suggestion& suggestion,
               const AutofillPopupDelegate::SuggestionPosition& position),
              (override));
  MOCK_METHOD(void,
              DidPerformButtonActionForSuggestion,
              (const Suggestion&),
              (override));
  MOCK_METHOD(bool, RemoveSuggestion, (const Suggestion&), (override));
  MOCK_METHOD(void, ClearPreviewedForm, (), (override));
  MOCK_METHOD(FillingProduct, GetMainFillingProduct, (), (const, override));

  base::WeakPtr<MockAutofillPopupDelegate> GetWeakPtr();

 private:
  base::WeakPtrFactory<MockAutofillPopupDelegate> weak_ptr_factory_{this};
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_BROWSER_UI_MOCK_AUTOFILL_POPUP_DELEGATE_H_
