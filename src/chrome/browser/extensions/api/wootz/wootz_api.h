// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_WOOTZ_WOOTZ_API_H_
#define CHROME_BROWSER_EXTENSIONS_API_WOOTZ_WOOTZ_API_H_

#include <set>
#include <string>

#include "base/memory/raw_ptr.h"
#include "base/scoped_observation.h"
#include "chrome/browser/extensions/extension_icon_manager.h"
#include "chrome/common/extensions/api/omnibox.h"
#include "components/search_engines/template_url_service.h"
#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_registry_observer.h"
#include "extensions/common/extension_id.h"
#include "ui/base/window_open_disposition.h"

class Profile;

namespace content {
class BrowserContext;
class WebContents;
}

namespace extensions {
class WootzInfoFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.info", WOOTZ_INFO)

  WootzInfoFunction() = default;

  WootzInfoFunction(
      const WootzInfoFunction&) = delete;
  WootzInfoFunction& operator=(
      const WootzInfoFunction&) = delete;

 protected:
  ~WootzInfoFunction() override {}

  ResponseAction Run() override;
};

class WootzLogFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.log", WOOTZ_LOG)

  WootzLogFunction() = default;

  WootzLogFunction(
      const WootzLogFunction&) = delete;
  WootzLogFunction& operator=(
      const WootzLogFunction&) = delete;

 protected:
  ~WootzLogFunction() override {}

  ResponseAction Run() override;
};
}

#endif  // CHROME_BROWSER_EXTENSIONS_API_WOOTZ_WOOTZ_API_H_
