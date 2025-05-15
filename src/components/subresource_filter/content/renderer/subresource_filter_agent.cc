// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/subresource_filter/content/renderer/subresource_filter_agent.h"

#include <utility>
#include <fstream>
#include <sstream>

#include "base/check.h"
#include "base/feature_list.h"
#include "base/functional/bind.h"
#include "base/memory/ref_counted.h"
#include "base/metrics/histogram_macros.h"
#include "base/not_fatal_until.h"
#include "base/time/time.h"
#include "components/subresource_filter/content/renderer/unverified_ruleset_dealer.h"
#include "components/subresource_filter/content/shared/common/subresource_filter_utils.h"
#include "components/subresource_filter/content/shared/renderer/web_document_subresource_filter_impl.h"
#include "components/subresource_filter/core/common/document_subresource_filter.h"
#include "components/subresource_filter/core/common/memory_mapped_ruleset.h"
#include "components/subresource_filter/core/common/scoped_timers.h"
#include "components/subresource_filter/core/common/time_measurements.h"
#include "content/public/common/content_features.h"
#include "content/public/common/url_constants.h"
#include "content/public/renderer/render_frame.h"
#include "ipc/ipc_message.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"
#include "third_party/blink/public/common/features.h"
#include "third_party/blink/public/common/frame/frame_ad_evidence.h"
#include "third_party/blink/public/platform/web_worker_fetch_context.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_document_loader.h"
#include "third_party/blink/public/web/web_element.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/web/web_node.h"
#include "third_party/blink/public/web/web_element_collection.h"
#include "url/url_constants.h"

namespace {

bool IsFencedFrameRoot(content::RenderFrame* frame) {
  // Unit tests may have a nullptr render_frame.
  if (!frame)
    return false;
  return frame->IsInFencedFrameTree() && frame->IsMainFrame();
}

}  // namespace

namespace subresource_filter {

SubresourceFilterAgent::SubresourceFilterAgent(
    content::RenderFrame* render_frame,
    UnverifiedRulesetDealer* ruleset_dealer)
    : content::RenderFrameObserver(render_frame),
      content::RenderFrameObserverTracker<SubresourceFilterAgent>(render_frame),
      ruleset_dealer_(ruleset_dealer) {
  CHECK(ruleset_dealer, base::NotFatalUntil::M129);
}

void SubresourceFilterAgent::Initialize() {
  const GURL& url = GetDocumentURL();
  // The initial empty document will always inherit activation.
  CHECK(ShouldInheritActivation(url), base::NotFatalUntil::M129);

  // We must check for provisional here because in that case 2 RenderFrames will
  // be created for the same FrameTreeNode in the browser. The browser service
  // only expects us to call SendFrameWasCreatedByAdScript() and
  // SendFrameIsAd() a single time each for a newly created RenderFrame,
  // so we must choose one. A provisional frame is created when a navigation is
  // performed cross-site and the navigation is done there to isolate it from
  // the previous frame tree. We choose to send this message from the initial
  // (non-provisional) "about:blank" frame that is created before the navigation
  // to match previous behaviour, and because this frame will always exist.
  // Whereas the provisional frame would only be created to perform the
  // navigation conditionally, so we ignore sending the IPC there.
  if (IsSubresourceFilterChild() && !IsFencedFrameRoot(render_frame()) &&
      !IsProvisional()) {
    // Note: We intentionally exclude fenced-frame roots here since they do not
    // create a RenderFrame in the creating renderer. By the time the fenced
    // frame initializes its RenderFrame in a new process,
    // IsFrameCreatedByAdScript will not see the creating call stack. Fenced
    // frames compute and send this information to the browser from
    // DidCreateFencedFrame which is called by the creating RenderFrame.
    // Additionally, there's no need to set evidence for the initial empty
    // subframe since the fenced frame is isolated from its embedder.
    if (IsFrameCreatedByAdScript())
      SendFrameWasCreatedByAdScript();

    // As this is the initial empty document, we won't have received any message
    // from the browser and so we must populate the ad evidence here.
    SetAdEvidenceForInitialEmptySubframe();
  }

  // `render_frame()` can be null in unit tests.
  if (render_frame()) {
    render_frame()
        ->GetAssociatedInterfaceRegistry()
        ->AddInterface<mojom::SubresourceFilterAgent>(base::BindRepeating(
            &SubresourceFilterAgent::OnSubresourceFilterAgentRequest,
            base::Unretained(this)));

    if (!IsSubresourceFilterChild()) {
      // If a root frame has an activated opener, we will activate the
      // subresource filter for the initial empty document, which was created
      // before the constructor for `this`. This ensures that a popup's final
      // document is appropriately activated, even when the the initial
      // navigation is aborted and there are no further documents created.
      // TODO(dcheng): Navigation is an asynchronous operation, and the opener
      // frame may have been destroyed between the time the window is opened
      // and the RenderFrame in the window is constructed leading us to here.
      // To avoid that race condition the activation state would need to be
      // determined without the use of the opener frame.
      if (GetInheritedActivationState(render_frame()).activation_level !=
          mojom::ActivationLevel::kDisabled) {
        ConstructFilter(GetInheritedActivationStateForNewDocument(), url);
      }
    } else {
      // Child frames always have a parent, so the empty initial document can
      // always inherit activation.
      ConstructFilter(GetInheritedActivationStateForNewDocument(), url);
    }
  }
}

SubresourceFilterAgent::~SubresourceFilterAgent() = default;

GURL SubresourceFilterAgent::GetDocumentURL() {
  return render_frame()->GetWebFrame()->GetDocument().Url();
}

bool SubresourceFilterAgent::IsSubresourceFilterChild() {
  return !render_frame()->IsMainFrame() ||
         render_frame()->IsInFencedFrameTree();
}

bool SubresourceFilterAgent::IsParentAdFrame() {
  // A fenced frame root should never ask this since it can't see the outer
  // frame tree. Its AdEvidence is always computed by the browser.
  CHECK(!IsFencedFrameRoot(render_frame()), base::NotFatalUntil::M129);
  return render_frame()->GetWebFrame()->Parent()->IsAdFrame();
}

bool SubresourceFilterAgent::IsProvisional() {
  return render_frame()->GetWebFrame()->IsProvisional();
}

bool SubresourceFilterAgent::IsFrameCreatedByAdScript() {
  CHECK(!IsFencedFrameRoot(render_frame()), base::NotFatalUntil::M129);
  return render_frame()->GetWebFrame()->IsFrameCreatedByAdScript();
}

void SubresourceFilterAgent::SetSubresourceFilterForCurrentDocument(
    std::unique_ptr<blink::WebDocumentSubresourceFilter> filter) {
  
  // Get the WebLocalFrame
  blink::WebLocalFrame* web_frame = render_frame()->GetWebFrame();
  if (!web_frame || !web_frame->GetDocumentLoader())
    return;
  
  // Connect our callback to be notified of blocked resources
  if (filter) {
    auto* filter_impl = static_cast<WebDocumentSubresourceFilterImpl*>(filter.get());
    filter_impl->SetBlockedResourceCallback(
        base::BindRepeating(&SubresourceFilterAgent::OnResourceBlockedByFilter,
                          base::Unretained(this)));
  }
  
  web_frame->GetDocumentLoader()->SetSubresourceFilter(filter.release());
}

void SubresourceFilterAgent::
    SignalFirstSubresourceDisallowedForCurrentDocument() {
  GetSubresourceFilterHost()->DidDisallowFirstSubresource();
}

void SubresourceFilterAgent::SendDocumentLoadStatistics(
    const mojom::DocumentLoadStatistics& statistics) {
  GetSubresourceFilterHost()->SetDocumentLoadStatistics(statistics.Clone());
}

void SubresourceFilterAgent::SendFrameIsAd() {
  GetSubresourceFilterHost()->FrameIsAd();
}

void SubresourceFilterAgent::SendFrameWasCreatedByAdScript() {
  CHECK(!IsFencedFrameRoot(render_frame()), base::NotFatalUntil::M129);
  GetSubresourceFilterHost()->FrameWasCreatedByAdScript();
}

bool SubresourceFilterAgent::IsAdFrame() {
  return render_frame()->GetWebFrame()->IsAdFrame();
}

void SubresourceFilterAgent::SetAdEvidence(
    const blink::FrameAdEvidence& ad_evidence) {
  render_frame()->GetWebFrame()->SetAdEvidence(ad_evidence);
}

const std::optional<blink::FrameAdEvidence>&
SubresourceFilterAgent::AdEvidence() {
  return render_frame()->GetWebFrame()->AdEvidence();
}

// static
mojom::ActivationState SubresourceFilterAgent::GetInheritedActivationState(
    content::RenderFrame* render_frame) {
  if (!render_frame)
    return mojom::ActivationState();

  // A fenced frame is isolated from its outer embedder so we cannot inspect
  // the parent's activation state. However, that's ok because the embedder
  // cannot script the fenced frame so we can wait until a navigation to set
  // activation state.
  if (IsFencedFrameRoot(render_frame))
    return mojom::ActivationState();

  blink::WebFrame* frame_to_inherit_from =
      render_frame->IsMainFrame() ? render_frame->GetWebFrame()->Opener()
                                  : render_frame->GetWebFrame()->Parent();

  if (!frame_to_inherit_from || !frame_to_inherit_from->IsWebLocalFrame())
    return mojom::ActivationState();

  blink::WebSecurityOrigin render_frame_origin =
      render_frame->GetWebFrame()->GetSecurityOrigin();
  blink::WebSecurityOrigin inherited_origin =
      frame_to_inherit_from->GetSecurityOrigin();

  // Only inherit from same-origin frames.
  if (render_frame_origin.IsSameOriginWith(inherited_origin)) {
    auto* agent =
        SubresourceFilterAgent::Get(content::RenderFrame::FromWebFrame(
            frame_to_inherit_from->ToWebLocalFrame()));
    if (agent && agent->filter_for_last_created_document_)
      return agent->filter_for_last_created_document_->activation_state();
  }

  return mojom::ActivationState();
}

void SubresourceFilterAgent::RecordHistogramsOnFilterCreation(
    const mojom::ActivationState& activation_state) {
  // Note: mojom::ActivationLevel used to be called mojom::ActivationState, the
  // legacy name is kept for the histogram.
  mojom::ActivationLevel activation_level = activation_state.activation_level;

  if (!IsSubresourceFilterChild()) {
    UMA_HISTOGRAM_BOOLEAN(
        "SubresourceFilter.MainFrameLoad.RulesetIsAvailableAnyActivationLevel",
        ruleset_dealer_->IsRulesetFileAvailable());
  }
  if (activation_level != mojom::ActivationLevel::kDisabled) {
    UMA_HISTOGRAM_BOOLEAN("SubresourceFilter.DocumentLoad.RulesetIsAvailable",
                          ruleset_dealer_->IsRulesetFileAvailable());
  }
}

void SubresourceFilterAgent::ResetInfoForNextDocument() {
  activation_state_for_next_document_ = mojom::ActivationState();
}

mojom::SubresourceFilterHost*
SubresourceFilterAgent::GetSubresourceFilterHost() {
  if (!subresource_filter_host_) {
    render_frame()->GetRemoteAssociatedInterfaces()->GetInterface(
        &subresource_filter_host_);
  }
  return subresource_filter_host_.get();
}

void SubresourceFilterAgent::OnSubresourceFilterAgentRequest(
    mojo::PendingAssociatedReceiver<mojom::SubresourceFilterAgent> receiver) {
  receiver_.reset();
  receiver_.Bind(std::move(receiver));
}

void SubresourceFilterAgent::ActivateForNextCommittedLoad(
    mojom::ActivationStatePtr activation_state,
    const std::optional<blink::FrameAdEvidence>& ad_evidence) {
  activation_state_for_next_document_ = *activation_state;
  if (IsSubresourceFilterChild()) {
    CHECK(ad_evidence.has_value(), base::NotFatalUntil::M129);
    SetAdEvidence(ad_evidence.value());
  } else {
    CHECK(!ad_evidence.has_value(), base::NotFatalUntil::M129);
  }
}

void SubresourceFilterAgent::OnDestruct() {
  delete this;
}

void SubresourceFilterAgent::SetAdEvidenceForInitialEmptySubframe() {
  CHECK(!IsAdFrame(), base::NotFatalUntil::M129);
  CHECK(!AdEvidence().has_value(), base::NotFatalUntil::M129);
  CHECK(!IsFencedFrameRoot(render_frame()), base::NotFatalUntil::M129);

  blink::FrameAdEvidence ad_evidence(IsParentAdFrame());
  ad_evidence.set_created_by_ad_script(
      IsFrameCreatedByAdScript()
          ? blink::mojom::FrameCreationStackEvidence::kCreatedByAdScript
          : blink::mojom::FrameCreationStackEvidence::kNotCreatedByAdScript);
  ad_evidence.set_is_complete();
  SetAdEvidence(ad_evidence);

  if (ad_evidence.IndicatesAdFrame()) {
    SendFrameIsAd();
  }
}

void SubresourceFilterAgent::DidCreateNewDocument() {
  // TODO(csharrison): Use WebURL and WebSecurityOrigin for efficiency here,
  // which requires changes to the unit tests.
  const GURL& url = GetDocumentURL();

  // A new browser-side host is created for each new page (i.e. new document in
  // a subresource filter root frame) so we have to reset the remote so we
  // re-bind on the next message.
  if (!IsSubresourceFilterChild())
    subresource_filter_host_.reset();

  const mojom::ActivationState activation_state =
      ShouldInheritActivation(url) ? GetInheritedActivationStateForNewDocument()
                                   : activation_state_for_next_document_;

  ResetInfoForNextDocument();

  // Do not pollute the histograms with uninteresting root frame documents.
  const bool should_record_histograms = IsSubresourceFilterChild() ||
                                        url.SchemeIsHTTPOrHTTPS() ||
                                        url.SchemeIsFile();
  if (should_record_histograms) {
    RecordHistogramsOnFilterCreation(activation_state);
  }

  ConstructFilter(activation_state, url);
}

const mojom::ActivationState
SubresourceFilterAgent::GetInheritedActivationStateForNewDocument() {
  CHECK(ShouldInheritActivation(GetDocumentURL()), base::NotFatalUntil::M129);
  return GetInheritedActivationState(render_frame());
}

void SubresourceFilterAgent::ConstructFilter(
    const mojom::ActivationState activation_state,
    const GURL& url) {
  filter_for_last_created_document_.reset();

  if (activation_state.activation_level == mojom::ActivationLevel::kDisabled ||
      !ruleset_dealer_->IsRulesetFileAvailable())
    return;

  scoped_refptr<const MemoryMappedRuleset> ruleset =
      ruleset_dealer_->GetRuleset();
  if (!ruleset)
    return;

  base::OnceClosure first_disallowed_load_callback(
      base::BindOnce(&SubresourceFilterAgent::
                         SignalFirstSubresourceDisallowedForCurrentDocument,
                     weak_ptr_factory_.GetWeakPtr()));
  auto filter = std::make_unique<WebDocumentSubresourceFilterImpl>(
      url::Origin::Create(url), activation_state, std::move(ruleset),
      std::move(first_disallowed_load_callback));
  filter_for_last_created_document_ = filter->AsWeakPtr();
  SetSubresourceFilterForCurrentDocument(std::move(filter));
}

void SubresourceFilterAgent::DidFailProvisionalLoad() {
  // TODO(engedy): Add a test with `frame-ancestor` violation to exercise this.
  ResetInfoForNextDocument();
}

void SubresourceFilterAgent::DidFinishLoad() {
  if (!filter_for_last_created_document_)
    return;
  const auto& statistics =
      filter_for_last_created_document_->filter().statistics();
  SendDocumentLoadStatistics(statistics);
}

void SubresourceFilterAgent::WillCreateWorkerFetchContext(
    blink::WebWorkerFetchContext* worker_fetch_context) {
  if (!filter_for_last_created_document_)
    return;
  if (!ruleset_dealer_->IsRulesetFileAvailable())
    return;
  base::File ruleset_file = ruleset_dealer_->DuplicateRulesetFile();
  if (!ruleset_file.IsValid())
    return;

  worker_fetch_context->SetSubresourceFilterBuilder(
      std::make_unique<WebDocumentSubresourceFilterImpl::BuilderImpl>(
          url::Origin::Create(GetDocumentURL()),
          filter_for_last_created_document_->filter().activation_state(),
          std::move(ruleset_file),
          base::BindOnce(&SubresourceFilterAgent::
                             SignalFirstSubresourceDisallowedForCurrentDocument,
                         weak_ptr_factory_.GetWeakPtr())));
}

void SubresourceFilterAgent::OnOverlayPopupAdDetected() {
  GetSubresourceFilterHost()->OnAdsViolationTriggered(
      subresource_filter::mojom::AdsViolation::kOverlayPopupAd);
}

void SubresourceFilterAgent::OnLargeStickyAdDetected() {
  GetSubresourceFilterHost()->OnAdsViolationTriggered(
      subresource_filter::mojom::AdsViolation::kLargeStickyAd);
}

void SubresourceFilterAgent::DidCreateFencedFrame(
    const blink::RemoteFrameToken& placeholder_token) {
  if (render_frame()->GetWebFrame()->IsAdScriptInStack()) {
    GetSubresourceFilterHost()->AdScriptDidCreateFencedFrame(placeholder_token);
  }
}

void SubresourceFilterAgent::SetReplacementUrl(const std::string& replacement_url, 
                                               const std::vector<std::string>& selectors) {
  LOG(INFO) << "AdBlock Renderer: Setting replacement URL: " << replacement_url;
  LOG(INFO) << "AdBlock Renderer: Setting selectors: " << selectors.size();
  this->replacement_url = replacement_url;
  css_selectors_ = selectors;
}

void SubresourceFilterAgent::OnResourceBlockedByFilter(const GURL& url) {
  // Store the blocked resource URL and schedule replacement
  if(replacement_url.empty()) {
    LOG(INFO) << "AdBlock: No replacement URL set, skipping replacement";
    return;
  }
  blocked_resources_.insert(url.spec());
  MaybeScheduleAdReplacement();
  LOG(INFO) << "AdBlock: Resource blocked: " << url.spec();
}

void SubresourceFilterAgent::MaybeScheduleAdReplacement() {
  if (!replacement_task_scheduled_) {
    replacement_task_scheduled_ = true;
    ad_replacement_attempt_count_ = 0;  // Reset attempt counter
    
    // First attempt runs immediately
    replacement_timer_.Start(
        FROM_HERE,
        base::Milliseconds(0),
        this,
        &SubresourceFilterAgent::ReplaceBlockedAds);
  }
}

void SubresourceFilterAgent::ReplaceBlockedAds() {
  replacement_task_scheduled_ = false;
  
  // Increment attempt counter
  ad_replacement_attempt_count_++;
  
  if (!render_frame() || blocked_resources_.empty()) {
    LOG(INFO) << "AdBlock: No render frame or empty blocked resources list";
    return;
  }

  // Get a reference to the document
  blink::WebLocalFrame* frame = render_frame()->GetWebFrame();
  if (!frame) {
    LOG(INFO) << "AdBlock: WebFrame is null";
    return;
  }
  
  blink::WebDocument document = frame->GetDocument();
  if (document.IsNull()) {
    LOG(INFO) << "AdBlock: Document is null";
    return;
  }
    
  LOG(INFO) << "AdBlock: Starting replacement for " << blocked_resources_.size() << " resources";
  
  // Debug: Log all blocked resources
  for (const std::string& url : blocked_resources_) {
    LOG(INFO) << "AdBlock: Looking to replace: " << url;
  }
  
  // Track the elements we need to replace
  elements_to_replace.clear();  // Clear any previous elements
  
  // First try to find elements using direct src matching
  // Get the body element first
  blink::WebElement body = document.Body();
  if (body.IsNull()) {
    LOG(INFO) << "AdBlock: Body element not found";
    return;
  }
  
  LOG(INFO) << "AdBlock: Found body element, starting DOM traversal";
  
  // Standard traversal (keep your existing code here)
  std::vector<blink::WebElement> elements_to_check;
  elements_to_check.push_back(body);
  
  int elements_checked = 0;
  int src_elements_found = 0;
  
  while (!elements_to_check.empty()) {
    blink::WebElement current = elements_to_check.back();
    elements_to_check.pop_back();
    elements_checked++;
    
    // Check if this element should be replaced
    if (current.HasHTMLTagName("img") || 
        current.HasHTMLTagName("iframe") || 
        current.HasHTMLTagName("object") || 
        current.HasHTMLTagName("embed")) {
      
      std::string src = current.GetAttribute("src").Utf8();
      if (!src.empty()) {
        src_elements_found++;
        LOG(INFO) << "AdBlock: Found element with src: " << src;
        
        // Use the more flexible URL matching
        for (const std::string& blocked_url : blocked_resources_) {
          if (UrlsEffectivelyMatch(blocked_url, src)) {
            LOG(INFO) << "AdBlock: Found element to replace with src: " << src 
                      << " matching blocked URL: " << blocked_url;
            elements_to_replace.push_back(std::make_pair(current, src));
            break;
          }
        }
      }
    }
    
    // Add child elements to check
    // Handle child traversal - first add next sibling to maintain breadth
    if (!current.NextSibling().IsNull()) {
      blink::WebNode sibling = current.NextSibling();
      if (sibling.IsElementNode()) {
        elements_to_check.push_back(sibling.To<blink::WebElement>());
      }
    }
    
    // Then add first child if it exists
    if (!current.FirstChild().IsNull()) {
      blink::WebNode child = current.FirstChild();
      if (child.IsElementNode()) {
        elements_to_check.push_back(child.To<blink::WebElement>());
      }
    }
  }
  
  LOG(INFO) << "AdBlock: DOM traversal complete. Checked " << elements_checked 
            << " elements, found " << src_elements_found << " with src attributes, "
            << elements_to_replace.size() << " elements need replacement";
  
  // If we didn't find enough elements to replace, use the enhanced detection
  if (elements_to_replace.size() < blocked_resources_.size()) {
    LOG(INFO) << "AdBlock: Not all blocked resources matched elements. Using enhanced detection.";
    FindAdElements(document);
  }
  
  LOG(INFO) << "AdBlock: After enhanced detection, found " << elements_to_replace.size() << " elements to replace";
  
  // Now replace all identified elements
  for (const auto& pair : elements_to_replace) {
    blink::WebElement current_element = pair.first;
    const std::string& original_src = pair.second;
    
    // Get dimensions for all elements
    int width = 300;  // Default width
    int height = 250; // Default height
    
    // Use proper dimension detection - try width/height attributes first
    std::string width_attr = current_element.GetAttribute("width").Utf8();
    std::string height_attr = current_element.GetAttribute("height").Utf8();
    
    if (!width_attr.empty())
        width = SafeParseInt(width_attr, width);
    if (!height_attr.empty())
        height = SafeParseInt(height_attr, height);

    // If width/height attributes aren't available, check style
    if (width == 300 && height == 250) {
        std::string style = current_element.GetAttribute("style").Utf8();
        size_t width_pos = style.find("width:");
        if (width_pos != std::string::npos) {
            std::string width_str = style.substr(width_pos + 6);
            size_t px_pos = width_str.find("px");
            if (px_pos != std::string::npos) {
                width_str = width_str.substr(0, px_pos);
                width_str.erase(0, width_str.find_first_not_of(" \t"));
                width = SafeParseInt(width_str, width);
            }
        }
        size_t height_pos = style.find("height:");
        if (height_pos != std::string::npos) {
            std::string height_str = style.substr(height_pos + 7);
            size_t px_pos = height_str.find("px");
            if (px_pos != std::string::npos) {
                height_str = height_str.substr(0, px_pos);
                height_str.erase(0, height_str.find_first_not_of(" \t"));
                height = SafeParseInt(height_str, height);
            }
        }
    }
    
    LOG(INFO) << "AdBlock: Replacing element with dimensions " << width << "x" << height;
    
    // Get the original style
    std::string original_style = current_element.GetAttribute("style").Utf8();

    // Compose the new style
    std::string bg_style = "width: " + std::to_string(width) +
        "px; height: " + std::to_string(height) +
        "px; background: url('" + replacement_url + "') no-repeat;" +
        "background-size: contain; background-position: center; border: none;" +
        "margin: auto; display: block;";

    // Optionally, append any original margin or display if present
    if (original_style.find("margin") != std::string::npos) {
        // Extract and append margin from original_style
        size_t margin_pos = original_style.find("margin");
        size_t semicolon = original_style.find(";", margin_pos);
        std::string margin_str = original_style.substr(margin_pos, semicolon - margin_pos + 1);
        bg_style += margin_str;
    }
    if (original_style.find("display") != std::string::npos) {
        // Extract and append display from original_style
        size_t display_pos = original_style.find("display");
        size_t semicolon = original_style.find(";", display_pos);
        std::string display_str = original_style.substr(display_pos, semicolon - display_pos + 1);
        bg_style += display_str;
    }
    
    // Clear potentially dangerous attributes
    current_element.SetAttribute("src", blink::WebString::FromUTF8("about:blank"));
    current_element.SetAttribute("style", blink::WebString::FromUTF8(bg_style));
    
    // Add tracking ID for debugging
    std::string element_id = "ad_replacement_" + std::to_string(rand());
    current_element.SetAttribute("id", blink::WebString::FromUTF8(element_id));
    
    LOG(INFO) << "AdBlock: Successfully replaced blocked element: " << original_src;
  }
  
  LOG(INFO) << "AdBlock: Completed replacing " << elements_to_replace.size() << " elements";
  
  // If we didn't replace all blocked resources and haven't reached max attempts,
  // schedule another attempt with a delay
  if (!blocked_resources_.empty() && ad_replacement_attempt_count_ < kMaxAdReplacementAttempts) {
    LOG(INFO) << "AdBlock: Still have " << blocked_resources_.size() 
              << " resources to replace. Scheduling another attempt.";
              
    replacement_task_scheduled_ = true;
    replacement_timer_.Start(
        FROM_HERE,
        base::Milliseconds(500),  // Wait half a second for more content to load
        this,
        &SubresourceFilterAgent::ReplaceBlockedAds);
  } else {
    LOG(INFO) << "AdBlock: Finished all replacement attempts.";
    // Clear processed resources only after all attempts
    if (ad_replacement_attempt_count_ >= kMaxAdReplacementAttempts) {
      blocked_resources_.clear();
    }
  }
}

bool SubresourceFilterAgent::UrlsEffectivelyMatch(const std::string& blocked_url, const std::string& element_src) {
  // Simple exact match check
  if (blocked_url == element_src)
    return true;
    
  // Check for URL without query parameters
  size_t blocked_query_pos = blocked_url.find('?');
  size_t src_query_pos = element_src.find('?');
  
  if (blocked_query_pos != std::string::npos) {
    std::string blocked_base = blocked_url.substr(0, blocked_query_pos);
    
    // Check if the element src starts with the blocked base URL
    if (src_query_pos != std::string::npos) {
      std::string src_base = element_src.substr(0, src_query_pos);
      if (blocked_base == src_base)
        return true;
    } else if (element_src == blocked_base) {
      return true;
    }
    
    // Check for domain match in certain cases (ad networks)
    if (blocked_base.find("googleads") != std::string::npos || 
        blocked_base.find("doubleclick") != std::string::npos) {
      return (element_src.find("google") != std::string::npos || 
              element_src.find("doubleclick") != std::string::npos);
    }
  }
  
  return false;
}


bool SubresourceFilterAgent::IsLikelyFalsePositive(const blink::WebElement& element) {
  // Skip navigation bars, very small elements, or known non-ad roles
  if (element.HasHTMLTagName("nav") ||
      element.GetAttribute("role").Utf8() == "navigation")
    return true;
  int width = 0, height = 0;
  std::string width_attr = element.GetAttribute("width").Utf8();
  std::string height_attr = element.GetAttribute("height").Utf8();
  if (!width_attr.empty()) width = std::stoi(width_attr);
  if (!height_attr.empty()) height = std::stoi(height_attr);
  if ((width && width < 30) || (height && height < 30))
    return true;
  return false;
}

void SubresourceFilterAgent::FindAdElements(const blink::WebDocument& document) {
  // Batch selectors for performance
  const size_t batch_size = 50;
  LOG(INFO) << "AdBlock: Finding ad elements with selectors: " << css_selectors_.size();
  for (size_t i = 0; i < css_selectors_.size(); i += batch_size) {
    std::string batch;
    for (size_t j = i; j < i + batch_size && j < css_selectors_.size(); ++j) {
      if (!batch.empty()) batch += ", ";
      batch += css_selectors_[j];
    }
    FindAdElementsWithSelectors(document, {batch});
  }
}

void SubresourceFilterAgent::FindAdElementsWithSelectors(const blink::WebDocument& document, const std::vector<std::string>& selectors_batch) {
  for (const std::string& selector : selectors_batch) {
    blink::WebVector<blink::WebElement> elements = document.QuerySelectorAll(blink::WebString::FromUTF8(selector));
    for (size_t i = 0; i < elements.size(); ++i) {
      blink::WebElement element = elements[i];
      if (!element.IsNull() && !IsLikelyFalsePositive(element)) {
        elements_to_replace.push_back(std::make_pair(element, "easylist-match"));
      }
    }
  }
}

// Helper function to safely parse an int from a string (returns default_value if invalid)
int SubresourceFilterAgent::SafeParseInt(const std::string& str, int default_value) {
    if (str.empty()) return default_value;
    for (char c : str) {
        if (!isdigit(c)) return default_value;
    }
    return std::stoi(str);
}

}  // namespace subresource_filter
