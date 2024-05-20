// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_AUTOFILL_AUTOFILL_CONTEXT_MENU_MANAGER_H_
#define CHROME_BROWSER_UI_AUTOFILL_AUTOFILL_CONTEXT_MENU_MANAGER_H_

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/types/strong_alias.h"
#include "components/autofill/content/browser/content_autofill_driver.h"
#include "components/autofill/core/common/unique_ids.h"
#include "components/renderer_context_menu/render_view_context_menu_base.h"
#include "components/renderer_context_menu/render_view_context_menu_observer.h"
#include "content/public/browser/context_menu_params.h"
#include "ui/base/models/simple_menu_model.h"

namespace password_manager {
class ContentPasswordManagerDriver;
}  // namespace password_manager

namespace autofill {

class AutofillField;
class PersonalDataManager;

// `AutofillContextMenuManager` is responsible for adding/executing Autofill
// related context menu items. `RenderViewContextMenu` is intended to own and
// control the lifetime of `AutofillContextMenuManager`.
// The options include:
// Provide Autofill feedback
// Fill in Form
class AutofillContextMenuManager : public RenderViewContextMenuObserver {
 public:
  // Represents command id used to denote a row in the context menu. The
  // command ids are created when the items are added to the context menu during
  // it's initialization.
  using CommandId = base::StrongAlias<class CommandIdTag, int>;

  AutofillContextMenuManager(PersonalDataManager* personal_data_manager,
                             RenderViewContextMenuBase* delegate,
                             ui::SimpleMenuModel* menu_model);
  ~AutofillContextMenuManager() override;
  AutofillContextMenuManager(const AutofillContextMenuManager&) = delete;
  AutofillContextMenuManager& operator=(const AutofillContextMenuManager&) =
      delete;

  // Adds items to the context menu.
  // Note: This doesn't use `RenderViewContextMenuObserver::InitMenu()`, since
  // Autofill context menu entries are conditioned on
  // `ContextMenuContentType::ITEM_GROUP_AUTOFILL`.
  void AppendItems();

  // `RenderViewContextMenuObserver` overrides.
  bool IsCommandIdSupported(int command_id) override;
  bool IsCommandIdEnabled(int command_id) override;
  void ExecuteCommand(int command_id) override;

  // Setter for `params_` used for testing purposes.
  void set_params_for_testing(content::ContextMenuParams params) {
    params_ = params;
  }

 private:
  // Conditionally adds the feedback manual fallback item if Autofill is
  // available for the field.
  void MaybeAddAutofillFeedbackItem();

  // Conditionally adds the address, payments and / or passwords Autofill manual
  // fallbacks to the context menu model depending on whether there's data to
  // suggest.
  void MaybeAddAutofillManualFallbackItems();

  // Checks if the plus address context menu entry can be shown for the
  // currently focused field.
  bool ShouldAddPlusAddressManualFallbackItem(
      ContentAutofillDriver& autofill_driver);

  // Checks if the manual fallback context menu entry can be shown for the
  // currently focused field.
  bool ShouldAddAddressManualFallbackItem(
      ContentAutofillDriver& autofill_driver);

  // Checks if the currently focused field is a password field and whether
  // password filling is enabled.
  bool ShouldAddPasswordsManualFallbackItem(
      password_manager::ContentPasswordManagerDriver& password_manager_driver);

  // Adds the passwords manual fallback context menu entries.
  // If the user has passwords saved, display "Select password" (if the user
  // is syncing) or "Passwords" (if the user is not syncing) option.
  // The latter doesn't open a submenu, instead it behaves like the "Select
  // password" entry.
  // If the user doesn't have passwords saved, display "Import passwords".
  // Additionally, a syncing user will have a "Suggest password" entry.
  void AddPasswordsManualFallbackItems(
      password_manager::ContentPasswordManagerDriver& password_manager_driver);

  // Emits metrics about showing the manual fallback context menu entries to the
  // user.
  // `address_option_shown` specifies whether address manual fallback was
  // available, same for `payments_option_shown`.
  void LogManualFallbackContextMenuEntryShown(
      ContentAutofillDriver* autofill_driver,
      bool address_option_shown,
      bool payments_option_shown);

  // Emits metrics about accepting the manual fallback context menu entries
  // shown to the user. `filling_product` defines which manual fallback option
  // was accepted.
  void LogManualFallbackContextMenuEntryAccepted(
      AutofillDriver& autofill_driver,
      const FillingProduct filling_product);

  // Triggers the feedback flow for Autofill command.
  void ExecuteAutofillFeedbackCommand(const LocalFrameToken& frame_token,
                                      AutofillManager& manager);

  // Triggers Plus Address suggestions on the field that the context menu was
  // opened on.
  void ExecuteFallbackForPlusAddressesCommand(AutofillDriver& driver);

  // Triggers Autofill payments suggestions on the field that the context menu
  // was opened on.
  void ExecuteFallbackForPaymentsCommand(AutofillDriver& driver);

  // Triggers passwords suggestions on the field that the context menu was
  // opened on.
  void ExecuteFallbackForPasswordsCommand(AutofillDriver& driver);

  // Triggers Autofill address suggestions on the field that the context menu
  // was opened on.
  void ExecuteFallbackForAddressesCommand(
      ContentAutofillDriver& autofill_driver);

  // Gets the `AutofillField` described by the `params_` from the `manager`.
  // The `frame_token` is used to map from the `params_` renderer id to a global
  // id.
  AutofillField* GetAutofillField(AutofillManager& manager,
                                  const LocalFrameToken& frame_token) const;

  // Dangling on linux-lacros-rel in:
  // AutofillContextMenuManagerFeedbackUILacrosBrowserTest
  //   .CloseTabWhileUIIsOpenShouldNotCrash.
  const raw_ptr<PersonalDataManager, DanglingUntriaged> personal_data_manager_;
  const raw_ptr<ui::SimpleMenuModel> menu_model_;
  const raw_ptr<RenderViewContextMenuBase> delegate_;
  ui::SimpleMenuModel passwords_submenu_model_;
  content::ContextMenuParams params_;

  base::WeakPtrFactory<AutofillContextMenuManager> weak_ptr_factory_{this};
};

}  // namespace autofill

#endif  // CHROME_BROWSER_UI_AUTOFILL_AUTOFILL_CONTEXT_MENU_MANAGER_H_
