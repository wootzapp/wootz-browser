// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/tabs/tab_group_deletion_dialog_controller.h"

#include <memory>
#include <string>

#include "base/functional/callback.h"
#include "base/no_destructor.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_dialogs.h"
#include "chrome/browser/ui/browser_element_identifiers.h"
#include "chrome/browser/ui/tabs/saved_tab_groups/saved_tab_group_keyed_service.h"
#include "chrome/browser/ui/tabs/saved_tab_groups/saved_tab_group_pref_names.h"
#include "chrome/browser/ui/tabs/saved_tab_groups/saved_tab_group_service_factory.h"
#include "chrome/common/pref_names.h"
#include "chrome/grit/generated_resources.h"
#include "components/prefs/pref_service.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/models/dialog_model.h"

namespace tab_groups {
DEFINE_LOCAL_ELEMENT_IDENTIFIER_VALUE(kDeletionDialogDontAskCheckboxId);
DEFINE_LOCAL_ELEMENT_IDENTIFIER_VALUE(kDeletionDialogCancelButtonId);
DEFINE_LOCAL_ELEMENT_IDENTIFIER_VALUE(kDeletionDialogOkButtonId);

namespace {
// The text that shows on the checkbox.
constexpr int kDontAskId = IDS_TAB_GROUP_DELETION_DIALOG_DONT_ASK;

// Body text for all delete actions.
constexpr int kDeleteBodySyncedId =
    IDS_TAB_GROUP_DELETION_DIALOG_BODY_SYNCED_DELETE;
constexpr int kDeleteBodyNotSyncedId =
    IDS_TAB_GROUP_DELETION_DIALOG_BODY_NOT_SYNCED_DELETE;

// For deletion, the text that shows on the dialog
constexpr int kDeleteTitleId = IDS_TAB_GROUP_DELETION_DIALOG_TITLE_DELETE;
constexpr int kDeleteOkTextId = IDS_TAB_GROUP_DELETION_DIALOG_OK_TEXT_DELETE;

// For ungrouping, the text that shows on the dialog.
constexpr int kUngroupTitleId = IDS_TAB_GROUP_DELETION_DIALOG_TITLE_UNGROUP;
constexpr int kUngroupSyncedBodyId =
    IDS_TAB_GROUP_DELETION_DIALOG_BODY_SYNCED_UNGROUP;
constexpr int kUngroupNotSyncedBodyId =
    IDS_TAB_GROUP_DELETION_DIALOG_BODY_NOT_SYNCED_UNGROUP;
constexpr int kUngroupOkTextId = IDS_TAB_GROUP_DELETION_DIALOG_OK_TEXT_UNGROUP;

// For closing the last tab, the text that shows on the dialog.
constexpr int kCloseTabAndDeleteTitleId =
    IDS_TAB_GROUP_DELETION_DIALOG_TITLE_CLOSE_TAB_AND_DELETE;
constexpr int kCloseTabAndDeleteOkTextId =
    IDS_TAB_GROUP_DELETION_DIALOG_OK_TEXT_DELETE;

// For removing the last tab, the text that shows on the dialog.
constexpr int kRemoveTabAndDeleteTitleId =
    IDS_TAB_GROUP_DELETION_DIALOG_TITLE_REMOVE_TAB_AND_DELETE;
constexpr int kRemoveTabAndDeleteOkTextId =
    IDS_TAB_GROUP_DELETION_DIALOG_OK_TEXT_DELETE;

struct DialogText {
  const std::u16string title;
  const std::u16string body;
  const std::u16string ok_text;
};

// Returns the list of strings that are needed for a given dialog type.
DialogText GetDialogText(Profile* profile,
                         DeletionDialogController::DialogType type) {
  tab_groups::SavedTabGroupKeyedService* const saved_tab_group_service =
      tab_groups::SavedTabGroupServiceFactory::GetForProfile(profile);
  bool is_sync_enabled = saved_tab_group_service &&
                         saved_tab_group_service->AreSavedTabGroupsSynced();

  switch (type) {
    case DeletionDialogController::DialogType::DeleteSingle: {
      return DialogText{
          l10n_util::GetStringUTF16(kDeleteTitleId),
          l10n_util::GetStringUTF16(is_sync_enabled ? kDeleteBodySyncedId
                                                    : kDeleteBodyNotSyncedId),
          l10n_util::GetStringUTF16(kDeleteOkTextId)};
    }
    case DeletionDialogController::DialogType::UngroupSingle: {
      return DialogText{
          l10n_util::GetStringUTF16(kUngroupTitleId),
          l10n_util::GetStringUTF16(is_sync_enabled ? kUngroupSyncedBodyId
                                                    : kUngroupNotSyncedBodyId),
          l10n_util::GetStringUTF16(kUngroupOkTextId)};
    }
    case DeletionDialogController::DialogType::RemoveTabAndDelete: {
      return DialogText{
          l10n_util::GetStringUTF16(kRemoveTabAndDeleteTitleId),
          l10n_util::GetStringUTF16(is_sync_enabled ? kDeleteBodySyncedId
                                                    : kDeleteBodyNotSyncedId),
          l10n_util::GetStringUTF16(kRemoveTabAndDeleteOkTextId)};
    }
    case DeletionDialogController::DialogType::CloseTabAndDelete: {
      return DialogText{
          l10n_util::GetStringUTF16(kCloseTabAndDeleteTitleId),
          l10n_util::GetStringUTF16(is_sync_enabled ? kDeleteBodySyncedId
                                                    : kDeleteBodyNotSyncedId),
          l10n_util::GetStringUTF16(kCloseTabAndDeleteOkTextId)};
    }
  }
}

// Returns the the value from the settings pref for a given dialog type.
bool IsDialogSkippedByUserSettings(Profile* profile,
                                   DeletionDialogController::DialogType type) {
  if (!profile) {
    return false;
  }
  PrefService* pref_service = profile->GetPrefs();
  switch (type) {
    case DeletionDialogController::DialogType::DeleteSingle: {
      return pref_service->GetBoolean(
          saved_tab_groups::prefs::kTabGroupsDeletionSkipDialogOnDelete);
    }
    case DeletionDialogController::DialogType::UngroupSingle: {
      return pref_service->GetBoolean(
          saved_tab_groups::prefs::kTabGroupsDeletionSkipDialogOnUngroup);
    }
    case DeletionDialogController::DialogType::RemoveTabAndDelete: {
      return pref_service->GetBoolean(
          saved_tab_groups::prefs::kTabGroupsDeletionSkipDialogOnRemoveTab);
    }
    case DeletionDialogController::DialogType::CloseTabAndDelete: {
      return pref_service->GetBoolean(
          saved_tab_groups::prefs::kTabGroupsDeletionSkipDialogOnCloseTab);
    }
  }
}

void SetSkipDialogForType(Profile* profile,
                          DeletionDialogController::DialogType type,
                          bool new_value) {
  if (!profile) {
    return;
  }

  PrefService* pref_service = profile->GetPrefs();
  switch (type) {
    case DeletionDialogController::DialogType::DeleteSingle: {
      return pref_service->SetBoolean(
          saved_tab_groups::prefs::kTabGroupsDeletionSkipDialogOnDelete,
          new_value);
    }
    case DeletionDialogController::DialogType::UngroupSingle: {
      return pref_service->SetBoolean(
          saved_tab_groups::prefs::kTabGroupsDeletionSkipDialogOnUngroup,
          new_value);
    }
    case DeletionDialogController::DialogType::RemoveTabAndDelete: {
      return pref_service->SetBoolean(
          saved_tab_groups::prefs::kTabGroupsDeletionSkipDialogOnRemoveTab,
          new_value);
    }
    case DeletionDialogController::DialogType::CloseTabAndDelete: {
      return pref_service->SetBoolean(
          saved_tab_groups::prefs::kTabGroupsDeletionSkipDialogOnCloseTab,
          new_value);
    }
  }
}

}  // anonymous namespace

DeletionDialogController::DialogState::DialogState(
    DialogType type_,
    ui::DialogModel* dialog_model_,
    base::OnceCallback<void()> on_ok_button_pressed_,
    base::OnceCallback<void()> on_cancel_button_pressed_)
    : type(type_),
      dialog_model(dialog_model_),
      on_ok_button_pressed(std::move(on_ok_button_pressed_)),
      on_cancel_button_pressed(std::move(on_cancel_button_pressed_)) {}

DeletionDialogController::DialogState::~DialogState() = default;

DeletionDialogController::DeletionDialogController(Browser* browser)
    : browser_(browser) {}

DeletionDialogController::~DeletionDialogController() = default;

bool DeletionDialogController::CanShowDialog() {
  return !IsShowingDialog();
}

bool DeletionDialogController::IsShowingDialog() {
  return state_ != nullptr;
}

bool DeletionDialogController::MaybeShowDialog(
    DialogType type,
    base::OnceCallback<void()> on_ok_callback) {
  if (!CanShowDialog()) {
    return false;
  }

  if (IsDialogSkippedByUserSettings(browser_->profile(), type)) {
    return false;
  }

  std::unique_ptr<ui::DialogModel> dialog_model = BuildDialogModel(type);

  state_ = std::make_unique<DeletionDialogController::DialogState>(
      type, dialog_model.get(), std::move(on_ok_callback), base::DoNothing());

  chrome::ShowBrowserModal(browser_, std::move(dialog_model));
  return true;
}

void DeletionDialogController::OnDialogOk() {
  if (state_->dialog_model &&
      state_->dialog_model
          ->GetCheckboxByUniqueId(kDeletionDialogDontAskCheckboxId)
          ->is_checked()) {
    SetSkipDialogForType(browser_->profile(), state_->type, true);
  }
  std::move(state_->on_ok_button_pressed).Run();
  state_.reset();
}

void DeletionDialogController::OnDialogCancel() {
  std::move(state_->on_cancel_button_pressed).Run();
  state_.reset();
}

std::unique_ptr<ui::DialogModel> DeletionDialogController::BuildDialogModel(
    DialogType type) {
  DialogText strings = GetDialogText(browser_->profile(), type);

  return ui::DialogModel::Builder()
      .SetTitle(strings.title)
      .AddParagraph(ui::DialogModelLabel(strings.body))
      .AddCheckbox(kDeletionDialogDontAskCheckboxId,
                   ui::DialogModelLabel(l10n_util::GetStringUTF16(kDontAskId)))
      .AddCancelButton(base::BindOnce(&DeletionDialogController::OnDialogCancel,
                                      base::Unretained(this)),
                       ui::DialogModel::Button::Params().SetEnabled(true).SetId(
                           kDeletionDialogCancelButtonId))
      .AddOkButton(base::BindOnce(&DeletionDialogController::OnDialogOk,
                                  base::Unretained(this)),
                   ui::DialogModel::Button::Params()
                       .SetLabel(strings.ok_text)
                       .SetEnabled(true)
                       .SetId(kDeletionDialogOkButtonId))
      .SetCloseActionCallback(base::BindOnce(
          [](DeletionDialogController* dialog_controller) {
            dialog_controller->state_.reset();
          },
          base::Unretained(this)))
      .Build();
}

}  // namespace tab_groups
