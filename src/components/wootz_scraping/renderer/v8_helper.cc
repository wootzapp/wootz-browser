/* Copyright (c) 2024 All rights reserved. */

#include "components/wootz_scraping/renderer/v8_helper.h"

#include <string_view>
#include <utility>

#include "components/safe_builtins/renderer/safe_builtins_helpers.h"
#include "gin/converter.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "v8/include/v8-function.h"
#include "v8/include/v8-microtask-queue.h"
#include "v8/include/v8-object.h"
#include "base/strings/utf_string_conversions.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/web/web_script_source.h"
#include "v8/include/v8-context.h"
#include "v8/include/v8-exception.h"
#include "v8/include/v8-isolate.h"
#include "v8/include/v8-local-handle.h"
#include "v8/include/v8-primitive.h"

namespace wootz_scraping {

v8::MaybeLocal<v8::Value> GetProperty(v8::Local<v8::Context> context,
                                     v8::Local<v8::Value> object,
                                     const std::string_view name) {
  v8::Local<v8::String> name_str = gin::StringToV8(context->GetIsolate(), name);
  v8::Local<v8::Object> object_obj;
  if (!object->ToObject(context).ToLocal(&object_obj)) {
    return v8::MaybeLocal<v8::Value>();
  }

  return object_obj->Get(context, name_str);
}

v8::Maybe<bool> CreateDataProperty(v8::Local<v8::Context> context,
                                  v8::Local<v8::Object> object,
                                  const std::string_view name,
                                  v8::Local<v8::Value> value) {
  v8::Local<v8::String> name_str = gin::StringToV8(context->GetIsolate(), name);
  return object->CreateDataProperty(context, name_str, value);
}

v8::MaybeLocal<v8::Value> CallMethodOfObject(
    blink::WebLocalFrame* web_frame,
    const std::string_view object_name,
    const std::string_view method_name,
    std::vector<v8::Local<v8::Value>>&& args) {
  if (web_frame->IsProvisional()) {
    return v8::Local<v8::Value>();
  }
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Local<v8::Context> context = web_frame->MainWorldScriptContext();
  v8::MicrotasksScope microtasks(isolate, context->GetMicrotaskQueue(),
                               v8::MicrotasksScope::kDoNotRunMicrotasks);
  v8::Local<v8::Value> object;
  if (!GetProperty(context, context->Global(), object_name).ToLocal(&object)) {
    return v8::Local<v8::Value>();
  }

  return CallMethodOfObject(web_frame, object, method_name, std::move(args));
}

v8::MaybeLocal<v8::Value> CallMethodOfObject(
    blink::WebLocalFrame* web_frame,
    v8::Local<v8::Value> object,
    const std::string_view method_name,
    std::vector<v8::Local<v8::Value>>&& args) {
  if (web_frame->IsProvisional()) {
    return v8::Local<v8::Value>();
  }
  v8::Local<v8::Context> context = web_frame->MainWorldScriptContext();
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Context::Scope context_scope(context);
  v8::MicrotasksScope microtasks(isolate, context->GetMicrotaskQueue(),
                               v8::MicrotasksScope::kDoNotRunMicrotasks);
  v8::Local<v8::Value> method;
  if (!GetProperty(context, object, method_name).ToLocal(&method)) {
    return v8::Local<v8::Value>();
  }

  if (method.IsEmpty() || !method->IsFunction()) {
    return v8::Local<v8::Value>();
  }

  return web_frame->CallFunctionEvenIfScriptDisabled(
      v8::Local<v8::Function>::Cast(method), object,
      static_cast<int>(args.size()), args.data());
}

v8::MaybeLocal<v8::Value> ExecuteScript(
    blink::WebLocalFrame* frame,
    const std::string& script) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Local<v8::Context> context = frame->MainWorldScriptContext();
  if (context.IsEmpty())
    return v8::MaybeLocal<v8::Value>();

  v8::Context::Scope context_scope(context);
  v8::EscapableHandleScope handle_scope(isolate);
  v8::Local<v8::String> source = v8::String::NewFromUtf8(
      isolate, script.c_str(), v8::NewStringType::kNormal, script.length())
      .ToLocalChecked();

  v8::Local<v8::Script> compiled_script;
  if (!v8::Script::Compile(context, source).ToLocal(&compiled_script)) {
    return v8::MaybeLocal<v8::Value>();
  }

  return compiled_script->Run(context);
}

std::string V8ToString(v8::Local<v8::Value> value) {
  if (value.IsEmpty() || !value->IsString())
    return std::string();

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::Local<v8::String> string_value = value->ToString(isolate->GetCurrentContext()).ToLocalChecked();
  
  int length = string_value->Length();
  std::vector<char> buffer(length + 1);
  string_value->WriteUtf8(isolate, buffer.data(), length + 1);
  
  return std::string(buffer.data(), length);
}

}  // namespace wootz_scraping