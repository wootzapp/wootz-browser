// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/action_url/content/renderer/action_url_agent.h"

#include "base/json/json_reader.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/thread_pool.h"
#include "base/values.h"
#include "content/public/renderer/render_frame.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/platform/web_url.h"
#include "third_party/blink/public/platform/web_vector.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_local_frame.h"

using blink::WebAnchorElement;
using blink::WebDocument;
using blink::WebLocalFrame;
using blink::WebString;
using blink::WebURL;
using blink::WebVector;
namespace action_url {
////////////////////////////////////////////////////////////////////////////////
// ActionUrlAgent, public:

ActionUrlAgent::ActionUrlAgent(content::RenderFrame* render_frame,
                               blink::AssociatedInterfaceRegistry* registry)
    : content::RenderFrameObserver(render_frame) {
  render_frame->GetWebFrame()->SetAnchorClient(this);
  registry->AddInterface<mojom::ActionUrlAgent>(base::BindRepeating(
      &ActionUrlAgent::BindPendingReceiver, base::Unretained(this)));
}

ActionUrlAgent::~ActionUrlAgent() = default;

void ActionUrlAgent::BindPendingReceiver(
    mojo::PendingAssociatedReceiver<mojom::ActionUrlAgent> pending_receiver) {
  receiver_.Bind(std::move(pending_receiver));
}

mojom::ActionUrlDriver& ActionUrlAgent::GetActionUrlDriver() {
  // if (IsPrerendering()) {
  //   if (!deferring_password_manager_driver_) {
  //     deferring_password_manager_driver_ =
  //         std::make_unique<DeferringPasswordManagerDriver>(this);
  //   }
  //   return *deferring_password_manager_driver_;
  // }

  // Lazily bind this interface.
  if (!action_url_driver_) {
    render_frame()->GetRemoteAssociatedInterfaces()->GetInterface(
        &action_url_driver_);
  }

  return *action_url_driver_;
}

void ActionUrlAgent::Reset() {
  process_anchor_after_dynamic_change_timer_.Stop();
  is_dom_content_loaded_ = false;
  action_block_counter_ = 1;
  renderer_anchor_cache_.clear();
  buffer_for_anchors_.clear();
  GetActionUrlDriver().Reset();
}

// mojom::ActionUrlAgent:
void ActionUrlAgent::SetUpHeader() {
  LOG(INFO) << "Unfurling :: " << __func__;
  render_frame()->GetWebFrame()->GetDocument().SetUpActionUrlHeader();
}

void ActionUrlAgent::ReplaceUrL(const std::string& json,
                                const AnchorData& anchor,
                                const GURL& action_url,
                                const std::string& tag) {
  LOG(INFO) << "Unfurling :: " << __func__;
  auto cached_anchor = renderer_anchor_cache_.find(anchor.renderer_id);
  if (cached_anchor == renderer_anchor_cache_.end()) {
    LOG(INFO) << "Unfurling :: anchor element not found";
    return;
  }

  WebAnchorElement anchor_element = cached_anchor->second;
  if (!anchor_element.IsConnected()) {
    LOG(INFO) << "Unfurling :: anchor element not connected to the dom tree anymore";
    return;
  }
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock()},
      base::BindOnce(&ActionUrlAgent::ParseJson, base::Unretained(this), json,
                     action_url, tag),
      base::BindOnce(&ActionUrlAgent::OnJsonParsed, base::Unretained(this),
                     anchor_element));
}

void ActionUrlAgent::SetUpScriptBlock() {
  LOG(INFO) << "Unfurling :: " << __func__;
  render_frame()->GetWebFrame()->GetDocument().SetUpActionUrlScriptBlock();
}

// blink::WebAnchorClient:
void ActionUrlAgent::DidAddAnchorElementDynamically(
    const blink::WebElement& element) {
  buffer_mu_.Acquire();
  buffer_for_anchors_.emplace_back(element.DynamicTo<WebAnchorElement>());
  buffer_mu_.Release();
  WaitTillDynamicChangeTimer(process_anchor_after_dynamic_change_timer_);
}

// content::RenderFrameObserver:
void ActionUrlAgent::DidCommitProvisionalLoad(ui::PageTransition transition) {
  Reset();
}

void ActionUrlAgent::DidCreateDocumentElement() {
  Reset();
  render_frame()->GetWebFrame()->GetDocument().ResetScriptState();
}

void ActionUrlAgent::DidDispatchDOMContentLoadedEvent() {
  // Parse the content and find all anchor elements
  LOG(INFO) << "Unfurling :: " << __func__;
  is_dom_content_loaded_ = true;
  render_frame()->GetWebFrame()->GetDocument().ResetScriptState();
  FindAnchorElementsOnPage(false);
}

void ActionUrlAgent::DidFinishLoad() {
  // Parse the content and find all anchor elements
}

void ActionUrlAgent::OnDestruct() {
  receiver_.reset();
}

void ActionUrlAgent::WaitTillDynamicChangeTimer(base::OneShotTimer& timer) {
  if (!is_dom_content_loaded_ || timer.IsRunning()) {
    return;
  }
  timer.Start(FROM_HERE, kAnchorSeenThrottle,
              base::BindOnce(&ActionUrlAgent::FindAnchorElementsOnPage,
                             base::Unretained(this), true));
}

// Top-level wrapper call to trigger DOM traversal to find anchor element.
void ActionUrlAgent::FindAnchorElementsOnPage(bool is_dynamic) {
  const blink::WebDocument doc = render_frame()->GetWebFrame()->GetDocument();
  if (doc.IsNull() || doc.Body().IsNull()) {
    return;
  }
  WebVector<WebAnchorElement> anchor_elements;
  if (is_dynamic) {
    buffer_mu_.Acquire();
    anchor_elements = buffer_for_anchors_;
    buffer_for_anchors_.clear();
    buffer_mu_.Release();
  } else {
    anchor_elements = doc.Anchors();
  }
  if (anchor_elements.size() == 0) {
    return;
  }

  LOG(INFO) << "Unfurling :: " << __func__
            << "; Anchors size: " << anchor_elements.size();

  std::vector<AnchorData> candidate_links;

  for (size_t iterator = 0; iterator < anchor_elements.size(); ++iterator) {
    auto& element = anchor_elements[iterator];
    blink::WebURL url = element.Url();
    if (!url.ProtocolIs(url::kHttpScheme) &&
        !url.ProtocolIs(url::kHttpsScheme) && !url.IsValid()) {
      continue;
    }
    if (url.GetString().Equals(
            "https://docs.dialect.to/documentation/actions/security") ||
        url.GetString().Equals("https://discord.gg/saydialect")) {
      // Thie block maybe added by this feature itself
      LOG(INFO) << "Unfurling :: Continuing as added by feature itself";
      continue;
    }
    std::unique_ptr<AnchorData> anchor_data(
        GetAnchorDataFromWebAnchor(element));
    if (!anchor_data) {
      continue;
    }

    renderer_anchor_cache_[anchor_data->renderer_id] = element;
    candidate_links.push_back(std::move(*anchor_data));
  }

  if (!candidate_links.empty()) {
    GetActionUrlDriver().AllAnchorsParsed(candidate_links);
  }
}

std::unique_ptr<AnchorData> ActionUrlAgent::GetAnchorDataFromWebAnchor(
    const WebAnchorElement& anchor_element) {
  AnchorData anchor;
  if (anchor_element.IsNull()) {
    DCHECK(anchor.renderer_id == 0);
    return nullptr;
  }
  anchor.url = GURL(anchor_element.Url());
  anchor.id_attribute = anchor_element.GetIdAttribute().Utf16();
  anchor.renderer_id = anchor_element.GetDomNodeId();
  return std::make_unique<AnchorData>(std::move(anchor));
}

base::ActionSpecJson ActionUrlAgent::ParseJson(const std::string& json_str,
                                               const GURL& action_url,
                                               const std::string& tag) {
  auto json = base::JSONReader::Read(json_str);
  base::ActionSpecJson action_spec;
  if (json && json->is_dict()) {
    const std::string* token;
    std::optional<bool> bool_token;
    action_spec.title =
        (token = json->GetDict().FindString("title")) ? *token : "";
    // action_spec.title = (token =
    // json->GetDict().FindString("title"))?base::UTF8ToUTF16(*token):std::u16string();
    action_spec.icon =
        (token = json->GetDict().FindString("icon")) ? *token : "";
    action_spec.description =
        (token = json->GetDict().FindString("description")) ? *token : "";
    action_spec.label =
        (token = json->GetDict().FindString("label")) ? *token : "";
    action_spec.error =
        (token = json->GetDict().FindString("error")) ? *token : "";
    bool_token = json->GetDict().FindBool("disabled");
    action_spec.disabled =
        (bool_token.has_value()) ? bool_token.value() : false;
    action_spec.site_url = action_url.host();
    action_spec.processDescription();
    const base::Value* links;
    const base::Value::List* action_list;
    if ((links = json->GetDict().Find("links")) &&
        (action_list = links->GetDict().FindList("actions"))) {
      for (const auto& action_token : *action_list) {
        if (action_token.is_dict()) {
          base::ActionSpecJson::Actions action;
          action.name =
              (token = action_token.GetDict().FindString("name")) ? *token : "";
          action.label = (token = action_token.GetDict().FindString("label"))
                             ? *token
                             : "";
          action.href =
              (token = action_token.GetDict().FindString("href")) ? *token : "";
          bool_token = action_token.GetDict().FindBool("required");
          action.required =
              (bool_token.has_value()) ? bool_token.value() : false;
          action.type =
              (token = action_token.GetDict().FindString("type")) ? *token : "";
          const base::Value::List* parameter_list;
          if ((parameter_list =
                   action_token.GetDict().FindList("parameters"))) {
            action.type = "form";
            for (const auto& param_token : *parameter_list) {
              if (param_token.is_dict()) {
                base::ActionSpecJson::Actions param;
                param.name = (token = param_token.GetDict().FindString("name"))
                                 ? *token
                                 : "";
                param.label =
                    (token = param_token.GetDict().FindString("label")) ? *token
                                                                        : "";
                param.href = (token = param_token.GetDict().FindString("href"))
                                 ? *token
                                 : "";
                param.patternDescription =
                    (token =
                         param_token.GetDict().FindString("patternDescription"))
                        ? *token
                        : "";
                bool_token = param_token.GetDict().FindBool("required");
                param.required =
                    (bool_token.has_value()) ? bool_token.value() : false;
                param.type = (token = param_token.GetDict().FindString("type"))
                                 ? *token
                                 : "";
                const base::Value::List* option_list;
                if ((option_list = param_token.GetDict().FindList("options"))) {
                  // param.type = "radio";
                  for (const auto& option_token : *option_list) {
                    if (option_token.is_dict()) {
                      base::ActionSpecJson::Options option;
                      option.value =
                          (token = option_token.GetDict().FindString("value"))
                              ? *token
                              : "";
                      option.label =
                          (token = option_token.GetDict().FindString("label"))
                              ? *token
                              : "";
                      bool_token = option_token.GetDict().FindBool("selected");
                      option.selected =
                          (bool_token.has_value()) ? bool_token.value() : false;
                      param.options.emplace_back(option);
                    }
                  }
                }
                param.ProcessType();
                action.parameters.emplace_back(param);
              }
            }
          }
          action.ProcessType();
          action_spec.links.emplace_back(action);
        }
      }
    }
  }

  // action_spec.PrintObject();
  action_spec.id = action_block_counter_;
  action_spec.tag = tag;
  action_block_counter_++;
  return action_spec;
}

void ActionUrlAgent::OnJsonParsed(WebAnchorElement anchor,
                                  base::ActionSpecJson action_spec) {
  anchor.ReplaceWithActionEelement(
      action_spec, base::BindOnce(&ActionUrlAgent::ActionBlockDrawCompleted,
                                  base::Unretained(this)));
}

void ActionUrlAgent::ActionBlockDrawCompleted() {
  LOG(INFO) << "Unfurling :: " << __func__;
  GetActionUrlDriver().OnBlockDrawCompleted();
}

}  // namespace action_url
