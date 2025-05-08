// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_HTML_ACTION_URL_SCRIPT_BLOCK_STATES_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_HTML_ACTION_URL_SCRIPT_BLOCK_STATES_H_

#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

class ScriptBlockStates {
 public:
  ScriptBlockStates(const ScriptBlockStates& manager) = delete;
  ScriptBlockStates& operator=(const ScriptBlockStates& manager) = delete;

  static ScriptBlockStates& GetInstance();

  void SetInputHandlerScriptState();

  void SetRadioButtonHandlerScriptState();

  void SetCheckBoxHandlerScriptState();

  void SetButtonHandlerScriptState();

  void SetSelectHandlerScriptState();

  void SetFormHandlerScriptState();

  void ResetScriptState();

  String GetScriptsToAdd();
  String GetCssScriptsToAdd();

 private:
  ScriptBlockStates();
  ~ScriptBlockStates() = default;

  String FormSubmitEventListener();
  String ButtonEventListner();
  String RadioButtonEventListener();
  String CheckboxEventListener();
  String InputEventListener();
  String ValidationBlock();
  String SelectInputEventListener();

  bool should_add_input_handler_script = false;
  bool should_add_radio_button_handler_script = false;
  bool should_add_check_box_handler_script = false;
  bool should_add_button_handler_script = false;
  bool should_add_select_handler_script = false;
  bool should_add_form_handler_script = false;
  bool input_script_added = false;
  bool radio_script_added = false;
  bool checkbox_script_added = false;
  bool button_script_added = false;
  bool select_script_added = false;
  bool form_script_added = false;
  bool validiation_script_added = false;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_HTML_ACTION_URL_SCRIPT_BLOCK_STATES_H_
