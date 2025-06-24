// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/action_url/content/browser/content_action_url_driver.h"

#include <utility>

#include "components/action_url/android/action_url_handler_android.h"
#include "components/action_url/content/browser/content_action_url_driver_factory.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/web_contents.h"
#include "chrome/browser/profiles/profile.h"
#include "components/prefs/pref_service.h"
#include "components/action_url/content/common/action_url_prefs.h"
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
  if (!rfh) {
    return false;
  }
  
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
  if (!render_frame_host_) {
    LOG(ERROR) << "AMIT ContentActionUrlDriver: render_frame_host is null!";
    return;
  }
  static unsigned next_free_id = 0;
  id_ = next_free_id++;
  handler_processing_remaining_ = 0;
  action_block_draw_remaining_ = 0;
  requested_for_header_ = false;

  if (!render_frame_host_->GetRemoteAssociatedInterfaces()) {
    LOG(ERROR) << "AMIT ContentActionUrlDriver: RemoteAssociatedInterfaces is null!";
    return;
  }
  
  render_frame_host_->GetRemoteAssociatedInterfaces()->GetInterface(
      &action_url_agent_);
  LOG(INFO) << "AMIT action_url_agent_ is bound: " << action_url_agent_.is_bound();
  LOG(INFO) << "AMIT ContentActionUrlDriver constructor after get interface";
}

ContentActionUrlDriver::~ContentActionUrlDriver() = default;

void ContentActionUrlDriver::BindPendingReceiver(
    mojo::PendingAssociatedReceiver<action_url::mojom::ActionUrlDriver>
        pending_receiver) {
  if (!render_frame_host_) {
    LOG(ERROR) << "AMIT BindPendingReceiver: render_frame_host_ is null!";
    return;
  }
  
  if (IsRenderFrameHostSupported(render_frame_host_)) {
    action_url_receiver_.Bind(std::move(pending_receiver));
  }
}

void ContentActionUrlDriver::DidNavigate() {
  if (!render_frame_host_) {
    LOG(ERROR) << "AMIT DidNavigate: render_frame_host_ is null!";
    action_url_receiver_.reset();
    return;
  }
  
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

  if (!render_frame_host_) {
    LOG(ERROR) << "AMIT AllAnchorsParsed: render_frame_host_ is null!";
    return;
  }

  Profile* profile = Profile::FromBrowserContext(render_frame_host_->GetBrowserContext());
  if (!profile->GetPrefs()->GetBoolean(action_url::prefs::kBlinksEnabled)) {
    LOG(INFO) << "AMIT Blinks are disabled";
    return;
  }
  else {
    LOG(INFO) << "AMIT Blinks are enabled";
  }

  LOG(INFO) << "AMIT All anchors parsed";
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
    LOG(INFO) << "AMIT Handler processing remaining before: " << handler_processing_remaining_;
    handler_processing_remaining_++;
    LOG(INFO) << "AMIT Handler processing remaining after: " << handler_processing_remaining_;
  }
}

void ContentActionUrlDriver::OnBlockDrawCompleted() {
  LOG(INFO) << "AMIT On block draw completed";
  LOG(INFO) << "Unfurling ::: " << __func__ << "; action_block_draw_remaining_: "
            << action_block_draw_remaining_;
  action_block_draw_remaining_--;
  // if (action_block_draw_remaining_ == 0) {
    LOG(INFO) << "AMIT action_block_draw_remaining_ is 0";
    LOG(INFO) << "GetActionUrlAgent: " << GetActionUrlAgent().is_bound();
    const auto& agent = GetActionUrlAgent();
    LOG(INFO) << "AMIT agent is not null";
    LOG(INFO) << "Unfurling ::" << __func__;
    LOG(INFO) << "AMIT Setting up action url script block in content action url driver";
    agent->SetUpScriptBlock();
    // }
  // }
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
    LOG(INFO) << "AMIT Action block draw remaining: " << action_block_draw_remaining_;
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
  if (!render_frame_host_) {
    LOG(ERROR) << "AMIT ProcessActionUrl: render_frame_host_ is null!";
    return;
  }

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
  LOG(INFO) << "AMIT GetActionUrlAgent";
  
  if (!render_frame_host_) {
    LOG(ERROR) << "AMIT GetActionUrlAgent: render_frame_host_ is null!";
    return action_url_agent_unbound_;
  }
  
  CHECK_NE(render_frame_host_->GetLifecycleState(),
           content::RenderFrameHost::LifecycleState::kPendingCommit);
  LOG(INFO) << "AMIT GetActionUrlAgent 2";
  LOG(INFO) << "AMIT IsRenderFrameHostSupported: " << IsRenderFrameHostSupported(render_frame_host_);
  LOG(INFO) << "AMIT action_url_agent_unbound_: " << action_url_agent_unbound_.is_bound();
  LOG(INFO) << "AMIT action_url_agent_: " << action_url_agent_.is_bound();
  return IsRenderFrameHostSupported(render_frame_host_)
             ? action_url_agent_
             : action_url_agent_unbound_;
}

}  // namespace action_url