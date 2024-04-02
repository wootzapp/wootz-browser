// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/accessibility/view_accessibility.h"

#include <utility>

#include "base/functional/callback.h"
#include "base/memory/ptr_util.h"
#include "base/ranges/algorithm.h"
#include "base/strings/utf_string_conversions.h"
#include "build/chromeos_buildflags.h"
#include "ui/accessibility/accessibility_features.h"
#include "ui/accessibility/ax_enums.mojom.h"
#include "ui/accessibility/platform/ax_platform_node.h"
#include "ui/accessibility/platform/ax_platform_node_delegate.h"
#include "ui/base/buildflags.h"
#include "ui/display/display.h"
#include "ui/display/screen.h"
#include "ui/views/accessibility/atomic_view_ax_tree_manager.h"
#include "ui/views/accessibility/views_ax_tree_manager.h"
#include "ui/views/accessibility/widget_ax_tree_id_map.h"
#include "ui/views/view.h"
#include "ui/views/widget/root_view.h"
#include "ui/views/widget/widget.h"

namespace views {

namespace {

bool IsValidRoleForViews(ax::mojom::Role role) {
  switch (role) {
    // These roles all have special meaning and shouldn't ever be
    // set on a View.
    case ax::mojom::Role::kDesktop:
    case ax::mojom::Role::kDocument:  // Used for ARIA role="document".
    case ax::mojom::Role::kIframe:
    case ax::mojom::Role::kIframePresentational:
    case ax::mojom::Role::kPdfRoot:
    case ax::mojom::Role::kPortal:
    case ax::mojom::Role::kRootWebArea:
    case ax::mojom::Role::kSvgRoot:
    case ax::mojom::Role::kUnknown:
      return false;

    default:
      return true;
  }
}

}  // namespace

#if !BUILDFLAG_INTERNAL_HAS_NATIVE_ACCESSIBILITY()
// static
std::unique_ptr<ViewAccessibility> ViewAccessibility::Create(View* view) {
  // Cannot use std::make_unique because constructor is protected.
  return base::WrapUnique(new ViewAccessibility(view));
}
#endif

ViewAccessibility::ViewAccessibility(View* view)
    : view_(view), focused_virtual_child_(nullptr) {
#if defined(USE_AURA) && !BUILDFLAG(IS_CHROMEOS_ASH)
  if (features::IsAccessibilityTreeForViewsEnabled()) {
    Widget* widget = view_->GetWidget();
    if (widget && widget->is_top_level() &&
        !WidgetAXTreeIDMap::GetInstance().HasWidget(widget)) {
      View* root_view = static_cast<View*>(widget->GetRootView());
      if (root_view && root_view == view) {
        ax_tree_manager_ = std::make_unique<views::ViewsAXTreeManager>(widget);
      }
    }
  }
#endif
}

ViewAccessibility::~ViewAccessibility() = default;

void ViewAccessibility::AddVirtualChildView(
    std::unique_ptr<AXVirtualView> virtual_view) {
  AddVirtualChildViewAt(std::move(virtual_view), virtual_children_.size());
}

void ViewAccessibility::AddVirtualChildViewAt(
    std::unique_ptr<AXVirtualView> virtual_view,
    size_t index) {
  DCHECK(virtual_view);
  DCHECK_LE(index, virtual_children_.size());

  if (virtual_view->parent_view() == this)
    return;
  DCHECK(!virtual_view->parent_view()) << "This |view| already has a View "
                                          "parent. Call RemoveVirtualChildView "
                                          "first.";
  DCHECK(!virtual_view->virtual_parent_view()) << "This |view| already has an "
                                                  "AXVirtualView parent. Call "
                                                  "RemoveChildView first.";
  virtual_view->set_parent_view(this);
  auto insert_iterator =
      virtual_children_.begin() + static_cast<ptrdiff_t>(index);
  virtual_children_.insert(insert_iterator, std::move(virtual_view));
}

std::unique_ptr<AXVirtualView> ViewAccessibility::RemoveVirtualChildView(
    AXVirtualView* virtual_view) {
  DCHECK(virtual_view);
  auto cur_index = GetIndexOf(virtual_view);
  if (!cur_index.has_value())
    return {};

  std::unique_ptr<AXVirtualView> child =
      std::move(virtual_children_[cur_index.value()]);
  virtual_children_.erase(virtual_children_.begin() +
                          static_cast<ptrdiff_t>(cur_index.value()));
  child->set_parent_view(nullptr);
  child->UnsetPopulateDataCallback();
  if (focused_virtual_child_ && child->Contains(focused_virtual_child_))
    OverrideFocus(nullptr);
  return child;
}

void ViewAccessibility::RemoveAllVirtualChildViews() {
  while (!virtual_children_.empty())
    RemoveVirtualChildView(virtual_children_.back().get());
}

bool ViewAccessibility::Contains(const AXVirtualView* virtual_view) const {
  DCHECK(virtual_view);
  for (const auto& virtual_child : virtual_children_) {
    // AXVirtualView::Contains() also checks if the provided virtual view is the
    // same as |this|.
    if (virtual_child->Contains(virtual_view))
      return true;
  }
  return false;
}

std::optional<size_t> ViewAccessibility::GetIndexOf(
    const AXVirtualView* virtual_view) const {
  DCHECK(virtual_view);
  const auto iter = base::ranges::find(virtual_children_, virtual_view,
                                       &std::unique_ptr<AXVirtualView>::get);
  return iter != virtual_children_.end()
             ? std::make_optional(
                   static_cast<size_t>(iter - virtual_children_.begin()))
             : std::nullopt;
}

void ViewAccessibility::GetAccessibleNodeData(ui::AXNodeData* data) const {
  data->id = GetUniqueId().Get();
  data->AddStringAttribute(ax::mojom::StringAttribute::kClassName,
                           view_->GetClassName());

  // Views may misbehave if their widget is closed; return an unknown role
  // rather than possibly crashing.
  const views::Widget* widget = view_->GetWidget();
  if (!widget || !widget->widget_delegate() || widget->IsClosed()) {
    data->role = ax::mojom::Role::kUnknown;
    data->SetRestriction(ax::mojom::Restriction::kDisabled);

    // TODO(accessibility): Returning early means that any custom data which
    // had been set via the Override functions is not included. Preserving
    // and exposing these properties might be worth doing, even in the case
    // of object destruction.

    // Ordinarily, a view cannot be focusable if its widget has already closed.
    // So, it would have been appropriate to set the focusable state to false in
    // this particular case. However, the `FocusManager` may sometimes try to
    // retrieve the focusable state of this view via
    // `View::IsAccessibilityFocusable()`, even after this view's widget has
    // been closed. Returning the wrong result might cause a crash, because the
    // focus manager might be expecting the result to be the same regardless of
    // the state of the view's widget.
    if (ViewAccessibility::IsAccessibilityFocusable()) {
      data->AddState(ax::mojom::State::kFocusable);
      // Set this node as intentionally nameless to avoid DCHECKs for a missing
      // name of a focusable.
      data->SetNameExplicitlyEmpty();
    }
    return;
  }

  view_->GetAccessibleNodeData(data);
  if (override_data_.role != ax::mojom::Role::kUnknown) {
    data->role = override_data_.role;
  }
  if (data->role == ax::mojom::Role::kAlertDialog) {
    // When an alert dialog is used, indicate this with xml-roles. This helps
    // JAWS understand that it's a dialog and not just an ordinary alert, even
    // though xml-roles is normally used to expose ARIA roles in web content.
    // Specifically, this enables the JAWS Insert+T read window title command.
    // Note: if an alert has focusable descendants such as buttons, it should
    // use kAlertDialog, not kAlert.
    data->AddStringAttribute(ax::mojom::StringAttribute::kRole, "alertdialog");
  }

  std::string name;
  if (override_data_.GetStringAttribute(ax::mojom::StringAttribute::kName,
                                        &name)) {
    if (!name.empty())
      data->SetNameChecked(name);
    else
      data->SetNameExplicitlyEmpty();
  }

  std::string description;
  if (override_data_.GetStringAttribute(
          ax::mojom::StringAttribute::kDescription, &description)) {
    if (!description.empty())
      data->SetDescription(description);
    else
      data->SetDescriptionExplicitlyEmpty();
  }

  if (override_data_.GetHasPopup() != ax::mojom::HasPopup::kFalse) {
    data->SetHasPopup(override_data_.GetHasPopup());
  }

  static constexpr ax::mojom::IntAttribute kOverridableIntAttributes[]{
      ax::mojom::IntAttribute::kDescriptionFrom,
      ax::mojom::IntAttribute::kNameFrom,
      ax::mojom::IntAttribute::kPosInSet,
      ax::mojom::IntAttribute::kSetSize,
  };
  for (auto attribute : kOverridableIntAttributes) {
    if (override_data_.HasIntAttribute(attribute)) {
      data->AddIntAttribute(attribute,
                            override_data_.GetIntAttribute(attribute));
    }
  }

  static constexpr ax::mojom::IntListAttribute kOverridableIntListAttributes[]{
      ax::mojom::IntListAttribute::kLabelledbyIds,
      ax::mojom::IntListAttribute::kDescribedbyIds,
      ax::mojom::IntListAttribute::kCharacterOffsets,
      ax::mojom::IntListAttribute::kWordStarts,
      ax::mojom::IntListAttribute::kWordEnds,
  };
  for (auto attribute : kOverridableIntListAttributes) {
    if (override_data_.HasIntListAttribute(attribute)) {
      data->AddIntListAttribute(attribute,
                                override_data_.GetIntListAttribute(attribute));
    }
  }

  if (!data->HasStringAttribute(ax::mojom::StringAttribute::kDescription)) {
    std::u16string tooltip = view_->GetTooltipText(gfx::Point());
    // Some screen readers announce the accessible description right after the
    // accessible name. Only use the tooltip as the accessible description if
    // it's different from the name, otherwise users might be puzzled as to why
    // their screen reader is announcing the same thing twice.
    if (!tooltip.empty() && tooltip != data->GetString16Attribute(
                                           ax::mojom::StringAttribute::kName)) {
      data->SetDescription(base::UTF16ToUTF8(tooltip));
    }
  }

  if (override_data_.HasBoolAttribute(ax::mojom::BoolAttribute::kSelected)) {
    data->AddBoolAttribute(
        ax::mojom::BoolAttribute::kSelected,
        override_data_.GetBoolAttribute(ax::mojom::BoolAttribute::kSelected));
  }

  data->relative_bounds.bounds = gfx::RectF(view_->GetBoundsInScreen());
  if (!override_data_.relative_bounds.bounds.IsEmpty()) {
    data->relative_bounds.bounds = override_data_.relative_bounds.bounds;
  }

  // We need to add the ignored state to all ignored Views, similar to how Blink
  // exposes ignored DOM nodes. Calling AXNodeData::IsIgnored() would also check
  // if the role is in the list of roles that are inherently ignored.
  // Furthermore, we add the ignored state if this View is a descendant of a
  // leaf View. We call this class's "IsChildOfLeaf" method instead of the one
  // in our platform specific subclass because subclasses determine if a node is
  // a leaf by (among other things) counting the number of unignored children,
  // which would create a circular definition of the ignored state.
  if (data->IsIgnored() || ViewAccessibility::IsChildOfLeaf()) {
    data->AddState(ax::mojom::State::kIgnored);
  }

  if (ViewAccessibility::IsAccessibilityFocusable())
    data->AddState(ax::mojom::State::kFocusable);

  if (!view_->GetVisible() && data->role != ax::mojom::Role::kAlert)
    data->AddState(ax::mojom::State::kInvisible);

  if (view_->context_menu_controller())
    data->AddAction(ax::mojom::Action::kShowContextMenu);

  DCHECK(!data->HasStringAttribute(ax::mojom::StringAttribute::kChildTreeId))
      << "Please annotate child tree ids using "
         "ViewAccessibility::OverrideChildTreeID.";
  if (child_tree_id_) {
    data->AddChildTreeId(child_tree_id_.value());

    if (widget && widget->GetNativeView() && display::Screen::GetScreen()) {
      const float scale_factor =
          display::Screen::GetScreen()
              ->GetDisplayNearestView(view_->GetWidget()->GetNativeView())
              .device_scale_factor();
      data->AddFloatAttribute(ax::mojom::FloatAttribute::kChildTreeScale,
                              scale_factor);
    }
  }

  // ***IMPORTANT***
  //
  // This step absolutely needs to be at the very end of the function in order
  // for us to catch all the attributes that have been set through a different
  // way than the ViewsAX AXNodeData push system. See `data_` for more info.

#if DCHECK_IS_ON()
  // This will help keep track of the attributes that have already
  // been migrated from the old system of computing AXNodeData for Views (pull),
  // to the new system (push). This will help ensure that new Views don't use
  // the old system for attributes that have already been migrated.
  // TODO(accessibility): Remove once migration is complete.
  views::ViewsAXCompletedAttributes::Validate(*data);
#endif

  views::ViewAccessibilityUtils::Merge(/*source*/ data_, /*destination*/ *data);

  // Nothing should be added beyond this point. Reach out to the Chromium
  // accessibility team in Slack, or to benjamin.beaudry@microsoft.com if you
  // absolutely need to add something past this point.
}

void ViewAccessibility::OverrideFocus(AXVirtualView* virtual_view) {
  DCHECK(!virtual_view || Contains(virtual_view))
      << "|virtual_view| must be nullptr or a descendant of this view.";
  focused_virtual_child_ = virtual_view;

  if (view_->HasFocus()) {
    if (focused_virtual_child_) {
      focused_virtual_child_->NotifyAccessibilityEvent(
          ax::mojom::Event::kFocus);
    } else {
      view_->NotifyAccessibilityEvent(ax::mojom::Event::kFocus, true);
    }
  }
}

bool ViewAccessibility::IsAccessibilityFocusable() const {
  // Descendants of leaf nodes should not be reported as focusable, because all
  // such descendants are not exposed to the accessibility APIs of any platform.
  // (See `AXNode::IsLeaf()` for more information.) We avoid calling
  // `IsChildOfLeaf()` for performance reasons, because `FocusManager` makes use
  // of this method, which means that it would be called frequently. However,
  // since all descendants of leaf nodes are ignored by default, and since our
  // testing framework enforces the condition that all ignored nodes should not
  // be focusable, if there is test coverage, such a situation will cause a test
  // failure.
  return view_->GetFocusBehavior() != View::FocusBehavior::NEVER &&
         GetIsEnabled() && view_->IsDrawn() &&
         !ViewAccessibility::GetIsIgnored();
}

bool ViewAccessibility::IsFocusedForTesting() const {
  return view_->HasFocus() && !focused_virtual_child_;
}

void ViewAccessibility::SetPopupFocusOverride() {
  NOTIMPLEMENTED();
}

void ViewAccessibility::EndPopupFocusOverride() {
  NOTIMPLEMENTED();
}

void ViewAccessibility::FireFocusAfterMenuClose() {
  view_->NotifyAccessibilityEvent(ax::mojom::Event::kFocusAfterMenuClose, true);
}

void ViewAccessibility::SetCharacterOffsets(
    const std::vector<int32_t>& offsets) {
  data_.AddIntListAttribute(ax::mojom::IntListAttribute::kCharacterOffsets,
                            offsets);
}

void ViewAccessibility::SetWordStarts(const std::vector<int32_t>& offsets) {
  data_.AddIntListAttribute(ax::mojom::IntListAttribute::kWordStarts, offsets);
}

void ViewAccessibility::SetWordEnds(const std::vector<int32_t>& offsets) {
  data_.AddIntListAttribute(ax::mojom::IntListAttribute::kWordEnds, offsets);
}

void ViewAccessibility::ClearTextOffsets() {
  data_.RemoveIntListAttribute(ax::mojom::IntListAttribute::kCharacterOffsets);
  data_.RemoveIntListAttribute(ax::mojom::IntListAttribute::kWordStarts);
  data_.RemoveIntListAttribute(ax::mojom::IntListAttribute::kWordEnds);
}

void ViewAccessibility::SetHasPopup(const ax::mojom::HasPopup has_popup) {
  data_.SetHasPopup(has_popup);
}

void ViewAccessibility::SetRole(const ax::mojom::Role role) {
  if (role == GetViewAccessibilityRole()) {
    return;
  }

  data_.role = role;
  if (role != ax::mojom::Role::kUnknown && role != ax::mojom::Role::kNone) {
    // TODO(javiercon): This is to temporarily work around the DCHECK
    // that wants to have a role to calculate a name-from: As of right now,
    // OverrideRole is getting migrated before OverrideName. This means that
    // when views call both in sequence and since OverrideRole is replaced by
    // this func data_ will have the role but override_data_ will have the name
    // (and not the role) so make sure to remove this once OverrideName is also
    // migrated.
    override_data_.role = role;
  }
}

void ViewAccessibility::SetRole(const ax::mojom::Role role,
                                const std::u16string& role_description) {
  if (role_description == data_.GetString16Attribute(
                              ax::mojom::StringAttribute::kRoleDescription)) {
    // No changes to the role description, update the role and return early.
    SetRole(role);
    return;
  }

  if (!role_description.empty()) {
    data_.AddStringAttribute(ax::mojom::StringAttribute::kRoleDescription,
                             base::UTF16ToUTF8(role_description));
  } else {
    data_.RemoveStringAttribute(ax::mojom::StringAttribute::kRoleDescription);
  }

  SetRole(role);
}

void ViewAccessibility::SetName(const std::string& name,
                                ax::mojom::NameFrom name_from) {
  DCHECK_NE(name_from, ax::mojom::NameFrom::kNone);
  // Ensure we have a current `name_from` value. For instance, the name might
  // still be an empty string, but a view is now indicating that this is by
  // design by setting `NameFrom::kAttributeExplicitlyEmpty`.
  DCHECK_EQ(name.empty(),
            name_from == ax::mojom::NameFrom::kAttributeExplicitlyEmpty)
      << "If the name is being removed to improve the user experience, "
         "|name_from| should be set to |kAttributeExplicitlyEmpty|.";
  data_.SetNameFrom(name_from);

  if (name == GetViewAccessibilityName()) {
    return;
  }

  if (name.empty()) {
    data_.RemoveStringAttribute(ax::mojom::StringAttribute::kName);
  } else {
    // |AXNodeData::SetName| expects a valid role. Some Views call |SetRole|
    // prior to setting the name. For those that don't, see if we can get the
    // default role from the View.
    // TODO(accessibility): This is a temporary workaround to avoid a DCHECK,
    // once we have migrated all Views to use the new setters and we always set
    // a role in the constructors for views, we can remove this.
    if (data_.role == ax::mojom::Role::kUnknown) {
      ui::AXNodeData data;
      view_->GetAccessibleNodeData(&data);
      if (data.role == ax::mojom::Role::kUnknown) {
        // TODO(accessibility): Remove this once the OverrideRole functions are
        // removed.
        data.role = override_data_.role;
      }
      data_.role = data.role;
    }

    data_.SetName(name);
  }

  view_->NotifyAccessibilityEvent(ax::mojom::Event::kTextChanged, true);
}

void ViewAccessibility::SetName(const std::u16string& name,
                                ax::mojom::NameFrom name_from) {
  std::string string_name = base::UTF16ToUTF8(name);
  SetName(string_name, name_from);
}

void ViewAccessibility::SetName(View& naming_view) {
  DCHECK_NE(view_, &naming_view);

  // TODO(javiercon): This is a temporary workaround to avoid the DCHECK below
  // in the scenario where the View's accessible name is being set through
  // either the GetAccessibleNodeData override pipeline or the SetAccessibleName
  // pipeline, which would make the call to `GetViewAccessibilityName` return an
  // empty string. (this is the case for `Label` view). Once these are migrated
  // we can remove this `if`, otherwise we must retrieve the name from there if
  // needed.
  if (naming_view.GetViewAccessibility().GetViewAccessibilityName().empty()) {
    ui::AXNodeData label_data;
    const_cast<View&>(naming_view).GetAccessibleNodeData(&label_data);
    const std::string& name =
        label_data.GetStringAttribute(ax::mojom::StringAttribute::kName);
    DCHECK(!name.empty());
    SetName(name, ax::mojom::NameFrom::kRelatedElement);
  } else {
    const std::string& name =
        naming_view.GetViewAccessibility().GetViewAccessibilityName();
    DCHECK(!name.empty());
    SetName(name, ax::mojom::NameFrom::kRelatedElement);
  }

  data_.AddIntListAttribute(
      ax::mojom::IntListAttribute::kLabelledbyIds,
      {naming_view.GetViewAccessibility().GetUniqueId().Get()});
}

const std::string& ViewAccessibility::GetViewAccessibilityName() const {
  return data_.GetStringAttribute(ax::mojom::StringAttribute::kName);
}

ax::mojom::Role ViewAccessibility::GetViewAccessibilityRole() const {
  return data_.role;
}

void ViewAccessibility::SetBounds(const gfx::RectF& bounds) {
  data_.relative_bounds.bounds = bounds;
}

void ViewAccessibility::SetPosInSet(int pos_in_set) {
  data_.AddIntAttribute(ax::mojom::IntAttribute::kPosInSet, pos_in_set);
}

void ViewAccessibility::SetSetSize(int set_size) {
  data_.AddIntAttribute(ax::mojom::IntAttribute::kSetSize, set_size);
}

void ViewAccessibility::ClearPosInSet() {
  data_.RemoveIntAttribute(ax::mojom::IntAttribute::kPosInSet);
}

void ViewAccessibility::ClearSetSize() {
  data_.RemoveIntAttribute(ax::mojom::IntAttribute::kSetSize);
}

void ViewAccessibility::SetIsEnabled(bool is_enabled) {
  if (is_enabled == GetIsEnabled()) {
    return;
  }

  if (!is_enabled) {
    data_.SetRestriction(ax::mojom::Restriction::kDisabled);
  } else if (data_.GetRestriction() == ax::mojom::Restriction::kDisabled) {
    // Take into account the possibility that the View is marked as readonly
    // but enabled. In other words, we can't just remove all restrictions,
    // unless the View is explicitly marked as disabled. Note that readonly is
    // another restriction state in addition to enabled and disabled, (see
    // `ax::mojom::Restriction`).
    data_.SetRestriction(ax::mojom::Restriction::kNone);
  }

  // TODO(crbug.com/1421682): We need a specific enabled-changed event for this.
  // Some platforms have specific state-changed events and this generic event
  // does not suggest what changed.
  view()->NotifyAccessibilityEvent(ax::mojom::Event::kStateChanged, true);
}

bool ViewAccessibility::GetIsEnabled() const {
  return data_.GetRestriction() != ax::mojom::Restriction::kDisabled;
}

void ViewAccessibility::SetDescription(
    const std::string& description,
    const ax::mojom::DescriptionFrom description_from) {
  DCHECK_EQ(
      description.empty(),
      description_from == ax::mojom::DescriptionFrom::kAttributeExplicitlyEmpty)
      << "If the description is being removed to improve the user experience, "
         "|description_from| should be set to |kAttributeExplicitlyEmpty|.";
  data_.SetDescriptionFrom(description_from);
  data_.SetDescription(description);
}

void ViewAccessibility::SetDescription(
    const std::u16string& description,
    const ax::mojom::DescriptionFrom description_from) {
  SetDescription(base::UTF16ToUTF8(description), description_from);
}

void ViewAccessibility::SetIsSelected(bool selected) {
  data_.AddBoolAttribute(ax::mojom::BoolAttribute::kSelected, selected);
}

void ViewAccessibility::SetIsIgnored(bool is_ignored) {
  if (is_ignored == data_.IsIgnored()) {
    return;
  }

  if (is_ignored) {
    data_.AddState(ax::mojom::State::kIgnored);
  } else {
    data_.RemoveState(ax::mojom::State::kIgnored);
  }

  view_->NotifyAccessibilityEvent(ax::mojom::Event::kTreeChanged, true);
}

bool ViewAccessibility::GetIsIgnored() const {
  return data_.HasState(ax::mojom::State::kIgnored);
}

void ViewAccessibility::OverrideRole(const ax::mojom::Role role) {
  DCHECK(IsValidRoleForViews(role)) << "Invalid role for Views.";
  override_data_.role = role;
}

void ViewAccessibility::OverrideName(const std::string& name,
                                     const ax::mojom::NameFrom name_from) {
  DCHECK_EQ(name.empty(),
            name_from == ax::mojom::NameFrom::kAttributeExplicitlyEmpty)
      << "If the name is being removed to improve the user experience, "
         "|name_from| should be set to |kAttributeExplicitlyEmpty|.";

  // |AXNodeData::SetName| expects a valid role. Some Views call |OverrideRole|
  // prior to overriding the name. For those that don't, see if we can get the
  // default role from the View.
  if (override_data_.role == ax::mojom::Role::kUnknown) {
    ui::AXNodeData data;
    view_->GetAccessibleNodeData(&data);
    override_data_.role = data.role;
  }

  override_data_.SetNameFrom(name_from);
  override_data_.SetNameChecked(name);
}

void ViewAccessibility::OverrideName(const std::u16string& name,
                                     const ax::mojom::NameFrom name_from) {
  OverrideName(base::UTF16ToUTF8(name), name_from);
}

void ViewAccessibility::OverrideNativeWindowTitle(const std::string& title) {
  NOTIMPLEMENTED() << "Only implemented on Mac for now.";
}

void ViewAccessibility::OverrideNativeWindowTitle(const std::u16string& title) {
  OverrideNativeWindowTitle(base::UTF16ToUTF8(title));
}

void ViewAccessibility::OverrideIsLeaf(bool value) {
  is_leaf_ = value;
}

bool ViewAccessibility::IsLeaf() const {
  return is_leaf_;
}

bool ViewAccessibility::IsChildOfLeaf() const {
  // Note to future developers: This method is called from
  // "GetAccessibleNodeData". We should avoid calling any methods in any of our
  // subclasses that might try and retrieve our AXNodeData, because this will
  // cause an infinite loop.
  // TODO(crbug.com/1100047): Make this method non-virtual and delete it from
  // all subclasses.
  if (const View* parent_view = view_->parent()) {
    const ViewAccessibility& view_accessibility =
        parent_view->GetViewAccessibility();
    if (view_accessibility.ViewAccessibility::IsLeaf())
      return true;
    return view_accessibility.ViewAccessibility::IsChildOfLeaf();
  }
  return false;
}

void ViewAccessibility::OverridePosInSet(int pos_in_set, int set_size) {
  override_data_.AddIntAttribute(ax::mojom::IntAttribute::kPosInSet,
                                 pos_in_set);
  override_data_.AddIntAttribute(ax::mojom::IntAttribute::kSetSize, set_size);
}

void ViewAccessibility::ClearPosInSetOverride() {
  override_data_.RemoveIntAttribute(ax::mojom::IntAttribute::kPosInSet);
  override_data_.RemoveIntAttribute(ax::mojom::IntAttribute::kSetSize);
}

void ViewAccessibility::SetNextFocus(Widget* widget) {
  if (widget)
    next_focus_ = widget->GetWeakPtr();
  else
    next_focus_ = nullptr;
}

void ViewAccessibility::SetPreviousFocus(Widget* widget) {
  if (widget)
    previous_focus_ = widget->GetWeakPtr();
  else
    previous_focus_ = nullptr;
}

Widget* ViewAccessibility::GetNextWindowFocus() const {
  return next_focus_.get();
}

Widget* ViewAccessibility::GetPreviousWindowFocus() const {
  return previous_focus_.get();
}

void ViewAccessibility::OverrideChildTreeID(ui::AXTreeID tree_id) {
  if (tree_id == ui::AXTreeIDUnknown())
    child_tree_id_ = std::nullopt;
  else
    child_tree_id_ = tree_id;
}

ui::AXTreeID ViewAccessibility::GetChildTreeID() const {
  return child_tree_id_ ? *child_tree_id_ : ui::AXTreeIDUnknown();
}

gfx::NativeViewAccessible ViewAccessibility::GetNativeObject() const {
  return nullptr;
}

void ViewAccessibility::NotifyAccessibilityEvent(ax::mojom::Event event_type) {
  Widget* const widget = view_->GetWidget();
  if (!widget || widget->IsClosed()) {
    return;
  }
  // Used for unit testing.
  if (accessibility_events_callback_)
    accessibility_events_callback_.Run(nullptr, event_type);
}

void ViewAccessibility::AnnounceAlert(const std::u16string& text) {
  if (auto* const widget = view_->GetWidget()) {
    if (auto* const root_view =
            static_cast<internal::RootView*>(widget->GetRootView())) {
      root_view->AnnounceTextAs(text,
                                ui::AXPlatformNode::AnnouncementType::kAlert);
    }
  }
}

void ViewAccessibility::AnnouncePolitely(const std::u16string& text) {
  if (auto* const widget = view_->GetWidget()) {
    if (auto* const root_view =
            static_cast<internal::RootView*>(widget->GetRootView())) {
      root_view->AnnounceTextAs(text,
                                ui::AXPlatformNode::AnnouncementType::kPolite);
    }
  }
}

void ViewAccessibility::AnnounceText(const std::u16string& text) {
  AnnounceAlert(text);
}

const ui::AXUniqueId& ViewAccessibility::GetUniqueId() const {
  return unique_id_;
}

ViewsAXTreeManager* ViewAccessibility::AXTreeManager() const {
  ViewsAXTreeManager* manager = nullptr;
#if defined(USE_AURA) && !BUILDFLAG(IS_CHROMEOS_ASH)
  Widget* widget = view_->GetWidget();

  // Don't return managers for closing Widgets.
  if (!widget || !widget->widget_delegate() || widget->IsClosed())
    return nullptr;

  manager = ax_tree_manager_.get();

  // ViewsAXTreeManagers are only created for top-level windows (Widgets). For
  // non top-level Views, look up the Widget's tree ID to retrieve the manager.
  if (!manager) {
    ui::AXTreeID tree_id =
        WidgetAXTreeIDMap::GetInstance().GetWidgetTreeID(widget);
    DCHECK_NE(tree_id, ui::AXTreeIDUnknown());
    manager = static_cast<views::ViewsAXTreeManager*>(
        ui::AXTreeManager::FromID(tree_id));
  }
#endif
  return manager;
}

AtomicViewAXTreeManager*
ViewAccessibility::GetAtomicViewAXTreeManagerForTesting() const {
  return nullptr;
}

gfx::NativeViewAccessible ViewAccessibility::GetFocusedDescendant() {
  if (focused_virtual_child_)
    return focused_virtual_child_->GetNativeObject();
  return view_->GetNativeViewAccessible();
}

const ViewAccessibility::AccessibilityEventsCallback&
ViewAccessibility::accessibility_events_callback() const {
  return accessibility_events_callback_;
}

void ViewAccessibility::set_accessibility_events_callback(
    ViewAccessibility::AccessibilityEventsCallback callback) {
  accessibility_events_callback_ = std::move(callback);
}

}  // namespace views
