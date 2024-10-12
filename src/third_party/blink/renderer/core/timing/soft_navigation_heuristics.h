// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_SOFT_NAVIGATION_HEURISTICS_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_SOFT_NAVIGATION_HEURISTICS_H_

#include <optional>

#include "base/gtest_prod_util.h"
#include "base/memory/stack_allocated.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/platform/heap/collection_support/heap_hash_set.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"
#include "third_party/blink/renderer/platform/scheduler/public/task_attribution_tracker.h"
#include "third_party/blink/renderer/platform/supplementable.h"
#include "third_party/blink/renderer/platform/wtf/hash_set.h"

namespace blink {
namespace scheduler {
class TaskAttributionInfo;
}  // namespace scheduler

class ScriptState;
class SoftNavigationContext;

namespace internal {

const char kPageLoadInternalSoftNavigationFromReferenceInvalidTiming[] =
    "PageLoad.Internal.SoftNavigationFromReferenceInvalidTiming";

// These values are recorded into a UMA histogram as scenarios where the start
// time of soft navigation ends up being 0. These entries
// should not be renumbered and the numeric values should not be reused. These
// entries should be kept in sync with the definition in
// tools/metrics/histograms/enums.xml
// TODO(crbug.com/1489583): Remove the code here and related code once the bug
// is resolved.
enum class SoftNavigationFromReferenceInvalidTimingReasons {
  kNullUserInteractionTsAndNotNullReferenceTs = 0,
  kUserInteractionTsAndReferenceTsBothNull = 1,
  kNullReferenceTsAndNotNullUserInteractionTs = 2,
  kUserInteractionTsAndReferenceTsBothNotNull = 3,
  kMaxValue = kUserInteractionTsAndReferenceTsBothNotNull,
};

CORE_EXPORT void
RecordUmaForPageLoadInternalSoftNavigationFromReferenceInvalidTiming(
    base::TimeTicks user_interaction_ts,
    base::TimeTicks reference_ts);
}  // namespace internal

// This class contains the logic for calculating Single-Page-App soft navigation
// heuristics. See https://github.com/WICG/soft-navigations
class CORE_EXPORT SoftNavigationHeuristics
    : public GarbageCollected<SoftNavigationHeuristics>,
      public Supplement<LocalDOMWindow>,
      public scheduler::TaskAttributionTracker::Observer {
  USING_PRE_FINALIZER(SoftNavigationHeuristics, Dispose);

 public:
  FRIEND_TEST_ALL_PREFIXES(SoftNavigationHeuristicsTest,
                           EarlyReturnOnInvalidPendingInteractionTimestamp);

  // This class defines a scope that would cover click or navigation related
  // events, in order for the SoftNavigationHeuristics class to be able to keep
  // track of them and their descendant tasks.
  class CORE_EXPORT EventScope {
    STACK_ALLOCATED();

   public:
    enum class Type { kKeyboard, kClick, kNavigate };

    ~EventScope();

    EventScope(EventScope&&);
    EventScope& operator=(EventScope&&);

   private:
    using ObserverScope = scheduler::TaskAttributionTracker::ObserverScope;
    using TaskScope = scheduler::TaskAttributionTracker::TaskScope;

    friend class SoftNavigationHeuristics;

    EventScope(SoftNavigationHeuristics*,
               std::optional<ObserverScope>,
               std::optional<TaskScope>);

    SoftNavigationHeuristics* heuristics_;
    std::optional<ObserverScope> observer_scope_;
    std::optional<TaskScope> task_scope_;
  };

  // Supplement boilerplate.
  static const char kSupplementName[];
  explicit SoftNavigationHeuristics(LocalDOMWindow& window);
  virtual ~SoftNavigationHeuristics() = default;
  static SoftNavigationHeuristics* From(LocalDOMWindow&);

  // GarbageCollected boilerplate.
  void Trace(Visitor*) const override;

  void Dispose();

  // The class's API.

  // Returns an id to be used for retrieving the associated task state during
  // commit, or nullopt if no `SoftNavigationContext` is associated with the
  // navigation.
  std::optional<scheduler::TaskAttributionId>
  AsyncSameDocumentNavigationStarted();

  void SameDocumentNavigationCommitted(const String& url,
                                       SoftNavigationContext*);
  bool ModifiedDOM();
  uint32_t SoftNavigationCount() { return soft_navigation_count_; }

  // TaskAttributionTracker::Observer's implementation.
  void OnCreateTaskScope(scheduler::TaskAttributionInfo&) override;

  void RecordPaint(LocalFrame*,
                   uint64_t painted_area,
                   bool is_modified_by_soft_navigation);

  EventScope CreateEventScope(EventScope::Type type,
                              bool is_new_interaction,
                              ScriptState*);

  // This method is called during the weakness processing stage of garbage
  // collection to remove items from `potential_soft_navigations_` and to detect
  // it becoming empty, in which case the heuristic is reset.
  void ProcessCustomWeakness(const LivenessBroker& info);

  bool GetInitialInteractionEncounteredForTest() {
    return initial_interaction_encountered_;
  }

 private:
  struct EventParameters {
    explicit EventParameters() = default;
    EventParameters(bool is_new_interaction, EventScope::Type type)
        : is_new_interaction(is_new_interaction), type(type) {}

    bool is_new_interaction = false;
    EventScope::Type type = EventScope::Type::kClick;
  };

  void RecordUmaForNonSoftNavigationInteraction(
      const SoftNavigationContext&) const;
  void ReportSoftNavigationToMetrics(LocalFrame*, SoftNavigationContext*) const;
  void SetIsTrackingSoftNavigationHeuristicsOnDocument(bool value) const;

  SoftNavigationContext* GetSoftNavigationContextForCurrentTask();
  void ResetHeuristic();
  void ResetPaintsIfNeeded();
  void CommitPreviousPaints(LocalFrame*);
  void EmitSoftNavigationEntryIfAllConditionsMet(SoftNavigationContext*);
  LocalFrame* GetLocalFrameIfNotDetached() const;
  void OnSoftNavigationEventScopeDestroyed();

  // This must only be called when `all_event_parameters_` is non-empty.
  const EventParameters& CurrentEventParameters() {
    return all_event_parameters_.back();
  }

  // The set of ongoing potential soft navigations. `SoftNavigationContext`
  // objects are added when they are the active context during an event handler
  // running in an `EventScope`. Entries are stored as untraced members to do
  // custom weak processing (see `ProcessCustomWeakness()`).
  HashSet<UntracedMember<const SoftNavigationContext>>
      potential_soft_navigations_;

  // The `SoftNavigationContext` of the "active interaction", if any.
  //
  // This is set to a new `SoftNavigationContext` when
  //   1. an `EventScope` is created for a new interaction (click, navigation,
  //      and keydown) and there isn't already an active `EventScope` on the
  //      stack for this `SoftNavigationHeuristics`. Note that the latter
  //      restriction causes the same context to be reused for nested
  //      `EventScope`s, which occur when the navigate event occurs within the
  //      scope of the input event.
  //
  //   2. an `EventScope` is created for a non-new interaction (keypress, keyup)
  //      and `active_interaction_context_` isn't set. These events typically
  //      follow a keydown, in which case the context created for that will be
  //      reused, but the context can be cleared if, for example, a click
  //      happens while a key is held.
  //
  // This is cleared when the outermost `EventScope` is destroyed if the scope
  // type is click or navigate. For keyboard events, which have multiple related
  // events, this remains alive until the next interaction.
  Member<SoftNavigationContext> active_interaction_context_;

  // The last soft navigation detected, which could be pending (not emitted)
  // until `paint_conditions_met_` is true.
  //
  // TODO(crbug.com/1510706): Remove this is if `paint_conditions_met_` isn't
  // reinstated since it is cleared immediately after emitting the entry.
  WeakMember<SoftNavigationContext> last_detected_soft_navigation_;

  uint32_t soft_navigation_count_ = 0;
  uint64_t softnav_painted_area_ = 0;
  uint64_t initial_painted_area_ = 0;
  uint64_t viewport_area_ = 0;
  bool did_commit_previous_paints_ = false;
  bool paint_conditions_met_ = false;
  bool initial_interaction_encountered_ = false;

  // `SoftNavigationEventScope`s can be nested in case a click/keyboard event
  // synchronously initiates a navigation. `all_event_parameters_` stores one
  // `EventParameters` per scope, with the most recent one in the back.
  WTF::Deque<EventParameters> all_event_parameters_;
};

}  // namespace blink

#endif  //  THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_SOFT_NAVIGATION_HEURISTICS_H_
