// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/public/web/web_node.h"

#include <memory>


#include "testing/gtest/include/gtest/gtest.h"

#include "third_party/blink/public/web/web_element.h"
#include "third_party/blink/public/web/web_element_collection.h"
#include "third_party/blink/renderer/core/css/style_engine.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/dom/element.h"
#include "third_party/blink/renderer/core/testing/page_test_base.h"
#include "third_party/blink/renderer/core/testing/sim/sim_request.h"
#include "third_party/blink/renderer/core/testing/sim/sim_test.h"
#include "third_party/blink/renderer/platform/testing/unit_test_helpers.h"

namespace blink {

class WebNodeTest : public PageTestBase {
 protected:
  void SetInnerHTML(const String& html) {
    GetDocument().documentElement()->setInnerHTML(html);
  }

  WebNode Root() { return WebNode(GetDocument().documentElement()); }
};

TEST_F(WebNodeTest, QuerySelectorMatches) {
  SetInnerHTML("<div id=x><span class=a></span></div>");
  WebElement element = Root().QuerySelector(AtomicString(".a"));
  EXPECT_FALSE(element.IsNull());
  EXPECT_TRUE(element.HasHTMLTagName("span"));
}

TEST_F(WebNodeTest, QuerySelectorDoesNotMatch) {
  SetInnerHTML("<div id=x><span class=a></span></div>");
  WebElement element = Root().QuerySelector(AtomicString("section"));
  EXPECT_TRUE(element.IsNull());
}

TEST_F(WebNodeTest, QuerySelectorError) {
  SetInnerHTML("<div></div>");
  WebElement element = Root().QuerySelector(AtomicString("@invalid-selector"));
  EXPECT_TRUE(element.IsNull());
}

TEST_F(WebNodeTest, GetElementsByHTMLTagName) {
  SetInnerHTML(
      "<body><LABEL></LABEL><svg "
      "xmlns='http://www.w3.org/2000/svg'><label></label></svg></body>");
  // WebNode::getElementsByHTMLTagName returns only HTML elements.
  WebElementCollection collection = Root().GetElementsByHTMLTagName("label");
  EXPECT_EQ(1u, collection.length());
  EXPECT_TRUE(collection.FirstItem().HasHTMLTagName("label"));
  // The argument should be lower-case.
  collection = Root().GetElementsByHTMLTagName("LABEL");
  EXPECT_EQ(0u, collection.length());
}

class WebNodeSimTest : public SimTest {};

TEST_F(WebNodeSimTest, IsFocused) {
  SimRequest main_resource("https://example.com/test.html", "text/html");
  SimSubresourceRequest css_resource("https://example.com/style.css",
                                     "text/css");

  LoadURL("https://example.com/test.html");
  WebView().MainFrameViewWidget()->Resize(gfx::Size(800, 600));

  main_resource.Write(R"HTML(
    <!DOCTYPE html>
    <link rel=stylesheet href=style.css>
    <input id=focusable>
  )HTML");

  css_resource.Start();

  WebNode input_node(GetDocument().getElementById(AtomicString("focusable")));
  EXPECT_FALSE(input_node.IsFocusable());
  EXPECT_FALSE(GetDocument().HaveRenderBlockingStylesheetsLoaded());

  main_resource.Finish();
  css_resource.Complete("dummy {}");
  test::RunPendingTasks();
  EXPECT_TRUE(input_node.IsFocusable());
}

TEST_F(WebNodeTest, CannotFindTextNodesThatAreNotContainers) {
  SetInnerHTML(R"HTML(
    <div><br class="not-a-container"/> Hello world! </div>
  )HTML");
  WebElement element = Root().QuerySelector(AtomicString(".not-a-container"));

  EXPECT_FALSE(element.IsNull());
  EXPECT_TRUE(element.FindAllTextNodesMatchingRegex(".*").empty());
}

TEST_F(WebNodeTest, CanFindTextNodesThatAreContainers) {
  SetInnerHTML(R"HTML(
    <body class="container"><div id="id"> Hello world! </div></body>
  )HTML");
  WebElement element = Root().QuerySelector(AtomicString(".container"));

  EXPECT_FALSE(element.IsNull());

  std::vector<WebNode> nodes =
      element.FindAllTextNodesMatchingRegex("^ Hello world! $");
  ASSERT_EQ(nodes.size(), 1U);
  EXPECT_EQ(element.GetDocument().GetElementById("id").FirstChild(), nodes[0]);
}

TEST_F(WebNodeTest, CannotFindTextNodesIfMatcherRejectsIt) {
  SetInnerHTML(R"HTML(
    <body class="container"><div> Hello world! </div></body>
  )HTML");
  WebElement element = Root().QuerySelector(AtomicString(".container"));

  EXPECT_FALSE(element.IsNull());
  EXPECT_TRUE(element.FindAllTextNodesMatchingRegex("(?!.*)").empty());
}

TEST_F(WebNodeTest, CannotFindTextNodesInNonTextInputElement) {
  SetInnerHTML(R"HTML(
    <body class="container">
      <input type="url" readonly="" value=" HeLLo WoRLd! ">
    </body>
  )HTML");
  WebElement element = Root().QuerySelector(AtomicString(".container"));

  EXPECT_FALSE(element.IsNull());
  EXPECT_TRUE(
      element.FindAllTextNodesMatchingRegex("^ HeLLo WoRLd! $").empty());
}

TEST_F(WebNodeTest, CannotFindTextNodesInNonReadonlyTextInputElement) {
  SetInnerHTML(R"HTML(
    <body class="container">
      <input type="text" value=" HeLLo WoRLd! ">
    </body>
  )HTML");
  WebElement element = Root().QuerySelector(AtomicString(".container"));

  EXPECT_FALSE(element.IsNull());
  EXPECT_TRUE(
      element.FindAllTextNodesMatchingRegex("^ HeLLo WoRLd! $").empty());
}

// content-visibility:hidden elements should not be focusable and should not
// have style/layout run on them when checking focusability.
// content-visibility:auto elements should be focusable and should have
// style/layout run on them when checking focusability.
TEST_F(WebNodeTest, IsFocusableInDisplayLock) {
  SetInnerHTML(R"HTML(
    <div style="content-visibility: hidden">
      <input id=input1>
    </div>
    <div style="height: 8000px"></div>
    <div style="content-visibility: auto">
      <input id=input2>
    </div>
  )HTML");
  auto* input1 = GetDocument().getElementById(AtomicString("input1"));
  auto* input2 = GetDocument().getElementById(AtomicString("input2"));
  WebNode web_input1(input1);
  WebNode web_input2(input2);

  EXPECT_FALSE(web_input1.IsFocusable());
  EXPECT_TRUE(web_input2.IsFocusable());
  EXPECT_FALSE(input1->GetLayoutObject());
  EXPECT_TRUE(input2->GetLayoutObject());
}

}  // namespace blink
