// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/replace_element/content/renderer/replace_element_agent.h"

#include <string>
#include <sstream>

#include "base/json/json_reader.h"
#include "base/values.h"

#include "content/public/renderer/render_frame.h"

#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_element.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/dom/element.h"
#include "third_party/blink/renderer/core/dom/text.h"
#include "third_party/blink/renderer/core/html/html_element.h"
#include "third_party/blink/renderer/platform/json/json_parser.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace {
  // Set of self-closing tags
    const std::set<std::string> selfClosingTags = {
        "img", "input", "br", "hr", "meta", "link"
    };

    std::string convertStyleToString(const base::Value::Dict& style) {
        std::stringstream ss;
        for (auto it = style.begin(); it != style.end(); ++it) {
            ss << it->first << ": " << it->second.GetString() << "; ";
        }
        return ss.str();
    }

    std::string convertAttributesToString(const base::Value::Dict& attributes) {
        std::stringstream ss;
        for (auto it = attributes.begin(); it != attributes.end(); ++it) {
            if (it->first == "style") {
                if (const base::Value::Dict* style = it->second.GetIfDict()) {
                    ss << "style=\"" << convertStyleToString(*style) << "\" ";
                }
            } else {
                // Handle numeric attributes (like width, height)
                if (it->second.is_string()) {
                    ss << it->first << "=\"" << it->second.GetString() << "\" ";
                } else if (it->second.is_int()) {
                    ss << it->first << "=\"" << it->second.GetInt() << "\" ";
                } else if (it->second.is_double()) {
                    ss << it->first << "=\"" << it->second.GetDouble() << "\" ";
                }
            }
        }
        return ss.str();
    }

    bool isSelfClosingTag(const std::string& elementType) {
        return selfClosingTags.find(elementType) != selfClosingTags.end();
    }

    std::string processElement(const base::Value::Dict& element) {
        std::stringstream html;
        
        // Get element type
        const std::string* elementType = element.FindString("element");
        if (!elementType) {
            return ""; // Skip invalid elements
        }

        // Opening tag
        html << "<" << *elementType;
        
        // Add attributes if present
        if (const base::Value::Dict* attributes = element.FindDict("attributes")) {
            html << " " << convertAttributesToString(*attributes);
        }

        // Handle self-closing tags differently
        if (isSelfClosingTag(*elementType)) {
            html << " />";
            return html.str();
        }

        html << ">";

        // Add content if present
        if (const std::string* content = element.FindString("content")) {
            html << *content;
        }

        // Process children if present
        if (const base::Value::List* children = element.FindList("children")) {
            for (const auto& child : *children) {
                if (const base::Value::Dict* childDict = child.GetIfDict()) {
                    html << processElement(*childDict);
                }
            }
        }

        // Closing tag (not needed for self-closing elements)
        html << "</" << *elementType << ">";

        return html.str();
    }

    std::string convertJSONToHTML(const std::string& jsonStr) {
        absl::optional<base::Value> parsed = base::JSONReader::Read(jsonStr);
        if (!parsed || !parsed->is_dict()) {
            return "Error parsing JSON";
        }
        
        return processElement(parsed->GetDict());
    }
}

namespace replace_element {

ReplaceElementAgent::ReplaceElementAgent(content::RenderFrame* render_frame,
                               blink::AssociatedInterfaceRegistry* registry)
    : content::RenderFrameObserver(render_frame) {
  registry->AddInterface<mojom::ReplaceElementAgent>(base::BindRepeating(
      &ReplaceElementAgent::BindPendingReceiver, base::Unretained(this)));
}

ReplaceElementAgent::~ReplaceElementAgent() = default;

void ReplaceElementAgent::BindPendingReceiver(
    mojo::PendingAssociatedReceiver<mojom::ReplaceElementAgent> pending_receiver) {
  receiver_.Bind(std::move(pending_receiver));
}

void ReplaceElementAgent::ReplaceElement(const std::string& element, const std::string& json_string) {
  LOG(INFO) << "ElementReplacer ReplaceElementAgent::ReplaceElement";
  LOG(INFO) << "ElementReplacer renderer side: element::" << element;
  LOG(INFO) << "ElementReplacer renderer side: json_data::" << json_string;
  // Get the current document
  blink::WebLocalFrame* frame = render_frame()->GetWebFrame();
  if (!frame)
    return;
  blink::WebDocument document = frame->GetDocument();
  if (document.IsNull())
    return;
  blink::WebString web_selector = blink::WebString::FromUTF8(element);
  // Find the element to replace
  blink::WebElement web_element = document.QuerySelector(web_selector);
  if (web_element.IsNull()) {
    LOG(INFO) << "ElementReplacer Element not found: " << element;
    return;
  }

   // Parse JSON
  std::string new_element = convertJSONToHTML(json_string);
  LOG(INFO) << "ElementReplacer new_element: "<<new_element;

  // Replace the element's content with the JSON data
  web_element.ReplaceElement(String(new_element));
}

void ReplaceElementAgent::DidDispatchDOMContentLoadedEvent() {
  LOG(INFO) << "ElementReplacer " << __func__;
  GetReplaceElementDriver().CallToDriver();
}

void ReplaceElementAgent::OnDestruct() {}

mojom::ReplaceElementDriver& ReplaceElementAgent::GetReplaceElementDriver() {
  // if (IsPrerendering()) {
  //   if (!deferring_password_manager_driver_) {
  //     deferring_password_manager_driver_ =
  //         std::make_unique<DeferringPasswordManagerDriver>(this);
  //   }
  //   return *deferring_password_manager_driver_;
  // }

  // Lazily bind this interface.
  if (!replace_element_driver_) {
    render_frame()->GetRemoteAssociatedInterfaces()->GetInterface(
        &replace_element_driver_);
  }

  return *replace_element_driver_;
}

}  // namespace replace_element