// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/action_url/content/browser/content_action_url_driver.h"

#include <utility>

#include "components/action_url/android/action_url_handler_android.h"
#include "components/action_url/content/browser/content_action_url_driver_factory.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "net/http/http_status_code.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "url/gurl.h"

namespace action_url {

constexpr int maxResponseSizeInKiB = 1024;

class ActionUrlHandlerAndroid;

namespace {

bool IsRenderFrameHostSupported(content::RenderFrameHost* rfh) {
  if (rfh->GetLifecycleState() ==
      content::RenderFrameHost::LifecycleState::kPendingCommit) {
    return true;
  }

  if (rfh->GetLifecycleState() ==
      content::RenderFrameHost::LifecycleState::kPrerendering) {
    return false;
  }
  return true;
}

}  // namespace

ContentActionUrlDriver::ContentActionUrlDriver(
    content::RenderFrameHost* render_frame_host)
    : render_frame_host_(render_frame_host) {
  static unsigned next_free_id = 0;
  id_ = next_free_id++;
  handler_processing_remaining_ = 0;
  action_block_draw_remaining_ = 0;
  requested_for_header_ = false;

  render_frame_host_->GetRemoteAssociatedInterfaces()->GetInterface(
      &action_url_agent_);
}

ContentActionUrlDriver::~ContentActionUrlDriver() = default;

void ContentActionUrlDriver::BindPendingReceiver(
    mojo::PendingAssociatedReceiver<action_url::mojom::ActionUrlDriver>
        pending_receiver) {
  if (IsRenderFrameHostSupported(render_frame_host_)) {
    action_url_receiver_.Bind(std::move(pending_receiver));
  }
}

void ContentActionUrlDriver::DidNavigate() {
  if (!IsRenderFrameHostSupported(render_frame_host_)) {
    action_url_receiver_.reset();
  }
}

int ContentActionUrlDriver::GetId() const {
  return id_;
}

// action_url::mojom::ActionUrlDriver:
void ContentActionUrlDriver::AllAnchorsParsed(
    const std::vector<action_url::AnchorData>& anchors_data) {
  LOG(INFO) << "Unfurling ::" << __func__ << "; Anchors size: " << anchors_data.size();

  std::vector<action_url::AnchorData> anchors = anchors_data;
  for (auto& anchor : anchors) {
    anchor.host_frame = render_frame_host_->GetFrameToken();
    anchor.process_id = render_frame_host_->GetProcess()->GetID();
    anchor.driver_id = GetId();
    auto handler = std::make_unique<ActionUrlHandlerAndroid>(anchor);
    handler->GetActionUrlForBlinkUrl(
        base::BindOnce(&ContentActionUrlDriver::ActionUrlFetched,
                       weak_factory_.GetWeakPtr(), anchor));
    handlers_list_.emplace_back(std::move(handler));
    handler_processing_remaining_++;
  }
}

void ContentActionUrlDriver::OnBlockDrawCompleted() {
  LOG(INFO) << "Unfurling ::: " << __func__ << "; action_block_draw_remaining_: "
            << action_block_draw_remaining_;
  action_block_draw_remaining_--;
  if (action_block_draw_remaining_ == 0) {
    if (const auto& agent = GetActionUrlAgent()) {
      LOG(INFO) << "Unfurling ::" << __func__;
      agent->SetUpScriptBlock();
    }
  }
}

void ContentActionUrlDriver::Reset() {
  handlers_list_.clear();
  handler_processing_remaining_ = 0;
  action_block_draw_remaining_ = 0;
  requested_for_header_ = false;
}

void ContentActionUrlDriver::ActionUrlFetched(action_url::AnchorData anchor,
                                              std::string acion_url,
                                              std::string tag) {
  if (anchor.driver_id != GetId()) {
    return;
  }

  handler_processing_remaining_--;

  if (!acion_url.empty()) {
    LOG(INFO) << "Unfurling ::" << __func__ << "Action url is: " << acion_url;
    if (!requested_for_header_) {
      if (const auto& agent = GetActionUrlAgent()) {
        agent->SetUpHeader();
        requested_for_header_ = true;
      }
    }
    action_block_draw_remaining_++;
    ProcessActionUrl(GURL(acion_url), anchor, tag);
  }

  if (handler_processing_remaining_ == 0) {
    handlers_list_.clear();
  }
}

void ContentActionUrlDriver::ProcessActionUrl(GURL action_url,
                                              action_url::AnchorData anchor,
                                              std::string tag) {
  std::unique_ptr<network::ResourceRequest> request =
      std::make_unique<network::ResourceRequest>();
  request->url = action_url;
  request->method = "GET";

  net::NetworkTrafficAnnotationTag traffic_annotation =
      net::DefineNetworkTrafficAnnotation("action_json_donloader", R"(
        semantics {
          sender: "Action json fetcher"
          description:
            "Download action json"
          trigger:
            "A page with action url link "
          data: "None."
          destination: WEBSITE
        }
        policy {
          cookies_allowed: NO
          setting:
            "This feature is enabled in settings."
          policy_exception_justification: "Not implemented."
        })");

  std::unique_ptr<network::SimpleURLLoader> url_loader =
      network::SimpleURLLoader::Create(std::move(request), traffic_annotation);

  network::SimpleURLLoader* url_loader_ptr = url_loader.get();
  url_loader_ptr->DownloadToString(
      render_frame_host_->GetStoragePartition()
          ->GetURLLoaderFactoryForBrowserProcess()
          .get(),
      base::BindOnce(&ContentActionUrlDriver::OnDownloadedJson,
                     weak_factory_.GetWeakPtr(), std::move(url_loader), anchor,
                     action_url, tag),
      maxResponseSizeInKiB * 1024);
}

void ContentActionUrlDriver::OnDownloadedJson(
    std::unique_ptr<network::SimpleURLLoader> url_loader,
    action_url::AnchorData anchor,
    GURL action_url,
    std::string tag,
    std::unique_ptr<std::string> response_body) {
  auto* response_info = url_loader->ResponseInfo();

  int response_code = response_info && response_info->headers
                          ? response_info->headers->response_code()
                          : url_loader->NetError();

  LOG(INFO) << "Unfurling ::: Response code is: " << response_code;
  if (response_code == net::HTTP_OK) {
    if (const auto& agent = GetActionUrlAgent()) {
      agent->ReplaceUrL(*response_body, anchor, action_url, tag);
    }
  } else {
    LOG(INFO) << "Unfurling ::: Couldn't download json. Response code is: "
              << response_code;
  }
}

const mojo::AssociatedRemote<action_url::mojom::ActionUrlAgent>&
ContentActionUrlDriver::GetActionUrlAgent() {
  CHECK_NE(render_frame_host_->GetLifecycleState(),
           content::RenderFrameHost::LifecycleState::kPendingCommit);

  return IsRenderFrameHostSupported(render_frame_host_)
             ? action_url_agent_
             : action_url_agent_unbound_;
}

}  // namespace action_url
