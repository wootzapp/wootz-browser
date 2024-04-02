// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/accessibility/media_app/ax_media_app_untrusted_handler.h"

#include <stdint.h>

#include <memory>

#include "ash/constants/ash_features.h"
#include "base/test/scoped_feature_list.h"
#include "chrome/browser/accessibility/media_app/ax_media_app.h"
#include "chrome/browser/accessibility/media_app/ax_media_app_handler_factory.h"
#include "chrome/browser/accessibility/media_app/test/fake_ax_media_app.h"
#include "chrome/browser/accessibility/media_app/test/test_ax_media_app_untrusted_handler.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/test_web_ui.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "services/screen_ai/buildflags/buildflags.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/accessibility/ax_enums.mojom.h"
#include "ui/accessibility/ax_tree.h"
#include "ui/accessibility/ax_tree_data.h"
#include "ui/accessibility/ax_tree_id.h"
#include "ui/accessibility/ax_tree_manager.h"

#if BUILDFLAG(ENABLE_SCREEN_AI_SERVICE)
#include <vector>

#include "base/strings/escape.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/accessibility_notification_waiter.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "services/screen_ai/public/test/fake_screen_ai_annotator.h"
#include "ui/accessibility/ax_action_data.h"
#include "ui/accessibility/ax_event_generator.h"
#include "ui/accessibility/ax_node.h"
#include "ui/accessibility/ax_tree.h"
#include "ui/accessibility/ax_tree_data.h"
#include "ui/accessibility/ax_tree_manager.h"
#include "ui/accessibility/ax_tree_serializer.h"
#include "ui/accessibility/platform/inspect/ax_inspect.h"
#include "url/gurl.h"
#endif  // BUILDFLAG(ENABLE_SCREEN_AI_SERVICE)

namespace ash::test {

using ash::media_app_ui::mojom::PageMetadataPtr;

namespace {

// Page coordinates are expressed as a `gfx::RectF`, so float values should be
// used.

// Gap or padding between pages.
constexpr float kTestPageGap = 2.0f;
constexpr float kTestPageWidth = 3.0f;
constexpr float kTestPageHeight = 8.0f;

// Use letters to generate fake IDs for fake page metadata. If more than 26
// pages are needed, more characters can be added.
constexpr std::string_view kTestPageIds = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// Create fake page metadata with pages of the same size positioned
// (kTestPageWidth + kTestPageGap) unit spaced apart.
std::vector<PageMetadataPtr> CreateFakePageMetadata(const uint64_t num_pages) {
  EXPECT_LE(num_pages, kTestPageIds.size())
      << "Can't make more than " << kTestPageIds.size() << " pages.";
  std::vector<PageMetadataPtr> fake_page_metadata;
  for (uint64_t i = 0; i < num_pages; ++i) {
    PageMetadataPtr page = ash::media_app_ui::mojom::PageMetadata::New();
    page->id = std::format("Page{}", kTestPageIds[i]);
    page->rect =
        gfx::RectF(/*x=*/0.0f, /*y=*/kTestPageGap * i + kTestPageHeight * i,
                   kTestPageWidth, kTestPageHeight);
    fake_page_metadata.push_back(std::move(page));
  }
  return fake_page_metadata;
}

std::vector<PageMetadataPtr> ClonePageMetadataPtrs(
    const std::vector<PageMetadataPtr>& metadata) {
  std::vector<PageMetadataPtr> fake_page_metadata;
  for (const PageMetadataPtr& page : metadata) {
    PageMetadataPtr cloned_page = mojo::Clone(page);
    fake_page_metadata.push_back(std::move(cloned_page));
  }
  return fake_page_metadata;
}

class AXMediaAppUntrustedHandlerTest : public InProcessBrowserTest {
 public:
  AXMediaAppUntrustedHandlerTest()
      : feature_list_(ash::features::kMediaAppPdfA11yOcr) {}
  AXMediaAppUntrustedHandlerTest(
      const AXMediaAppUntrustedHandlerTest&) = delete;
  AXMediaAppUntrustedHandlerTest& operator=(
      const AXMediaAppUntrustedHandlerTest&) = delete;
  ~AXMediaAppUntrustedHandlerTest() override = default;

  void SetUpOnMainThread() override {
    InProcessBrowserTest::SetUpOnMainThread();
    ASSERT_NE(nullptr, AXMediaAppHandlerFactory::GetInstance());
    mojo::PendingRemote<ash::media_app_ui::mojom::OcrUntrustedPage> pageRemote;
    mojo::PendingReceiver<ash::media_app_ui::mojom::OcrUntrustedPage>
        pageReceiver = pageRemote.InitWithNewPipeAndPassReceiver();

    handler_ = std::make_unique<TestAXMediaAppUntrustedHandler>(
        *browser()->profile(), std::move(pageRemote));
    ASSERT_NE(nullptr, handler_.get());
    // TODO(b/309860428): Delete MediaApp interface - after we implement all
    // Mojo APIs, it should not be needed any more.
    handler_->SetMediaAppForTesting(&fake_media_app_);
#if BUILDFLAG(ENABLE_SCREEN_AI_SERVICE)
    handler_->SetIsOcrServiceEnabledForTesting();
    handler_->SetScreenAIAnnotatorForTesting(
        fake_annotator_.BindNewPipeAndPassRemote());
#endif  // BUILDFLAG(ENABLE_SCREEN_AI_SERVICE)
  }

  void TearDownOnMainThread() override {
    ASSERT_NE(nullptr, handler_.get());
    handler_.reset();
    InProcessBrowserTest::TearDownOnMainThread();
  }

 protected:
  void WaitForOcringPages(uint64_t number_of_pages) const {
    for (uint64_t i = 0; i < number_of_pages; ++i) {
      handler_->FlushForTesting();
    }
  }

  FakeAXMediaApp fake_media_app_;
  std::unique_ptr<TestAXMediaAppUntrustedHandler> handler_;
#if BUILDFLAG(ENABLE_SCREEN_AI_SERVICE)
  screen_ai::test::FakeScreenAIAnnotator fake_annotator_{
      /*create_empty_result=*/false};
#endif  // BUILDFLAG(ENABLE_SCREEN_AI_SERVICE)

 private:
  base::test::ScopedFeatureList feature_list_;
};

}  // namespace

#if BUILDFLAG(ENABLE_SCREEN_AI_SERVICE)
IN_PROC_BROWSER_TEST_F(AXMediaAppUntrustedHandlerTest, PageMetadataUpdated) {
  const size_t kTestNumPages = 3;
  std::vector<PageMetadataPtr> fake_metadata =
      CreateFakePageMetadata(kTestNumPages);
  handler_->PageMetadataUpdated(ClonePageMetadataPtrs(fake_metadata));
  WaitForOcringPages(kTestNumPages);

  ASSERT_EQ(kTestNumPages, fake_media_app_.PageIdsWithBitmap().size());
  // Make sure the OCR service went through all the pages provided in the
  // earlier call to `PageMetadataUpdated()`, since on first load all pages are
  // dirty.
  EXPECT_EQ("PageA", fake_media_app_.PageIdsWithBitmap()[0]);
  EXPECT_EQ("PageB", fake_media_app_.PageIdsWithBitmap()[1]);
  EXPECT_EQ("PageC", fake_media_app_.PageIdsWithBitmap()[2]);

  const std::map<const std::string, std::unique_ptr<ui::AXTreeManager>>& pages =
      handler_->GetPagesForTesting();
  ASSERT_EQ(3u, pages.size());
  for (auto const& [_, page] : pages) {
    ASSERT_NE(nullptr, page.get());
    ASSERT_NE(nullptr, page->ax_tree());
  }

  EXPECT_EQ(
      "AXTree has_parent_tree title=Screen AI\nid=-2 staticText "
      "name=Testing (0, 0)-(3, 8)\n",
      pages.at(fake_metadata[0]->id)->ax_tree()->ToString());
  EXPECT_EQ(
      "AXTree has_parent_tree title=Screen AI\nid=-3 staticText "
      "name=Testing (0, 10)-(3, 8)\n",
      pages.at(fake_metadata[1]->id)->ax_tree()->ToString());
  EXPECT_EQ(
      "AXTree has_parent_tree title=Screen AI\nid=-4 staticText "
      "name=Testing (0, 20)-(3, 8)\n",
      pages.at(fake_metadata[2]->id)->ax_tree()->ToString());
  EXPECT_EQ(
      "AXTree has_parent_tree title=PDF document\n"
      "id=1 pdfRoot FOCUSABLE name=PDF document containing 3 pages "
      "name_from=attribute clips_children child_ids=2,3,4 (0, 0)-(3, 28) "
      "text_align=left restriction=readonly scroll_x_min=0 scroll_y_min=0 "
      "scrollable=true is_line_breaking_object=true\n"
      "  id=2 region name=Page 1 name_from=attribute has_child_tree (0, 0)-"
      "(3, 8) restriction=readonly is_page_breaking_object=true\n"
      "  id=3 region name=Page 2 name_from=attribute has_child_tree (0, 10)-"
      "(3, 8) restriction=readonly is_page_breaking_object=true\n"
      "  id=4 region name=Page 3 name_from=attribute has_child_tree (0, 20)-"
      "(3, 8) restriction=readonly is_page_breaking_object=true\n",
      handler_->GetDocumentTreeToStringForTesting());

  // Relocate all the pages 3 units to the left and resize the second page. This
  // is similar to a scenario that might happen if the second page was rotated.
  fake_metadata[0]->rect =
      gfx::RectF(/*x=*/-3, /*y=*/0,
                 /*width=*/kTestPageWidth, /*height=*/kTestPageHeight);
  fake_metadata[1]->rect = gfx::RectF(
      /*x=*/-3, /*y=*/10, /*width=*/kTestPageHeight, /*height=*/kTestPageWidth);
  fake_metadata[2]->rect =
      gfx::RectF(/*x=*/-3, /*y=*/15,
                 /*width=*/kTestPageWidth, /*height=*/kTestPageHeight);
  handler_->PageMetadataUpdated(ClonePageMetadataPtrs(fake_metadata));

  // Subsequent calls to PageMetadataUpdated() should not cause any page to be
  // marked as dirty.
  ASSERT_EQ(3u, fake_media_app_.PageIdsWithBitmap().size());

  const std::map<const std::string, std::unique_ptr<ui::AXTreeManager>>&
      pages2 = handler_->GetPagesForTesting();
  ASSERT_EQ(3u, pages2.size());
  for (auto const& [_, page] : pages2) {
    ASSERT_NE(nullptr, page.get());
    ASSERT_NE(nullptr, page->ax_tree());
  }

  EXPECT_EQ(
      "AXTree has_parent_tree title=Screen AI\nid=-2 staticText "
      "name=Testing (-3, 0)-(3, 8)\n",
      pages2.at(fake_metadata[0]->id)->ax_tree()->ToString());
  EXPECT_EQ(
      "AXTree has_parent_tree title=Screen AI\nid=-3 staticText "
      "name=Testing (-3, 10)-(8, 3)\n",
      pages2.at(fake_metadata[1]->id)->ax_tree()->ToString());
  EXPECT_EQ(
      "AXTree has_parent_tree title=Screen AI\nid=-4 staticText "
      "name=Testing (-3, 15)-(3, 8)\n",
      pages2.at(fake_metadata[2]->id)->ax_tree()->ToString());
  EXPECT_EQ(
      "AXTree has_parent_tree title=PDF document\n"
      "id=1 pdfRoot FOCUSABLE name=PDF document containing 3 pages "
      "name_from=attribute clips_children child_ids=2,3,4 (-3, 0)-(8, 23) "
      "text_align=left restriction=readonly scroll_x_min=-3 scroll_y_min=0 "
      "scrollable=true is_line_breaking_object=true\n"
      "  id=2 region name=Page 1 name_from=attribute has_child_tree (-3, 0)-"
      "(3, 8) restriction=readonly is_page_breaking_object=true\n"
      "  id=3 region name=Page 2 name_from=attribute has_child_tree (-3, 10)-"
      "(8, 3) restriction=readonly is_page_breaking_object=true\n"
      "  id=4 region name=Page 3 name_from=attribute has_child_tree (-3, 15)-"
      "(3, 8) restriction=readonly is_page_breaking_object=true\n",
      handler_->GetDocumentTreeToStringForTesting());
}

IN_PROC_BROWSER_TEST_F(AXMediaAppUntrustedHandlerTest,
                       PageContentsUpdatedEdit) {
  const size_t kTestNumPages = 3;
  std::vector<PageMetadataPtr> fake_metadata =
      CreateFakePageMetadata(kTestNumPages);
  handler_->PageMetadataUpdated(ClonePageMetadataPtrs(fake_metadata));
  WaitForOcringPages(kTestNumPages);

  // All pages must have gone through OCR.
  ASSERT_EQ(kTestNumPages, fake_media_app_.PageIdsWithBitmap().size());
  EXPECT_EQ("PageA", fake_media_app_.PageIdsWithBitmap()[0]);
  EXPECT_EQ("PageB", fake_media_app_.PageIdsWithBitmap()[1]);
  EXPECT_EQ("PageC", fake_media_app_.PageIdsWithBitmap()[2]);

  // Mark the second page as dirty.
  handler_->PageContentsUpdated("PageB");
  WaitForOcringPages(1u);

  ASSERT_EQ(4u, fake_media_app_.PageIdsWithBitmap().size());
  EXPECT_EQ("PageA", fake_media_app_.PageIdsWithBitmap()[0]);
  EXPECT_EQ("PageB", fake_media_app_.PageIdsWithBitmap()[1]);
  EXPECT_EQ("PageC", fake_media_app_.PageIdsWithBitmap()[2]);
  EXPECT_EQ("PageB", fake_media_app_.PageIdsWithBitmap()[3]);
}

IN_PROC_BROWSER_TEST_F(AXMediaAppUntrustedHandlerTest,
                       PageMetadataUpdated_PageRotated) {
  constexpr size_t kTestNumPages = 4u;
  std::vector<PageMetadataPtr> fake_metadata =
      CreateFakePageMetadata(kTestNumPages);
  handler_->PageMetadataUpdated(ClonePageMetadataPtrs(fake_metadata));
  WaitForOcringPages(kTestNumPages);

  // All pages must have gone through OCR.
  ASSERT_EQ(kTestNumPages, fake_media_app_.PageIdsWithBitmap().size());
  EXPECT_EQ("PageA", fake_media_app_.PageIdsWithBitmap()[0]);
  EXPECT_EQ("PageB", fake_media_app_.PageIdsWithBitmap()[1]);
  EXPECT_EQ("PageC", fake_media_app_.PageIdsWithBitmap()[2]);
  EXPECT_EQ("PageD", fake_media_app_.PageIdsWithBitmap()[3]);

  // 'Rotate' the third page, moving the other pages to fit it.
  fake_metadata[2]->rect = gfx::RectF(
      /*x=*/-2.5,
      /*y=*/fake_metadata[1]->rect.y() + kTestPageHeight + kTestPageGap,
      /*width=*/kTestPageHeight, /*height=*/kTestPageWidth);
  fake_metadata[3]->rect = gfx::RectF(
      /*x=*/0, /*y=*/fake_metadata[2]->rect.y() + kTestPageWidth + kTestPageGap,
      /*width=*/kTestPageWidth, /*height=*/kTestPageHeight);
  handler_->PageMetadataUpdated(ClonePageMetadataPtrs(fake_metadata));
  handler_->PageContentsUpdated("PageC");
  WaitForOcringPages(1u);

  ASSERT_EQ(5u, fake_media_app_.PageIdsWithBitmap().size());
  EXPECT_EQ("PageA", fake_media_app_.PageIdsWithBitmap()[0]);
  EXPECT_EQ("PageB", fake_media_app_.PageIdsWithBitmap()[1]);
  EXPECT_EQ("PageC", fake_media_app_.PageIdsWithBitmap()[2]);
  EXPECT_EQ("PageD", fake_media_app_.PageIdsWithBitmap()[3]);
  EXPECT_EQ("PageC", fake_media_app_.PageIdsWithBitmap()[4]);

  EXPECT_EQ(
      "AXTree has_parent_tree title=PDF document\n"
      "id=1 pdfRoot FOCUSABLE name=PDF document containing 4 pages "
      "name_from=attribute clips_children child_ids=2,3,4,5 (-2.5, 0)-(8, 33) "
      "text_align=left restriction=readonly scroll_x_min=-2 scroll_y_min=0 "
      "scrollable=true is_line_breaking_object=true\n"
      "  id=2 region name=Page 1 name_from=attribute has_child_tree (0, 0)-"
      "(3, 8) restriction=readonly is_page_breaking_object=true\n"
      "  id=3 region name=Page 2 name_from=attribute has_child_tree (0, 10)-"
      "(3, 8) restriction=readonly is_page_breaking_object=true\n"
      "  id=4 region name=Page 3 name_from=attribute has_child_tree (-2.5, 20)-"
      "(8, 3) restriction=readonly is_page_breaking_object=true\n"
      "  id=5 region name=Page 4 name_from=attribute has_child_tree (0, 25)-"
      "(3, 8) restriction=readonly is_page_breaking_object=true\n",
      handler_->GetDocumentTreeToStringForTesting());
}

IN_PROC_BROWSER_TEST_F(AXMediaAppUntrustedHandlerTest,
                       PageMetadataUpdated_PagesReordered) {
  constexpr size_t kTestNumPages = 3u;
  std::vector<PageMetadataPtr> fake_metadata =
      CreateFakePageMetadata(kTestNumPages);
  handler_->PageMetadataUpdated(ClonePageMetadataPtrs(fake_metadata));
  WaitForOcringPages(kTestNumPages);

  // All pages must have gone through OCR.
  ASSERT_EQ(kTestNumPages, fake_media_app_.PageIdsWithBitmap().size());
  EXPECT_EQ("PageA", fake_media_app_.PageIdsWithBitmap()[0]);
  EXPECT_EQ("PageB", fake_media_app_.PageIdsWithBitmap()[1]);
  EXPECT_EQ("PageC", fake_media_app_.PageIdsWithBitmap()[2]);

  const std::map<const std::string, AXMediaAppPageMetadata>& page_metadata =
      handler_->GetPageMetadataForTesting();
  EXPECT_EQ(3u, page_metadata.size());
  EXPECT_EQ(1u, page_metadata.at("PageA").page_num);
  EXPECT_EQ(2u, page_metadata.at("PageB").page_num);
  EXPECT_EQ(3u, page_metadata.at("PageC").page_num);

  const std::map<const std::string, std::unique_ptr<ui::AXTreeManager>>& pages =
      handler_->GetPagesForTesting();
  EXPECT_EQ(3u, pages.size());
  const ui::AXTreeID& child_tree_id_page_a =
      pages.at("PageA")->GetParentTreeID();
  const ui::AXTreeID& child_tree_id_page_c =
      pages.at("PageC")->GetParentTreeID();

  // 'Reorder' the pages by swapping the first with the third page.
  std::swap(fake_metadata.at(0u), fake_metadata.at(2u));
  handler_->PageMetadataUpdated(ClonePageMetadataPtrs(fake_metadata));

  // No OCRing should have taken place, since the pages have only been
  // reordered, but not changed or rotated.
  ASSERT_EQ(kTestNumPages, fake_media_app_.PageIdsWithBitmap().size());
  EXPECT_EQ("PageA", fake_media_app_.PageIdsWithBitmap()[0]);
  EXPECT_EQ("PageB", fake_media_app_.PageIdsWithBitmap()[1]);
  EXPECT_EQ("PageC", fake_media_app_.PageIdsWithBitmap()[2]);

  EXPECT_EQ(3u, page_metadata.size());
  EXPECT_EQ(3u, page_metadata.at("PageA").page_num);
  EXPECT_EQ(2u, page_metadata.at("PageB").page_num);
  EXPECT_EQ(1u, page_metadata.at("PageC").page_num);

  EXPECT_EQ(3u, pages.size());
  const ui::AXTreeID& new_child_tree_id_page_a =
      pages.at("PageA")->GetParentTreeID();
  const ui::AXTreeID& new_child_tree_id_page_c =
      pages.at("PageC")->GetParentTreeID();
  EXPECT_EQ(child_tree_id_page_a, new_child_tree_id_page_c);
  EXPECT_EQ(child_tree_id_page_c, new_child_tree_id_page_a);

  // We'll also use the locations of pages one and three as a proxy to determine
  // if their were in fact skipped.
  EXPECT_EQ(
      "AXTree has_parent_tree title=PDF document\n"
      "id=1 pdfRoot FOCUSABLE name=PDF document containing 3 pages "
      "name_from=attribute clips_children child_ids=2,3,4 (0, 0)-(3, 28) "
      "text_align=left restriction=readonly scroll_x_min=0 scroll_y_min=0 "
      "scrollable=true is_line_breaking_object=true\n"
      "  id=2 region name=Page 1 name_from=attribute has_child_tree (0, "
      "20)-(3, 8) restriction=readonly is_page_breaking_object=true\n"
      "  id=3 region name=Page 2 name_from=attribute has_child_tree (0, "
      "10)-(3, 8) restriction=readonly is_page_breaking_object=true\n"
      "  id=4 region name=Page 3 name_from=attribute has_child_tree (0, 0)-(3, "
      "8) restriction=readonly is_page_breaking_object=true\n",
      handler_->GetDocumentTreeToStringForTesting());
}

IN_PROC_BROWSER_TEST_F(AXMediaAppUntrustedHandlerTest, StitchDocumentTree) {
  const char* html = R"HTML(
      <!DOCTYPE html>
      <html>
      <body>
        <canvas width="200" height="200">
          <p>Text that is replaced by child tree.</p>
        </canvas>
      </body>
      </html>
      )HTML";

  content::AccessibilityNotificationWaiter load_waiter(
      browser()->tab_strip_model()->GetActiveWebContents(), ui::kAXModeComplete,
      ax::mojom::Event::kLoadComplete);
  GURL html_data_url("data:text/html," +
                     base::EscapeQueryParamValue(html, /*use_plus=*/false));
  ASSERT_NE(nullptr, ui_test_utils::NavigateToURL(browser(), html_data_url));
  ASSERT_TRUE(load_waiter.WaitForNotification());
  EXPECT_EQ(
      "rootWebArea htmlTag='#document'\n"
      "++genericContainer htmlTag='html'\n"
      "++++genericContainer htmlTag='body'\n"
      "++++++canvas htmlTag='canvas'\n"
      "++++++++staticText name='<newline>          '\n"
      "++++++++staticText name='Text that is replaced by child tree.'\n"
      "++++++++staticText name='<newline>        '\n",
      browser()
          ->tab_strip_model()
          ->GetActiveWebContents()
          ->DumpAccessibilityTree(
              /*internal=*/true,
              /*property_filters=*/{
                  ui::AXPropertyFilter("htmlTag", ui::AXPropertyFilter::ALLOW),
                  ui::AXPropertyFilter("name", ui::AXPropertyFilter::ALLOW)}));

  content::AccessibilityNotificationWaiter child_tree_added_waiter(
      browser()->tab_strip_model()->GetActiveWebContents(), ui::kAXModeComplete,
      ui::AXEventGenerator::Event::CHILDREN_CHANGED);
  const size_t kTestNumPages = 1u;
  std::vector<PageMetadataPtr> fake_metadata =
      CreateFakePageMetadata(kTestNumPages);
  handler_->PageMetadataUpdated(ClonePageMetadataPtrs(fake_metadata));
  WaitForOcringPages(kTestNumPages);
  ASSERT_TRUE(child_tree_added_waiter.WaitForNotification());
  ASSERT_EQ(
      "rootWebArea htmlTag='#document'\n"
      "++genericContainer htmlTag='html'\n"
      "++++genericContainer htmlTag='body'\n"
      "++++++canvas htmlTag='canvas'\n",
      browser()
          ->tab_strip_model()
          ->GetActiveWebContents()
          ->DumpAccessibilityTree(
              /*internal=*/true,
              /*property_filters=*/{
                  ui::AXPropertyFilter("htmlTag", ui::AXPropertyFilter::ALLOW),
                  ui::AXPropertyFilter("name", ui::AXPropertyFilter::ALLOW)}));

  const ui::AXNode* canvas = browser()
                                 ->tab_strip_model()
                                 ->GetActiveWebContents()
                                 ->GetAccessibilityRootNode()
                                 ->GetFirstChild()
                                 ->GetFirstChild()
                                 ->GetFirstChild();
  ASSERT_NE(nullptr, canvas);
  EXPECT_NE(
      "", canvas->GetStringAttribute(ax::mojom::StringAttribute::kChildTreeId));
  const ui::AXNode* pdf_root =
      canvas->GetFirstUnignoredChildCrossingTreeBoundary();
  ASSERT_NE(nullptr, pdf_root);
  EXPECT_EQ(ax::mojom::Role::kPdfRoot, pdf_root->GetRole());
}

IN_PROC_BROWSER_TEST_F(AXMediaAppUntrustedHandlerTest,
                       SendAXTreeToAccessibilityService) {
  handler_->EnablePendingSerializedUpdatesForTesting();
  constexpr size_t kTestNumPages = 3u;
  std::vector<PageMetadataPtr> fake_metadata =
      CreateFakePageMetadata(kTestNumPages);
  handler_->PageMetadataUpdated(ClonePageMetadataPtrs(fake_metadata));
  WaitForOcringPages(kTestNumPages);

  // All pages must have gone through OCR.
  ASSERT_EQ(kTestNumPages, fake_media_app_.PageIdsWithBitmap().size());
  EXPECT_EQ("PageA", fake_media_app_.PageIdsWithBitmap()[0]);
  EXPECT_EQ("PageB", fake_media_app_.PageIdsWithBitmap()[1]);
  EXPECT_EQ("PageC", fake_media_app_.PageIdsWithBitmap()[2]);

  const std::vector<const ui::AXTreeUpdate>& pending_serialized_updates =
      handler_->GetPendingSerializedUpdatesForTesting();
  // Three updates, one for each page, plus one update for the document that
  // contains them.
  ASSERT_EQ(kTestNumPages + 1u, pending_serialized_updates.size());
  EXPECT_EQ(
      "AXTreeUpdate tree data:\n"
      "AXTreeUpdate: root id -2\n"
      "id=-2 staticText name=Testing (0, 0)-(3, 8)\n",
      pending_serialized_updates[0].ToString());
  EXPECT_EQ(
      "AXTreeUpdate tree data:\n"
      "AXTreeUpdate: root id -3\n"
      "id=-3 staticText name=Testing (0, 10)-(3, 8)\n",
      pending_serialized_updates[1].ToString());
  EXPECT_EQ(
      "AXTreeUpdate tree data:\n"
      "AXTreeUpdate: root id -4\n"
      "id=-4 staticText name=Testing (0, 20)-(3, 8)\n",
      pending_serialized_updates[2].ToString());
  EXPECT_EQ(
      "AXTreeUpdate tree data:\nAXTreeUpdate: root id 1\n"
      "id=1 pdfRoot FOCUSABLE name=PDF document containing 3 pages "
      "name_from=attribute clips_children child_ids=2,3,4 (0, 0)-(3, 28) "
      "text_align=left restriction=readonly scroll_x_min=0 scroll_y_min=0 "
      "scrollable=true is_line_breaking_object=true\n"
      "  id=2 region name=Page 1 name_from=attribute has_child_tree (0, 0)-(3, "
      "8) restriction=readonly is_page_breaking_object=true\n"
      "  id=3 region name=Page 2 name_from=attribute has_child_tree (0, "
      "10)-(3, 8) restriction=readonly is_page_breaking_object=true\n"
      "  id=4 region name=Page 3 name_from=attribute has_child_tree (0, "
      "20)-(3, 8) restriction=readonly is_page_breaking_object=true\n",
      pending_serialized_updates[3].ToString());

  // Rotate the second page. It should update the location of all pages.
  fake_metadata[1]->rect =
      gfx::RectF(/*x=*/0.0f, kTestPageHeight + kTestPageGap, kTestPageHeight,
                 kTestPageWidth);
  handler_->PageMetadataUpdated(ClonePageMetadataPtrs(fake_metadata));
  handler_->PageContentsUpdated("PageB");
  WaitForOcringPages(1u);

  // Only the second page must have gone through OCR, but all the pages must
  // have had their location updated.
  ASSERT_EQ(kTestNumPages + 1u, fake_media_app_.PageIdsWithBitmap().size());
  EXPECT_EQ("PageB", fake_media_app_.PageIdsWithBitmap().back());

  // For the location changes: Three updates for changing the location of three
  // pages, plus one for the document that contains them.
  //
  // For the rotated page: One update for deleting the rotated page, plus one
  // update for the document.
  ASSERT_EQ(kTestNumPages * 2u + 4u, pending_serialized_updates.size());
  EXPECT_EQ(
      "AXTreeUpdate: root id -2\n"
      "id=-2 staticText name=Testing (0, 0)-(3, 8)\n",
      pending_serialized_updates[4].ToString());
  EXPECT_EQ(
      "AXTreeUpdate: root id -3\n"
      "id=-3 staticText name=Testing (0, 10)-(8, 3)\n",
      pending_serialized_updates[5].ToString());
  EXPECT_EQ(
      "AXTreeUpdate: root id -4\n"
      "id=-4 staticText name=Testing (0, 20)-(3, 8)\n",
      pending_serialized_updates[6].ToString());
  EXPECT_EQ(
      "AXTreeUpdate: root id 1\n"
      "id=1 pdfRoot FOCUSABLE name=PDF document containing 3 pages "
      "name_from=attribute clips_children child_ids=2,3,4 (0, 0)-(8, 28) "
      "text_align=left restriction=readonly scroll_x_min=0 scroll_y_min=0 "
      "scrollable=true is_line_breaking_object=true\n",
      pending_serialized_updates[7].ToString());
  EXPECT_EQ(
      "AXTreeUpdate tree data:\n"
      "AXTreeUpdate: clear node -3\n"
      "AXTreeUpdate: root id -5\n"
      "id=-5 staticText name=Testing (0, 10)-(8, 3)\n",
      pending_serialized_updates[8].ToString());
  EXPECT_EQ(
      "AXTreeUpdate: root id 1\n"
      "id=1 pdfRoot FOCUSABLE name=PDF document containing 3 pages "
      "name_from=attribute clips_children child_ids=2,3,4 (0, 0)-(8, 28) "
      "text_align=left restriction=readonly scroll_x_min=0 scroll_y_min=0 "
      "scrollable=true is_line_breaking_object=true\n",
      pending_serialized_updates[9].ToString());
}

IN_PROC_BROWSER_TEST_F(AXMediaAppUntrustedHandlerTest, ScrollUpAndDown) {
  constexpr size_t kTestNumPages = 3u;
  std::vector<PageMetadataPtr> fake_metadata =
      CreateFakePageMetadata(kTestNumPages);
  handler_->PageMetadataUpdated(ClonePageMetadataPtrs(fake_metadata));
  WaitForOcringPages(kTestNumPages);

  // All pages must have gone through OCR.
  ASSERT_EQ(kTestNumPages, fake_media_app_.PageIdsWithBitmap().size());
  EXPECT_EQ("PageA", fake_media_app_.PageIdsWithBitmap()[0]);
  EXPECT_EQ("PageB", fake_media_app_.PageIdsWithBitmap()[1]);
  EXPECT_EQ("PageC", fake_media_app_.PageIdsWithBitmap()[2]);

  // View the second page by scrolling to it.
  handler_->ViewportUpdated(
      gfx::RectF(/*x=*/0.0f, /*y=*/kTestPageHeight + kTestPageGap,
                 kTestPageWidth, kTestPageHeight),
      /*scale_factor=*/1.0f);

  ui::AXActionData scroll_action_data;
  scroll_action_data.action = ax::mojom::Action::kScrollUp;
  scroll_action_data.target_tree_id = handler_->GetDocumentTreeIDForTesting();
  handler_->PerformAction(scroll_action_data);
  EXPECT_EQ(gfx::RectF(/*x=*/0.0f, /*y=*/kTestPageGap, kTestPageWidth,
                       kTestPageHeight),
            fake_media_app_.ViewportBox());

  // Scroll up again, which should only scroll to the top of the document, i.e.
  // viewport should not get a negative y value.
  handler_->PerformAction(scroll_action_data);
  EXPECT_EQ(
      gfx::RectF(/*x =*/0.0f, /*y=*/0.0f, kTestPageWidth, kTestPageHeight),
      fake_media_app_.ViewportBox());

  // View the second page again by scrolling to it.
  handler_->ViewportUpdated(
      gfx::RectF(/*x=*/0.0f, /*y=*/kTestPageHeight + kTestPageGap,
                 kTestPageWidth, kTestPageHeight),
      /*scale_factor=*/1.0f);

  scroll_action_data.action = ax::mojom::Action::kScrollDown;
  handler_->PerformAction(scroll_action_data);
  EXPECT_EQ(gfx::RectF(/*x=*/0.0f, /*y=*/kTestPageGap + kTestPageHeight * 2.0f,
                       kTestPageWidth, kTestPageHeight),
            fake_media_app_.ViewportBox());

  // Scroll down again, which should only scroll to the bottom of the document
  // but not further.
  handler_->PerformAction(scroll_action_data);
  EXPECT_EQ(
      gfx::RectF(/*x=*/0.0f, /*y=*/(kTestPageGap + kTestPageHeight) * 2.0f,
                 kTestPageWidth, kTestPageHeight),
      fake_media_app_.ViewportBox());
}

IN_PROC_BROWSER_TEST_F(AXMediaAppUntrustedHandlerTest, ScrollLeftAndRight) {
  constexpr float kTestViewportWidth = kTestPageWidth / 3.0f;
  constexpr float kTestViewportHeight = kTestPageHeight;
  constexpr size_t kTestNumPages = 3u;
  std::vector<PageMetadataPtr> fake_metadata =
      CreateFakePageMetadata(kTestNumPages);
  handler_->PageMetadataUpdated(ClonePageMetadataPtrs(fake_metadata));
  WaitForOcringPages(kTestNumPages);

  // All pages must have gone through OCR.
  ASSERT_EQ(kTestNumPages, fake_media_app_.PageIdsWithBitmap().size());
  EXPECT_EQ("PageA", fake_media_app_.PageIdsWithBitmap()[0]);
  EXPECT_EQ("PageB", fake_media_app_.PageIdsWithBitmap()[1]);
  EXPECT_EQ("PageC", fake_media_app_.PageIdsWithBitmap()[2]);

  // View the center part of the second page by scrolling to it.
  handler_->ViewportUpdated(gfx::RectF(/*x=*/kTestViewportWidth,
                                       /*y=*/kTestPageHeight + kTestPageGap,
                                       kTestViewportWidth, kTestViewportHeight),
                            /*scale_factor=*/1.0f);

  ui::AXActionData scroll_action_data;
  scroll_action_data.action = ax::mojom::Action::kScrollLeft;
  scroll_action_data.target_tree_id = handler_->GetDocumentTreeIDForTesting();
  handler_->PerformAction(scroll_action_data);
  EXPECT_EQ(gfx::RectF(/*x=*/0.0f, /*y=*/kTestPageHeight + kTestPageGap,
                       kTestViewportWidth, kTestViewportHeight),
            fake_media_app_.ViewportBox());

  // No scrolling should happen because we are already at the leftmost position
  // of the second page.
  handler_->PerformAction(scroll_action_data);
  EXPECT_EQ(gfx::RectF(/*x=*/0.0f, /*y=*/kTestPageHeight + kTestPageGap,
                       kTestViewportWidth, kTestViewportHeight),
            fake_media_app_.ViewportBox());

  // View the rightmost part of the second page again by scrolling to it.
  handler_->ViewportUpdated(gfx::RectF(/*x=*/kTestViewportWidth * 2.0f,
                                       /*y=*/kTestViewportHeight + kTestPageGap,
                                       kTestViewportWidth, kTestViewportHeight),
                            /*scale_factor=*/1.0f);

  scroll_action_data.action = ax::mojom::Action::kScrollRight;
  handler_->PerformAction(scroll_action_data);
  EXPECT_EQ(gfx::RectF(/*x=*/kTestPageWidth - kTestViewportWidth,
                       /*y=*/kTestViewportHeight + kTestPageGap,
                       kTestViewportWidth, kTestViewportHeight),
            fake_media_app_.ViewportBox());

  handler_->PerformAction(scroll_action_data);
  EXPECT_EQ(gfx::RectF(/*x=*/kTestPageWidth - 1.0f,
                       /*y=*/kTestViewportHeight + kTestPageGap,
                       kTestViewportWidth, kTestViewportHeight),
            fake_media_app_.ViewportBox());
}

IN_PROC_BROWSER_TEST_F(AXMediaAppUntrustedHandlerTest, ScrollToMakeVisible) {
  constexpr float kPageX = 0.0f;
  constexpr float kPageY = 0.0f;
  constexpr float kViewportWidth = 2.0f;
  constexpr float kViewportHeight = 4.0f;
  std::vector<PageMetadataPtr> fake_metadata;
  PageMetadataPtr fake_page = ash::media_app_ui::mojom::PageMetadata::New();
  fake_page->id = std::format("Page{}", kTestPageIds[0]);
  fake_page->rect =
      gfx::RectF(/*x=*/kPageX, /*y=*/kPageY, kTestPageWidth, kTestPageHeight);
  fake_metadata.push_back(std::move(fake_page));
  handler_->PageMetadataUpdated(ClonePageMetadataPtrs(fake_metadata));
  WaitForOcringPages(1u);

  // All pages must have gone through OCR.
  ASSERT_EQ(1u, fake_media_app_.PageIdsWithBitmap().size());
  EXPECT_EQ("PageA", fake_media_app_.PageIdsWithBitmap()[0]);

  ui::AXActionData scroll_action_data;
  scroll_action_data.action = ax::mojom::Action::kScrollToMakeVisible;
  scroll_action_data.target_tree_id =
      handler_->GetPagesForTesting().at(fake_metadata[0]->id)->GetTreeID();
  ASSERT_NE(nullptr,
            handler_->GetPagesForTesting().at(fake_metadata[0]->id)->GetRoot());
  scroll_action_data.target_node_id =
      handler_->GetPagesForTesting().at(fake_metadata[0]->id)->GetRoot()->id();

  // "Scroll to make visible", which should scroll forward.
  handler_->ViewportUpdated(
      gfx::RectF(/*x=*/0.0f, /*y=*/0.0f, kViewportWidth, kViewportHeight),
      /*scale_factor=*/1.0f);
  handler_->PerformAction(scroll_action_data);
  EXPECT_EQ(gfx::RectF(/*x=*/kPageX + kTestPageWidth - kViewportWidth,
                       /*y=*/kPageY + kTestPageHeight - kViewportHeight,
                       kViewportWidth, kViewportHeight),
            fake_media_app_.ViewportBox());
  handler_->ViewportUpdated(
      gfx::RectF(/*x=*/0.0f, /*y=*/kPageY, kViewportWidth, kViewportHeight),
      /*scale_factor=*/1.0f);
  handler_->PerformAction(scroll_action_data);
  EXPECT_EQ(gfx::RectF(/*x=*/kPageX + kTestPageWidth - kViewportWidth,
                       /*y=*/kPageY + kTestPageHeight - kViewportHeight,
                       kViewportWidth, kViewportHeight),
            fake_media_app_.ViewportBox());

  // "Scroll to make visible", which should scroll backward.
  handler_->ViewportUpdated(gfx::RectF(/*x=*/kPageX + kTestPageWidth - 1.0f,
                                       /*y=*/kPageY + kTestPageHeight - 1.0f,
                                       kViewportWidth, kViewportHeight),
                            /*scale_factor=*/1.0f);
  handler_->PerformAction(scroll_action_data);
  EXPECT_EQ(gfx::RectF(kPageX, kPageY, kViewportWidth, kViewportHeight),
            fake_media_app_.ViewportBox());
  handler_->ViewportUpdated(
      gfx::RectF(/*x=*/kPageX + kTestPageWidth, /*y=*/kPageY + kTestPageHeight,
                 kViewportWidth, kViewportHeight),
      /*scale_factor=*/1.0f);
  handler_->PerformAction(scroll_action_data);
  EXPECT_EQ(gfx::RectF(kPageX, kPageY, kViewportWidth, kViewportHeight),
            fake_media_app_.ViewportBox());

  // No scrolling should be needed because page can fit into viewport.
  handler_->ViewportUpdated(
      gfx::RectF(kPageX, kPageY, kTestPageWidth, kTestPageHeight),
      /*scale_factor=*/1.0f);
  handler_->PerformAction(scroll_action_data);
  EXPECT_EQ(gfx::RectF(kPageX, kPageY, kTestPageWidth, kTestPageHeight),
            fake_media_app_.ViewportBox());

  // Viewport can only display part of the page; so "scroll to make visible"
  // should only scroll to the top-left corner.
  handler_->ViewportUpdated(
      gfx::RectF(/*x=*/kPageX + kTestPageWidth - kViewportWidth,
                 /*y-*/ kPageY + kTestPageHeight - kViewportHeight,
                 kViewportWidth, kViewportHeight),
      /*scale_factor=*/1.0f);
  handler_->PerformAction(scroll_action_data);
  EXPECT_EQ(gfx::RectF(kPageX, kPageY, kViewportWidth, kViewportHeight),
            fake_media_app_.ViewportBox());
}
#endif  // BUILDFLAG(ENABLE_SCREEN_AI_SERVICE)

}  // namespace ash::test
