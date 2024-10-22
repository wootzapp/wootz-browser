// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/hello_world/hello_world_api.h"
#include "base/json/json_writer.h"
#include "extensions/browser/extension_function.h"

namespace extensions {

ExtensionFunction::ResponseAction HelloWorldSayHelloFunction::Run() {
  base::Value::Dict result;
  result.Set("message", "Hello, World!");

  std::string json_string;
  base::JSONWriter::Write(result, &json_string);

  return RespondNow(WithArguments(json_string));
}

}  // namespace extensions