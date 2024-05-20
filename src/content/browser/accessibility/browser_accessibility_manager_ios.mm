// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "content/browser/accessibility/browser_accessibility_manager_ios.h"

#import <UIKit/UIKit.h>

namespace content {

// static
BrowserAccessibilityManager* BrowserAccessibilityManager::Create(
    const ui::AXTreeUpdate& initial_tree,
    ui::AXPlatformTreeManagerDelegate* delegate) {
  return new BrowserAccessibilityManagerIOS(initial_tree, delegate);
}

// static
BrowserAccessibilityManager* BrowserAccessibilityManager::Create(
    ui::AXPlatformTreeManagerDelegate* delegate) {
  return new BrowserAccessibilityManagerIOS(
      BrowserAccessibilityManagerIOS::GetEmptyDocument(), delegate);
}

BrowserAccessibilityManagerIOS*
BrowserAccessibilityManager::ToBrowserAccessibilityManagerIOS() {
  return static_cast<BrowserAccessibilityManagerIOS*>(this);
}

BrowserAccessibilityManagerIOS::BrowserAccessibilityManagerIOS(
    const ui::AXTreeUpdate& initial_tree,
    ui::AXPlatformTreeManagerDelegate* delegate)
    : BrowserAccessibilityManager(delegate) {
  Initialize(initial_tree);
}

BrowserAccessibilityManagerIOS::~BrowserAccessibilityManagerIOS() = default;

// static
ui::AXTreeUpdate BrowserAccessibilityManagerIOS::GetEmptyDocument() {
  ui::AXNodeData empty_document;
  empty_document.id = ui::kInitialEmptyDocumentRootNodeID;
  empty_document.role = ax::mojom::Role::kRootWebArea;
  ui::AXTreeUpdate update;
  update.root_id = empty_document.id;
  update.nodes.push_back(empty_document);
  return update;
}

void BrowserAccessibilityManagerIOS::OnAtomicUpdateFinished(
    ui::AXTree* tree,
    bool root_changed,
    const std::vector<Change>& changes) {
  BrowserAccessibilityManager::OnAtomicUpdateFinished(tree, root_changed,
                                                      changes);
  BrowserAccessibility* root = GetBrowserAccessibilityRoot();
  if (!root) {
    return;
  }

  UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification,
                                  root->GetNativeViewAccessible());
}

gfx::Rect BrowserAccessibilityManagerIOS::GetViewBoundsInScreenCoordinates()
    const {
  ui::AXPlatformTreeManagerDelegate* delegate = GetDelegateFromRootManager();
  if (!delegate) {
    return gfx::Rect();
  }

  UIView* view = delegate->AccessibilityGetNativeViewAccessible();
  if (!view) {
    return gfx::Rect();
  }
  gfx::Rect bounds = delegate->AccessibilityGetViewBounds();
  bounds = gfx::Rect(
      UIAccessibilityConvertFrameToScreenCoordinates(bounds.ToCGRect(), view));
  return ScaleToEnclosingRect(bounds, device_scale_factor());
}

}  // namespace content
