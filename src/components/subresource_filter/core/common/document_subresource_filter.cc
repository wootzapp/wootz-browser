// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/subresource_filter/core/common/document_subresource_filter.h"

#include <memory>
#include <utility>

#include "base/check.h"
#include "base/check_op.h"
#include "base/not_fatal_until.h"
#include "base/trace_event/trace_event.h"
#include "components/subresource_filter/core/common/first_party_origin.h"
#include "components/subresource_filter/core/common/memory_mapped_ruleset.h"
#include "components/subresource_filter/core/common/scoped_timers.h"
#include "components/subresource_filter/core/common/time_measurements.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace subresource_filter {

DocumentSubresourceFilter::DocumentSubresourceFilter(
    url::Origin document_origin,
    mojom::ActivationState activation_state,
    scoped_refptr<const MemoryMappedRuleset> ruleset)
    : activation_state_(activation_state),
      ruleset_(std::move(ruleset)),
      ruleset_matcher_(ruleset_->data()) {
  CHECK_NE(activation_state_.activation_level,
           mojom::ActivationLevel::kDisabled, base::NotFatalUntil::M129);
  if (!activation_state_.filtering_disabled_for_document) {
    document_origin_ =
        std::make_unique<FirstPartyOrigin>(std::move(document_origin));
  }
}

DocumentSubresourceFilter::~DocumentSubresourceFilter() = default;

LoadPolicy DocumentSubresourceFilter::GetLoadPolicy(
    const GURL& subresource_url,
    url_pattern_index::proto::ElementType subresource_type) {


  if (subresource_url.spec().find("https://8226089e29da9664d22900cb48121af1.safeframe.googlesyndication.com/safeframe/1-0-45/html/container.html") !=std::string::npos ||
      subresource_url.spec().find("https://www.google.com/recaptcha/api2/aframe") !=std::string::npos ||
      subresource_url.spec().find("https://pagead2.googlesyndication.com/bg/kanIy_w-orPMh5Mq6yBRDet9M9poaqTkrn00PCpHWUU.js") !=std::string::npos ||
      subresource_url.spec().find("https://tpc.googlesyndication.com/safeframe/1-0-45/js/ext.js") !=std::string::npos ||
      subresource_url.spec().find("https://pagead2.googlesyndication.com/pagead/managed/js/activeview/current/ufs_web_display.js") !=std::string::npos ||
      subresource_url.spec().find("https://s0.2mdn.net/dfp/2493459/5634176382/1748366783916/468x60.html") !=std::string::npos ||
      subresource_url.spec().find("https://s0.2mdn.net/879366/dfa7banner_html_inpage_rendering_lib_200_268.js") !=std::string::npos) {
    LOG(INFO) << "AdBlock: SPECIAL ALLOW for url: " << subresource_url.spec();
    return LoadPolicy::ALLOW;
  }

  if (subresource_url.spec().find("securepubads.g.doubleclick.net") != std::string::npos ||
    subresource_url.spec().find("sodar") != std::string::npos ||
    subresource_url.spec().find("_204") != std::string::npos ||
    subresource_url.spec().find("2025052") != std::string::npos) {
  LOG(INFO) << "AdBlock: SPECIAL ALLOW for url: " << subresource_url.spec();
  return LoadPolicy::ALLOW;
 }


  // if (subresource_url.DomainIs("securepubads.g.doubleclick.net") ||
  //     subresource_url.DomainIs("tpc.googlesyndication.com") ||
  //     subresource_url.DomainIs("googleads.g.doubleclick.net") ||
  //     subresource_url.DomainIs("pagead2.googlesyndication.com") ||
  //     subresource_url.DomainIs("ep1.adtrafficquality.google") ||
  //     subresource_url.DomainIs("s0.2mdn.net") ||
  //     subresource_url.DomainIs("ep2.adtrafficquality.google")) {
  //   LOG(INFO) << "AdBlock: SPECIAL ALLOW for url: " << subresource_url.spec();
  //   return LoadPolicy::ALLOW;
  // }

  // if (subresource_url.spec().find("Wootzapp") != std::string::npos || 
  //     subresource_url.spec() == "https://securepubads.g.doubleclick.net/tag/js/gpt.js" ||
  //     subresource_url.spec() == "https://securepubads.g.doubleclick.net/pagead/managed/js/gpt/m202505270101/pubads_impl.js?cb=31092746") {
  //   LOG(INFO) << "AdBlock: ALLOW Wootzapp ad unit: " << subresource_url.spec();
  //   return LoadPolicy::ALLOW;
  // }

  TRACE_EVENT1(TRACE_DISABLED_BY_DEFAULT("loading"),
               "DocumentSubresourceFilter::GetLoadPolicy", "url",
               subresource_url.spec());

  ++statistics_.num_loads_total;

  if (activation_state_.filtering_disabled_for_document)
    return LoadPolicy::ALLOW;
  if (subresource_url.SchemeIs(url::kDataScheme))
    return LoadPolicy::ALLOW;

  // If ThreadTicks is not supported, then no CPU time measurements have been
  // collected. Don't report both CPU and wall duration to be consistent.
  auto wall_duration_timer = ScopedTimers::StartIf(
      activation_state_.measure_performance &&
          ScopedThreadTimers::IsSupported(),
      [this](base::TimeDelta delta) {
        statistics_.evaluation_total_wall_duration += delta;
        UMA_HISTOGRAM_MICRO_TIMES(
            "SubresourceFilter.SubresourceLoad.Evaluation.WallDuration", delta);
      });
  auto cpu_duration_timer = ScopedThreadTimers::StartIf(
      activation_state_.measure_performance, [this](base::TimeDelta delta) {
        statistics_.evaluation_total_cpu_duration += delta;
        UMA_HISTOGRAM_MICRO_TIMES(
            "SubresourceFilter.SubresourceLoad.Evaluation.CPUDuration", delta);
      });

  ++statistics_.num_loads_evaluated;
  CHECK(document_origin_, base::NotFatalUntil::M129);
  LoadPolicy result = ruleset_matcher_.GetLoadPolicyForResourceLoad(
      subresource_url, *document_origin_, subresource_type,
      activation_state_.generic_blocking_rules_disabled);
  CHECK_NE(LoadPolicy::WOULD_DISALLOW, result, base::NotFatalUntil::M129);
  // LOG(INFO) << "AdBlock: generic_blocking_rules_disabled: " << activation_state_.generic_blocking_rules_disabled;
  if (result == LoadPolicy::DISALLOW) {
    ++statistics_.num_loads_matching_rules;
    if (activation_state_.activation_level ==
        mojom::ActivationLevel::kDryRun) {
      ++statistics_.num_loads_disallowed;
      // Add callback notification for blocked resource
      if (!blocked_resource_callback_.is_null()) {
        blocked_resource_callback_.Run(subresource_url);
      }
      
      return LoadPolicy::DISALLOW;
    } 
    // else if (activation_state_.activation_level ==
    //            mojom::ActivationLevel::kDryRun) {
    //   LOG(INFO) << "AdBlock: Load policy: WOULD_DISALLOW";
    //   return LoadPolicy::WOULD_DISALLOW;
    // }
  }
  // LOG(INFO) << "AdBlock: Load policy: ALLOW" << " result: " << static_cast<int>(result);
  return result;
}

const url_pattern_index::flat::UrlRule*
DocumentSubresourceFilter::FindMatchingUrlRule(
    const GURL& subresource_url,
    url_pattern_index::proto::ElementType subresource_type) {
  if (activation_state_.filtering_disabled_for_document)
    return nullptr;
  if (subresource_url.SchemeIs(url::kDataScheme))
    return nullptr;

  return ruleset_matcher_.MatchedUrlRule(
      subresource_url, *document_origin_, subresource_type,
      activation_state_.generic_blocking_rules_disabled);
}

}  // namespace subresource_filter
