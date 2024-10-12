// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/quick_answers/read_write_cards_manager_impl.h"

#include <memory>
#include <string>
#include <vector>

#include "base/command_line.h"
#include "base/functional/callback.h"
#include "base/hash/sha1.h"
#include "base/strings/string_piece.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/ui/quick_answers/quick_answers_controller_impl.h"
#include "chrome/browser/ui/views/editor_menu/editor_menu_controller_impl.h"
#include "chrome/browser/ui/views/editor_menu/utils/editor_types.h"
#include "chrome/browser/ui/views/mahi/mahi_menu_controller.h"
#include "chromeos/components/quick_answers/public/cpp/quick_answers_state.h"
#include "chromeos/components/quick_answers/quick_answers_client.h"
#include "chromeos/constants/chromeos_features.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/context_menu_params.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "third_party/blink/public/mojom/context_menu/context_menu.mojom-shared.h"

namespace chromeos {

ReadWriteCardsManagerImpl::ReadWriteCardsManagerImpl()
    : quick_answers_controller_(
          std::make_unique<QuickAnswersControllerImpl>(ui_controller_)) {
  quick_answers_controller_->SetClient(
      std::make_unique<quick_answers::QuickAnswersClient>(
          g_browser_process->shared_url_loader_factory(),
          quick_answers_controller_->GetQuickAnswersDelegate()));

  if (chromeos::features::IsOrcaEnabled()) {
    editor_menu_controller_ =
        std::make_unique<editor_menu::EditorMenuControllerImpl>();
  }

  if (chromeos::features::IsMahiEnabled()) {
    mahi_menu_controller_.emplace(ui_controller_);
  }
}

ReadWriteCardsManagerImpl::~ReadWriteCardsManagerImpl() = default;

void ReadWriteCardsManagerImpl::FetchController(
    const content::ContextMenuParams& params,
    content::BrowserContext* context,
    editor_menu::FetchControllersCallback callback) {
  // Skip password input field.
  const bool is_password_field =
      params.form_control_type == blink::mojom::FormControlType::kInputPassword;
  if (is_password_field) {
    std::move(callback).Run({});
    return;
  }
  if (editor_menu_controller_ && params.is_editable) {
    editor_menu_controller_->SetBrowserContext(context);
    editor_menu_controller_->GetEditorMode(base::BindOnce(
        &ReadWriteCardsManagerImpl::OnGetEditorModeResult,
        weak_factory_.GetWeakPtr(), params, std::move(callback)));
    return;
  }

  std::move(callback).Run(GetMahiOrQuickAnswerControllersIfEligible(params));
}

void ReadWriteCardsManagerImpl::SetContextMenuBounds(
    const gfx::Rect& context_menu_bounds) {
  ui_controller_.SetContextMenuBounds(context_menu_bounds);
}

void ReadWriteCardsManagerImpl::OnGetEditorModeResult(
    const content::ContextMenuParams& params,
    editor_menu::FetchControllersCallback callback,
    editor_menu::EditorMode editor_mode) {
  if (editor_mode != editor_menu::EditorMode::kBlocked) {
    std::move(callback).Run({editor_menu_controller_->GetWeakPtr()});
    return;
  }
  editor_menu_controller_->LogEditorMode(editor_menu::EditorMode::kBlocked);
  std::move(callback).Run(GetMahiOrQuickAnswerControllersIfEligible(params));
}

std::vector<base::WeakPtr<chromeos::ReadWriteCardController>>
ReadWriteCardsManagerImpl::GetMahiOrQuickAnswerControllersIfEligible(
    const content::ContextMenuParams& params) {
  std::vector<base::WeakPtr<chromeos::ReadWriteCardController>> result;

  if (mahi_menu_controller_ && chromeos::features::IsMahiEnabled()) {
    result.emplace_back(mahi_menu_controller_->GetWeakPtr());
  }

  if (QuickAnswersState::Get()->is_eligible() &&
      !params.selection_text.empty() && quick_answers_controller_) {
    result.emplace_back(quick_answers_controller_->GetWeakPtr());
  }

  return result;
}

}  // namespace chromeos
