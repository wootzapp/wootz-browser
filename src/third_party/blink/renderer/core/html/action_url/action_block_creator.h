// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_HTML_ACTION_URL_ACTION_BLOCK_CREATOR_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_HTML_ACTION_URL_ACTION_BLOCK_CREATOR_H_

#include "base/android/action_spec_json.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"
#include "third_party/blink/renderer/platform/wtf/vector.h"

namespace blink {

class CORE_EXPORT ActionBlockCreator {
 public:
  ActionBlockCreator(base::ActionSpecJson action, String blink_url);
  ~ActionBlockCreator();

  void CreateBlocks();
  String getActionBlock();
  String getScriptData();

 private:
  String CreateIconDiv();

  String CreateLowerLayout();
  String CreateSiteUrlWithIcon();
  String CreateTitle();
  String CreateDescription();
  String CreateWarningBlock();

  String DrawContentVector(
      std::vector<base::ActionSpecJson::FormattedText> content);

  String CreateActionBlock();
  String CreateTypeSpecificActionBlock(base::ActionSpecJson::Actions action,
                                       bool is_standalone,
                                       std::string href,
                                       String label);
  String CreateBasicStructure();

  String CreateRadioButton(base::ActionSpecJson::Actions action);
  String CreateRadioOption(base::ActionSpecJson::Options option,
                           String counter,
                           String name);
  String CreateCheckBox(base::ActionSpecJson::Actions action);
  String CreateCheckBoxOption(base::ActionSpecJson::Options option,
                              String counter,
                              String name);

  String CreateSelectButton(base::ActionSpecJson::Actions action);

  String CreateFormElement(
      std::vector<base::ActionSpecJson::Actions> parameters,
      std::string href,
      String label);
  String CreateTextAreaBox(base::ActionSpecJson::Actions action);
  String CreateInputBox(String placeholder,
                        String name,
                        bool isRequired,
                        bool is_standalone,
                        std::string href,
                        String label,
                        std::string type);
  String CreateButton(String button_label,
                      bool is_disabled,
                      String href,
                      bool is_submit_button,
                      bool standalone_button);

  String EnableAllScript();

  String data_;
  String script_data_;
  base::ActionSpecJson action_spec_;
  String blink_url_;
  int radio_checkbox_counter_ = 1;
};

}  // namespace blink
#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_HTML_ACTION_URL_ACTION_BLOCK_CREATOR_H_
