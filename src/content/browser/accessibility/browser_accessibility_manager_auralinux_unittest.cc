// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/accessibility/browser_accessibility_manager_auralinux.h"

#include <atk/atk.h>

#include "base/functional/bind.h"
#include "content/browser/accessibility/browser_accessibility.h"
#include "content/browser/accessibility/browser_accessibility_manager.h"
#include "content/public/test/browser_task_environment.h"
#include "content/public/test/scoped_accessibility_mode_override.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/accessibility/ax_enums.mojom.h"
#include "ui/accessibility/platform/ax_platform_node.h"
#include "ui/accessibility/platform/ax_platform_node_auralinux.h"
#include "ui/accessibility/platform/test_ax_platform_tree_manager_delegate.h"
#include "ui/base/glib/scoped_gsignal.h"

namespace content {

class BrowserAccessibilityManagerAuraLinuxTest : public ::testing::Test {
 public:
  BrowserAccessibilityManagerAuraLinuxTest() = default;

  BrowserAccessibilityManagerAuraLinuxTest(
      const BrowserAccessibilityManagerAuraLinuxTest&) = delete;
  BrowserAccessibilityManagerAuraLinuxTest& operator=(
      const BrowserAccessibilityManagerAuraLinuxTest&) = delete;

  ~BrowserAccessibilityManagerAuraLinuxTest() override = default;

 protected:
  std::unique_ptr<ui::TestAXPlatformTreeManagerDelegate>
      test_browser_accessibility_delegate_;

 private:
  void SetUp() override;
  std::unique_ptr<ScopedAccessibilityModeOverride> ax_mode_override_;

  // See crbug.com/1349124
  BrowserTaskEnvironment task_environment_;
};

void BrowserAccessibilityManagerAuraLinuxTest::SetUp() {
  test_browser_accessibility_delegate_ =
      std::make_unique<ui::TestAXPlatformTreeManagerDelegate>();
  ax_mode_override_ =
      std::make_unique<ScopedAccessibilityModeOverride>(ui::kAXModeComplete);
}

TEST_F(BrowserAccessibilityManagerAuraLinuxTest, TestEmitChildrenChanged) {
  ui::AXTreeUpdate initial_state;
  ui::AXTreeID tree_id = ui::AXTreeID::CreateNewAXTreeID();
  initial_state.tree_data.tree_id = tree_id;
  initial_state.has_tree_data = true;
  initial_state.tree_data.loaded = true;
  initial_state.root_id = 1;
  initial_state.nodes.resize(3);
  initial_state.nodes[0].id = 1;
  initial_state.nodes[0].child_ids = {2};
  initial_state.nodes[0].role = ax::mojom::Role::kRootWebArea;
  initial_state.nodes[1].id = 2;
  initial_state.nodes[1].child_ids = {3};
  initial_state.nodes[1].role = ax::mojom::Role::kStaticText;
  initial_state.nodes[2].id = 3;
  initial_state.nodes[2].role = ax::mojom::Role::kInlineTextBox;

  std::unique_ptr<BrowserAccessibilityManager> manager(
      BrowserAccessibilityManager::Create(
          initial_state, test_browser_accessibility_delegate_.get()));

  AtkObject* atk_root =
      manager->GetBrowserAccessibilityRoot()->GetNativeViewAccessible();
  ui::AXPlatformNodeAuraLinux* root_document_root_node =
      static_cast<ui::AXPlatformNodeAuraLinux*>(
          ui::AXPlatformNode::FromNativeViewAccessible(atk_root));
  {
    ScopedGSignal signal1(
        atk_root, "children-changed::remove",
        base::BindRepeating(
            +[](AtkObject* obj, int index, gpointer child, gpointer user_data) {
              EXPECT_EQ(ATK_ROLE_DOCUMENT_WEB, atk_object_get_role(obj));
            }));
    EXPECT_TRUE(signal1.Connected());
    ScopedGSignal signal2(
        atk_root, "children-changed::add",
        base::BindRepeating(
            +[](AtkObject* obj, int index, gpointer child, gpointer user_data) {
              EXPECT_EQ(ATK_ROLE_DOCUMENT_WEB, atk_object_get_role(obj));
            }));
    EXPECT_TRUE(signal2.Connected());
  }
  BrowserAccessibility* static_text_accessible =
      manager->GetBrowserAccessibilityRoot()->PlatformGetChild(0);
  // StaticText node triggers 'children-changed' event to the parent,
  // ATK_ROLE_DOCUMENT_WEB, when subtree is changed.
  BrowserAccessibilityManagerAuraLinux* aura_linux_manager =
      manager->ToBrowserAccessibilityManagerAuraLinux();
  aura_linux_manager->FireSubtreeCreatedEvent(static_text_accessible);
  aura_linux_manager->OnSubtreeWillBeDeleted(manager->ax_tree(),
                                             static_text_accessible->node());

  AtkObject* atk_object = root_document_root_node->ChildAtIndex(0);
  {
    ScopedGSignal signal3(
        atk_object, "children-changed::remove",
        base::BindRepeating(
            +[](AtkObject*, int index, gpointer child, gpointer user_data) {
              EXPECT_TRUE(false) << "should not be reached.";
            }));
    EXPECT_TRUE(signal3.Connected());
    ScopedGSignal signal4(
        atk_object, "children-changed::add",
        base::BindRepeating(
            +[](AtkObject* obj, int index, gpointer child, gpointer user_data) {
              EXPECT_TRUE(false) << "should not be reached.";
            }));
    EXPECT_TRUE(signal4.Connected());
  }

  // The static text is a platform leaf.
  ASSERT_EQ(0U, static_text_accessible->PlatformChildCount());
  ASSERT_EQ(1U, static_text_accessible->InternalChildCount());

  BrowserAccessibility* inline_text_accessible =
      static_text_accessible->InternalGetChild(0);
  // PlatformLeaf node such as InlineText should not trigger
  // 'children-changed' event to the parent when subtree is changed.
  aura_linux_manager->FireSubtreeCreatedEvent(inline_text_accessible);
  aura_linux_manager->OnSubtreeWillBeDeleted(manager->ax_tree(),
                                             inline_text_accessible->node());
}

}  // namespace content
