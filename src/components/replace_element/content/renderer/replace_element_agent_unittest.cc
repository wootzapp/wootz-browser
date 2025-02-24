// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/replace_element/content/renderer/replace_element_agent.h"

#include "content/public/renderer/render_frame.h"
#include "content/public/test/render_view_test.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/platform/web_string.h"
#include "third_party/blink/public/web/web_document.h"
#include "third_party/blink/public/web/web_element.h"
#include "third_party/blink/public/web/web_local_frame.h"

namespace replace_element {

class ReplaceElementAgentTest : public content::RenderViewTest {
 protected:
  void SetUp() override {
    RenderViewTest::SetUp();
    replace_element_agent_ =
        std::make_unique<ReplaceElementAgent>(GetMainRenderFrame(), &registry_);
  }

  void TearDown() override {
    replace_element_agent_.reset();
    RenderViewTest::TearDown();
  }

  content::RenderFrame* GetMainRenderFrame() {
    return static_cast<content::RenderFrame*>(
        RenderViewTest::GetMainRenderFrame());
  }

  std::unique_ptr<ReplaceElementAgent> replace_element_agent_;
  blink::AssociatedInterfaceRegistry registry_;
};

TEST_F(ReplaceElementAgentTest, ReplaceElementURLTest) {
  // Setup initial HTML
  LoadHTML(R"(
    <html>
    <body>
        <h1>Welcome to My Testing HTML Page</h1>
        <p>This url will be replaced: <a href="https://example.com">example.com</a></p>
    </body>
    </html>
  )");

  // Create JSON string representing new element
  std::string json_string = R"({
  "element": "a",
  "attributes": {
    "href": "https://new_replace_url.com",
    "target": "_blank",
    "style": {
      "color": "#0066cc",
      "textDecoration": "none",
      "fontWeight": "bold"
    }
  },
  "content": "This is New URL which replace the previous one"
})";

  // Call ReplaceElement
  replace_element_agent_->ReplaceElement("body > p > a", json_string);

  // Get the updated element
  blink::WebElement element = GetMainFrame()->GetDocument().QuerySelector(
      blink::WebString::FromUTF8("body > p > a"));

  // Verify the element was replaced
  EXPECT_FALSE(element.IsNull());
  EXPECT_EQ(element.TextContent().Utf8(),
            "This is New URL which replace the previous one");
}

TEST_F(ReplaceElementAgentTest, ReplaceElementDivTest) {
  // Setup initial HTML
  LoadHTML(R"(
    <html>
    <body>
        <h1>Welcome to My Testing HTML Page</h1>
        <div class="container">
            <p>This "div" will be replaced:
        </div>
    </body>
    </html>
  )");

  // Create JSON string with nested elements
  std::string json_string = R"({
  "element": "div",
  "attributes": {
    "id": "main-container",
    "class": "container flex-box",
    "style": {
      "backgroundColor": "#ffffff",
      "padding": "20px"
    }
  },
  "children": [
    {
      "element": "div",
      "content": "This is a replaced div element"
    }
  ]
})";

  // Call ReplaceElement
  replace_element_agent_->ReplaceElement("body > div > div", json_string);

  // Get the updated element
  blink::WebElement element = GetMainFrame()->GetDocument().QuerySelector(
      blink::WebString::FromUTF8("body > div > div"));

  // Verify the element was replaced and has children
  EXPECT_FALSE(element.IsNull());
  EXPECT_EQ(element.TextContent().Utf8(), "This is a replaced div element");
}
TEST_F(ReplaceElementAgentTest, ReplaceElementImageTest) {
  // Setup initial HTML
  LoadHTML(R"(
    <html>
    <body>
        <h1>Welcome to My Testing HTML Page</h1>
        <div>
            <p> The below image will be replace: </p>
        </div>
        <img src="https://media.phillyvoice.com/media/images/270520_Community_Hulu_Netflix.2e16d0ba.fill-735x490.png"
            alt="Placeholder Image">
    </body>
    </html>
  )");

  // Create JSON string with nested elements
  std::string json_string = R"({
  "element": "div",
  "attributes": {
    "id": "main-container",
    "class": "container flex-box",
    "style": {
      "backgroundColor": "#ffffff",
      "padding": "20px"
    }
  },
  "children": [
    {
      "element": "img",
      "attributes": {
        "src": "https://www.wootzapp.com/image/logo2.png",
        "alt": "Description",
        "width": "300",
        "height": "200",
        "class": "responsive-img",
        "style": {
          "objectFit": "cover",
          "borderRadius": "8px"
        }
      }
    },
    {
      "element": "div",
      "content": "Wootz Browser Logo"
    }
  ]
})";

  // Call ReplaceElement
  replace_element_agent_->ReplaceElement("body > div > div", json_string);

  // Get the updated element
  blink::WebElement element = GetMainFrame()->GetDocument().QuerySelector(
      blink::WebString::FromUTF8("body > div > div"));

  // Verify the element was replaced and has children
  EXPECT_FALSE(element.IsNull());
  EXPECT_EQ(element.TextContent().Utf8(), "Wootz Browser Logo");
}

TEST_F(ReplaceElementAgentTest, ReplaceElementNotFound) {
  // Setup initial HTML
  LoadHTML("<html><body></body></html>");

  // Try to replace non-existent element
  replace_element_agent_->ReplaceElement("#non-existent", "{}");

  // Verify document wasn't modified
  EXPECT_EQ(GetMainFrame()->GetDocument().Body().TextContent().Utf8(), "");
}

}  // namespace replace_element
