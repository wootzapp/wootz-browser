// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_SCREEN_AI_SCREEN_AI_AX_TREE_SERIALIZER_H_
#define SERVICES_SCREEN_AI_SCREEN_AI_AX_TREE_SERIALIZER_H_

#include <memory>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "ui/accessibility/ax_node.h"
#include "ui/accessibility/ax_node_data.h"
#include "ui/accessibility/ax_serializable_tree.h"
#include "ui/accessibility/ax_tree_serializer.h"
#include "ui/accessibility/ax_tree_source.h"
#include "ui/accessibility/ax_tree_update.h"

namespace ui {

class AXTreeID;

}  // namespace ui

namespace screen_ai {

// Enables an accessibility tree containing the results of the Screen AI process
// to be efficiently serialized into an `AXTreeUpdate`. This efficiency is most
// evident when partial updates to the tree have to be made.
class ScreenAIAXTreeSerializer final {
 public:
  ScreenAIAXTreeSerializer(const ui::AXTreeID& parent_tree_id,
                           std::vector<ui::AXNodeData>&& nodes);
  ScreenAIAXTreeSerializer(const ScreenAIAXTreeSerializer&) = delete;
  ScreenAIAXTreeSerializer& operator=(const ScreenAIAXTreeSerializer&) = delete;
  ~ScreenAIAXTreeSerializer();

  ui::AXTreeUpdate Serialize() const;

  const ui::AXTree* tree_for_testing() const { return tree_.get(); }

 private:
  const std::unique_ptr<ui::AXSerializableTree> tree_;
  std::unique_ptr<
      ui::AXTreeSource<const ui::AXNode*, ui::AXTreeData*, ui::AXNodeData>>
      tree_source_;
  mutable std::unique_ptr<ui::AXTreeSerializer<const ui::AXNode*,
                                               std::vector<const ui::AXNode*>,
                                               ui::AXTreeUpdate*,
                                               ui::AXTreeData*,
                                               ui::AXNodeData>>
      serializer_;
};

}  // namespace screen_ai

#endif  // SERVICES_SCREEN_AI_SCREEN_AI_AX_TREE_SERIALIZER_H_
