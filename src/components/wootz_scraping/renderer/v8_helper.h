/* Copyright (c) 2024 All rights reserved. */

#ifndef COMPONENTS_wootz_scraping_RENDERER_V8_HELPER_H_
#define COMPONENTS_wootz_scraping_RENDERER_V8_HELPER_H_

#include <string>
#include <string_view>
#include <vector>

#include "third_party/blink/public/web/web_local_frame.h"
#include "v8/include/v8-context.h"
#include "v8/include/v8-local-handle.h"
#include "v8/include/v8-value.h"

namespace blink {
class WebLocalFrame;
}  // namespace blink

namespace wootz_scraping {

// Get a property from a JavaScript object
v8::MaybeLocal<v8::Value> GetProperty(v8::Local<v8::Context> context,
                                     v8::Local<v8::Value> object,
                                     const std::string_view name);

// Create a data property on a JavaScript object
v8::Maybe<bool> CreateDataProperty(v8::Local<v8::Context> context,
                                  v8::Local<v8::Object> object,
                                  const std::string_view name,
                                  v8::Local<v8::Value> value);

// Call a method on a JavaScript object by name
v8::MaybeLocal<v8::Value> CallMethodOfObject(
    blink::WebLocalFrame* web_frame,
    const std::string_view object_name,
    const std::string_view method_name,
    std::vector<v8::Local<v8::Value>>&& args);

// Call a method on a JavaScript object
v8::MaybeLocal<v8::Value> CallMethodOfObject(
    blink::WebLocalFrame* web_frame,
    v8::Local<v8::Value> object,
    const std::string_view method_name,
    std::vector<v8::Local<v8::Value>>&& args);

// Execute a script in a web frame
v8::MaybeLocal<v8::Value> ExecuteScript(blink::WebLocalFrame* web_frame,
                                       const std::string& script);

// Convert a v8 value to a string
std::string V8ToString(v8::Local<v8::Value> value);

}  // namespace wootz_scraping

#endif  // COMPONENTS_wootz_scraping_RENDERER_V8_HELPER_H_