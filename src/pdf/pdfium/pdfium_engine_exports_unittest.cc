// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <optional>
#include <string>
#include <utility>

#include "base/check_op.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "base/test/mock_callback.h"
#include "pdf/pdf.h"
#include "pdf/pdf_engine.h"
#include "services/screen_ai/buildflags/buildflags.h"
#include "services/screen_ai/public/mojom/screen_ai_service.mojom.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/pdfium/public/cpp/fpdf_scopers.h"
#include "third_party/pdfium/public/fpdf_text.h"
#include "third_party/pdfium/public/fpdfview.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/geometry/size_f.h"

namespace chrome_pdf {

namespace {

class ScopedLibraryInitializer {
 public:
  ScopedLibraryInitializer() {
    InitializeSDK(/*enable_v8=*/true, /*use_skia=*/false,
                  FontMappingMode::kNoMapping);
  }
  ~ScopedLibraryInitializer() { ShutdownSDK(); }
};

class PDFiumEngineExportsTest : public testing::Test {
 public:
  PDFiumEngineExportsTest() = default;
  PDFiumEngineExportsTest(const PDFiumEngineExportsTest&) = delete;
  PDFiumEngineExportsTest& operator=(const PDFiumEngineExportsTest&) = delete;
  ~PDFiumEngineExportsTest() override = default;

 protected:
  void SetUp() override {
    pdf_data_dir_ = base::PathService::CheckedGet(base::DIR_SRC_TEST_DATA_ROOT)
                        .Append(FILE_PATH_LITERAL("pdf"))
                        .Append(FILE_PATH_LITERAL("test"))
                        .Append(FILE_PATH_LITERAL("data"));
  }

  const base::FilePath& pdf_data_dir() const { return pdf_data_dir_; }

 private:
  base::FilePath pdf_data_dir_;
};

// Returns all characters in the page.
std::string GetText(base::span<const uint8_t> pdf, int page_index) {
  ScopedFPDFDocument document(
      FPDF_LoadMemDocument64(pdf.data(), pdf.size(), nullptr));
  CHECK(document);
  ScopedFPDFPage page(FPDF_LoadPage(document.get(), page_index));
  CHECK(page);
  ScopedFPDFTextPage text_page(FPDFText_LoadPage(page.get()));
  CHECK(text_page);
  int char_count = FPDFText_CountChars(text_page.get());
  CHECK_GE(char_count, 0);
  std::u16string text;
  text.reserve(char_count);
  for (int i = 0; i < char_count; ++i) {
    unsigned int char_code = FPDFText_GetUnicode(text_page.get(), i);
    text += static_cast<char16_t>(char_code);
  }
  return base::UTF16ToUTF8(text);
}

}  // namespace

TEST_F(PDFiumEngineExportsTest, GetPDFDocInfo) {
  base::FilePath pdf_path =
      pdf_data_dir().Append(FILE_PATH_LITERAL("hello_world2.pdf"));
  std::string pdf_data;
  ASSERT_TRUE(base::ReadFileToString(pdf_path, &pdf_data));

  auto pdf_span = base::as_bytes(base::make_span(pdf_data));
  ASSERT_TRUE(GetPDFDocInfo(pdf_span, nullptr, nullptr));

  int page_count;
  float max_page_width;
  ASSERT_TRUE(GetPDFDocInfo(pdf_span, &page_count, &max_page_width));
  EXPECT_EQ(2, page_count);
  EXPECT_DOUBLE_EQ(200.0, max_page_width);
}

TEST_F(PDFiumEngineExportsTest, GetPDFPageSizeByIndex) {
  // TODO(thestig): Use a better PDF for this test, as hello_world2.pdf's page
  // dimensions are uninteresting.
  base::FilePath pdf_path =
      pdf_data_dir().Append(FILE_PATH_LITERAL("hello_world2.pdf"));
  std::string pdf_data;
  ASSERT_TRUE(base::ReadFileToString(pdf_path, &pdf_data));

  auto pdf_span = base::as_bytes(base::make_span(pdf_data));
  int page_count;
  ASSERT_TRUE(GetPDFDocInfo(pdf_span, &page_count, nullptr));
  ASSERT_EQ(2, page_count);
  for (int page_index = 0; page_index < page_count; ++page_index) {
    std::optional<gfx::SizeF> page_size =
        GetPDFPageSizeByIndex(pdf_span, page_index);
    ASSERT_TRUE(page_size.has_value());
    EXPECT_EQ(gfx::SizeF(200, 200), page_size.value());
  }
}

TEST_F(PDFiumEngineExportsTest, ConvertPdfPagesToNupPdf) {
  base::FilePath pdf_path =
      pdf_data_dir().Append(FILE_PATH_LITERAL("rectangles.pdf"));
  std::string pdf_data;
  ASSERT_TRUE(base::ReadFileToString(pdf_path, &pdf_data));

  std::vector<base::span<const uint8_t>> pdf_buffers;
  std::vector<uint8_t> output_pdf_buffer = ConvertPdfPagesToNupPdf(
      pdf_buffers, 1, gfx::Size(612, 792), gfx::Rect(22, 20, 570, 750));
  EXPECT_TRUE(output_pdf_buffer.empty());

  pdf_buffers.push_back(base::as_bytes(base::make_span(pdf_data)));
  pdf_buffers.push_back(base::as_bytes(base::make_span(pdf_data)));
  output_pdf_buffer = ConvertPdfPagesToNupPdf(
      pdf_buffers, 2, gfx::Size(612, 792), gfx::Rect(22, 20, 0, 750));
  EXPECT_TRUE(output_pdf_buffer.empty());
  output_pdf_buffer = ConvertPdfPagesToNupPdf(
      pdf_buffers, 2, gfx::Size(612, 792), gfx::Rect(22, 20, 570, 0));
  EXPECT_TRUE(output_pdf_buffer.empty());
  output_pdf_buffer = ConvertPdfPagesToNupPdf(
      pdf_buffers, 2, gfx::Size(612, 792), gfx::Rect(300, 20, 570, 750));
  EXPECT_TRUE(output_pdf_buffer.empty());
  output_pdf_buffer = ConvertPdfPagesToNupPdf(
      pdf_buffers, 2, gfx::Size(612, 792), gfx::Rect(22, 400, 570, 750));
  EXPECT_TRUE(output_pdf_buffer.empty());
  output_pdf_buffer = ConvertPdfPagesToNupPdf(
      pdf_buffers, 2, gfx::Size(612, 792), gfx::Rect(22, 20, 570, 750));
  ASSERT_GT(output_pdf_buffer.size(), 0U);

  base::span<const uint8_t> output_pdf_span =
      base::make_span(output_pdf_buffer);
  int page_count;
  ASSERT_TRUE(GetPDFDocInfo(output_pdf_span, &page_count, nullptr));
  ASSERT_EQ(1, page_count);

  std::optional<gfx::SizeF> page_size =
      GetPDFPageSizeByIndex(output_pdf_span, 0);
  ASSERT_TRUE(page_size.has_value());
  EXPECT_EQ(gfx::SizeF(792, 612), page_size.value());
}

TEST_F(PDFiumEngineExportsTest, ConvertPdfDocumentToNupPdf) {
  base::FilePath pdf_path =
      pdf_data_dir().Append(FILE_PATH_LITERAL("rectangles_multi_pages.pdf"));
  std::string pdf_data;
  ASSERT_TRUE(base::ReadFileToString(pdf_path, &pdf_data));

  base::span<const uint8_t> pdf_buffer;
  std::vector<uint8_t> output_pdf_buffer = ConvertPdfDocumentToNupPdf(
      pdf_buffer, 1, gfx::Size(612, 792), gfx::Rect(32, 20, 570, 750));
  EXPECT_TRUE(output_pdf_buffer.empty());

  pdf_buffer = base::as_bytes(base::make_span(pdf_data));
  output_pdf_buffer = ConvertPdfDocumentToNupPdf(
      pdf_buffer, 4, gfx::Size(612, 792), gfx::Rect(22, 20, 570, 750));
  ASSERT_GT(output_pdf_buffer.size(), 0U);

  base::span<const uint8_t> output_pdf_span =
      base::make_span(output_pdf_buffer);
  int page_count;
  ASSERT_TRUE(GetPDFDocInfo(output_pdf_span, &page_count, nullptr));
  ASSERT_EQ(2, page_count);
  for (int page_index = 0; page_index < page_count; ++page_index) {
    std::optional<gfx::SizeF> page_size =
        GetPDFPageSizeByIndex(output_pdf_span, page_index);
    ASSERT_TRUE(page_size.has_value());
    EXPECT_EQ(gfx::SizeF(612, 792), page_size.value());
  }
}

#if BUILDFLAG(ENABLE_SCREEN_AI_SERVICE)
constexpr char kExpectedText[] = "Hello World! 你好！🙂";

TEST_F(PDFiumEngineExportsTest, Searchify) {
  base::FilePath pdf_path =
      pdf_data_dir().Append(FILE_PATH_LITERAL("image_alt_text.pdf"));
  std::optional<std::vector<uint8_t>> pdf_buffer =
      base::ReadFileToBytes(pdf_path);
  ASSERT_TRUE(pdf_buffer.has_value());

  base::MockCallback<base::RepeatingCallback<
      screen_ai::mojom::VisualAnnotationPtr(const SkBitmap&)>>
      perform_ocr_callback;
  EXPECT_CALL(perform_ocr_callback, Run)
      .Times(3)
      .WillRepeatedly([](const SkBitmap& bitmap) {
        CHECK(!bitmap.empty());
        auto annotation = screen_ai::mojom::VisualAnnotation::New();
        auto line_box = screen_ai::mojom::LineBox::New();
        line_box->baseline_box = gfx::Rect(0, 0, 100, 100);
        line_box->baseline_box_angle = 0;
        line_box->bounding_box = gfx::Rect(0, 0, 100, 100);
        line_box->bounding_box_angle = 0;
        auto word_box = screen_ai::mojom::WordBox::New();
        word_box->word = kExpectedText;
        word_box->bounding_box = gfx::Rect(0, 0, 100, 100);
        word_box->bounding_box_angle = 0;
        line_box->words.push_back(std::move(word_box));
        annotation->lines.push_back(std::move(line_box));
        return annotation;
      });
  {
    ScopedLibraryInitializer initializer;
    EXPECT_THAT(GetText(*pdf_buffer, 0),
                testing::Not(testing::HasSubstr(kExpectedText)));
  }
  std::vector<uint8_t> output_pdf_buffer =
      Searchify(*pdf_buffer, perform_ocr_callback.Get());
  ASSERT_FALSE(output_pdf_buffer.empty());
  {
    ScopedLibraryInitializer initializer;
    EXPECT_THAT(GetText(output_pdf_buffer, 0),
                testing::HasSubstr(kExpectedText));
  }
}

TEST_F(PDFiumEngineExportsTest, SearchifyBroken) {
  base::FilePath pdf_path =
      pdf_data_dir().Append(FILE_PATH_LITERAL("image_alt_text.pdf"));
  std::optional<std::vector<uint8_t>> pdf_buffer =
      base::ReadFileToBytes(pdf_path);
  ASSERT_TRUE(pdf_buffer.has_value());

  auto broken_perform_ocr_callback =
      base::BindRepeating([](const SkBitmap& bitmap) {
        auto annotation = screen_ai::mojom::VisualAnnotationPtr();
        // Simulate the scenarios that fail to set the value of `annotation`.
        CHECK(!annotation);
        return annotation;
      });
  std::vector<uint8_t> output_pdf_buffer =
      Searchify(*pdf_buffer, std::move(broken_perform_ocr_callback));
  EXPECT_TRUE(output_pdf_buffer.empty());
}
#endif  // BUILDFLAG(ENABLE_SCREEN_AI_SERVICE)

}  // namespace chrome_pdf
