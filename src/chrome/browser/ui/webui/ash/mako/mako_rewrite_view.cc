// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/ash/mako/mako_rewrite_view.h"

#include "content/public/common/input/native_web_keyboard_event.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/display/screen.h"
#include "ui/gfx/geometry/rect.h"

namespace ash {

namespace {

constexpr int kMakoAnchorVerticalPadding = 16;
constexpr int kMakoScreenEdgePadding = 16;
constexpr int kMakoRewriteCornerRadius = 20;

// Height threshold of the mako rewrite UI which determines its screen position.
// Tall UI is centered on the display screen containing the caret, while short
// UI is anchored at the caret.
constexpr int kMakoRewriteHeightThreshold = 400;

}  // namespace

MakoRewriteView::MakoRewriteView(WebUIContentsWrapper* contents_wrapper,
                                 const gfx::Rect& caret_bounds)
    : WebUIBubbleDialogView(nullptr, contents_wrapper->GetWeakPtr()),
      caret_bounds_(caret_bounds) {
  set_has_parent(false);
  set_corner_radius(kMakoRewriteCornerRadius);
  // Disable the default offscreen adjustment so that we can customise it.
  set_adjust_if_offscreen(false);
}

MakoRewriteView::~MakoRewriteView() = default;

void MakoRewriteView::ResizeDueToAutoResize(content::WebContents* source,
                                            const gfx::Size& new_size) {
  WebUIBubbleDialogView::ResizeDueToAutoResize(source, new_size);
  gfx::Rect screen_work_area = display::Screen::GetScreen()
                                   ->GetDisplayMatching(caret_bounds_)
                                   .work_area();
  screen_work_area.Inset(kMakoScreenEdgePadding);

  // If the contents is very tall, just place it at the center of the screen.
  if (new_size.height() > kMakoRewriteHeightThreshold) {
    SetArrowWithoutResizing(views::BubbleBorder::FLOAT);
    SetAnchorRect(screen_work_area);
    return;
  }

  // Otherwise, try to place it under at the bottom left of the selection.
  gfx::Rect anchor = caret_bounds_;
  anchor.Outset(gfx::Outsets::VH(kMakoAnchorVerticalPadding, 0));
  gfx::Rect mako_contents_bounds(anchor.bottom_left(), new_size);

  // If horizontally offscreen, just move it to the right edge of the screen.
  if (mako_contents_bounds.right() > screen_work_area.right()) {
    mako_contents_bounds.set_x(screen_work_area.right() - new_size.width());
  }

  // If vertically offscreen, try above the selection.
  if (mako_contents_bounds.bottom() > screen_work_area.bottom()) {
    mako_contents_bounds.set_y(anchor.y() - new_size.height());
  }
  // If still vertically offscreen, just move it to the bottom of the screen.
  if (mako_contents_bounds.y() < screen_work_area.y()) {
    mako_contents_bounds.set_y(screen_work_area.bottom() - new_size.height());
  }

  // Compute widget bounds, which includes the border and shadow around the main
  // contents. Then, adjust again to ensure the whole widget is onscreen.
  gfx::Rect widget_bounds(mako_contents_bounds);
  widget_bounds.Inset(-GetBubbleFrameView()->bubble_border()->GetInsets());
  widget_bounds.AdjustToFit(screen_work_area);

  GetWidget()->SetBounds(widget_bounds);
}

bool MakoRewriteView::HandleKeyboardEvent(
    content::WebContents* source,
    const content::NativeWebKeyboardEvent& event) {
  if (event.GetType() == content::NativeWebKeyboardEvent::Type::kRawKeyDown &&
      event.dom_key == ui::DomKey::ESCAPE) {
    return true;
  }

  return WebUIBubbleDialogView::HandleKeyboardEvent(source, event);
}

BEGIN_METADATA(MakoRewriteView)
END_METADATA

}  // namespace ash
