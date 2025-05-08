// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_ACTION_SPEC_JSON_H_
#define BASE_ANDROID_ACTION_SPEC_JSON_H_

#include <memory>
#include <string>
#include <vector>

#include "base/base_export.h"

namespace base {

class BASE_EXPORT ActionSpecJson {
 public:
  // Enum to represent different element types
  enum class ElementType {
    PARAGRAPH,
    ORDERED_LIST_ITEM,
    UNORDERED_LIST_ITEM,
    HEADER,
    HORIZONTAL_RULE,
    BREAK,
  };

  struct FormattedText {
    std::string text;
    bool is_bold = false;
    bool is_italic = false;
    bool is_link = false;
    std::string url = "";
  };

  // Struct to represent a structured document element
  struct DocumentElement {
    DocumentElement();
    DocumentElement(const DocumentElement&);
    DocumentElement& operator=(const DocumentElement&);
    DocumentElement(DocumentElement&&);
    DocumentElement& operator=(DocumentElement&&);
    ~DocumentElement();

    ElementType type;
    std::vector<FormattedText> content;
  };

  struct Options {
    std::string label = "";
    std::string value = "";
    bool selected = false;
  };

  struct Actions {
    Actions();
    Actions(const Actions&);
    Actions& operator=(const Actions&);
    Actions(Actions&&);
    Actions& operator=(Actions&&);
    ~Actions();

    std::string name = "";
    std::string label = "";
    std::string href = "";
    std::vector<Actions> parameters;
    std::vector<Options> options;
    bool required = false;
    std::string type = "unknown";
    std::string patternDescription = "";

    void ProcessType();
    void PrintObject() const;
  };

  ActionSpecJson();
  ActionSpecJson(const ActionSpecJson&);
  ActionSpecJson& operator=(const ActionSpecJson&);
  ActionSpecJson(ActionSpecJson&&);
  ActionSpecJson& operator=(ActionSpecJson&&);
  ~ActionSpecJson();

  // Member variables
  std::string icon = "";
  std::string title = "";
  std::string description = "";
  std::string label = "";
  std::string error = "";
  std::string site_url = "";
  bool disabled = false;
  std::vector<Actions> links;
  std::vector<DocumentElement> descriptionParts;
  bool all_description_is_bullet = true;
  int id = -1;
  std::string tag = "";

  void parseStringAndURLFormattedText(const std::string& input,
                                      std::vector<FormattedText>& result);
  void parseURLFormattedText(const std::string& input,
                             std::vector<FormattedText>& result);
  void parseItalicFormattedText(const std::string& input,
                                std::vector<FormattedText>& result);
  std::vector<FormattedText> parseFormattedText(const std::string& input);
  std::vector<DocumentElement> parse(const std::string& markdown);
  void processDescription();
  void PrintObject() const;
};

}  // namespace base

#endif  // BASE_ANDROID_ACTION_SPEC_JSON_H_
