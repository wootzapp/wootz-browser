// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/permissions/embedded_permission_prompt_previously_denied_view.h"

#include "base/memory/weak_ptr.h"

#include "chrome/browser/ui/url_identity.h"
#include "components/permissions/features.h"
#include "components/strings/grit/components_strings.h"
#include "components/vector_icons/vector_icons.h"
#include "ui/base/l10n/l10n_util.h"

DEFINE_CLASS_ELEMENT_IDENTIFIER_VALUE(
    EmbeddedPermissionPromptPreviouslyDeniedView,
    kAllowThisTimeId);

EmbeddedPermissionPromptPreviouslyDeniedView::
    EmbeddedPermissionPromptPreviouslyDeniedView(
        Browser* browser,
        base::WeakPtr<EmbeddedPermissionPromptViewDelegate> delegate)
    : EmbeddedPermissionPromptBaseView(browser, delegate) {}

EmbeddedPermissionPromptPreviouslyDeniedView::
    ~EmbeddedPermissionPromptPreviouslyDeniedView() = default;

std::u16string
EmbeddedPermissionPromptPreviouslyDeniedView::GetAccessibleWindowTitle() const {
  return GetWindowTitle();
}

std::u16string EmbeddedPermissionPromptPreviouslyDeniedView::GetWindowTitle()
    const {
  return l10n_util::GetStringUTF16(IDS_EMBEDDED_PROMPT_PREVIOUSLY_NOT_ALLOWED);
}

void EmbeddedPermissionPromptPreviouslyDeniedView::RunButtonCallback(
    int button_id) {
  if (!delegate()) {
    return;
  }

  ButtonType button = GetButtonType(button_id);

  if (button == ButtonType::kContinueNotAllowing) {
    delegate()->Acknowledge();
  }

  if (button == ButtonType::kAllowThisTime) {
    delegate()->AllowThisTime();
  }

  if (button == ButtonType::kAllow) {
    delegate()->Allow();
  }
}

std::vector<
    EmbeddedPermissionPromptPreviouslyDeniedView::RequestLineConfiguration>
EmbeddedPermissionPromptPreviouslyDeniedView::GetRequestLinesConfiguration()
    const {
  return {};
}

std::vector<EmbeddedPermissionPromptPreviouslyDeniedView::ButtonConfiguration>
EmbeddedPermissionPromptPreviouslyDeniedView::GetButtonsConfiguration() const {
  std::vector<ButtonConfiguration> buttons;
  buttons.emplace_back(
      l10n_util::GetStringUTF16(IDS_EMBEDDED_PROMPT_CONTINUE_NOT_ALLOWING),
      ButtonType::kContinueNotAllowing, ui::ButtonStyle::kTonal);

    buttons.emplace_back(
        l10n_util::GetStringUTF16(IDS_PERMISSION_ALLOW_THIS_TIME),
        ButtonType::kAllowThisTime, ui::ButtonStyle::kTonal, kAllowThisTimeId);

  return buttons;
}
