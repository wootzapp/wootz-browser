// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_AUTOFILL_AUTOFILL_POPUP_CONTROLLER_IMPL_H_
#define CHROME_BROWSER_UI_AUTOFILL_AUTOFILL_POPUP_CONTROLLER_IMPL_H_

#include <optional>
#include <string>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "chrome/browser/ui/autofill/autofill_popup_controller.h"
#include "chrome/browser/ui/autofill/autofill_popup_hide_helper.h"
#include "chrome/browser/ui/autofill/next_idle_time_ticks.h"
#include "chrome/browser/ui/autofill/popup_controller_common.h"
#include "components/autofill/core/browser/ui/popup_hiding_reasons.h"
#include "components/autofill/core/common/aliases.h"
#include "components/password_manager/core/browser/password_manager_metrics_util.h"
#include "content/public/browser/render_widget_host.h"
#include "third_party/abseil-cpp/absl/types/variant.h"

class Profile;

namespace content {
struct NativeWebKeyboardEvent;
class WebContents;
}  // namespace content

namespace gfx {
class RectF;
}  // namespace gfx

namespace ui {
class AXPlatformNode;
}

namespace autofill {

class AutofillPopupDelegate;
class AutofillPopupView;

// Sub-popups and their parent popups are connected by providing children
// with links to their parents. This interface defines the API exposed by
// these links.
class ExpandablePopupParentControllerImpl {
 private:
  friend class AutofillPopupControllerImpl;

  // Creates a view for a sub-popup. On rare occasions opening the sub-popup
  // may fail (e.g. when there is no room to open the sub-popup or the popup
  // is in the middle of destroying and  has no widget already),
  // `nullptr` is returned in these cases.
  virtual base::WeakPtr<AutofillPopupView> CreateSubPopupView(
      base::WeakPtr<AutofillPopupController> sub_controller) = 0;

  // Returns the number of popups above this one. For example, if `this` is the
  // second popup, `GetPopupLevel()` returns 1, if `this` is the root popup,
  // it returns 0.
  virtual int GetPopupLevel() const = 0;
};

// This class is a controller for an AutofillPopupView. It implements
// AutofillPopupController to allow calls from AutofillPopupView. The
// other, public functions are available to its instantiator.
class AutofillPopupControllerImpl
    : public AutofillPopupController,
      public ExpandablePopupParentControllerImpl {
 public:
  AutofillPopupControllerImpl(const AutofillPopupControllerImpl&) = delete;
  AutofillPopupControllerImpl& operator=(const AutofillPopupControllerImpl&) =
      delete;

  // Creates a new `AutofillPopupControllerImpl`, or reuses `previous` if the
  // construction arguments are the same. `previous` may be invalidated by this
  // call. The controller will listen for keyboard input routed to
  // `web_contents` while the popup is showing, unless `web_contents` is NULL.
  static base::WeakPtr<AutofillPopupControllerImpl> GetOrCreate(
      base::WeakPtr<AutofillPopupControllerImpl> previous,
      base::WeakPtr<AutofillPopupDelegate> delegate,
      content::WebContents* web_contents,
      gfx::NativeView container_view,
      const gfx::RectF& element_bounds,
      base::i18n::TextDirection text_direction,
      int32_t form_control_ax_id);

  // Shows the popup, or updates the existing popup with the given values.
  virtual void Show(std::vector<Suggestion> suggestions,
                    AutofillSuggestionTriggerSource trigger_source,
                    AutoselectFirstSuggestion autoselect_first_suggestion);

  // Updates the data list values currently shown with the popup.
  virtual void UpdateDataListValues(base::span<const SelectOption> options);

  // Informs the controller that the popup may not be hidden by stale data or
  // interactions with native Chrome UI. This state remains active until the
  // view is destroyed.
  void PinView();

  // Hides the popup and destroys the controller. This also invalidates
  // `delegate_`.
  void Hide(PopupHidingReason reason) override;

  // Invoked when the view was destroyed by by someone other than this class.
  void ViewDestroyed() override;

  // Handles a key press event and returns whether the event should be swallowed
  // (meaning that no other handler, in not particular the default handler, can
  // process it).
  bool HandleKeyPressEvent(const content::NativeWebKeyboardEvent& event);

  // AutofillPopupController:
  void OnSuggestionsChanged() override;
  void SelectSuggestion(int index) override;
  void UnselectSuggestion() override;
  void AcceptSuggestion(int index) override;
  void PerformButtonActionForSuggestion(int index) override;
  bool RemoveSuggestion(
      int list_index,
      AutofillMetrics::SingleEntryRemovalMethod removal_method) override;
  int GetLineCount() const override;
  std::vector<Suggestion> GetSuggestions() const override;
  const Suggestion& GetSuggestionAt(int row) const override;
  std::u16string GetSuggestionMainTextAt(int row) const override;
  std::u16string GetSuggestionMinorTextAt(int row) const override;
  std::vector<std::vector<Suggestion::Text>> GetSuggestionLabelsAt(
      int row) const override;
  bool GetRemovalConfirmationText(int list_index,
                                  std::u16string* title,
                                  std::u16string* body) override;
  FillingProduct GetMainFillingProduct() const override;
  bool ShouldIgnoreMouseObservedOutsideItemBoundsCheck() const override;
  base::WeakPtr<AutofillPopupController> OpenSubPopup(
      const gfx::RectF& anchor_bounds,
      std::vector<Suggestion> suggestions,
      AutoselectFirstSuggestion autoselect_first_suggestion) override;
  std::optional<AutofillClient::PopupScreenLocation> GetPopupScreenLocation()
      const override;
  void HideSubPopup() override;

  void KeepPopupOpenForTesting() { keep_popup_open_for_testing_ = true; }

  // Disables show thresholds. See the documentation of the member for details.
  void DisableThresholdForTesting(bool disable_threshold) {
    disable_threshold_for_testing_ = disable_threshold;
  }

  void SetViewForTesting(base::WeakPtr<AutofillPopupView> view) {
    view_ = std::move(view);
    time_view_shown_ = NextIdleTimeTicks::CaptureNextIdleTimeTicks();
  }

  int GetLineCountForTesting() const { return GetLineCount(); }

 protected:
  AutofillPopupControllerImpl(
      base::WeakPtr<AutofillPopupDelegate> delegate,
      content::WebContents* web_contents,
      gfx::NativeView container_view,
      const gfx::RectF& element_bounds,
      base::i18n::TextDirection text_direction,
      int32_t form_control_ax_id,
      base::RepeatingCallback<void(
          gfx::NativeWindow,
          Profile*,
          password_manager::metrics_util::PasswordMigrationWarningTriggers)>
          show_pwd_migration_warning_callback,
      std::optional<base::WeakPtr<ExpandablePopupParentControllerImpl>> parent);
  ~AutofillPopupControllerImpl() override;

  gfx::NativeView container_view() const override;
  content::WebContents* GetWebContents() const override;
  const gfx::RectF& element_bounds() const override;
  base::i18n::TextDirection GetElementTextDirection() const override;

  // Returns true if the popup still has non-options entries to show the user.
  bool HasSuggestions() const;

  // Set the Autofill entry values. Exposed to allow tests to set these values
  // without showing the popup.
  void SetSuggestions(std::vector<Suggestion> suggestions);

  base::WeakPtr<AutofillPopupControllerImpl> GetWeakPtr();

  // Raise an accessibility event to indicate the controls relation of the
  // form control of the popup and popup itself has changed based on the popup's
  // show or hide action.
  void FireControlsChangedEvent(bool is_show);

  // Gets the root AXPlatformNode for our WebContents, which can be used
  // to find the AXPlatformNode specifically for the autofill text field.
  virtual ui::AXPlatformNode* GetRootAXPlatformNodeForWebContents();

  // Hides `view_` unless it is null and then deletes `this`.
  virtual void HideViewAndDie();

 private:
  // Clear the internal state of the controller. This is needed to ensure that
  // when the popup is reused it doesn't leak values between uses.
  void ClearState();

  // Returns true iff the focused frame has a pointer lock, which may be used to
  // trick the user into accepting some suggestion (crbug.com/1239496). In such
  // a case, we should hide the popup.
  bool IsPointerLocked() const;

  // ExpandablePopupParentControllerImpl:
  base::WeakPtr<AutofillPopupView> CreateSubPopupView(
      base::WeakPtr<AutofillPopupController> controller) override;
  int GetPopupLevel() const override;

  // Returns `true` if this popup has no parent, and `false` for sub-popups.
  bool IsRootPopup() const;

  base::WeakPtr<content::WebContents> web_contents_;
  PopupControllerCommon controller_common_;
  base::WeakPtr<AutofillPopupView> view_;
  base::WeakPtr<AutofillPopupDelegate> delegate_;

  // A helper class for capturing key press events associated with a
  // `content::RenderFrameHost`.
  class KeyPressObserver {
   public:
    explicit KeyPressObserver(AutofillPopupControllerImpl* observer);
    ~KeyPressObserver();

    void Observe(content::RenderFrameHost* rfh);
    void Reset();

   private:
    const raw_ref<AutofillPopupControllerImpl> observer_;
    content::GlobalRenderFrameHostId rfh_;
    content::RenderWidgetHost::KeyPressEventCallback handler_;
  } key_press_observer_{this};

  // The time the view was shown the last time. It is used to safeguard against
  // accepting suggestions too quickly after a the popup view was shown (see the
  // `show_threshold` parameter of `AcceptSuggestion`).
  NextIdleTimeTicks time_view_shown_;

  // An override to suppress minimum show thresholds. It should only be set
  // during tests that cannot mock time (e.g. the autofill interactive
  // browsertests).
  bool disable_threshold_for_testing_ = false;

  // If set to true, the popup will never be hidden because of stale data or if
  // the user interacts with native UI.
  bool is_view_pinned_ = false;

  // The current Autofill query values.
  std::vector<Suggestion> suggestions_;

  // The trigger source of the `suggestions_`.
  AutofillSuggestionTriggerSource trigger_source_ =
      AutofillSuggestionTriggerSource::kUnspecified;

  // The AX ID of the field on which Autofill was triggered.
  int32_t form_control_ax_id_ = 0;

  // If set to true, the popup will stay open regardless of external changes on
  // the machine that would normally cause the popup to be hidden.
  bool keep_popup_open_for_testing_ = false;

  // Callback invoked to try to show the password migration warning on Android.
  // Used to facilitate testing.
  // TODO(crbug.com/1454469): Remove when the warning isn't needed anymore.
  base::RepeatingCallback<void(
      gfx::NativeWindow,
      Profile*,
      password_manager::metrics_util::PasswordMigrationWarningTriggers)>
      show_pwd_migration_warning_callback_;

  // Timer to close a fading popup.
  base::OneShotTimer fading_popup_timer_;

  // Whether the popup should ignore mouse observed outside check.
  bool should_ignore_mouse_observed_outside_item_bounds_check_ = false;

  // Parent's popup controller. The root popup doesn't have a parent, but in
  // sub-popups it must be present.
  const std::optional<base::WeakPtr<ExpandablePopupParentControllerImpl>>
      parent_controller_;

  // The open sub-popup controller if any, `nullptr` otherwise.
  base::WeakPtr<AutofillPopupControllerImpl> sub_popup_controller_;

  // This is a helper which detects events that should hide the popup.
  std::optional<AutofillPopupHideHelper> popup_hide_helper_;

  // AutofillPopupControllerImpl deletes itself. To simplify memory management,
  // we delete the object asynchronously.
  base::WeakPtrFactory<AutofillPopupControllerImpl>
      self_deletion_weak_ptr_factory_{this};

  base::WeakPtrFactory<AutofillPopupControllerImpl> weak_ptr_factory_{this};
};

}  // namespace autofill

#endif  // CHROME_BROWSER_UI_AUTOFILL_AUTOFILL_POPUP_CONTROLLER_IMPL_H_
