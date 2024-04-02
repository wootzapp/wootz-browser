// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_STYLE_POSITION_TRY_OPTIONS_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_STYLE_POSITION_TRY_OPTIONS_H_

#include <array>
#include "third_party/blink/renderer/core/style/computed_style_constants.h"
#include "third_party/blink/renderer/core/style/scoped_css_name.h"

namespace blink {

// https://drafts.csswg.org/css-anchor-position-1/#typedef-position-try-options-try-tactic
//
// Note that we have to preserve the individual "flips"
// in the order they were specified.
using TryTacticList = std::array<TryTactic, 3>;

static inline constexpr TryTacticList kNoTryTactics = {
    TryTactic::kNone, TryTactic::kNone, TryTactic::kNone};

class CORE_EXPORT PositionTryOption {
  DISALLOW_NEW();

 public:
  PositionTryOption(const ScopedCSSName* name, TryTacticList tactic_list)
      : position_try_name_(name), tactic_list_(tactic_list) {}

  const TryTacticList& GetTryTactic() const { return tactic_list_; }
  const ScopedCSSName* GetPositionTryName() const { return position_try_name_; }

  bool operator==(const PositionTryOption& other) const;

  void Trace(Visitor* visitor) const;

 private:
  Member<const ScopedCSSName> position_try_name_;
  TryTacticList tactic_list_;
};

class CORE_EXPORT PositionTryOptions
    : public GarbageCollected<PositionTryOptions> {
 public:
  PositionTryOptions(HeapVector<PositionTryOption> options)
      : options_(std::move(options)) {}

  const HeapVector<PositionTryOption>& GetOptions() const { return options_; }
  bool operator==(const PositionTryOptions& other) const;
  void Trace(Visitor* visitor) const;

 private:
  HeapVector<PositionTryOption> options_;
};

}  // namespace blink

namespace WTF {

template <>
struct VectorTraits<blink::PositionTryOption>
    : VectorTraitsBase<blink::PositionTryOption> {
  static const bool kCanClearUnusedSlotsWithMemset = true;
  static const bool kCanInitializeWithMemset = true;
  static const bool kCanMoveWithMemcpy = true;
  static const bool kCanTraceConcurrently = true;
};

}  // namespace WTF

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_STYLE_POSITION_TRY_OPTIONS_H_
