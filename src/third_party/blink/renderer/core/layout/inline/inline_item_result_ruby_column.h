// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_LAYOUT_INLINE_INLINE_ITEM_RESULT_RUBY_COLUMN_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_LAYOUT_INLINE_INLINE_ITEM_RESULT_RUBY_COLUMN_H_

#include "third_party/blink/renderer/core/layout/inline/line_info.h"

namespace blink {

// A data specific to InlineItemResults with InlineItem::kOpenRubyColumn type.
struct InlineItemResultRubyColumn
    : public GarbageCollected<InlineItemResultRubyColumn> {
  void Trace(Visitor* visitor) const {
    visitor->Trace(base_line);
    visitor->Trace(annotation_line_list);
    visitor->Trace(ruby_break_token);
  }

  // A LineInfo for the base level.
  LineInfo base_line;

  // A list of annotation level LineInfo.  [0] is the outermost one.
  HeapVector<LineInfo, 1> annotation_line_list;

  // A list of ruby-position values.  The size of this list must be same as
  // annotation_line_list.
  Vector<RubyPosition, 1> position_list;

  // This is true if a ruby column is split into multiple segments by line
  // breaking, and this InlineItemResult represents the second or later
  // segment.
  bool is_continuation = false;

  // A break token at the end of this ruby column.
  // It is stored here because of ease of rewind.
  Member<RubyBreakTokenData> ruby_break_token;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_LAYOUT_INLINE_INLINE_ITEM_RESULT_RUBY_COLUMN_H_
