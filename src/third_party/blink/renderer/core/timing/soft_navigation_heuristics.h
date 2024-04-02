// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_SOFT_NAVIGATION_HEURISTICS_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_SOFT_NAVIGATION_HEURISTICS_H_

#include <optional>

#include "base/containers/enum_set.h"
#include "base/gtest_prod_util.h"
#include "base/memory/stack_allocated.h"
#include "third_party/blink/public/common/scheduler/task_attribution_id.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/platform/heap/collection_support/heap_hash_set.h"
#include "third_party/blink/renderer/platform/heap/garbage_collected.h"
#include "third_party/blink/renderer/platform/scheduler/public/task_attribution_tracker.h"
#include "third_party/blink/renderer/platform/supplementable.h"
#include "third_party/blink/renderer/platform/wtf/hash_set.h"

namespace blink {

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
    friend class SoftNavigationHeuristics;

    EventScope(SoftNavigationHeuristics*, scheduler::TaskAttributionTracker*);

    SoftNavigationHeuristics* heuristics_;
    std::optional<scheduler::TaskAttributionTracker::ObserverScope>
        observer_scope_;
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
  void SameDocumentNavigationStarted();
  void SameDocumentNavigationCommitted(const String& url);
  bool ModifiedDOM();
  uint32_t SoftNavigationCount() { return soft_navigation_count_; }

  // TaskAttributionTracker::Observer's implementation.
  void OnCreateTaskScope(scheduler::TaskAttributionInfo&) override;

  void RecordPaint(LocalFrame*,
                   uint64_t painted_area,
                   bool is_modified_by_soft_navigation);

  EventScope CreateEventScope(EventScope::Type type, bool is_new_interaction);

  // This method is called during the weakness processing stage of garbage
  // collection, and it's used to detect `potential_soft_navigation_tasks_`
  // becoming empty.
  void ProcessCustomWeakness(const LivenessBroker& info);

  bool GetInitialInteractionEncounteredForTest() {
    return initial_interaction_encountered_;
  }

  scheduler::TaskAttributionIdType GetLastInteractionTaskIdForTest() const {
    return last_interaction_task_id_.value();
  }

 private:
  enum FlagType : uint8_t {
    kURLChange,
    kMainModification,
  };

  using FlagTypeSet = base::EnumSet<FlagType, kURLChange, kMainModification>;
  struct PerInteractionData : public GarbageCollected<PerInteractionData> {
    // The timestamp just before the event responding to the user's interaction
    // started processing. In case of multiple events for a single interaction
    // (e.g. a keyboard key press resulting in keydown, keypress, and keyup),
    // this timestamp would be the time before processing started on the first
    // event.
    base::TimeTicks user_interaction_timestamp;
    FlagTypeSet flag_set;
    String url;
    void Trace(Visitor*) const {}
  };

  struct EventParameters {
    explicit EventParameters() = default;
    EventParameters(bool is_new_interaction, EventScope::Type type)
        : is_new_interaction(is_new_interaction), type(type) {}

    bool is_new_interaction = false;
    EventScope::Type type = EventScope::Type::kClick;
  };

  void ReportSoftNavigationToMetrics(LocalFrame* frame) const;
  void RecordUmaForNonSoftNavigationInteractions() const;
  void CheckSoftNavigationConditions(const PerInteractionData& data);
  void SetIsTrackingSoftNavigationHeuristicsOnDocument(bool value) const;

  std::optional<scheduler::TaskAttributionId>
  GetUserInteractionAncestorTaskIfAny();
  std::optional<scheduler::TaskAttributionId> SetFlagIfDescendantAndCheck(
      FlagType);
  void ResetHeuristic();
  void ResetPaintsIfNeeded();
  void CommitPreviousPaints(LocalFrame*);
  void EmitSoftNavigationEntryIfAllConditionsMet(LocalFrame*);
  LocalFrame* GetLocalFrameIfNotDetached() const;
  void UserInitiatedInteraction();
  void SetCurrentTimeAsStartTime();
  void OnSoftNavigationEventScopeDestroyed();

  PerInteractionData* GetCurrentInteractionData(scheduler::TaskAttributionId);

  // This must only be called when `all_event_parameters_` is non-empty.
  const EventParameters& CurrentEventParameters() {
    return all_event_parameters_.back();
  }

  HeapHashSet<WeakMember<const scheduler::TaskAttributionInfo>>
      potential_soft_navigation_tasks_;
  WTF::HashMap<scheduler::TaskAttributionIdType,
               std::optional<scheduler::TaskAttributionId>>
      soft_navigation_descendant_cache_;
  bool did_reset_paints_ = false;
  bool did_commit_previous_paints_ = false;
  HeapHashMap<scheduler::TaskAttributionIdType, Member<PerInteractionData>>
      interaction_task_id_to_interaction_data_;
  base::TimeTicks pending_interaction_timestamp_;
  std::optional<scheduler::TaskAttributionId>
      last_soft_navigation_ancestor_task_;
  Member<const PerInteractionData> soft_navigation_interaction_data_;
  WTF::HashMap<scheduler::TaskAttributionIdType,
               scheduler::TaskAttributionIdType>
      task_id_to_interaction_task_id_;
  uint32_t soft_navigation_count_ = 0;
  uint64_t softnav_painted_area_ = 0;
  uint64_t initial_painted_area_ = 0;
  uint64_t viewport_area_ = 0;
  scheduler::TaskAttributionId last_interaction_task_id_;
  bool soft_navigation_conditions_met_ = false;
  bool paint_conditions_met_ = false;
  bool initial_interaction_encountered_ = false;
  // `SoftNavigationEventScope`s can be nested in case a click/keyboard event
  // synchronously initiates a navigation. `all_event_parameters_` stores one
  // `EventParameters` per scope, with the most recent one in the back.
  WTF::Deque<EventParameters> all_event_parameters_;
  // Used to synchronize resetting the heuristic when
  // `potential_soft_navigation_tasks_` becomes empty during GC.
  bool has_potential_soft_navigation_task_ = false;
};

}  // namespace blink

#endif  //  THIRD_PARTY_BLINK_RENDERER_CORE_TIMING_SOFT_NAVIGATION_HEURISTICS_H_
