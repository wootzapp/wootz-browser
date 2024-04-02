// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ENTERPRISE_WATERMARK_WATERMARK_VIEW_H_
#define CHROME_BROWSER_ENTERPRISE_WATERMARK_WATERMARK_VIEW_H_

#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/view.h"

namespace gfx {
class Canvas;
class RenderText;
}

namespace enterprise_watermark {

// WatermarkView represents a view that can superimpose a watermark on top of
// other views. The view should be appropriately sized using its parent's layout
// manager.
class WatermarkView : public views::View {
  METADATA_HEADER(WatermarkView, views::View)

 public:
  WatermarkView();
  explicit WatermarkView(std::string text);
  ~WatermarkView() override;

  // Set this to a translucent value for testing. Useful for visualizing the
  // view's bounds when performing transformations.
  void SetBackgroundColor(SkColor color);

  // `text` must be UTF-8 encoded.
  void SetString(const std::string& text);

  // views::View
  void OnPaint(gfx::Canvas* canvas) override;

 private:
  // Helper function to draw a single block of text with the data in
  // `text_fill_` and `text_outline_` at the provided coordinates.
  void DrawTextBlock(gfx::Canvas* canvas, int x, int y);

  // The width/height of individual blocks of text, including spacing.
  int block_width_offset() const;
  int block_height_offset() const;

  // Calculate X/Y bounds for the rotates canvas to cover the entire `bounds`
  // the waternarj is overlapping.
  int min_x(double angle, const gfx::Rect& bounds) const;
  int max_x(double angle, const gfx::Rect& bounds) const;
  int min_y(double angle, const gfx::Rect& bounds) const;
  int max_y(double angle, const gfx::Rect& bounds) const;

  // Background color of the whole `WatermarkView`. This is normally
  // transparent, but can be an arbitrary color for testing with the
  // "watermark_app" target.
  SkColor background_color_;

  // Height/width required to draw all the lines in `text_fill_`/`text_outline_`
  // in a single block.
  int block_width_ = 0;
  int block_height_ = 0;

  // Height of a single line of `text_fill_` and `text_fill_`.
  int single_line_height_ = 0;

  // Containers for the fill/outline representations of each line in a single
  // text block. This is done to avoid calling methods like
  // `RenderText::SetText` as much as possible as that would invalidate that
  // object's layout cache, and to avoid running into multiline issues for mixed
  // character set cases.
  std::vector<std::unique_ptr<gfx::RenderText>> text_fill_;
  std::vector<std::unique_ptr<gfx::RenderText>> text_outline_;
};

}  // namespace enterprise_watermark

#endif
