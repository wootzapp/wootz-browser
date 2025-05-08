// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/html/action_url/action_block_creator.h"

#include <regex>

#include "third_party/blink/renderer/platform/data_resource_helper.h"
#include "third_party/blink/public/resources/grit/blink_resources.h"
#include "third_party/blink/renderer/core/html/action_url/script_block_states.h"
#include "base/strings/utf_string_conversions.h"
#include "third_party/blink/renderer/platform/wtf/text/string_builder.h"
namespace {

std::string getBaseUrl(std::string& fullUrl, const std::string& spec) {
    // Find the base part of the URL (before any parameters)
    if (!fullUrl.starts_with("http://") && !fullUrl.starts_with("https://")) {
      fullUrl = "https://"+spec + fullUrl;
    }
    size_t queryStart = fullUrl.find('?');
    if (queryStart == std::string::npos) {
        return fullUrl+"?";
    }

    // Split the query parameters
    std::string baseUrlPart = fullUrl.substr(0, queryStart + 1);
    std::string queryPart = fullUrl.substr(queryStart + 1);

    std::istringstream ss(queryPart);
    std::string param;
    std::vector<std::string> validParams;

    // Process each parameter
    while (std::getline(ss, param, '&')) {
        // Check if the parameter does NOT contain {} placeholders
        if (param.find('{') == std::string::npos || param.find('}') == std::string::npos) {
            validParams.push_back(param);
        }
    }

    // Reconstruct the URL with valid parameters
    if (!validParams.empty()) {
        baseUrlPart += validParams[0];
        for (size_t i = 1; i < validParams.size(); ++i) {
            baseUrlPart += "&" + validParams[i];
        }
        baseUrlPart += "&";
    }
    return baseUrlPart;
}

String ConvertUTF16String(std::string str) {
  return String(base::UTF8ToUTF16(str).c_str());
}

}  // namespace

namespace blink {

ActionBlockCreator::ActionBlockCreator(base::ActionSpecJson action,
                                       String blink_url)
    : action_spec_(action), blink_url_(blink_url) {}

ActionBlockCreator::~ActionBlockCreator() = default;

String ActionBlockCreator::getActionBlock() {
    return data_;
}

String ActionBlockCreator::getScriptData() {
    return script_data_;
}

void ActionBlockCreator::CreateBlocks() {
  script_data_ = "";
  data_ = (R"HTML(<div class="action-block-div">
          <div class="w-full max-w-md">
						<div class="w-full max-w-md">
							<div class="blink dial-light">
								<div class="border-stroke-primary bg-bg-primary shadow-action w-full cursor-default overflow-hidden rounded-2xl border">)HTML")
  + CreateIconDiv() + CreateLowerLayout() +
  (R"HTML(      </div>
						  </div>
						</div>
					</div>
        </div>)HTML");
  //(R"HTML()HTML")
}

String ActionBlockCreator::CreateIconDiv() {
  String icon_div = (R"HTML(<div onclick="window.location.href=')HTML") +blink_url_+ (R"HTML('" class="block max-h-[100cqw] overflow-y-hidden px-5 pt-5">
										            <img class="aspect-auto w-full rounded-xl object-cover object-center" src=")HTML") + String(action_spec_.icon) +(R"HTML(" alt="action-image">
									          </div> )HTML");
  return icon_div;
}

String ActionBlockCreator::CreateLowerLayout() {
  String lower_layout = (R"HTML(<div class="flex flex-col p-5">)HTML") +
                        CreateSiteUrlWithIcon() +
                        CreateTitle()+
                        CreateDescription() +
                        CreateWarningBlock() +
                        CreateActionBlock()+
                        (R"HTML(</div>)HTML");
  return lower_layout;
}

String ActionBlockCreator::CreateSiteUrlWithIcon() {
  String site_url_with_icon = 
            (R"HTML(<div class="mb-1 flex items-center gap-2">
											<span class="text-subtext text-text-link min-w-0 truncate">)HTML") + String(action_spec_.site_url) + (R"HTML(</span>)HTML") +
							(R"HTML(<a href="https://docs.dialect.to/documentation/actions/security" target="_blank" rel="noopener noreferrer" class="mb-0.5 flex items-center">)HTML");

  if(action_spec_.tag == "malicious") {
    site_url_with_icon = site_url_with_icon + (R"HTML(<div class="group bg-transparent-error text-subtext inline-flex items-center justify-center gap-1 rounded-full font-semibold leading-none px-1.5 py-1">
                                                  <span class="text-text-error group-hover:text-text-error-hover mt-0.5 transition-colors motion-reduce:transition-none">Blocked</span>
                                                  <div class="text-icon-error group-hover:text-icon-error-hover transition-colors motion-reduce:transition-none">)HTML");
  }
  else{
    site_url_with_icon = site_url_with_icon + (R"HTML(<div class="group bg-transparent-grey text-subtext inline-flex items-center justify-center gap-1 rounded-full font-semibold leading-none aspect-square p-1">
													<div class="text-icon-primary group-hover:text-icon-primary-hover transition-colors motion-reduce:transition-none">)HTML");
  }

	site_url_with_icon = site_url_with_icon +	UncompressResourceAsString(IDR_ACTION_BLOCK_INFO_SHIELD) +
									(R"HTML(</div>
												</div>
											</a>
										</div>)HTML");
  return site_url_with_icon;
}

String ActionBlockCreator::CreateTitle() {
  String title = (R"HTML(<span class="text-text text-text-primary mb-1.5 break-words font-semibold">)HTML") +
                  ConvertUTF16String(action_spec_.title) +
                  (R"HTML(</span>)HTML");
  return title;
}

String ActionBlockCreator::DrawContentVector(std::vector<base::ActionSpecJson::FormattedText> content) {
  String contents = "";
  for(const auto& formatted_part : content) {
    if(formatted_part.is_bold) {
      contents = contents + (R"HTML(<strong>)HTML")+ ConvertUTF16String(formatted_part.text) +(R"HTML(</strong>)HTML");
    } else if(formatted_part.is_italic) {
      contents = contents + (R"HTML(<em>)HTML")+ ConvertUTF16String(formatted_part.text) +(R"HTML(</em>)HTML");
    } else if(formatted_part.is_link) {
      contents = contents + (R"HTML(<a href=")HTML")+String(formatted_part.url)+ (R"HTML(" class="text-text-link hover:text-text-link-hover underline transition-colors motion-reduce:transition-none" target="_blank" rel="noopener noreferrer">)HTML") +ConvertUTF16String(formatted_part.text)+(R"HTML(</a>)HTML");
    } else {
      contents = contents + ConvertUTF16String(formatted_part.text);
    }
  }
  return contents;
}

String ActionBlockCreator::CreateDescription() {
  String ending_tags[] = {String("</p>"),String("</ol>"),String("</ul>")};
  String description =
      (R"HTML(<span class="text-subtext text-text-secondary mb-4 break-words">
                <div>)HTML");
  base::ActionSpecJson::ElementType current_tracking = base::ActionSpecJson::ElementType::BREAK;
  for(const auto& part : action_spec_.descriptionParts){
    if((current_tracking == base::ActionSpecJson::ElementType::ORDERED_LIST_ITEM ||
       current_tracking == base::ActionSpecJson::ElementType::UNORDERED_LIST_ITEM ||
       current_tracking == base::ActionSpecJson::ElementType::PARAGRAPH)
       && current_tracking != part.type) {
        description = description + ending_tags[int(current_tracking)];
    }
    if(part.type == base::ActionSpecJson::ElementType::HORIZONTAL_RULE) {
      description = description + (R"HTML(<hr class="my-[0.5em]">)HTML");
    }
    else if(part.type == base::ActionSpecJson::ElementType::HEADER) {
      description = description + (R"HTML(<p class="mb-[0.35em] last:mb-0">)HTML")+DrawContentVector(part.content)+(R"HTML(</p>)HTML");
    }
    else if(part.type == base::ActionSpecJson::ElementType::UNORDERED_LIST_ITEM) {
      if(current_tracking != part.type) {  //First element of unordered list
        description = description + (R"HTML(<ul class="list-inside list-disc [li>&amp;]:ps-4">)HTML");
        // current_tracking_list_type = part.type;
      }
      description = description + (R"HTML(<li>)HTML")+DrawContentVector(part.content)+(R"HTML(</li>)HTML");
    }
    else if(part.type == base::ActionSpecJson::ElementType::ORDERED_LIST_ITEM) {
      if(current_tracking != part.type) {  //First element of ordered list
        description = description + (R"HTML(<ol class="list-inside list-decimal [li>&amp;]:ps-4">)HTML");
        // current_tracking_list_type = part.type;
      }
      description = description + (R"HTML(<li>)HTML")+DrawContentVector(part.content)+(R"HTML(</li>)HTML");
    }
    else if(part.type == base::ActionSpecJson::ElementType::PARAGRAPH) {
      if(current_tracking != base::ActionSpecJson::ElementType::PARAGRAPH) {
        description = description + (R"HTML(<p class="mb-[0.35em] last:mb-0">)HTML");
      }
      description = description + DrawContentVector(part.content);
    }

    current_tracking = part.type;
  }
  if(current_tracking == base::ActionSpecJson::ElementType::ORDERED_LIST_ITEM ||
       current_tracking == base::ActionSpecJson::ElementType::UNORDERED_LIST_ITEM ||
       current_tracking == base::ActionSpecJson::ElementType::PARAGRAPH) {
        description = description + ending_tags[int(current_tracking)];
    }

  description = description + (R"HTML(</div>
                                  </span>)HTML");
  return description;
}

String ActionBlockCreator::CreateWarningBlock() {
  String warning = "";
  if(action_spec_.tag == "malicious") {
    warning = warning + (R"HTML(
    <div id="warning_)HTML") + String(std::to_string(action_spec_.id)) + (R"HTML(" class="mb-4">
        <div class="bg-transparent-error text-text-error border-stroke-error rounded-lg border p-3 text-subtext">
            <p>This Action or it's origin has been flagged as an unsafe action, &amp; has been blocked. If you believe this action has been blocked in error, please <a href="https://discord.gg/saydialect" class="cursor-pointer underline" target="_blank" rel="noopener noreferrer">submit an issue</a>.</p>
                <button onclick="enableAll_)HTML")+String(std::to_string(action_spec_.id))+(R"HTML(()" class="hover:text-text-error-hover mt-3 font-semibold transition-colors motion-reduce:transition-none">Ignore warning &amp; proceed</button>
        </div>
    </div>
    )HTML");
    script_data_ = script_data_ + EnableAllScript();
  }
  return warning;
}

//(R"HTML()HTML")
String ActionBlockCreator::CreateActionBlock() {
  String action_block = (R"HTML(<div id="::action_block_)HTML")+ String(std::to_string(action_spec_.id)) +(R"HTML(::" class="flex flex-col gap-3">)HTML");
  if(action_spec_.links.empty()) {
    // Basic structure with only one button
    action_block = action_block + CreateBasicStructure();
  }
  else {
    bool is_start_of_button_block = true;
    for(const auto& action : action_spec_.links) {
      if(action.type == "button" || action.type == "post") {
        if(is_start_of_button_block) {
          action_block = action_block + (R"HTML(<div class="flex flex-wrap items-center gap-2">)HTML");
          is_start_of_button_block = false;
        }
        action_block = action_block +  CreateButton(ConvertUTF16String(action.label), false, String(action.href), false, false);
      }
      else if(action.type == "form") {
        if(!is_start_of_button_block) {  // a button block stated but not finished
          is_start_of_button_block = true;
          action_block = action_block + (R"HTML(</div>)HTML");  // finish the button block here
        }
        action_block = action_block + CreateFormElement(action.parameters, action.href, ConvertUTF16String(action.label));
      }
    }
    if(!is_start_of_button_block) {  // a button block stated but not finished
      is_start_of_button_block = true;
      action_block = action_block + (R"HTML(</div>)HTML");  // finish the button block here
    }
  }
  action_block = action_block + (R"HTML(</div>)HTML");
  return action_block;
}

String ActionBlockCreator::CreateTypeSpecificActionBlock(
    base::ActionSpecJson::Actions action,
    bool is_standalone,
    std::string href,
    String label) {
  String type_specific_block = "";

  if (action.type == "inputText" || action.type == "text" ||
      action.type == "email" || action.type == "number" ||
      action.type == "datetime-local" || action.type == "url" || action.type == "date") {
    type_specific_block =
        type_specific_block + CreateInputBox(ConvertUTF16String(action.label),
                                             String(action.name),
                                             action.required, is_standalone,
                                             href, label, action.type);
  } else if (action.type == "textarea") {
    type_specific_block = type_specific_block + CreateTextAreaBox(action);
  } else if (action.type == "radio") {
    type_specific_block = type_specific_block + CreateRadioButton(action);
  } else if (action.type == "checkbox") {
    type_specific_block = type_specific_block + CreateCheckBox(action);
  } else if (action.type == "select") {
    type_specific_block = type_specific_block + CreateSelectButton(action);
  }

  return type_specific_block;
}

String ActionBlockCreator::CreateBasicStructure() {
  String basic_block = (R"HTML(<div class="flex flex-wrap items-center gap-2">)HTML");
  basic_block = basic_block + CreateButton(ConvertUTF16String(action_spec_.label), action_spec_.disabled, String(), false, false);
  basic_block = basic_block + (R"HTML(</div>)HTML");
  return basic_block;
}

//(R"HTML()HTML")
String ActionBlockCreator::CreateSelectButton(base::ActionSpecJson::Actions action) {
  String select_block = (R"HTML(
  <div>
    <div class="border-input-stroke peer relative flex min-h-10 flex-wrap items-center gap-1.5 gap-y-2 border p-1.5 transition-colors motion-reduce:transition-none focus-within:has-[:invalid]:border-input-stroke-error focus-within:has-[:valid]:border-input-stroke-selected focus-within:hover:has-[:invalid]:border-input-stroke-error focus-within:hover:has-[:valid]:border-input-stroke-selected hover:has-[:enabled]:border-input-stroke-hover rounded-input">
      <div class="flex min-w-0 flex-[10] basis-1/2 items-center gap-1.5 pl-2.5">)HTML");
  select_block = select_block + (R"HTML(<select onchange="OnSelectInputListener(this)" name=")HTML")+ String(action.name) +(R"HTML(" class="bg-input-bg text-text-input placeholder:text-text-input-placeholder disabled:text-text-input-disabled min-h-7 min-w-0 flex-1 truncate outline-none mr-1.5 cursor-pointer")HTML");
  if(action.required) {
    select_block = select_block + (R"HTML( required )HTML");
  }
  if(action_spec_.tag == "malicious") {
    select_block = select_block + (R"HTML( disabled )HTML");
  }

  select_block = select_block + (R"HTML(><option disabled value=":r0:" selected>)HTML")+ConvertUTF16String(action.label);
  if(action.required) {
    select_block = select_block + (R"HTML(*</option>)HTML");
  }
  else{
    select_block = select_block +(R"HTML(</option>)HTML");
  }
  for(const auto& option : action.options) {
    select_block = select_block + (R"HTML(<option value=")HTML")+String(option.value)+(R"HTML(">)HTML")+ConvertUTF16String(option.label)+(R"HTML(</option>)HTML");
  }
  select_block = select_block + (R"HTML(</select>
                                      </div>
                                    </div>
                                  </div>)HTML");

  ScriptBlockStates::GetInstance().SetSelectHandlerScriptState();
  return select_block;
}

//(R"HTML()HTML")
String ActionBlockCreator::CreateRadioButton(base::ActionSpecJson::Actions action) {
  String radio_block = (R"HTML(<div class="py-1.5">
                                  <div class>)HTML");
  radio_block = radio_block + (R"HTML(<div class="mb-1"><label class="text-subtext text-text-input block font-semibold">)HTML") +ConvertUTF16String(action.label);
  if(action.required) {
    radio_block = radio_block + (R"HTML(*</label></div>)HTML");
  }
  else{
    radio_block = radio_block +(R"HTML(</label></div>)HTML");
  }
  radio_block = radio_block + (R"HTML(<div class="pt-2 flex flex-col gap-3">)HTML");
  
  for(const auto& option : action.options) {
    radio_block = radio_block + CreateRadioOption(option, String(std::to_string(radio_checkbox_counter_)), String(action.name));
    radio_checkbox_counter_++;
  }
  radio_block = radio_block + (R"HTML(</div>
                                    </div>
                                  </div>)HTML");

  ScriptBlockStates::GetInstance().SetRadioButtonHandlerScriptState();
  return radio_block;
}

String ActionBlockCreator::CreateRadioOption(
    base::ActionSpecJson::Options option,
    String counter,
    String name) {
  String radio_option_block = (R"HTML(<div class="inline-flex">
                                        <button class="flex h-full gap-2.5 cursor-pointer">
                                          <div class="flex h-full items-center">)HTML");
  radio_option_block = radio_option_block + (R"HTML(<input class="hidden" type="radio" value=")HTML") + String(option.value)+ (R"HTML(" name=")HTML") +name+ (R"HTML(")HTML");
  if(action_spec_.tag == "malicious") {
    radio_option_block = radio_option_block + (R"HTML( disabled)HTML");
  }
  radio_option_block = radio_option_block + (R"HTML(>)HTML");
  if(option.selected) {
    radio_option_block = radio_option_block + (R"HTML(<div role="radio" name=")HTML")+name+(R"HTML(" id=":r)HTML") +counter+(R"HTML(:" aria-labelledby=":r)HTML")+ counter+ (R"HTML(:_label" onclick="handleRadioSelection(this)" class="mt-0.5 flex aspect-square h-[16px] items-center justify-center rounded-full border transition-colors motion-reduce:transition-none border-input-stroke-selected bg-input-bg-selected">)HTML");
    radio_option_block = radio_option_block + (R"HTML(<div class="aspect-square h-[8px] rounded-full bg-input-bg"></div> </div> </div>)HTML");
  }
  else {
    radio_option_block = radio_option_block + (R"HTML(<div role="radio" name=")HTML")+name+(R"HTML(" id=":r)HTML") +counter+(R"HTML(:" aria-labelledby=":r)HTML")+ counter+ (R"HTML(:_label" onclick="handleRadioSelection(this)" class="mt-0.5 flex aspect-square h-[16px] items-center justify-center rounded-full border transition-colors motion-reduce:transition-none border-input-stroke bg-input-bg">)HTML");
    radio_option_block = radio_option_block + (R"HTML(<div class="aspect-square h-[8px] rounded-full bg-input-bg invisible"></div> </div> </div>)HTML");
  }
  radio_option_block = radio_option_block + (R"HTML(<label onclick="handleRadioSelection(this)" name="label_token" class="cursor-[inherit] text-text text-text-input" id=":r)HTML") +counter+(R"HTML(:_label">)HTML")+ConvertUTF16String(option.label)+(R"HTML(</label>)HTML");
  radio_option_block = radio_option_block +(R"HTML(</button>
                                            </div>)HTML");

  return radio_option_block;
}

//(R"HTML()HTML")
String ActionBlockCreator::CreateCheckBox(base::ActionSpecJson::Actions action) {
  String check_box = (R"HTML(<div class="py-1.5">
                                  <div class>)HTML");
  check_box = check_box + (R"HTML(<div class="mb-1"><label class="text-subtext text-text-input block font-semibold">)HTML") +ConvertUTF16String(action.label);
  if(action.required) {
    check_box = check_box + (R"HTML(*</label></div>)HTML");
  }
  else{
    check_box = check_box +(R"HTML(</label></div>)HTML");
  }
  check_box = check_box + (R"HTML(<div class="pt-2 flex flex-col gap-3">)HTML");

  for(const auto& option : action.options) {
    check_box = check_box + CreateCheckBoxOption(option, String(std::to_string(radio_checkbox_counter_)), String(action.name));
    radio_checkbox_counter_++;
  }
  check_box = check_box + (R"HTML(</div>
                                    </div>
                                  </div>)HTML");

  ScriptBlockStates::GetInstance().SetCheckBoxHandlerScriptState();
  return check_box;
}

String ActionBlockCreator::CreateCheckBoxOption(
    base::ActionSpecJson::Options option,
    String counter,
    String name) {
  String checkbox_option_block = (R"HTML(<div class="inline-flex">
                                        <button class="flex h-full gap-2.5 cursor-pointer">
                                          <div class="flex h-full items-center">)HTML");
  checkbox_option_block = checkbox_option_block + (R"HTML(<input class="hidden" type="checkbox" value=")HTML") + String(option.value)+ (R"HTML(" name=")HTML") +name+ (R"HTML(")HTML");
  if(action_spec_.tag == "malicious") {
    checkbox_option_block = checkbox_option_block + (R"HTML( disabled)HTML");
  }
  checkbox_option_block = checkbox_option_block + (R"HTML(>)HTML");
  if(option.selected) {
    checkbox_option_block = checkbox_option_block + (R"HTML( <span role="checkbox" name=")HTML")+name+(R"HTML(" id=":r)HTML") +counter+(R"HTML(:" aria-labelledby=":r)HTML")+ counter+ (R"HTML(:_label" onclick="handleCheckboxSelection(this)" class="mt-0.5 flex aspect-square h-[16px] items-center justify-center rounded-lg border transition-colors motion-reduce:transition-none border-input-stroke-selected bg-input-bg-selected">)HTML");
    checkbox_option_block = checkbox_option_block + (R"HTML(
                                                  <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="none" viewBox="0 0 16 16" preserveAspectRatio="xMidYMid meet" class="h-full w-full text-input-bg block">
                                                      <path fill="currentColor" fill-rule="evenodd" d="M12.6 5.398a.85.85 0 0 1-.001 1.202l-4.784 4.773a.85.85 0 0 1-1.193.008L3.934 8.77a.85.85 0 0 1 1.184-1.22l2.088 2.027 4.192-4.182a.85.85 0 0 1 1.202.002Z" clip-rule="evenodd"></path>
                                                   </svg>
                                                  </span>
                                                </div>)HTML");
  }
  else {
    checkbox_option_block = checkbox_option_block + (R"HTML( <span role="checkbox" name=")HTML")+name+(R"HTML(" id=":r)HTML") +counter+(R"HTML(:" aria-labelledby=":r)HTML")+ counter+ (R"HTML(:_label" onclick="handleCheckboxSelection(this)" class="mt-0.5 flex aspect-square h-[16px] items-center justify-center rounded-lg border transition-colors motion-reduce:transition-none border-input-stroke bg-input-bg">)HTML");
    checkbox_option_block = checkbox_option_block + (R"HTML(
                                                  <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="none" viewBox="0 0 16 16" preserveAspectRatio="xMidYMid meet" class="h-full w-full text-input-bg hidden">
                                                      <path fill="currentColor" fill-rule="evenodd" d="M12.6 5.398a.85.85 0 0 1-.001 1.202l-4.784 4.773a.85.85 0 0 1-1.193.008L3.934 8.77a.85.85 0 0 1 1.184-1.22l2.088 2.027 4.192-4.182a.85.85 0 0 1 1.202.002Z" clip-rule="evenodd"></path>
                                                   </svg>
                                                  </span>
                                                </div>)HTML");
  }
  checkbox_option_block = checkbox_option_block + (R"HTML(<label onclick="handleCheckboxSelection(this)" name="checkbox_label_token" class="cursor-[inherit] text-text text-text-input" id=":r)HTML") +counter+(R"HTML(:_label">)HTML")+ConvertUTF16String(option.label)+(R"HTML(</label>)HTML");
  checkbox_option_block = checkbox_option_block +(R"HTML(</button>
                                            </div>)HTML");

  return checkbox_option_block;
}

String ActionBlockCreator::CreateFormElement(
    std::vector<base::ActionSpecJson::Actions> parameters,
    std::string href,
    String label) {
  std::string modified_href = getBaseUrl(href, action_spec_.site_url);
  String form_block =
      (R"HTML(<form id=":action_block:_form_id:" method="post" onsubmit="handleFormSubmission(event, this, ')HTML")+String(modified_href)+ (R"HTML(')">)HTML");
  form_block = form_block + (R"HTML(<div class="flex flex-col gap-3">)HTML");
  bool is_standalone = false;
  if(parameters.size() == 1 && parameters[0].type == "inputText"){
    is_standalone = true;
  }
  for(const auto& action : parameters) {
    form_block =  form_block + CreateTypeSpecificActionBlock(action, is_standalone, href, label);
  }
  if(!is_standalone) {
    form_block = form_block + CreateButton(label, true, String(href), true, false);
  }
  form_block = form_block + (R"HTML(</div>)HTML");
  form_block = form_block + (R"HTML(</form>)HTML");

  ScriptBlockStates::GetInstance().SetFormHandlerScriptState();
  return form_block;
}

//(R"HTML()HTML")
String ActionBlockCreator::CreateTextAreaBox(base::ActionSpecJson::Actions action) {
  String text_area_block = (R"HTML(<div>
   <div class="border-input-stroke peer relative flex min-h-10 flex-wrap items-center gap-1.5 gap-y-2 border p-1.5 transition-colors motion-reduce:transition-none focus-within:has-[:invalid]:border-input-stroke-error focus-within:has-[:valid]:border-input-stroke-selected focus-within:hover:has-[:invalid]:border-input-stroke-error focus-within:hover:has-[:valid]:border-input-stroke-selected hover:has-[:enabled]:border-input-stroke-hover rounded-input">)HTML");
  text_area_block = text_area_block + (R"HTML(<div class="flex min-w-0 flex-[10] basis-1/2 items-center gap-1.5 pl-2.5"><textarea oninput="onInputListener(this)" placeholder=")HTML") + ConvertUTF16String(action.label);
  if(action.required) {
    text_area_block = text_area_block + (R"HTML(*" required)HTML");
  }
  else{
    text_area_block = text_area_block + (R"HTML(")HTML");
  }
  text_area_block = text_area_block+ (R"HTML( rows="3" name=")HTML")+String(action.name)+(R"HTML(" class="bg-input-bg text-text-input placeholder:text-text-input-placeholder disabled:text-text-input-disabled min-h-7 min-w-0 flex-1 truncate outline-none")HTML"); 

  if(action_spec_.tag == "malicious") {
    text_area_block = text_area_block + (R"HTML( disabled )HTML");
  }

  text_area_block = text_area_block + (R"HTML(></textarea>
                                            </div>
                                          </div>)HTML");
  if(!action.patternDescription.empty()){
    text_area_block = text_area_block + (R"HTML(<div class="text-caption text-text-secondary peer-[:focus-within:has(:invalid)]:text-text-error mt-1.5 font-medium"><span>)HTML")
                        + ConvertUTF16String(action.patternDescription) + (R"HTML(</span></div>)HTML");
  }

  text_area_block = text_area_block +(R"HTML(</div>)HTML");
  ScriptBlockStates::GetInstance().SetInputHandlerScriptState();
  return text_area_block;
}

String ActionBlockCreator::CreateInputBox(String placeholder,
                                          String name,
                                          bool isRequired,
                                          bool is_standalone,
                                          std::string href,
                                          String label,
                                          std::string type) {
  String input_block = (R"HTML(<div>
                        <div class="border-input-stroke peer relative flex min-h-10 flex-wrap items-center gap-1.5 gap-y-2 border p-1.5 transition-colors motion-reduce:transition-none focus-within:has-[:invalid]:border-input-stroke-error focus-within:has-[:valid]:border-input-stroke-selected focus-within:hover:has-[:invalid]:border-input-stroke-error focus-within:hover:has-[:valid]:border-input-stroke-selected hover:has-[:enabled]:border-input-stroke-hover )HTML");
  if(is_standalone) {
    input_block = input_block + (R"HTML(rounded-input-standalone">)HTML");
  }
  else{
    input_block = input_block + (R"HTML(rounded-input">)HTML");
  }

  input_block = input_block + (R"HTML(<div class="flex min-w-0 flex-[10] basis-1/2 items-center gap-1.5 pl-2.5">)HTML");
  if(type == "email") {
    input_block = input_block + (R"HTML(<div>
                                          <label>)HTML");
    input_block = input_block +	UncompressResourceAsString(IDR_ACTION_BLOCK_EMAIL) +
									(R"HTML( </label>
                        </div>)HTML");
  }
  else if(type == "number") {
    input_block = input_block + (R"HTML(<div>
                                          <label>)HTML");
    input_block = input_block +	UncompressResourceAsString(IDR_ACTION_BLOCK_HASH) +
									(R"HTML( </label>
                        </div>)HTML");
  }
  else if(type == "datetime-local" || type == "date") {
    input_block = input_block + (R"HTML(<div>
                                          <label>)HTML");
    input_block = input_block +	UncompressResourceAsString(IDR_ACTION_BLOCK_CALENDAR) +
									(R"HTML( </label>
                        </div>)HTML");
  }
  else if(type == "url") {
    input_block = input_block + (R"HTML(<div>
                                          <label>)HTML");
    input_block = input_block +	UncompressResourceAsString(IDR_ACTION_BLOCK_URL) +
									(R"HTML( </label>
                        </div>)HTML");
  }

  input_block = input_block + (R"HTML(<input oninput="onInputListener(this)" name=")HTML") + name+ (R"HTML(" placeholder=")HTML") + placeholder ;
  if(isRequired) {
    input_block = input_block + (R"HTML(*" required )HTML");
  }
  else{
    input_block = input_block + (R"HTML(")HTML");
  }

  if(action_spec_.tag == "malicious") {
    input_block = input_block + (R"HTML( disabled )HTML");
  }
  input_block = input_block + (R"HTML(class="bg-input-bg text-text-input placeholder:text-text-input-placeholder disabled:text-text-input-disabled min-h-7 min-w-0 flex-1 truncate outline-none" value)HTML");

  if(type == "email") {
    input_block = input_block + (R"HTML( type="email">)HTML");
  }
  else if(type == "number") {
    input_block = input_block + (R"HTML( step="any" type="number">)HTML");
  }
  else if(type == "datetime-local") {
    input_block = input_block + (R"HTML( type="datetime-local">)HTML");
  }
  else if(type == "date") {
    input_block = input_block + (R"HTML( type="date">)HTML");
  }
  else if(type == "url") {
    input_block = input_block + (R"HTML( type="url">)HTML");
  }
  else{
    input_block = input_block + (R"HTML( type="text">)HTML");
  }
  input_block = input_block + (R"HTML(</div>)HTML");

  if(is_standalone){
    input_block = input_block + CreateButton(label, true, String(href), true, true);
  }
  input_block = input_block + (R"HTML(</div>
                                    </div>)HTML");
  ScriptBlockStates::GetInstance().SetInputHandlerScriptState();
  return input_block;
}

//(R"HTML()HTML")
String ActionBlockCreator::CreateButton(String button_label,
                                        bool is_disabled,
                                        String href,
                                        bool is_form_submit_button,
                                        bool standalone_button) {
  if (!href.StartsWith("http://") && !href.StartsWith("https://")) {
    href = String("https://")+String(action_spec_.site_url) + href;
  }
  String button_layout = "";
  if(standalone_button) {
    button_layout = button_layout + (R"HTML(<div class="max-w-full flex-1 whitespace-nowrap">)HTML");
  } else {
    button_layout = button_layout + (R"HTML( <div class="flex flex-grow basis-[calc(33.333%-2*4px)]">)HTML");
  }

  if(is_form_submit_button) {  // its a form so button should be disabled
    button_layout = button_layout + (R"HTML(<button type="submit" class="rounded-button text-text relative flex w-full items-center justify-center text-nowrap px-5 py-3 font-semibold transition-colors motion-reduce:transition-none)HTML");
    button_layout = button_layout + (R"HTML( bg-button-disabled text-text-button-disabled" disabled>)HTML");
  }
  else {
   
    button_layout = button_layout + (R"HTML(<button onclick="handleButtonClick(')HTML") + href +  (R"HTML(')"class="rounded-button text-text relative flex w-full items-center justify-center text-nowrap px-5 py-3 font-semibold transition-colors motion-reduce:transition-none)HTML");
    if(is_disabled) {
      button_layout = button_layout + (R"HTML( bg-button-disabled text-text-button-disabled" disabled>)HTML");
    }
    else {
      // button_layout = button_layout + (R"HTML( bg-button text-text-button hover:bg-button-hover">)HTML");
      button_layout = button_layout + (R"HTML( bg-button text-text-button hover:bg-button-hover")HTML");
      if(action_spec_.tag == "malicious") {
        button_layout = button_layout + (R"HTML( disabled)HTML");
      }
      button_layout = button_layout + (R"HTML(>)HTML");
      ScriptBlockStates::GetInstance().SetButtonHandlerScriptState();
    }
  }
  
  button_layout = button_layout + (R"HTML(<span class="min-w-0 truncate">)HTML") + button_label;
  button_layout = button_layout +(R"HTML(</span>
													  </button>
												</div>)HTML");

  return button_layout;
}

String ActionBlockCreator::EnableAllScript() {
  String enable_form_script = (R"HTML(
      function enableAll_)HTML")+String(std::to_string(action_spec_.id))+(R"HTML(() {
        var action_div = document.getElementById("::action_block_)HTML")+String(std::to_string(action_spec_.id))+ (R"HTML(::");
        var warning_div = document.getElementById("warning_)HTML") +String(std::to_string(action_spec_.id))+(R"HTML(");
        var elements = action_div.querySelectorAll('input, select, textarea, button');
        elements.forEach(element => {
            element.disabled = false; // enable all form elements
        });
        warning_div.remove();
    }
  )HTML");
  return enable_form_script;
}

}  // namespace blink
