// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_COMPOSE_PROACTIVE_NUDGE_TRACKER_H_
#define CHROME_BROWSER_COMPOSE_PROACTIVE_NUDGE_TRACKER_H_

#include <map>
#include <memory>
#include <optional>
#include <string>

#include "base/functional/callback_forward.h"
#include "base/memory/raw_ref.h"
#include "base/memory/weak_ptr.h"
#include "components/autofill/content/browser/scoped_autofill_managers_observation.h"
#include "components/autofill/core/browser/ui/suggestion.h"
#include "components/autofill/core/common/unique_ids.h"
#include "components/compose/core/browser/compose_metrics.h"
#include "components/segmentation_platform/public/segmentation_platform_service.h"

namespace compose {

// This class is a state machine tracking whether the proactive nudge should
// show for Compose. It has 4 states:
//   - UNINITIALIZED,
//   - WAITING,
//   - REQUESTED,
//   - SHOWN
//
// Generally, states transition forward through the list (skipping states if
// required). If the active form field changes (or the form loses focus), the
// state is reset to UNINITIALIZED.
//
// The state is represented by an optional `State` struct.
// * If the struct is `std::nullopt` then the state is UNINITIALIZED.
// * If the struct has a value, the value of `show_state` differentiates between
//   the remaining states.
// * The Delegate is called at the transition from WAITING to REQUESTED.
// * Unintuitively, `ProactiveNudgeRequestedForFormField` can cause a transition
//   from REQUESTED to SHOWN. Compose interacts with Autofill such that
//   it cannot directly show the nudge; instead it requests the Autofill Agent
//   for the current frame to ask for values to fill. Thus, the entry point is
//   the same both for new nudge states, and for the final step of actually
//   showing the nudge.
class ProactiveNudgeTracker : public autofill::AutofillManager::Observer {
 public:
  class Delegate {
   public:
    virtual void ShowProactiveNudge(autofill::FormGlobalId form,
                                    autofill::FieldGlobalId field) = 0;
  };

  enum class ShowState { kWaiting, kCanBeShown, kShown };

  class State : public base::SupportsWeakPtr<State> {
   public:
    State();
    virtual ~State();

    autofill::FormGlobalId form;
    autofill::FieldGlobalId field;
    std::u16string initial_text_value;
    std::optional<segmentation_platform::ClassificationResult>
        segmentation_result = std::nullopt;
    base::OneShotTimer timer;
    bool timer_complete = false;

    ShowState show_state = ShowState::kWaiting;
  };

  ProactiveNudgeTracker(
      segmentation_platform::SegmentationPlatformService* segmentation_service,
      Delegate* delegate);

  ~ProactiveNudgeTracker() override;

  // Call so that focus events can be obtained from the AutofillManager for this
  // `web_contents`.
  void StartObserving(content::WebContents* web_contents);

  // If the current state is UNINITIALIZED, begins tracking the state of a form
  // field, and updates the state to WAITING.
  //
  // IF the current state is REQUESTED, updates the state to SHOWN.
  //
  // If the state is not UNINITIALIZED or REQUESTED,
  // ProactiveNudgeRequestedForFormField is a no-op.
  //
  // Returns true if the nudge can be shown immediately.
  bool ProactiveNudgeRequestedForFormField(
      const autofill::FormFieldData& field_to_track);

  void FocusChangedInPage();

  void Clear();

  void ComposeSessionCompleted(autofill::FieldRendererId field_renderer_id,
                               ComposeSessionCloseReason session_close_reason,
                               const compose::ComposeSessionEvents& events);
  void OnUserDisabledNudge(bool single_site_only);

  // autofill::AutofillManager::Observer:
  void OnAfterFocusOnFormField(autofill::AutofillManager& manager,
                               autofill::FormGlobalId form,
                               autofill::FieldGlobalId field) override;

 private:
  class EngagementTracker;
  bool SegmentationStateIsValid();
  void ResetState();
  void ShowTimerElapsed();
  void GotClassificationResult(
      base::WeakPtr<State> state,
      const segmentation_platform::ClassificationResult& result);
  void MaybeShowProactiveNudge();
  bool MatchesCurrentField(autofill::FormGlobalId form,
                           autofill::FieldGlobalId field);
  void CollectTrainingData(
      const segmentation_platform::TrainingRequestId training_request_id,
      ProactiveNudgeDerivedEngagement engagement);
  bool ShouldShow(const State& state);

  std::unique_ptr<State> state_;

  std::map<autofill::FieldRendererId, std::unique_ptr<EngagementTracker>>
      engagement_trackers_;

  raw_ptr<segmentation_platform::SegmentationPlatformService>
      segmentation_service_;
  raw_ptr<Delegate> delegate_;

  autofill::ScopedAutofillManagersObservation autofill_managers_observation_{
      this};
  base::WeakPtrFactory<ProactiveNudgeTracker> weak_ptr_factory_{this};
};

}  // namespace compose
#endif  // CHROME_BROWSER_COMPOSE_PROACTIVE_NUDGE_TRACKER_H_
