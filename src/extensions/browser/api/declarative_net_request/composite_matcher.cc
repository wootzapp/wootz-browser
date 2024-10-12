// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/api/declarative_net_request/composite_matcher.h"

#include <functional>
#include <iterator>
#include <set>
#include <utility>
#include <vector>

#include "base/containers/contains.h"
#include "base/metrics/histogram_macros.h"
#include "base/ranges/algorithm.h"
#include "base/time/time.h"
#include "base/timer/elapsed_timer.h"
#include "components/url_pattern_index/flat/url_pattern_index_generated.h"
#include "extensions/browser/api/declarative_net_request/constants.h"
#include "extensions/browser/api/declarative_net_request/flat/extension_ruleset_generated.h"
#include "extensions/browser/api/declarative_net_request/request_action.h"
#include "extensions/browser/api/declarative_net_request/request_params.h"
#include "extensions/browser/api/declarative_net_request/utils.h"
#include "extensions/common/api/declarative_net_request/constants.h"

namespace extensions::declarative_net_request {
namespace flat_rule = url_pattern_index::flat;
using PageAccess = PermissionsData::PageAccess;
using ActionInfo = CompositeMatcher::ActionInfo;

namespace {

bool AreIDsUnique(const CompositeMatcher::MatcherList& matchers) {
  std::set<RulesetID> ids;
  for (const auto& matcher : matchers) {
    bool did_insert = ids.insert(matcher->id()).second;
    if (!did_insert)
      return false;
  }

  return true;
}

// Helper to log the time taken in CompositeMatcher::GetAction.
class ScopedGetBeforeRequestActionTimer {
 public:
  explicit ScopedGetBeforeRequestActionTimer(RulesetMatchingStage stage)
      : stage_(stage) {}
  ~ScopedGetBeforeRequestActionTimer() {
    switch (stage_) {
      case RulesetMatchingStage::kOnBeforeRequest:
        UMA_HISTOGRAM_CUSTOM_MICROSECONDS_TIMES(
            "Extensions.DeclarativeNetRequest.EvaluateBeforeRequestTime."
            "SingleExtension2",
            timer_.Elapsed(), base::Microseconds(1), base::Milliseconds(50),
            50);
        break;
      case RulesetMatchingStage::kOnHeadersReceived:
        UMA_HISTOGRAM_CUSTOM_MICROSECONDS_TIMES(
            "Extensions.DeclarativeNetRequest.EvaluateHeadersReceivedTime."
            "SingleExtension2",
            timer_.Elapsed(), base::Microseconds(1), base::Milliseconds(50),
            50);
        break;
    }
  }

 private:
  RulesetMatchingStage stage_;
  base::ElapsedTimer timer_;
};

}  // namespace

ActionInfo::ActionInfo() = default;
ActionInfo::ActionInfo(std::optional<RequestAction> action,
                       bool notify_request_withheld)
    : action(std::move(action)),
      notify_request_withheld(notify_request_withheld) {}

ActionInfo::~ActionInfo() = default;

ActionInfo::ActionInfo(ActionInfo&&) = default;
ActionInfo& ActionInfo::operator=(ActionInfo&& other) = default;

CompositeMatcher::CompositeMatcher(MatcherList matchers,
                                   const ExtensionId& extension_id,
                                   HostPermissionsAlwaysRequired mode)
    : matchers_(std::move(matchers)),
      extension_id_(extension_id),
      host_permissions_always_required_(mode) {
  DCHECK(AreIDsUnique(matchers_));
}

CompositeMatcher::~CompositeMatcher() = default;

const RulesetMatcher* CompositeMatcher::GetMatcherWithID(RulesetID id) const {
  auto it = base::ranges::find(matchers_, id, &RulesetMatcher::id);
  return it == matchers_.end() ? nullptr : it->get();
}

void CompositeMatcher::AddOrUpdateRuleset(
    std::unique_ptr<RulesetMatcher> matcher) {
  MatcherList matchers;
  matchers.push_back(std::move(matcher));
  AddOrUpdateRulesets(std::move(matchers));
}

void CompositeMatcher::AddOrUpdateRulesets(MatcherList matchers) {
  std::set<RulesetID> ids_to_remove;
  for (const auto& matcher : matchers)
    ids_to_remove.insert(matcher->id());

  RemoveRulesetsWithIDs(ids_to_remove);
  matchers_.insert(matchers_.end(), std::make_move_iterator(matchers.begin()),
                   std::make_move_iterator(matchers.end()));
  OnMatchersModified();
}

void CompositeMatcher::RemoveRulesetsWithIDs(const std::set<RulesetID>& ids) {
  size_t erased_count = std::erase_if(
      matchers_, [&ids](const std::unique_ptr<RulesetMatcher>& matcher) {
        return base::Contains(ids, matcher->id());
      });

  if (erased_count > 0)
    OnMatchersModified();
}

std::set<RulesetID> CompositeMatcher::ComputeStaticRulesetIDs() const {
  std::set<RulesetID> result;
  for (const std::unique_ptr<RulesetMatcher>& matcher : matchers_) {
    if (matcher->id() == kDynamicRulesetID)
      continue;

    result.insert(matcher->id());
  }

  return result;
}

ActionInfo CompositeMatcher::GetAction(
    const RequestParams& params,
    RulesetMatchingStage stage,
    PermissionsData::PageAccess page_access) const {
  ScopedGetBeforeRequestActionTimer timer(stage);

  bool always_require_host_permissions =
      host_permissions_always_required_ == HostPermissionsAlwaysRequired::kTrue;
  if (always_require_host_permissions) {
    // We shouldn't be evaluating this ruleset if host permissions are always
    // required but this extension doesn't have access to the request.
    DCHECK(page_access == PermissionsData::PageAccess::kAllowed ||
           page_access == PermissionsData::PageAccess::kWithheld);
  }

  std::optional<RequestAction> final_action;

  // The priority of the highest priority matching allow or allowAllRequests
  // rule for this matcher's extension for the current request, or std::nullopt
  // otherwise. This also serves as the minimum priority needed for a rule to be
  // matched.
  std::optional<uint64_t>& max_allow_rule_priority_for_request =
      params.allow_rule_max_priority[extension_id_];

  for (const auto& matcher : matchers_) {
    std::optional<RequestAction> action = matcher->GetAction(params, stage);
    // TODO(crbug.com/40727004): Allow/AllowAllRequests rules matched can still
    // take effect for stages of the request past the one they're matched in. If
    // they are the max priority action, they should be returned instead of
    // silently causing no action to be matched.
    if (!action || action->index_priority <=
                       max_allow_rule_priority_for_request.value_or(0)) {
      continue;
    }

    if (action->IsAllowOrAllowAllRequests()) {
      max_allow_rule_priority_for_request =
          std::max(max_allow_rule_priority_for_request.value_or(0),
                   action->index_priority);
    }

    final_action =
        GetMaxPriorityAction(std::move(final_action), std::move(action));
  }

  if (!final_action)
    return ActionInfo();

  bool requires_host_permission =
      always_require_host_permissions ||
      final_action->type == RequestAction::Type::REDIRECT;
  if (!requires_host_permission || page_access == PageAccess::kAllowed) {
    return ActionInfo(std::move(final_action),
                      false /* notify_request_withheld */);
  }

  // `requires_host_permission` is true and `page_access` is withheld or denied.
  bool notify_request_withheld = page_access == PageAccess::kWithheld &&
                                 !final_action->IsAllowOrAllowAllRequests();
  return ActionInfo(std::nullopt, notify_request_withheld);
}

std::vector<RequestAction> CompositeMatcher::GetModifyHeadersActions(
    const RequestParams& params,
    RulesetMatchingStage stage) const {
  std::vector<RequestAction> modify_headers_actions;
  DCHECK(params.allow_rule_max_priority.contains(extension_id_));

  // The priority of the highest priority matching allow or allowAllRequests
  // rule within this matcher, or std::nullopt if no such rule exists.
  std::optional<uint64_t> max_allow_rule_priority =
      params.allow_rule_max_priority[extension_id_];

  for (const auto& matcher : matchers_) {
    // Plumb |max_allow_rule_priority| into GetModifyHeadersActions so that
    // modifyHeaders rules with priorities less than or equal to the highest
    // priority matching allow/allowAllRequests rule are ignored.
    std::vector<RequestAction> actions_for_matcher =
        matcher->GetModifyHeadersActions(params, stage,
                                         max_allow_rule_priority);

    modify_headers_actions.insert(
        modify_headers_actions.end(),
        std::make_move_iterator(actions_for_matcher.begin()),
        std::make_move_iterator(actions_for_matcher.end()));
  }

  // Sort |modify_headers_actions| in descending order of priority.
  std::sort(modify_headers_actions.begin(), modify_headers_actions.end(),
            std::greater<>());
  return modify_headers_actions;
}

bool CompositeMatcher::HasAnyExtraHeadersMatcher() const {
  if (!has_any_extra_headers_matcher_.has_value())
    has_any_extra_headers_matcher_ = ComputeHasAnyExtraHeadersMatcher();
  return has_any_extra_headers_matcher_.value();
}

void CompositeMatcher::OnRenderFrameCreated(content::RenderFrameHost* host) {
  for (auto& matcher : matchers_)
    matcher->OnRenderFrameCreated(host);
}

void CompositeMatcher::OnRenderFrameDeleted(content::RenderFrameHost* host) {
  for (auto& matcher : matchers_)
    matcher->OnRenderFrameDeleted(host);
}

void CompositeMatcher::OnDidFinishNavigation(
    content::NavigationHandle* navigation_handle) {
  for (auto& matcher : matchers_)
    matcher->OnDidFinishNavigation(navigation_handle);
}

bool CompositeMatcher::HasRulesets(RulesetMatchingStage stage) const {
  return base::ranges::any_of(
      matchers_, [stage](const std::unique_ptr<RulesetMatcher>& matcher) {
        return matcher->GetRulesCount(stage) > 0;
      });
}

void CompositeMatcher::OnMatchersModified() {
  DCHECK(AreIDsUnique(matchers_));

  // Clear the renderers' cache so that they take the updated rules into
  // account.
  ClearRendererCacheOnNavigation();

  has_any_extra_headers_matcher_.reset();
}

bool CompositeMatcher::ComputeHasAnyExtraHeadersMatcher() const {
  for (const auto& matcher : matchers_) {
    if (matcher->IsExtraHeadersMatcher())
      return true;
  }
  return false;
}

}  // namespace extensions::declarative_net_request
