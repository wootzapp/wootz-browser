// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/lacros/debug_interface_lacros.h"

#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/browser_window.h"
#include "chromeos/crosapi/mojom/debug_interface.mojom.h"
#include "chromeos/lacros/lacros_service.h"
#include "chromeos/ui/wm/debug_util.h"
#include "ui/aura/client/screen_position_client.h"
#include "ui/aura/env.h"
#include "ui/aura/window.h"
#include "ui/compositor/debug_utils.h"
#include "ui/views/debug_utils.h"
#include "ui/views/widget/widget.h"

namespace crosapi {

DebugInterfaceLacros::DebugInterfaceLacros() {
  chromeos::LacrosService* service = chromeos::LacrosService::Get();
  if (!service->IsAvailable<crosapi::mojom::DebugInterfaceRegisterer>()) {
    return;
  }
  service->GetRemote<crosapi::mojom::DebugInterfaceRegisterer>()
      ->RegisterDebugInterface(receiver_.BindNewPipeAndPassRemoteWithVersion());
}

DebugInterfaceLacros::~DebugInterfaceLacros() = default;

void DebugInterfaceLacros::PrintUiHierarchy(mojom::PrintTarget target) {
  switch (target) {
    case mojom::PrintTarget::kLayer:
      PrintLayerHierarchy();
      break;
    case mojom::PrintTarget::kWindow:
      PrintWindowHierarchy();
      break;
    case mojom::PrintTarget::kView:
      PrintViewHierarchy();
      break;
    default:
      NOTREACHED();
      break;
  }
}

void DebugInterfaceLacros::PrintLayerHierarchy() {
  std::ostringstream out;
  out << "\n";
  for (Browser* browser : *BrowserList::GetInstance()) {
    aura::Window* window = browser->window()->GetNativeWindow();
    aura::Window* root_window = window->GetRootWindow();
    ui::Layer* layer = root_window->layer();
    if (layer) {
      gfx::Point last_mouse_location =
          aura::Env::GetInstance()->last_mouse_location();
      aura::client::ScreenPositionClient* client =
          aura::client::GetScreenPositionClient(root_window);
      if (client) {
        client->ConvertPointFromScreen(root_window, &last_mouse_location);
      }
      ui::PrintLayerHierarchy(layer, last_mouse_location, &out);
    }
  }
  LOG(ERROR) << out.str();
}

void DebugInterfaceLacros::PrintWindowHierarchy() {
  std::ostringstream out;
  out << "\n";

  aura::Window::Windows windows;
  for (Browser* browser : *BrowserList::GetInstance()) {
    aura::Window* window = browser->window()->GetNativeWindow();
    if (window->GetRootWindow()) {
      window = window->GetRootWindow();
    }
    windows.push_back(window);
  }
  chromeos::wm::PrintWindowHierarchy(windows, /*scrub_data=*/false, &out);
  LOG(ERROR) << out.str();
}

void DebugInterfaceLacros::PrintViewHierarchy() {
  std::ostringstream out;
  out << "\n";
  for (Browser* browser : *BrowserList::GetInstance()) {
    aura::Window* window = browser->window()->GetNativeWindow();
    views::Widget* widget = views::Widget::GetWidgetForNativeView(window);
    out << "WidgetInfo: ";
    views::PrintWidgetInformation(*widget, /*detailed*/ true, &out);
    views::PrintViewHierarchy(widget->GetRootView(), &out);
  }
  LOG(ERROR) << out.str();
}

}  // namespace crosapi
