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
#include "third_party/blink/public/web/web_script_source.h"

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

void SubresourceFilterAgent::SetReplacementEnabled(
    bool enabled, 
    const std::string& ad_unit_path,
    const std::string& id_prefix,
    const std::string& script_url,
    const std::string& sizes_json,
    const std::vector<std::string>& selectors) {
  replacement_enabled_ = enabled;
  ad_unit_path_ = ad_unit_path;
  id_prefix_ = id_prefix;
  script_url_ = script_url;
  sizes_json_ = sizes_json;
  css_selectors_ = selectors;
}

void SubresourceFilterAgent::OnResourceBlockedByFilter(const GURL& url) {
  if(!replacement_enabled_ || css_selectors_.empty()) {
    return;
  }
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
  ad_replacement_attempt_count_++;
  
  if (!render_frame()) {
    LOG(INFO) << "AdBlock: No render frame";
    return;
  }

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

  // On first attempt, inject GPT and configure
  if (ad_replacement_attempt_count_ == 1) {
    InjectGPTScript();
  }

  elements_to_replace.clear();
  FindAdElements(document);

  LOG(INFO) << "AdBlock: Found " << elements_to_replace.size() << " elements to replace";

  if (!elements_to_replace.empty()) {
    // Step 1: Modify all elements first
    for (size_t i = 0; i < elements_to_replace.size(); i++) {
      std::string ad_id = id_prefix_ + std::to_string(i);
      
      if (elements_to_replace[i].HasHTMLTagName("iframe")) {
        elements_to_replace[i].SetAttribute("src", blink::WebString::FromUTF8("about:blank"));
      }
      
      elements_to_replace[i].SetAttribute("id", blink::WebString::FromUTF8(ad_id));
      std::string style = "min-width: 300px; min-height: 60px; max-width: 100%; overflow: hidden; "
                  "display: flex; justify-content: center; align-items: center; margin: 0 auto; text-align: center;";
      elements_to_replace[i].SetAttribute("style", blink::WebString::FromUTF8(style));
    }
    
    // Step 2: Single script to process all elements by count
    std::string direct_script = base::StringPrintf(R"(
      (function() {
        // Initialize GPT command queue immediately
        window.googletag = window.googletag || {cmd: []};
        
        // Configuration from browser
        var adUnitPath = '%s';
        var idPrefix = '%s';
        var adSizes = %s;
        var elementCount = %zu;
        
        // Track which ads have been processed and protected
        if (!window.wootzappProcessedAds) window.wootzappProcessedAds = {};
        if (!window.wootzappProtectedElements) window.wootzappProtectedElements = {};
        if (!window.wootzappSlots) window.wootzappSlots = new Map();
        
        // Viewport detection function - needed globally
        function isInViewport(el) {
          if (!el || !el.getBoundingClientRect) return false;
          try {
            var rect = el.getBoundingClientRect();
            // Increased threshold to 1000px below viewport to preload more ads
            return (
              rect.top < (window.innerHeight || document.documentElement.clientHeight) + 1000 &&
              rect.bottom > -250
            );
          } catch(e) {
            return false;
          }
        }
        
        // Container processing function - defined before it's called
        function processContainers(containers) {
          for(var i = 0; i < containers.length; i++) {
            var container = containers[i];
            if (!container || !container.id) continue;
            
            var id = container.id;
            
            try {
              // Skip if already processed in this page view
              if (window.wootzappProcessedAds[id]) continue;
              
              // Make sure element still exists in DOM
              if (!document.getElementById(id)) {
                console.warn('Wootzapp: Container no longer in DOM: ' + id);
                continue;
              }
              
              window.wootzappProcessedAds[id] = true;
              
              // Clear the container
              container.innerHTML = '';
              
              // Check if slot is already defined
              var alreadyDefined = false;
              var existingSlot = null;
              
              if (googletag.pubads && googletag.pubads().getSlots) {
                var slots = googletag.pubads().getSlots();
                for (var j = 0; j < slots.length; j++) {
                  if (slots[j].getSlotElementId() === id) {
                    alreadyDefined = true;
                    existingSlot = slots[j];
                    break;
                  }
                }
              }
              
              if (!alreadyDefined) {
                // Define a new ad slot using configuration
                var slot = googletag.defineSlot(adUnitPath, adSizes, id);
                if (slot) {
                  slot.addService(googletag.pubads());
                  
                  // Verify element still exists before display
                  if (document.getElementById(id)) {
                    googletag.display(id);
                    console.log('Wootzapp: Defined and displayed new slot for ' + id);
                  } else {
                    console.warn('Wootzapp: Element disappeared before display: ' + id);
                  }
                }
              } else if (existingSlot) {
                // Refresh existing slot
                googletag.pubads().refresh([existingSlot]);
                console.log('Wootzapp: Refreshed existing slot for ' + id);
              }
            } catch(e) {
              console.error('Wootzapp: Error processing ad for ' + id, e);
            }
          }
        }
        
        // Batch processing function - defined before it's called
        function processContainerBatches(visibleContainers, nonVisibleContainers, batchSize) {
          // First batch: Process visible containers immediately
          for (var i = 0; i < visibleContainers.length; i += batchSize) {
            var batch = visibleContainers.slice(i, i + batchSize);
            processContainers(batch);
          }
          
          // Process ALL non-visible containers with minimal delay
          if (nonVisibleContainers.length > 0) {
            setTimeout(function() {
              // Process all non-visible containers in smaller batches
              for (var i = 0; i < nonVisibleContainers.length; i += batchSize) {
                var batch = nonVisibleContainers.slice(i, i + batchSize);
                // Use a small incremental delay to prevent browser freezing
                setTimeout(function(batchToProcess) {
                  return function() { 
                    processContainers(batchToProcess); 
                  };
                }(batch), i * 20); // Only 20ms delay between batches
              }
            }, 50); // Reduced initial delay
          }
        }
        
        // PROTECTION: Prevent third-party scripts from modifying our ad containers
        function protectAdContainers() {
          var containers = document.querySelectorAll('[id^="' + idPrefix + '"]');
          if (!containers || containers.length === 0) return;
          
          console.log('Wootzapp: Protecting ' + containers.length + ' ad containers');
          
          // Set up MutationObserver to prevent unwanted modifications
          if (window.MutationObserver && !window.wootzappObserver) {
            window.wootzappObserver = new MutationObserver(function(mutations) {
              mutations.forEach(function(mutation) {
                // Allow GPT to make changes (these have specific patterns)
                if (mutation.addedNodes.length) {
                  for (var i = 0; i < mutation.addedNodes.length; i++) {
                    var node = mutation.addedNodes[i];
                    // Only allow iframes from doubleclick or specific GPT elements
                    if (node.nodeName === 'IFRAME' && 
                        (node.src && (node.src.indexOf('doubleclick') > -1 || 
                                    node.src.indexOf('googlesyndication') > -1))) {
                      console.log('Wootzapp: Allowing GPT iframe insertion');
                      return; // This is a legitimate GPT change
                    }
                    // Allow GPT's own divs
                    if (node.id && node.id.indexOf('google_ads_iframe_') === 0) {
                      return; // This is a legitimate GPT element
                    }
                  }
                }
                
                // For any other changes to our containers, log and restore
                if (mutation.target && mutation.target.id && 
                    mutation.target.id.indexOf(idPrefix) === 0) {
                  // Only block complete replacements or removals
                  if (mutation.type === 'childList' && 
                      (mutation.removedNodes.length > 1 || mutation.target.innerHTML === '')) {
                    console.warn('Wootzapp: Blocked unauthorized modification of ad container', mutation.target.id);
                    // Force refresh the ad instead of blocking (more reliable)
                    googletag.cmd.push(function() {
                      var slots = googletag.pubads().getSlots();
                      for (var i = 0; i < slots.length; i++) {
                        if (slots[i].getSlotElementId() === mutation.target.id) {
                          googletag.pubads().refresh([slots[i]]);
                          break;
                        }
                      }
                    });
                  }
                }
              });
            });
            
            // Observe all containers with specific configuration
            containers.forEach(function(container) {
              window.wootzappObserver.observe(container, { 
                childList: true,
                attributes: true,
                subtree: true
              });
            });
            
            console.log('Wootzapp: Set up observer for ' + containers.length + ' ad containers');
          }
          
          // PROTECTION: Override key element methods to prevent container removal
          containers.forEach(function(container) {
            var id = container.id;
            
            // Skip if already protected
            if (window.wootzappProtectedElements[id]) {
              return;
            }
            
            // Mark as protected
            window.wootzappProtectedElements[id] = true;
            container.setAttribute('data-wootzapp-protected', 'true');
            
            try {
              // Add protection against direct removal
              var originalRemove = container.remove;
              container.remove = function() {
                console.warn('Wootzapp: Blocked attempt to remove ad container:', this.id);
                return false;
              };
              
              // Only define innerHTML property if not already defined
              if (!Object.getOwnPropertyDescriptor(container, 'innerHTML')) {
                // Add protection against innerHTML changes
                Object.defineProperty(container, 'innerHTML', {
                  configurable: true, // Allow reconfiguration if needed
                  set: function(value) {
                    // Allow empty or GPT content
                    if (value === '' || value.indexOf('google') > -1) {
                      return Element.prototype.__lookupSetter__('innerHTML').call(this, value);
                    }
                    console.warn('Wootzapp: Blocked attempt to change ad container content:', this.id);
                    return false;
                  },
                  get: function() {
                    return Element.prototype.__lookupGetter__('innerHTML').call(this);
                  }
                });
              }
            } catch(e) {
              console.error('Wootzapp: Error while protecting container ' + id, e);
            }
          });
        }
        
        // PROTECTION: Override ad-related functions that might replace our ads
        if (!window.wootzappProtectionApplied) {
          // Intercept common ad insertion methods
          var originalDocumentWrite = document.write;
          document.write = function(html) {
            // Check if this would affect our containers
            if (html && html.indexOf(idPrefix) > -1) {
              console.warn('Wootzapp: Blocked document.write attempt that would affect our ads');
              return;
            }
            return originalDocumentWrite.apply(this, arguments);
          };
          
          // Track protection state
          window.wootzappProtectionApplied = true;
          
          // Set up periodic protection check (less frequent)
          setInterval(function() {
            try {
              protectAdContainers();
            } catch(e) {
              console.error('Wootzapp: Error in protection interval', e);
            }
          }, 3000);
        }
        
        // Set up scroll event handler to load ads as they come into view
        if (!window.wootzappScrollHandlerAdded) {
          window.wootzappScrollHandlerAdded = true;
          window.addEventListener('scroll', function() {
            // Debounce scroll events
            if (window.wootzappScrollTimeout) clearTimeout(window.wootzappScrollTimeout);
            window.wootzappScrollTimeout = setTimeout(function() {
              var containers = document.querySelectorAll('[id^="' + idPrefix + '"]');
              var containersToProcess = [];
              
              // Find containers now in viewport that haven't been processed
              for (var i = 0; i < containers.length; i++) {
                if (isInViewport(containers[i]) && 
                    !window.wootzappProcessedAds[containers[i].id]) {
                  containersToProcess.push(containers[i]);
                }
              }
              
              // Process these newly visible containers
              if (containersToProcess.length > 0) {
                processContainers(containersToProcess);
              }
            }, 100);
          }, { passive: true });
        }
        
        // Apply protection BEFORE defining ads to prevent race conditions
        try {
          protectAdContainers();
        } catch(e) {
          console.error('Wootzapp: Error in initial protection', e);
        }
        
        // Define and display ads
        googletag.cmd.push(function() {
          try {
            // COMBINED CONTAINER DISCOVERY - Both querySelectorAll and iterative method
            var allContainers = [];
            var processedIds = new Set();
            
            // METHOD 1: Find containers using querySelectorAll
            var selectorContainers = document.querySelectorAll('[id^="' + idPrefix + '"]');
            for (var i = 0; i < selectorContainers.length; i++) {
              var container = selectorContainers[i];
              if (container && container.id) {
                allContainers.push(container);
                processedIds.add(container.id);
              }
            }
            
            // METHOD 2: Find containers using iterative method
            for (var i = 0; i < elementCount; i++) {
              var elementId = idPrefix + i;
              var container = document.getElementById(elementId);
              
              if (container && !processedIds.has(elementId)) {
                allContainers.push(container);
                processedIds.add(elementId);
              }
            }
            
            console.log('Wootzapp Combined: Found ' + allContainers.length + ' total containers (querySelectorAll: ' + selectorContainers.length + ', iterative: ' + (allContainers.length - selectorContainers.length) + ')');
            
            // Configure GPT only once (do this first for better parallelization)
            if (!window.wootzappServicesEnabled) {
              googletag.pubads().enableSingleRequest();
              googletag.enableServices();
              window.wootzappServicesEnabled = true;
            }
            
            // OPTIMIZATION: Process visible containers first
            var visibleContainers = [];
            var nonVisibleContainers = [];
            
            // Sort containers by visibility
            for(var i = 0; i < allContainers.length; i++) {
              if (isInViewport(allContainers[i])) {
                visibleContainers.push(allContainers[i]);
              } else {
                nonVisibleContainers.push(allContainers[i]);
              }
            }
            
            // Process containers in batches
            var batchSize = 5;
            processContainerBatches(visibleContainers, nonVisibleContainers, batchSize);
            
          } catch(e) {
            console.error('Wootzapp: Error in main ad script:', e);
          }
        });
        
      })();
    )", ad_unit_path_.c_str(), id_prefix_.c_str(), sizes_json_.c_str(), elements_to_replace.size());
    
    frame->ExecuteScript(blink::WebScriptSource(blink::WebString::FromUTF8(direct_script)));
    
    LOG(INFO) << "AdBlock: Configured " << elements_to_replace.size() << " GPT ad slots";
  }
  
  // If we haven't reached max attempts, schedule another attempt with a delay
  if (ad_replacement_attempt_count_ < kMaxAdReplacementAttempts) {
    replacement_task_scheduled_ = true;
    replacement_timer_.Start(
        FROM_HERE,
        base::Milliseconds(6000),
        this,
        &SubresourceFilterAgent::ReplaceBlockedAds);
  }
}

void SubresourceFilterAgent::InjectGPTScript() {
  static bool script_already_injected = false;
  if (!render_frame() || script_already_injected || gpt_injected_) return;
  
  script_already_injected = true;
  gpt_injected_ = true;
  
  blink::WebLocalFrame* frame = render_frame()->GetWebFrame();
  if (!frame) return;
  
  // Use script_url_ from configuration instead of hardcoded URL
  std::string script_code = base::StringPrintf(R"(
    (function() {
      if (window.wootzappGptInjected) return;
      window.wootzappGptInjected = true;
      
      // Add preconnect for faster network connection
      var preconnect = document.createElement('link');
      preconnect.rel = 'preconnect';
      preconnect.href = 'https://securepubads.g.doubleclick.net';
      document.head.appendChild(preconnect);
      
      // Initialize GPT early
      window.googletag = window.googletag || {cmd: []};
      
      // Load script with high priority
      var gptScript = document.createElement('script');
      gptScript.async = true;
      gptScript.src = '%s';
      gptScript.crossOrigin = 'anonymous';
      gptScript.setAttribute('fetchpriority', 'high');
      document.head.appendChild(gptScript);
    })();
  )", script_url_.c_str());

  frame->ExecuteScript(blink::WebScriptSource(blink::WebString::FromUTF8(script_code)));
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
        elements_to_replace.push_back(element);
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

