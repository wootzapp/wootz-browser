// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/ash/login/oobe_screens_handler_factory.h"

#include "base/check.h"
#include "chrome/browser/ash/login/oobe_screen.h"
#include "chrome/browser/ash/login/screens/consumer_update_screen.h"
#include "chrome/browser/ash/login/screens/gaia_info_screen.h"
#include "chrome/browser/ash/login/screens/osauth/local_data_loss_warning_screen.h"
#include "chrome/browser/ash/login/wizard_controller.h"
#include "chrome/browser/ui/webui/ash/login/consumer_update_screen_handler.h"
#include "chrome/browser/ui/webui/ash/login/drive_pinning_screen_handler.h"
#include "chrome/browser/ui/webui/ash/login/gaia_info_screen_handler.h"
#include "chrome/browser/ui/webui/ash/login/gesture_navigation_screen_handler.h"
#include "chrome/browser/ui/webui/ash/login/mojom/screens_common.mojom.h"
#include "chrome/browser/ui/webui/ash/login/mojom/screens_factory.mojom.h"
#include "chrome/browser/ui/webui/ash/login/mojom/screens_oobe.mojom.h"
#include "chrome/browser/ui/webui/ash/login/osauth/local_data_loss_warning_screen_handler.h"
#include "chrome/browser/ui/webui/ash/login/packaged_license_screen_handler.h"

namespace ash {

OobeScreensHandlerFactory::OobeScreensHandlerFactory(
    mojo::PendingReceiver<screens_factory::mojom::ScreensFactory>
        pending_receiver) {
  // If WizardController is unavailable during frontend element construction,
  // store the pending receiver. Upon WizardController creation, bind the stored
  // receiver.
  // TODO(b/329384403) Implement browser_test for the scenario where
  // WizardController is unavailable during frontend element construction.
  if (WizardController::default_controller()) {
    page_factory_receiver_.Bind(std::move(pending_receiver));
  } else {
    peding_receiver_ = std::move(pending_receiver);
  }
}

OobeScreensHandlerFactory::~OobeScreensHandlerFactory() = default;

void OobeScreensHandlerFactory::BindScreensHandlerFactory() {
  if (peding_receiver_.is_valid() && !page_factory_receiver_.is_bound()) {
    page_factory_receiver_.Bind(std::move(peding_receiver_));
  }
}

void OobeScreensHandlerFactory::UnbindScreensHandlerFactory() {
  page_factory_receiver_.reset();
}

void OobeScreensHandlerFactory::CreateDrivePinningScreenHandler(
    mojo::PendingRemote<screens_common::mojom::DrivePinningPage> page,
    mojo::PendingReceiver<screens_common::mojom::DrivePinningPageHandler>
        receiver) {
  CHECK(WizardController::default_controller());
  DrivePinningScreen* drive_pinning =
      WizardController::default_controller()->GetScreen<DrivePinningScreen>();
  drive_pinning->BindRemoteAndReceiver(std::move(page), std::move(receiver));
}

void OobeScreensHandlerFactory::CreateGestureNavigationPageHandler(
    mojo::PendingReceiver<screens_common::mojom::GestureNavigationPageHandler>
        receiver) {
  CHECK(WizardController::default_controller());
  GestureNavigationScreen* gesture_navigation =
      WizardController::default_controller()
          ->GetScreen<GestureNavigationScreen>();
  gesture_navigation->BindReceiver(std::move(receiver));
}

void OobeScreensHandlerFactory::CreateGaiaInfoScreenHandler(
    mojo::PendingRemote<screens_common::mojom::GaiaInfoPage> page,
    mojo::PendingReceiver<screens_common::mojom::GaiaInfoPageHandler>
        receiver) {
  CHECK(WizardController::default_controller());
  GaiaInfoScreen* gaia_info =
      WizardController::default_controller()->GetScreen<GaiaInfoScreen>();
  gaia_info->BindRemoteAndReceiver(std::move(page), std::move(receiver));
}

void OobeScreensHandlerFactory::CreateConsumerUpdatePageHandler(
    mojo::PendingRemote<screens_oobe::mojom::ConsumerUpdatePage> page,
    mojo::PendingReceiver<screens_oobe::mojom::ConsumerUpdatePageHandler>
        handler) {
  CHECK(WizardController::default_controller());
  ConsumerUpdateScreen* consumer_update =
      WizardController::default_controller()->GetScreen<ConsumerUpdateScreen>();
  consumer_update->BindRemoteAndReceiver(std::move(page), std::move(handler));
}

void OobeScreensHandlerFactory::CreatePackagedLicensePageHandler(
    mojo::PendingReceiver<screens_oobe::mojom::PackagedLicensePageHandler>
        receiver) {
  CHECK(WizardController::default_controller());
  PackagedLicenseScreen* packaged_license =
      WizardController::default_controller()
          ->GetScreen<PackagedLicenseScreen>();
  packaged_license->BindReceiver(std::move(receiver));
}

void OobeScreensHandlerFactory::CreateLocalDataLossWarningPageHandler(
    mojo::PendingReceiver<
        screens_osauth::mojom::LocalDataLossWarningPageHandler> receiver) {
  CHECK(WizardController::default_controller());
  LocalDataLossWarningScreen* local_data_loss_warning =
      WizardController::default_controller()
          ->GetScreen<LocalDataLossWarningScreen>();
  local_data_loss_warning->BindReceiver(std::move(receiver));
}

}  // namespace ash
