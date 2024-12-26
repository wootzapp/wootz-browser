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
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"

class Profile;

namespace content {
class BrowserContext;
class WebContents;
}  // namespace content

namespace extensions {
class WootzInfoFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.info", WOOTZ_INFO)

  WootzInfoFunction() = default;

  WootzInfoFunction(const WootzInfoFunction&) = delete;
  WootzInfoFunction& operator=(const WootzInfoFunction&) = delete;

 protected:
  ~WootzInfoFunction() override {}

  ResponseAction Run() override;
};

class WootzHelloWorldFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.helloWorld", WOOTZ_HELLOWORLD)

  WootzHelloWorldFunction() = default;

  WootzHelloWorldFunction(const WootzHelloWorldFunction&) = delete;
  WootzHelloWorldFunction& operator=(const WootzHelloWorldFunction&) = delete;

 protected:
  ~WootzHelloWorldFunction() override {}

  ResponseAction Run() override;
};

class WootzShowDialogFunction : public ExtensionFunction {
    DECLARE_EXTENSION_FUNCTION("wootz.showDialog", WOOTZ_SHOWDIALOG)
    WootzShowDialogFunction() = default;

    WootzShowDialogFunction(const WootzShowDialogFunction&) = delete;
    WootzShowDialogFunction& operator=(const WootzShowDialogFunction&) = delete;

    ResponseAction Run() override;
  protected:
    ~WootzShowDialogFunction() override {}
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

// background service api
class WootzSetJobFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.setJob", WOOTZ_SETJOB)
  WootzSetJobFunction() = default;
  WootzSetJobFunction(const WootzSetJobFunction&) = delete;
  WootzSetJobFunction& operator=(const WootzSetJobFunction&) = delete;

 protected:
  ~WootzSetJobFunction() override = default;
  ResponseAction Run() override;
};

class WootzRemoveJobFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.removeJob", WOOTZ_REMOVEJOB)
  WootzRemoveJobFunction() = default;
  WootzRemoveJobFunction(const WootzRemoveJobFunction&) = delete;
  WootzRemoveJobFunction& operator=(const WootzRemoveJobFunction&) = delete;

 protected:
  ~WootzRemoveJobFunction() override = default;
  ResponseAction Run() override;
};

class WootzGetJobsFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("wootz.getJobs", WOOTZ_GETJOBS)
  WootzGetJobsFunction() = default;
  WootzGetJobsFunction(const WootzGetJobsFunction&) = delete;
  WootzGetJobsFunction& operator=(const WootzGetJobsFunction&) = delete;

 protected:
  ~WootzGetJobsFunction() override = default;
  ResponseAction Run() override;
};

class WootzListJobsFunction : public ExtensionFunction {
public:
 DECLARE_EXTENSION_FUNCTION("wootz.listJobs", WOOTZ_LISTJOBS)

 WootzListJobsFunction() = default;

 WootzListJobsFunction(const WootzListJobsFunction&) = delete;
 WootzListJobsFunction& operator=(const WootzListJobsFunction&) = delete;

protected:
 ~WootzListJobsFunction() override {}

 ResponseAction Run() override;
};

class WootzCleanJobsFunction : public ExtensionFunction {
public:
 DECLARE_EXTENSION_FUNCTION("wootz.cleanJobs", WOOTZ_CLEANJOBS)

 WootzCleanJobsFunction() = default;

 WootzCleanJobsFunction(const WootzCleanJobsFunction&) = delete;
 WootzCleanJobsFunction& operator=(const WootzCleanJobsFunction&) = delete;

protected:
 ~WootzCleanJobsFunction() override {}

 ResponseAction Run() override;
};


}  // namespace extensions
#endif  // CHROME_BROWSER_EXTENSIONS_API_WOOTZ_WOOTZ_API_H_