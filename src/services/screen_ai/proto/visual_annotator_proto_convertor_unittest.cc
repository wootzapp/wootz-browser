// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/screen_ai/proto/visual_annotator_proto_convertor.h"

#include <string>

#include "services/screen_ai/proto/chrome_screen_ai.pb.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/accessibility/ax_tree_update.h"

namespace {

void InitLineBox(chrome_screen_ai::LineBox* line_box,
                 int32_t x,
                 int32_t y,
                 int32_t width,
                 int32_t height,
                 const char* text,
                 const char* language,
                 chrome_screen_ai::Direction direction,
                 int32_t block_id,
                 int32_t order_within_block,
                 float angle) {
  chrome_screen_ai::Rect* rect = line_box->mutable_bounding_box();
  rect->set_x(x);
  rect->set_y(y);
  rect->set_width(width);
  rect->set_height(height);
  rect->set_angle(angle);
  line_box->set_utf8_string(text);
  line_box->set_language(language);
  line_box->set_direction(direction);
  line_box->set_block_id(block_id);
  line_box->set_order_within_block(order_within_block);
}

void InitWordBox(chrome_screen_ai::WordBox* word_box,
                 int32_t x,
                 int32_t y,
                 int32_t width,
                 int32_t height,
                 const char* text,
                 const char* language,
                 chrome_screen_ai::Direction direction,
                 bool has_space_after,
                 int32_t background_rgb_value,
                 int32_t foreground_rgb_value,
                 float angle) {
  chrome_screen_ai::Rect* rect = word_box->mutable_bounding_box();
  rect->set_x(x);
  rect->set_y(y);
  rect->set_width(width);
  rect->set_height(height);
  rect->set_angle(angle);
  word_box->set_utf8_string(text);
  word_box->set_language(language);
  word_box->set_direction(direction);
  word_box->set_has_space_after(has_space_after);
  word_box->set_estimate_color_success(true);
  word_box->set_background_rgb_value(background_rgb_value);
  word_box->set_foreground_rgb_value(foreground_rgb_value);
}

}  // namespace

namespace screen_ai {

using ScreenAIVisualAnnotatorProtoConvertorTest = testing::Test;

TEST_F(ScreenAIVisualAnnotatorProtoConvertorTest,
       VisualAnnotationToAXTreeUpdate_LayoutExtractionResults) {
  chrome_screen_ai::VisualAnnotation annotation;
  gfx::Rect snapshot_bounds(800, 900);

  screen_ai::ResetNodeIDForTesting();

  {
    chrome_screen_ai::UIComponent* component_0 = annotation.add_ui_component();
    chrome_screen_ai::UIComponent::PredictedType* type_0 =
        component_0->mutable_predicted_type();
    type_0->set_enum_type(chrome_screen_ai::UIComponent::BUTTON);
    chrome_screen_ai::Rect* box_0 = component_0->mutable_bounding_box();
    box_0->set_x(0);
    box_0->set_y(1);
    box_0->set_width(2);
    box_0->set_height(3);
    box_0->set_angle(90.0f);

    chrome_screen_ai::UIComponent* component_1 = annotation.add_ui_component();
    chrome_screen_ai::UIComponent::PredictedType* type_1 =
        component_1->mutable_predicted_type();
    type_1->set_string_type("Signature");
    chrome_screen_ai::Rect* box_1 = component_1->mutable_bounding_box();
    // `x`, `y`, and `angle` should be defaulted to 0 since they are singular
    // proto3 fields, not proto2.
    box_1->set_width(5);
    box_1->set_height(5);
  }

  {
    const ui::AXTreeUpdate update =
        VisualAnnotationToAXTreeUpdate(annotation, snapshot_bounds);

    const std::string expected_update(
        "id=-2 dialog child_ids=-3,-4 (0, 0)-(800, 900)\n"
        "  id=-3 button offset_container_id=-2 (0, 1)-(2, 3)"
        " transform=[ 0 -1 0 0\n  1 0 0 0\n  0 0 1 0\n  0 0 0 1 ]\n"
        "\n"
        "  id=-4 genericContainer offset_container_id=-2 (0, 0)-(5, 5) "
        "role_description=Signature\n");
    EXPECT_EQ(expected_update, update.ToString());
  }
}

TEST_F(ScreenAIVisualAnnotatorProtoConvertorTest,
       VisualAnnotationToAXTreeUpdate_OcrResults) {
  chrome_screen_ai::VisualAnnotation annotation;
  gfx::Rect snapshot_bounds(800, 900);

  screen_ai::ResetNodeIDForTesting();

  {
    chrome_screen_ai::LineBox* line_0 = annotation.add_lines();

    InitWordBox(line_0->add_words(),
                /*x=*/100,
                /*y=*/100,
                /*width=*/250,
                /*height=*/20,
                /*text=*/"Hello",
                /*language=*/"en",
                /*direction=*/chrome_screen_ai::DIRECTION_RIGHT_TO_LEFT,
                /*has_space_after=*/true,
                /*background_rgb_value=*/0xffffff00,
                /*foreground_rgb_value=*/0x00000000,  // Black on white.
                /*angle=*/0);

    InitWordBox(line_0->add_words(),
                /*x=*/350,
                /*y=*/100,
                /*width=*/250,
                /*height=*/20,
                /*text=*/"world",
                /*language=*/"en",
                /*direction=*/chrome_screen_ai::DIRECTION_RIGHT_TO_LEFT,
                /*has_space_after=*/false,
                /*background_rgb_value=*/0xffffff00,
                /*foreground_rgb_value=*/0xff000000,  // Blue on white.
                /*angle=*/0);

    InitLineBox(line_0,
                /*x=*/100,
                /*y=*/100,
                /*width=*/500,
                /*height=*/20,
                /*text=*/"Hello world",
                /*language=*/"en",
                /*direction=*/chrome_screen_ai::DIRECTION_RIGHT_TO_LEFT,
                /*block_id=*/1,
                /*order_within_block=*/1,
                /*angle=*/0);
  }

  {
    const ui::AXTreeUpdate update =
        VisualAnnotationToAXTreeUpdate(annotation, snapshot_bounds);

    const std::string expected_update(
        "AXTreeUpdate: root id -2\n"
        "id=-2 region child_ids=-3,-5,-8 (0, 0)-(800, 900) "
        "is_page_breaking_object=true\n"
        "  id=-3 banner child_ids=-4 (0, 0)-(1, 1)\n"
        "    id=-4 staticText name=Start of extracted text (0, 0)-(1, 1)\n"
        "  id=-5 paragraph child_ids=-6 (100, 100)-(500, 20)\n"
        "    id=-6 staticText name=Hello world child_ids=-7 "
        "offset_container_id=-5 (100, 100)-(500, 20) text_direction=rtl "
        "language=en\n"
        "      id=-7 inlineTextBox name=Hello world (100, 100)-(500, 20) "
        "background_color=&FFFFFF00 color=&0 text_direction=rtl language=en "
        "word_starts=0,6 word_ends=6,11\n"
        "  id=-8 contentInfo child_ids=-9 (800, 900)-(1, 1)\n"
        "    id=-9 staticText name=End of extracted text (800, 900)-(1, 1)\n");
    EXPECT_EQ(expected_update, update.ToString());
  }
}

TEST_F(ScreenAIVisualAnnotatorProtoConvertorTest,
       ConvertProtoToVisualAnnotation_OcrResults) {
  chrome_screen_ai::VisualAnnotation annotation;

  {
    chrome_screen_ai::LineBox* line_0 = annotation.add_lines();

    InitWordBox(line_0->add_words(),
                /*x=*/100,
                /*y=*/100,
                /*width=*/250,
                /*height=*/20,
                /*text=*/"Hello",
                /*language=*/"en",
                /*direction=*/chrome_screen_ai::DIRECTION_RIGHT_TO_LEFT,
                /*has_space_after=*/true,
                /*background_rgb_value=*/0xffffff00,
                /*foreground_rgb_value=*/0x00000000,  // Black on white.
                /*angle=*/1);

    InitWordBox(line_0->add_words(),
                /*x=*/350,
                /*y=*/100,
                /*width=*/250,
                /*height=*/20,
                /*text=*/"world",
                /*language=*/"en",
                /*direction=*/chrome_screen_ai::DIRECTION_LEFT_TO_RIGHT,
                /*has_space_after=*/false,
                /*background_rgb_value=*/0xffffff00,
                /*foreground_rgb_value=*/0xff000000,  // Blue on white.
                /*angle=*/2);

    InitLineBox(line_0,
                /*x=*/100,
                /*y=*/100,
                /*width=*/500,
                /*height=*/20,
                /*text=*/"Hello world",
                /*language=*/"en",
                /*direction=*/chrome_screen_ai::DIRECTION_RIGHT_TO_LEFT,
                /*block_id=*/1,
                /*order_within_block=*/1,
                /*angle=*/1.5);

    chrome_screen_ai::Rect* line_baseline_box = line_0->mutable_baseline_box();
    line_baseline_box->set_x(110);
    line_baseline_box->set_y(110);
    line_baseline_box->set_width(510);
    line_baseline_box->set_height(30);
    line_baseline_box->set_angle(11.5);
  }

  {
    mojom::VisualAnnotationPtr annot =
        ConvertProtoToVisualAnnotation(annotation);
    EXPECT_EQ(annot->lines.size(), static_cast<unsigned long>(1));
    mojom::LineBoxPtr& line = annot->lines[0];
    EXPECT_EQ(line->bounding_box.x(), 100);
    EXPECT_EQ(line->bounding_box.y(), 100);
    EXPECT_EQ(line->bounding_box.width(), 500);
    EXPECT_EQ(line->bounding_box.height(), 20);
    EXPECT_EQ(line->bounding_box_angle, 1.5);
    EXPECT_EQ(line->baseline_box.x(), 110);
    EXPECT_EQ(line->baseline_box.y(), 110);
    EXPECT_EQ(line->baseline_box.width(), 510);
    EXPECT_EQ(line->baseline_box.height(), 30);
    EXPECT_EQ(line->baseline_box_angle, 11.5);
    EXPECT_EQ(line->text_line, "Hello world");
    EXPECT_EQ(line->block_id, 1);
    EXPECT_EQ(line->order_within_block, 1);
    EXPECT_EQ(line->words.size(), static_cast<unsigned long>(2));

    mojom::WordBoxPtr& word_0 = line->words[0];
    EXPECT_EQ(word_0->word, "Hello");
    EXPECT_EQ(word_0->dictionary_word, false);
    EXPECT_EQ(word_0->language, "en");
    EXPECT_EQ(word_0->has_space_after, true);
    EXPECT_EQ(word_0->bounding_box.x(), 100);
    EXPECT_EQ(word_0->bounding_box.y(), 100);
    EXPECT_EQ(word_0->bounding_box.width(), 250);
    EXPECT_EQ(word_0->bounding_box.height(), 20);
    EXPECT_EQ(word_0->bounding_box_angle, 1);
    EXPECT_EQ(word_0->direction, mojom::Direction::DIRECTION_RIGHT_TO_LEFT);

    mojom::WordBoxPtr& word_1 = line->words[1];
    EXPECT_EQ(word_1->word, "world");
    EXPECT_EQ(word_1->dictionary_word, false);
    EXPECT_EQ(word_1->language, "en");
    EXPECT_EQ(word_1->has_space_after, false);
    EXPECT_EQ(word_1->bounding_box.x(), 350);
    EXPECT_EQ(word_1->bounding_box.y(), 100);
    EXPECT_EQ(word_1->bounding_box.width(), 250);
    EXPECT_EQ(word_1->bounding_box.height(), 20);
    EXPECT_EQ(word_1->bounding_box_angle, 2);
    EXPECT_EQ(word_1->direction, mojom::Direction::DIRECTION_LEFT_TO_RIGHT);
  }
}

}  // namespace screen_ai
