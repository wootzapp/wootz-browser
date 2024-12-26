// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_HELLO_WORLD_HELLO_WORLD_API_H_
#define CHROME_BROWSER_EXTENSIONS_API_HELLO_WORLD_HELLO_WORLD_API_H_

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

namespace extensions {

class HelloWorldSayHelloFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("helloWorld.sayHello", HELLOWORLD_SAYHELLO)

  HelloWorldSayHelloFunction() = default;

  HelloWorldSayHelloFunction(const HelloWorldSayHelloFunction&) = delete;
  HelloWorldSayHelloFunction& operator=(const HelloWorldSayHelloFunction&) = delete;

 protected:
  ~HelloWorldSayHelloFunction() override {}

  // ExtensionFunction:
  ResponseAction Run() override;
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_HELLO_WORLD_HELLO_WORLD_API_H_