// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_ACCESSIBILITY_VIEW_ACCESSIBILITY_H_
#define UI_VIEWS_ACCESSIBILITY_VIEW_ACCESSIBILITY_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/functional/callback_forward.h"
#include "base/memory/raw_ptr.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "ui/accessibility/ax_enums.mojom-forward.h"
#include "ui/accessibility/ax_node_data.h"
#include "ui/accessibility/platform/ax_unique_id.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/views/accessibility/ax_virtual_view.h"
#include "ui/views/accessibility/view_accessibility_utils.h"
#include "ui/views/views_export.h"
#include "ui/views/widget/widget_observer.h"

namespace ui {

class AXPlatformNodeDelegate;

}  // namespace ui

namespace views {

class AtomicViewAXTreeManager;
class View;
class Widget;

// An object that manages the accessibility interface for a View.
//
// The default accessibility properties of a View is determined by calling
// |View::GetAccessibleNodeData()|, which is overridden by many |View|
// subclasses. |ViewAccessibility| lets you override these for a particular
// view.
//
// In most cases, subclasses of |ViewAccessibility| own the |AXPlatformNode|
// that implements the native accessibility APIs on a specific platform.
class VIEWS_EXPORT ViewAccessibility : public WidgetObserver {
 public:
  using AccessibilityEventsCallback =
      base::RepeatingCallback<void(const ui::AXPlatformNodeDelegate*,
                                   const ax::mojom::Event)>;
  using AXVirtualViews = AXVirtualView::AXVirtualViews;

  static std::unique_ptr<ViewAccessibility> Create(View* view);

  ViewAccessibility(const ViewAccessibility&) = delete;
  ViewAccessibility& operator=(const ViewAccessibility&) = delete;
  ~ViewAccessibility() override;

  // Modifies |node_data| to reflect the current accessible state of the
  // associated View, taking any custom overrides into account
  // (see OverrideFocus, etc. below).
  virtual void GetAccessibleNodeData(ui::AXNodeData* node_data) const;

  void NotifyEvent(ax::mojom::Event event_type, bool send_native_event = true);

  // Made to be overridden on platforms that need the temporary
  // `AtomicViewAXTreeManager` to enable more accessibility functionalities for
  // Views. See crbug.com/1468416 for more info.
  virtual void EnsureAtomicViewAXTreeManager() {}

  void SetIgnoreMissingWidgetForTesting(bool value) {
    ignore_missing_widget_for_testing_ = value;
  }

  //
  // The following methods get or set accessibility attributes (in the owning
  // View's AXNodeData), overrideing any identical attributes which might have
  // been set by the owning View in its View::GetAccessibleNodeData() method.
  //
  // Note that accessibility string attributes are only used if non-empty, so
  // you can't override a string with the empty string.
  //

  // Sets one of our virtual descendants as having the accessibility focus. This
  // means that if this view has the system focus, it will set the accessibility
  // focus to the provided descendant virtual view instead. Set this to nullptr
  // if none of our virtual descendants should have the accessibility focus. It
  // is illegal to set this to any virtual view that is currently not one of our
  // descendants and this is enforced by a DCHECK.
  void OverrideFocus(AXVirtualView* virtual_view);

  // Returns whether this view is focusable when the user uses an accessibility
  // aid or the keyboard, even though it may not be normally focusable. Note
  // that if using the keyboard, on macOS the preference "Full Keyboard Access"
  // needs to be turned on.
  virtual bool IsAccessibilityFocusable() const;

  // Used for testing. Returns true if this view is considered focused.
  virtual bool IsFocusedForTesting() const;

  // Call when this is the active descendant of a popup view that temporarily
  // takes over focus. It is only necessary to use this for menus like autofill,
  // where the actual focus is in content.
  // When the popup closes, call EndPopupFocusOverride().
  virtual void SetPopupFocusOverride();

  // Call when popup closes, if it used SetPopupFocusOverride().
  virtual void EndPopupFocusOverride();

  // Call when a menu closes, to restore focus to where it was previously.
  virtual void FireFocusAfterMenuClose();

  // Convenience function to set common accessibility properties during view
  // construction/initialization. It should only be used to define property
  // values as part of the creation of this view; not to provide property-
  // change updates. This function will only modify properties for which a value
  // has been explicitly set.
  void SetProperties(
      std::optional<ax::mojom::Role> role = std::nullopt,
      std::optional<std::u16string> name = std::nullopt,
      std::optional<std::u16string> description = std::nullopt,
      std::optional<std::u16string> role_description = std::nullopt,
      std::optional<ax::mojom::NameFrom> name_from = std::nullopt,
      std::optional<ax::mojom::DescriptionFrom> description_from =
          std::nullopt);

  // Sets/gets whether or not this view's descendants should be included in
  // the accessibility tree. It is the functional equivalent of calling
  // `SetAccessibleIsIgnored` on each and every view descendant of this
  // view. The default value is false, which is appropriate for most views.
  // Note that you should not set this property if a view has no descendants.
  // It is essential that you do not set this property to true on containers
  // which have one or more descendants which are focusable or otherwise
  // interactive as this would make those descendants completely inaccessible.
  // If no value has been set, the "leafiness" of this view will be based on
  // the type of children (if any).
  void SetIsLeaf(bool value);
  virtual bool IsLeaf() const;

  // Returns true if an ancestor of this node (not including itself) is a
  // leaf node, meaning that this node is not actually exposed to any
  // platform's accessibility layer.
  virtual bool IsChildOfLeaf() const;

  // Returns true if we heuristically pruned (ignored) this view from the
  // accessibility tree.
  bool GetIsPruned() const;

  void SetCharacterOffsets(const std::vector<int32_t>& offsets);

  void SetWordStarts(const std::vector<int32_t>& offsets);

  void SetWordEnds(const std::vector<int32_t>& offsets);

  void ClearTextOffsets();

  void SetHasPopup(const ax::mojom::HasPopup has_popup);

  void SetRole(const ax::mojom::Role role);

  // Sets the accessible role along with a customized string to be used by
  // assistive technologies to present the role. When there is no role
  // description provided, assistive technologies will use either the default
  // role descriptions we provide (which are currently located in a number of
  // places. See crbug.com/1290866) or the value provided by their platform. As
  // a general rule, it is preferable to not override the role string. Please
  // seek review from accessibility OWNERs when using this function.
  void SetRole(ax::mojom::Role role, const std::u16string& role_description);

  // This function cannot follow the established pattern and be named GetRole()
  // because of a function of the same name in AXPlatformNodeDelegate.
  // ViewAXPlatformNodeDelegate extends both ViewAccessibility and
  // AXPlatformNodeDelegate, which would lead to conflicts and confusion.
  // TODO(accessibility): Rename to GetRole once the ViewsAX project is
  // completed and we don't have ViewAXPlatformNodeDelegate anymore.
  ax::mojom::Role GetCachedRole() const;

  // For the same reasons as GetCachedRole, this function cannot
  // follow the established pattern and be named GetName()
  // TODO(accessibility): Rename to GetName once the ViewsAX project is
  // completed and we don't have ViewAXPlatformNodeDelegate anymore.
  const std::string& GetCachedName() const;

  // Returns the source type of the accessible name.
  //
  // This function cannot currently be named GetNameFrom() because of a function
  // of the same name in AXPlatformNodeDelegate. ViewAXPlatformNodeDelegate
  // extends both ViewAccessibility and AXPlatformNodeDelegate.
  // TODO(accessibility): Rename to GetNameFrom once the ViewsAX project is
  // completed and we don't have ViewAXPlatformNodeDelegate anymore.
  ax::mojom::NameFrom GetCachedNameFrom() const;

  // Sets the accessible name to the specified string and source type.
  // To indicate that this view should never have an accessible name, e.g. to
  // prevent screen readers from speaking redundant information, set the type to
  // `kAttributeExplicitlyEmpty`. NOTE: Do not use `kAttributeExplicitlyEmpty`
  // on a view which may or may not have a name depending on circumstances. Also
  // please seek review from accessibility OWNERs when removing the name,
  // especially for views which are focusable or otherwise interactive.
  // The source type options are:
  //
  // * kNone: No name provided.
  // * kAttribute: Name from a flat string (e.g. aria-label or View). This is
  //   the default value.
  // * kAttributeExplicitlyEmpty: Name removed for accessibility reasons.
  // * kCaption: Name from a table caption.
  // * kContents: Name from the displayed text (e.g. label or link).
  // * kPlaceholder: Name from a textfield placeholder.
  // * kRelatedElement: Name from another object in the UI(e.g. figcaption or
  //   View).
  // * kTitle: Name from a title attribute or element (HTML or SVG).
  // * kValue: Name from a value attribute (e.g. button).
  // * kPopoverAttribute: Name from a tooltip-style popover.
  void SetName(const std::string& name, ax::mojom::NameFrom name_from);
  void SetName(const std::u16string& name, ax::mojom::NameFrom name_from);
  void SetName(const std::string& name);
  void SetName(const std::u16string& name);

  // Sets the accessible name of this view to that of `naming_view`. Often
  // `naming_view` is a `views::Label`, but any view with an accessible name
  // will work.
  void SetName(View& naming_view);

  void SetBounds(const gfx::RectF& bounds);

  void SetIsSelected(bool selected);

  // Hides this view from the accessibility APIs. Keep in mind that this is not
  // the sole determinant of whether the ignored state is set. See
  // `UpdateIgnoredState`.
  void SetIsIgnored(bool is_ignored);
  virtual bool GetIsIgnored() const;

  // Note that `pos_in_set` starts from 1 not 0.
  void SetPosInSet(int pos_in_set);
  void SetSetSize(int set_size);
  void ClearPosInSet();
  void ClearSetSize();

  void SetActiveDescendant(views::View& view);
  void ClearActiveDescendant();

  // Sets/gets whether or not this view should be marked as "enabled" for the
  // purpose exposing this state in the accessibility tree. As a general rule,
  // it is not advisable to mark a View as enabled in the accessibility tree,
  // while the real View is actually disabled, because such a View will not
  // respond to user actions.
  void SetIsEnabled(bool is_enabled);
  bool GetIsEnabled() const;

  void SetDescription(const std::string& description,
                      const ax::mojom::DescriptionFrom description_from =
                          ax::mojom::DescriptionFrom::kAriaDescription);
  void SetDescription(const std::u16string& description,
                      const ax::mojom::DescriptionFrom description_from =
                          ax::mojom::DescriptionFrom::kAriaDescription);
  void SetDescription(View& describing_view);
  // This function cannot follow the established pattern and be named
  // GetDescription() because of a function of the same name in
  // AXPlatformNodeDelegate. ViewAXPlatformNodeDelegate extends both
  // ViewAccessibility and AXPlatformNodeDelegate, which would lead to conflicts
  // and confusion.
  // TODO(accessibility): Rename to GetDescription once the ViewsAX project is
  // completed and we don't have ViewAXPlatformNodeDelegate anymore.
  std::u16string GetCachedDescription() const;

  void SetCheckedState(ax::mojom::CheckedState checked_state);
  void RemoveCheckedState();

  // Sets the platform-specific accessible name/title property of the
  // NativeViewAccessible window. This is needed on platforms where the name
  // of the NativeViewAccessible window is automatically calculated by the
  // platform's accessibility API. For instance on the Mac, the label of the
  // NativeWidgetMacNSWindow of a JavaScript alert is taken from the name of
  // the child RootView. Note: the first function does the string conversion
  // and calls the second, thus only the latter needs to be implemented by
  // interested platforms.
  void OverrideNativeWindowTitle(const std::u16string& title);
  virtual void OverrideNativeWindowTitle(const std::string& title);

  // Sets whether this View hides all its descendants from the accessibility
  // tree that is exposed to platform APIs. This is similar, but not exactly
  // identical to aria-hidden="true".
  //
  // Note that this attribute does not cross widget boundaries, i.e. if a sub
  // widget is a descendant of this View, it will not be marked hidden. This
  // should not happen in practice as widgets are not children of Views.
  // Deprecated. Use ViewAccessibility::SetIsLeaf instead.
  // See https://crbug.com/324485311.
  void OverrideIsLeaf(bool value);

  // Override the next or previous focused widget. Some assistive technologies,
  // such as screen readers, may utilize this information to transition focus
  // from the beginning or end of one widget to another when navigating by its
  // default navigation method.
  void SetNextFocus(Widget* widget);
  void SetPreviousFocus(Widget* widget);
  Widget* GetNextWindowFocus() const;
  Widget* GetPreviousWindowFocus() const;

  void SetShowContextMenu(bool show_context_menu);

  void SetState(ax::mojom::State state, bool is_enabled);

  // Updates the focusable state of the `data_` object.
  // The view is considered focusable if it is not set to never receive focus
  // This function must be called whenever an attribute that can affect the
  // focusable state changes
  void UpdateFocusableState();

  // This function recursively updates the focusable state of the `data_` member
  // and that of the view's children. Then it updates the focusable state of the
  // current view.
  void UpdateFocusableStateRecursive();

  // Updates the invisible state of the `data_` object. The view is considered
  // invisible if it is not visible and its role is not kAlert.
  void UpdateInvisibleState();

  // Override the child tree id.
  void OverrideChildTreeID(ui::AXTreeID tree_id);
  ui::AXTreeID GetChildTreeID() const;

  // Returns the accessibility object that represents the View whose
  // accessibility is managed by this instance. This may be an AXPlatformNode or
  // it may be a native accessible object implemented by another class.
  virtual gfx::NativeViewAccessible GetNativeObject() const;

  // Causes the screen reader to announce |text|. If the current user is not
  // using a screen reader, has no effect. AnnouncePolitely() will speak
  // the given string. AnnounceAlert() may make a stronger attempt to be
  // noticeable; the screen reader may say something like "Alert: hello"
  // instead of just "hello", and may interrupt any existing text being spoken.
  // However, the screen reader may also treat the two calls the same.
  // AnnounceText() is a deprecated alias for AnnounceAlert().
  // TODO(crbug.com/40287811) - Migrate all callers of AnnounceText() to
  // one of the other two methods.
  virtual void AnnounceAlert(const std::u16string& text);
  virtual void AnnouncePolitely(const std::u16string& text);
  virtual void AnnounceText(const std::u16string& text);

  virtual const ui::AXUniqueId& GetUniqueId() const;

  View* view() const { return view_; }
  AXVirtualView* FocusedVirtualChild() const { return focused_virtual_child_; }

  virtual AtomicViewAXTreeManager* GetAtomicViewAXTreeManagerForTesting() const;

  //
  // Methods for managing virtual views.
  //

  // Adds |virtual_view| as a child of this View. We take ownership of our
  // virtual children.
  void AddVirtualChildView(std::unique_ptr<AXVirtualView> virtual_view);

  // Adds |virtual_view| as a child of this View at an index.
  // We take ownership of our virtual children.
  void AddVirtualChildViewAt(std::unique_ptr<AXVirtualView> virtual_view,
                             size_t index);

  // Removes |virtual_view| from this View. The virtual view's parent will
  // change to nullptr. Hands ownership back to the caller.
  std::unique_ptr<AXVirtualView> RemoveVirtualChildView(
      AXVirtualView* virtual_view);

  // Removes all the virtual children from this View.
  // The virtual views are deleted.
  void RemoveAllVirtualChildViews();

  const AXVirtualViews& virtual_children() const { return virtual_children_; }

  // Returns true if |virtual_view| is contained within the hierarchy of this
  // View, even as an indirect descendant.
  bool Contains(const AXVirtualView* virtual_view) const;

  // Returns the index of |virtual_view|, or nullopt if |virtual_view| is not a
  // child of this View.
  std::optional<size_t> GetIndexOf(const AXVirtualView* virtual_view) const;

  // Returns the native accessibility object associated with the AXVirtualView
  // descendant that is currently focused. If no virtual descendants are
  // present, or no virtual descendant has been marked as focused, returns the
  // native accessibility object associated with this view.
  gfx::NativeViewAccessible GetFocusedDescendant();

  // If true, moves accessibility focus to an ancestor.
  void set_propagate_focus_to_ancestor(bool value) {
    propagate_focus_to_ancestor_ = value;
  }

  bool propagate_focus_to_ancestor() { return propagate_focus_to_ancestor_; }

  // If true, ensures an AtomicViewAXTreeManager is created for this view.
  void set_needs_ax_tree_manager(bool value) { needs_ax_tree_manager_ = value; }

  bool needs_ax_tree_manager() { return needs_ax_tree_manager_; }

  // Used for testing. Allows a test to watch accessibility events.
  const AccessibilityEventsCallback& accessibility_events_callback() const;
  void set_accessibility_events_callback(AccessibilityEventsCallback callback);

  // Widget Observer
  // Views may misbehave if their widget is closed; set "null-like" attributes
  // rather than possibly crashing.
  void OnWidgetClosing(Widget* widget) override;

 protected:
  explicit ViewAccessibility(View* view);

  virtual void FireNativeEvent(ax::mojom::Event event_type);

  // Used for testing. Called every time an accessibility event is fired.
  AccessibilityEventsCallback accessibility_events_callback_;

 private:
  // Prune/Unprune all descendant views from the accessibility tree. We prune
  // for two reasons: 1) The view has been explicitly marked as a leaf node, 2)
  // The view is focusable and lacks focusable descendants (e.g. a button with a
  // label and/or an image).
  void PruneSubtree();
  void UnpruneSubtree();

  // Updates the ignored state of the `data_` object.
  // The view is considered ignored if it should be ignored as per
  // `should_be_ignored_`, or if it has been pruned (`pruned_`), or if its role
  // is 'kNone'.
  void UpdateIgnoredState();

  void OnWidgetClosingRecursive();

  void SetDataForClosedWidget(ui::AXNodeData* data) const;

  // Weak. Owns this.
  const raw_ptr<View> view_;

  // If there are any virtual children, they override any real children.
  // We own our virtual children.
  AXVirtualViews virtual_children_;

  // The virtual child that is currently focused.
  // This is nullptr if no virtual child is focused.
  // See also OverrideFocus() and GetFocusedDescendant().
  raw_ptr<AXVirtualView> focused_virtual_child_;

  const ui::AXUniqueId unique_id_{ui::AXUniqueId::Create()};

  // Contains data that is populated by the setters in this class.
  // This member is tied to the ViewsAX project. Which is introducing a new
  // system to set accessible properties in a "push" fashion (instead of pull).
  // Authors are encouraged to start using it today, and it will eventually
  // replace the old system. For now, while the migration to the new system
  // happens, we allow the old system to coexist with he new one by just
  // unioning the data from both systems. This is done in
  // GetAccessibleNodeData().
  ui::AXNodeData data_;

  // If set to true, anything that is a descendant of this view will be hidden
  // from accessibility.
  // DEPRECATED: This is being replaced by is_leaf_.
  // TODO(javiercon): Remove this once OverrideIsLeaf is removed.
  bool overridden_is_leaf_ = false;

  // If set to true, anything that is a descendant of this view will be hidden
  // from accessibility by 'pruning' it from the tree, and setting `pruned_` to
  // true.
  bool is_leaf_ = false;

  bool pruned_ = false;

  // This is set to true when the view is explicitly marked as ignored by
  // `SetIsIgnored`. It is not the only condition that will cause a view to have
  // the ignored accessible state, as `pruned_` and `is_leaf_` can also cause
  // this. See `UpdateIgnoredState`.
  bool should_be_ignored_ = false;

  // Used by the Views system to help some assistive technologies, such as
  // screen readers, transition focus from one widget to another.
  base::WeakPtr<Widget> next_focus_ = nullptr;
  base::WeakPtr<Widget> previous_focus_ = nullptr;

  // This view's child tree id.
  std::optional<ui::AXTreeID> child_tree_id_;

  // Whether to move accessibility focus to an ancestor.
  bool propagate_focus_to_ancestor_ = false;

  // Whether we need to ensure an AtomicViewAXTreeManager is created for this
  // View.
  bool needs_ax_tree_manager_ = false;

  // Prevents accessibility events from being fired during initialization of
  // the owning View.
  bool pause_accessibility_events_ = false;

  bool ignore_missing_widget_for_testing_ = false;

  bool is_widget_closed_ = false;

  base::ScopedObservation<Widget, WidgetObserver> observation_{this};
};

class IgnoreMissingWidgetForTestingScopedSetter {
 public:
  explicit IgnoreMissingWidgetForTestingScopedSetter(
      ViewAccessibility& view_accessibility)
      : view_accessibility_(&view_accessibility) {
    view_accessibility_->SetIgnoreMissingWidgetForTesting(true);
  }
  ~IgnoreMissingWidgetForTestingScopedSetter() {
    view_accessibility_->SetIgnoreMissingWidgetForTesting(false);
  }

 private:
  raw_ptr<ViewAccessibility> view_accessibility_;
};

}  // namespace views

#endif  // UI_VIEWS_ACCESSIBILITY_VIEW_ACCESSIBILITY_H_
