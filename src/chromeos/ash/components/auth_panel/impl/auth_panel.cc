// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/ash/components/auth_panel/impl/auth_panel.h"

#include <memory>
#include <vector>

#include "ash/login/ui/non_accessible_view.h"
#include "ash/strings/grit/ash_strings.h"
#include "ash/style/ash_color_id.h"
#include "base/logging.h"
#include "base/notreached.h"
#include "chromeos/ash/components/auth_panel/impl/auth_factor_store.h"
#include "chromeos/ash/components/auth_panel/impl/auth_panel_event_dispatcher.h"
#include "chromeos/ash/components/auth_panel/impl/factor_auth_view.h"
#include "chromeos/ash/components/auth_panel/impl/factor_auth_view_factory.h"
#include "chromeos/ash/components/osauth/public/auth_factor_status_consumer.h"
#include "chromeos/ash/components/osauth/public/common_types.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/views/bubble/bubble_border.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/toggle_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/flex_layout.h"
#include "ui/views/layout/layout_types.h"

namespace ash {

AuthPanel::AuthPanel(
    std::unique_ptr<FactorAuthViewFactory> view_factory,
    std::unique_ptr<AuthFactorStoreFactory> store_factory,
    std::unique_ptr<AuthPanelEventDispatcherFactory> event_dispatcher_factory,
    base::OnceClosure on_end_authentication,
    base::RepeatingClosure on_prefered_size_changed,
    AuthHubConnector* connector)
    : event_dispatcher_factory_(std::move(event_dispatcher_factory)),
      view_factory_(std::move(view_factory)),
      store_factory_(std::move(store_factory)),
      on_end_authentication_(std::move(on_end_authentication)),
      on_preferred_size_changed_(std::move(on_prefered_size_changed)),
      auth_hub_connector_(connector) {
  SetLayoutManager(std::make_unique<views::FlexLayout>())
      ->SetOrientation(views::LayoutOrientation::kVertical)
      .SetMainAxisAlignment(views::LayoutAlignment::kCenter)
      .SetCrossAxisAlignment(views::LayoutAlignment::kCenter)
      .SetCollapseMargins(false);

  InitializeViewPlaceholders();
}

AuthPanel::~AuthPanel() = default;

void AuthPanel::InitializeViewPlaceholders() {
  // The order in which the views will be laid out in AuthPanel. We create
  // placeholder spots for them in the UI beforehand, and fill them out later
  // depending on the presence of factors.
  std::vector<AshAuthFactor> view_order{
      AshAuthFactor::kGaiaPassword,
  };

  for (AshAuthFactor factor : view_order) {
    views_[factor] = AddChildView(std::make_unique<NonAccessibleView>());
    views_[factor]->SetLayoutManager(std::make_unique<views::FlexLayout>());
  }
}

void AuthPanel::InitializeUi(AuthFactorsSet factors,
                             AuthHubConnector* connector) {
  bool has_local_password = factors.Has(AshAuthFactor::kLocalPassword);
  bool has_gaia_password = factors.Has(AshAuthFactor::kGaiaPassword);

  // We currently only support having either local passwords or gaia passwords,
  // but not both.
  CHECK(!has_local_password || !has_gaia_password);

  std::optional<AshAuthFactor> password_type = std::nullopt;

  if (has_local_password) {
    password_type = AshAuthFactor::kLocalPassword;
  } else if (has_gaia_password) {
    password_type = AshAuthFactor::kGaiaPassword;
  }

  store_ =
      store_factory_->CreateAuthFactorStore(ash::Shell::Get(), connector,
                                            /*password_type=*/password_type);
  event_dispatcher_ =
      event_dispatcher_factory_->CreateAuthPanelEventDispatcher(store_.get());

  for (auto&& factor : factors) {
    views_[factor]->AddChildView(view_factory_->CreateFactorAuthView(
        factor, store_.get(), event_dispatcher_.get()));
    event_dispatcher_->DispatchEvent(factor,
                                     AuthFactorState::kCheckingForPresence);
  }

  on_preferred_size_changed_.Run();
}

void AuthPanel::OnFactorListChanged(FactorsStatusMap factors_with_status) {
  for (const auto& [factor, status] : factors_with_status) {
    event_dispatcher_->DispatchEvent(factor, status);
  }
}

void AuthPanel::OnFactorStatusesChanged(FactorsStatusMap incremental_update) {
  for (const auto& [factor, status] : incremental_update) {
    event_dispatcher_->DispatchEvent(factor, status);
  }
}

void AuthPanel::OnFactorCustomSignal(AshAuthFactor factor) {
  NOTIMPLEMENTED();
}

void AuthPanel::OnFactorAuthFailure(AshAuthFactor factor) {
  event_dispatcher_->DispatchEvent(
      factor, AuthPanelEventDispatcher::AuthVerdict::kFailure);
}

void AuthPanel::OnFactorAuthSuccess(AshAuthFactor factor) {
  event_dispatcher_->DispatchEvent(
      factor, AuthPanelEventDispatcher::AuthVerdict::kSuccess);
}

void AuthPanel::OnEndAuthentication() {
  auth_hub_connector_ = nullptr;
  std::move(on_end_authentication_).Run();
}

BEGIN_METADATA(AuthPanel)
END_METADATA

}  // namespace ash
