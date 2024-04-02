// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/chrome_web_ui_configs.h"

#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "chrome/browser/ui/webui/accessibility/accessibility_ui.h"
#include "chrome/browser/ui/webui/autofill_and_password_manager_internals/autofill_internals_ui.h"
#include "chrome/browser/ui/webui/autofill_and_password_manager_internals/password_manager_internals_ui.h"
#include "content/public/browser/webui_config_map.h"
#include "extensions/buildflags/buildflags.h"
#include "printing/buildflags/buildflags.h"

#if !BUILDFLAG(IS_ANDROID)
#include "chrome/browser/ui/webui/bookmarks/bookmarks_ui.h"
#include "chrome/browser/ui/webui/commerce/product_specifications_ui.h"
#include "chrome/browser/ui/webui/downloads/downloads_ui.h"
#include "chrome/browser/ui/webui/history/history_ui.h"
#endif  // !BUILDFLAG(IS_ANDROID)

#if !BUILDFLAG(IS_CHROMEOS_LACROS)
#include "chrome/browser/ui/webui/bluetooth_internals/bluetooth_internals_ui.h"  // nogncheck
#endif  // !BUILDFLAG(IS_CHROMEOS_LACROS)

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "chrome/browser/ui/webui/extensions/extensions_ui.h"
#endif  // !BUILDFLAG(ENABLE_EXTENSIONS)

#if BUILDFLAG(ENABLE_PRINT_PREVIEW)
#include "chrome/browser/ui/webui/print_preview/print_preview_ui.h"
#endif  // BUILDFLAG(ENABLE_PRINT_PREVIEW)

#if BUILDFLAG(IS_CHROMEOS_ASH)
#include "chrome/browser/ui/webui/ash/chrome_web_ui_configs_chromeos.h"
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

void RegisterChromeWebUIConfigs() {
  // Don't add calls to `AddWebUIConfig()` for Ash-specific WebUIs here. Add
  // them in chrome_web_ui_configs_chromeos.cc.
#if BUILDFLAG(IS_CHROMEOS_ASH)
  ash::RegisterAshChromeWebUIConfigs();
#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

  auto& map = content::WebUIConfigMap::GetInstance();
  map.AddWebUIConfig(std::make_unique<AccessibilityUIConfig>());
  map.AddWebUIConfig(std::make_unique<AutofillInternalsUIConfig>());
  map.AddWebUIConfig(std::make_unique<PasswordManagerInternalsUIConfig>());

#if !BUILDFLAG(IS_CHROMEOS_LACROS)
  map.AddWebUIConfig(std::make_unique<BluetoothInternalsUIConfig>());
#endif  // !BUILDFLAG(IS_CHROMEOS_LACROS)

#if !BUILDFLAG(IS_ANDROID)
  map.AddWebUIConfig(std::make_unique<BookmarksUIConfig>());
  map.AddWebUIConfig(
      std::make_unique<commerce::ProductSpecificationsUIConfig>());
  map.AddWebUIConfig(std::make_unique<DownloadsUIConfig>());
  map.AddWebUIConfig(std::make_unique<HistoryUIConfig>());
#endif  // !BUILDFLAG(IS_ANDROID)

#if BUILDFLAG(ENABLE_EXTENSIONS)
  map.AddWebUIConfig(std::make_unique<extensions::ExtensionsUIConfig>());
#endif  // !BUILDFLAG(ENABLE_EXTENSIONS)

#if BUILDFLAG(ENABLE_PRINT_PREVIEW)
  map.AddWebUIConfig(std::make_unique<printing::PrintPreviewUIConfig>());
#endif  // BUILDFLAG(ENABLE_PRINT_PREVIEW)
}
