// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/views_features.h"

#include "base/feature_list.h"
#include "build/build_config.h"

namespace views::features {

// Please keep alphabetized.

// Implements desktop widgets reparenting. Enabling this feature allows them
// to function correctly as tab modals.
BASE_FEATURE(kDesktopWidgetReparentAura,
             "kDesktopWidgetReparentAura",
             base::FEATURE_ENABLED_BY_DEFAULT);

// Use a high-contrast style for ink drops when in platform high-contrast mode,
// including full opacity and a high-contrast color
BASE_FEATURE(kEnablePlatformHighContrastInkDrop,
             "EnablePlatformHighContrastInkDrop",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Only paint views that are invalidated/dirty (i.e. a paint was directly
// scheduled on those views) as opposed to painting all views that intersect
// an invalid rectangle on the layer.
BASE_FEATURE(kEnableViewPaintOptimization,
             "EnableViewPaintOptimization",
             base::FEATURE_DISABLED_BY_DEFAULT);

// Used to enable keyboard-accessible tooltips in Views UI, as opposed
// to kKeyboardAccessibleTooltip in //ui/base/ui_base_features.cc.
BASE_FEATURE(kKeyboardAccessibleTooltipInViews,
             "KeyboardAccessibleTooltipInViews",
             base::FEATURE_ENABLED_BY_DEFAULT);

}  // namespace views::features
