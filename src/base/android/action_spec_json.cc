// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/action_spec_json.h"

#include <algorithm>
#include <cstdint>
#include <regex>
#include <sstream>
#include <string>

#include "base/logging.h"

namespace base {

ActionSpecJson::ActionSpecJson() = default;

ActionSpecJson::ActionSpecJson(const ActionSpecJson&) = default;

ActionSpecJson& ActionSpecJson::operator=(const ActionSpecJson&) = default;

ActionSpecJson::ActionSpecJson(ActionSpecJson&&) = default;

ActionSpecJson& ActionSpecJson::operator=(ActionSpecJson&&) = default;

ActionSpecJson::~ActionSpecJson() = default;

ActionSpecJson::Actions::Actions() = default;

ActionSpecJson::Actions::Actions(const Actions&) = default;

ActionSpecJson::Actions& ActionSpecJson::Actions::operator=(const Actions&) =
    default;

ActionSpecJson::Actions::Actions(Actions&&) = default;

ActionSpecJson::Actions& ActionSpecJson::Actions::operator=(Actions&&) =
    default;

ActionSpecJson::Actions::~Actions() = default;

ActionSpecJson::DocumentElement::DocumentElement() = default;

ActionSpecJson::DocumentElement::DocumentElement(const DocumentElement&) =
    default;

ActionSpecJson::DocumentElement& ActionSpecJson::DocumentElement::operator=(
    const DocumentElement&) = default;

ActionSpecJson::DocumentElement::DocumentElement(DocumentElement&&) = default;

ActionSpecJson::DocumentElement& ActionSpecJson::DocumentElement::operator=(
    DocumentElement&&) = default;

ActionSpecJson::DocumentElement::~DocumentElement() = default;

void ActionSpecJson::Actions::ProcessType() {
  if (!type.empty()) {
    return;  // radio  || form || post || select
  }

  if (parameters.empty()) {
    if (href.empty()) {
      if (!name.empty()) {
        type = "inputText";
      } else {
        type = "unknown";
      }
    } else {
      type = "button";
    }
  } else {
    type = "unknown";
  }
}

void ActionSpecJson::Actions::PrintObject() const {
  LOG(INFO) << "Unfurling :: name:" << this->name;
  LOG(INFO) << "Unfurling :: label:" << this->label;
  LOG(INFO) << "Unfurling :: href:" << this->href;
  LOG(INFO) << "Unfurling :: required:" << this->required;
  LOG(INFO) << "Unfurling :: type:" << this->type;
  LOG(INFO) << "Unfurling :: parameters size:" << this->parameters.size();
  for (const auto& action : this->parameters) {
    LOG(INFO) << "Unfurling :: =======parameters==========";
    action.PrintObject();
  }
  LOG(INFO) << "Unfurling :: options size:" << this->options.size();
  for (const auto& option : this->options) {
    LOG(INFO) << "Unfurling :: value:" << option.value;
    LOG(INFO) << "Unfurling :: label:" << option.label;
    LOG(INFO) << "Unfurling :: selected:" << option.selected;
  }
}

void ActionSpecJson::parseStringAndURLFormattedText(
    const std::string& input,
    std::vector<FormattedText>& result) {
  std::regex link_regex(
      R"((https?:\/\/|www\.)[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,}(\/[^\s]*)?)");

  // Split the input into segments
  std::sregex_iterator it(input.begin(), input.end(), link_regex);
  std::sregex_iterator end;

  size_t last_pos = 0;
  // Process URL text
  while (it != end) {
    // Add text before URL segment
    std::smatch match = *it;
    if (static_cast<size_t>(match.position()) > last_pos) {
      std::string plain_text = input.substr(
          last_pos, static_cast<size_t>(match.position()) - last_pos);
      result.push_back({plain_text, false, false, false, ""});
    }

    result.push_back({match.str(), false, false, true, match.str()});

    last_pos = static_cast<size_t>(match.position() + match.length());
    ++it;
  }
  if (last_pos < input.length()) {
    std::string remaining = input.substr(last_pos);
    result.push_back({remaining, false, false, false, ""});
  }
}

void ActionSpecJson::parseURLFormattedText(const std::string& input,
                                           std::vector<FormattedText>& result) {
  std::regex link_regex("\\[(.+?)\\]\\((.+?)\\)");

  // Split the input into segments
  std::sregex_iterator it(input.begin(), input.end(), link_regex);
  std::sregex_iterator end;

  size_t last_pos = 0;
  // Process URL text
  while (it != end) {
    // Add text before URL segment
    if (static_cast<size_t>(it->position()) > last_pos) {
      std::string plain_text = input.substr(
          last_pos, static_cast<size_t>(it->position()) - last_pos);
      parseStringAndURLFormattedText(plain_text, result);
    }

    result.push_back({it->str(1), false, false, true, it->str(2)});

    last_pos = static_cast<size_t>(it->position() + it->length());
    ++it;
  }
  if (last_pos < input.length()) {
    std::string remaining = input.substr(last_pos);
    parseStringAndURLFormattedText(remaining, result);
  }
}

void ActionSpecJson::parseItalicFormattedText(
    const std::string& input,
    std::vector<ActionSpecJson::FormattedText>& result) {
  std::regex italic_regex("\\*(.*?)\\*");

  // Split the input into segments
  std::sregex_iterator it(input.begin(), input.end(), italic_regex);
  std::sregex_iterator end;

  size_t last_pos = 0;
  // Process italic text
  while (it != end) {
    // Add text before italic segment
    if (static_cast<size_t>(it->position()) > last_pos) {
      std::string plain_text = input.substr(
          last_pos, static_cast<size_t>(it->position()) - last_pos);
      parseURLFormattedText(plain_text, result);
    }

    // Add italic text
    result.push_back({it->str(1), false, true, false, ""});

    last_pos = static_cast<size_t>(it->position() + it->length());
    ++it;
  }
  if (last_pos < input.length()) {
    std::string remaining = input.substr(last_pos);
    parseURLFormattedText(remaining, result);
  }
}

std::vector<ActionSpecJson::FormattedText> ActionSpecJson::parseFormattedText(
    const std::string& input) {
  std::vector<FormattedText> result;
  std::regex bold_regex("\\*\\*(.*?)\\*\\*");

  // Split the input into segments
  std::sregex_iterator it(input.begin(), input.end(), bold_regex);
  std::sregex_iterator end;

  size_t last_pos = 0;

  // Process bold text
  while (it != end) {
    // Add text before bold segment
    if (static_cast<size_t>(it->position()) > last_pos) {
      std::string plain_text = input.substr(
          last_pos, static_cast<size_t>(it->position()) - last_pos);
      parseItalicFormattedText(plain_text, result);
    }

    // Add bold text
    result.push_back({it->str(1), true, false, false, ""});

    last_pos = static_cast<size_t>(it->position() + it->length());
    ++it;
  }
  if (last_pos < input.length()) {
    std::string remaining = input.substr(last_pos);
    parseItalicFormattedText(remaining, result);
  }

  return result;
}

std::vector<ActionSpecJson::DocumentElement> ActionSpecJson::parse(
    const std::string& markdown) {
  std::istringstream iss(markdown);
  std::vector<DocumentElement> elements;
  std::string line;
  std::regex pattern(R"(^\d\.)");

  while (std::getline(iss, line)) {
    // Trim leading and trailing whitespaces
    line.erase(0, line.find_first_not_of(" \t"));
    line.erase(line.find_last_not_of(" \t") + 1);
    DocumentElement current;

    if (line.empty()) {
      current.type = ElementType::BREAK;
    }

    // Process headers
    else if (line.substr(0, 3) == "###") {
      line = line.substr(3);
      line.erase(0, line.find_first_not_of(" \t"));

      current.type = ElementType::HEADER;
      current.content = parseFormattedText(line);
    }
    // Process horizontal rule
    else if (line == "---") {
      current.type = ElementType::HORIZONTAL_RULE;
    }
    // Process ordered lists
    else if (std::regex_search(line, pattern)) {
      line = line.substr(2);
      line.erase(0, line.find_first_not_of(" \t"));

      current.type = ElementType::ORDERED_LIST_ITEM;
      current.content = parseFormattedText(line);
    }
    // Process Unordered lists
    else if (line.rfind("- ", 0) == 0) {
      line = line.substr(2);
      line.erase(0, line.find_first_not_of(" \t"));

      current.type = ElementType::UNORDERED_LIST_ITEM;
      current.content = parseFormattedText(line);
    }
    // Process paragraphs
    else {
      current.type = ElementType::PARAGRAPH;
      current.content = parseFormattedText(line);
    }
    elements.push_back(std::move(current));
  }

  return elements;
}

void ActionSpecJson::processDescription() {
  this->descriptionParts = parse(this->description);
}

void ActionSpecJson::PrintObject() const {
  LOG(INFO) << "Unfurling :: title:" << this->title;
  LOG(INFO) << "Unfurling :: icon:" << this->icon;
  LOG(INFO) << "Unfurling :: label:" << this->label;
  LOG(INFO) << "Unfurling :: error:" << this->error;
  LOG(INFO) << "Unfurling :: site_url:" << this->site_url;
  LOG(INFO) << "Unfurling :: disabled:" << this->disabled;
  LOG(INFO) << "Unfurling :: tag:" << this->tag;
  LOG(INFO) << "Unfurling :: links size:" << this->links.size();
  for (const auto& action : this->links) {
    LOG(INFO) << "Unfurling :: =======Action==========";
    action.PrintObject();
  }
}

}  // namespace base
