// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/layout/hit_test_location.h"
#include "third_party/blink/renderer/core/paint/paint_layer.h"
#include "third_party/blink/renderer/core/testing/core_unit_test_helper.h"

namespace blink {

using LayoutImageTest = RenderingTest;

TEST_F(LayoutImageTest, HitTestUnderTransform) {
  SetBodyInnerHTML(R"HTML(
    <div style='transform: translateX(50px)'>
      <img id=target style='width: 20px; height: 20px'/>
    </div>
  )HTML");

  const auto& target = *GetElementById("target");
  HitTestLocation location(PhysicalOffset(60, 10));
  HitTestResult result(
      HitTestRequest(HitTestRequest::kReadOnly | HitTestRequest::kActive |
                     HitTestRequest::kAllowChildFrameContent),
      location);
  GetLayoutView().HitTest(location, result);
  EXPECT_EQ(PhysicalOffset(60, 10), result.PointInInnerNodeFrame());
  EXPECT_EQ(target, result.InnerNode());
}

TEST_F(LayoutImageTest, NeedsVisualOverflowRecalc) {
  SetBodyInnerHTML(R"HTML(
    <div id="target" style="position: relative; width: 100px;">
      <img id="img" style="position: absolute; width: 100%;">
    </div>
  )HTML");
  UpdateAllLifecyclePhasesForTest();

  const auto* img_layer = GetLayoutBoxByElementId("img")->Layer();
  GetElementById("target")->SetInlineStyleProperty(CSSPropertyID::kWidth, "200px");
  EXPECT_FALSE(img_layer->NeedsVisualOverflowRecalc());

  GetDocument().View()->UpdateLifecycleToLayoutClean(
      DocumentUpdateReason::kTest);
  EXPECT_TRUE(img_layer->NeedsVisualOverflowRecalc());
}

}  // namespace blink
