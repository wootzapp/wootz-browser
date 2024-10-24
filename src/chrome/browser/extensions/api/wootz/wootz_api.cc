// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/wootz/wootz_api.h"

#include <stddef.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "base/functional/bind.h"
#include "base/lazy_instance.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "build/build_config.h"
#include "chrome/browser/profiles/profile.h"
#include "components/search_engines/template_url_service.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/common/extension_id.h"
#include "ui/gfx/image/image.h"
#include "base/android/build_info.h"
#include "base/json/json_writer.h"

namespace extensions {

ExtensionFunction::ResponseAction WootzInfoFunction::Run() {
    const base::android::BuildInfo* build_info = base::android::BuildInfo::GetInstance();

    // Create a dictionary to store the relevant build information
    base::Value::Dict build_info_dict;
    build_info_dict.Set("device", build_info->device());
    build_info_dict.Set("manufacturer", build_info->manufacturer());
    build_info_dict.Set("model", build_info->model());
    build_info_dict.Set("brand", build_info->brand());
    build_info_dict.Set("sdk_int", build_info->sdk_int());
    build_info_dict.Set("android_build_id", build_info->android_build_id());
    build_info_dict.Set("android_build_fp", build_info->android_build_fp());
    build_info_dict.Set("gms_version_code", build_info->gms_version_code());
    build_info_dict.Set("host_package_name", build_info->host_package_name());
    build_info_dict.Set("package_name", build_info->package_name());
    build_info_dict.Set("package_version_code", build_info->package_version_code());
    build_info_dict.Set("package_version_name", build_info->package_version_name());
    build_info_dict.Set("abi_name", build_info->abi_name());
    build_info_dict.Set("is_tv", build_info->is_tv());
    build_info_dict.Set("is_automotive", build_info->is_automotive());
    build_info_dict.Set("is_foldable", build_info->is_foldable());

    std::string json_string;
    base::JSONWriter::Write(build_info_dict, &json_string);

    return RespondNow(WithArguments(json_string));
}

ExtensionFunction::ResponseAction WootzLogFunction::Run() {
  if (args().size() != 1)
      return RespondNow(NoArguments());

  LOG(ERROR) << "CONSOLE.LOG: " << args()[0];

  return RespondNow(NoArguments());
}
}  // namespace extensions
