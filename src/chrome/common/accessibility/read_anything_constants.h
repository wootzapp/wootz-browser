// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_COMMON_ACCESSIBILITY_READ_ANYTHING_CONSTANTS_H_
#define CHROME_COMMON_ACCESSIBILITY_READ_ANYTHING_CONSTANTS_H_

#include <set>
#include <string>

#include "ui/accessibility/ax_mode.h"

// Various constants used throughout the Read Anything feature.
namespace string_constants {

extern const char kReadAnythingPlaceholderFontName[];
extern const char kReadAnythingPlaceholderVoiceName[];
extern const char kLetterSpacingHistogramName[];
extern const char kLineSpacingHistogramName[];
extern const char kColorHistogramName[];
extern const char kFontNameHistogramName[];
extern const char kFontScaleHistogramName[];
extern const char kSettingsChangeHistogramName[];
extern const char kScrollEventHistogramName[];
extern const char kEmptyStateHistogramName[];
extern const char kLanguageHistogramName[];
extern const char kPDFPageStart[];
extern const char kPDFPageEnd[];

extern const std::set<std::string> GetNonSelectableUrls();

}  // namespace string_constants

namespace {

// Group id for the toolbar
inline constexpr int kToolbarGroupId = 0;

// Visual constants for Read Anything feature.
inline constexpr int kInternalInsets = 8;
inline constexpr int kSeparatorTopBottomPadding = 4;
inline constexpr int kMinimumComboboxWidth = 110;

inline constexpr int kButtonPadding = 2;
inline constexpr int kIconSize = 16;
inline constexpr int kFontSizeIconSize = kIconSize + kInternalInsets;
inline constexpr int kColorsIconSize = 24;
inline constexpr int kLinkToggleIconSize = 26;
inline constexpr int kSpacingIconSize = 20;

// Used for text formatting correction in PDFs. This value should match the line
// width limit in app.html.
inline constexpr int kMaxLineWidth = 60;

// Audio constants for Read Aloud feature.
// Speech rate is a multiplicative scale where 1 is the baseline.
inline constexpr double kReadAnythingDefaultSpeechRate = 1;

// Font size in em
inline constexpr double kReadAnythingDefaultFontScale = 1;
inline constexpr double kReadAnythingMinimumFontScale = 0.5;
inline constexpr double kReadAnythingMaximumFontScale = 4.5;
inline constexpr double kReadAnythingFontScaleIncrement = 0.25;

// Display settings.
inline constexpr bool kReadAnythingDefaultLinksEnabled = true;

// The maximum number of times the label is shown in the omnibox icon.
inline constexpr int kReadAnythingOmniboxIconLabelShownCountMax = 3;

const char* kLanguagesSupportedByPoppins[] = {
    "af", "ca", "cs", "da", "de", "en", "es", "et", "fi", "fil",
    "fr", "hi", "hr", "hu", "id", "it", "lt", "lv", "mr", "ms",
    "nl", "pl", "pt", "ro", "sk", "sl", "sv", "sw", "tr"};

const char* kLanguagesSupportedByComicNeue[] = {
    "af", "ca", "cs", "da", "de", "en", "es", "et", "fi", "fil", "fr", "hr",
    "hu", "id", "it", "ms", "nl", "pl", "pt", "sk", "sl", "sv", "sw"};

const char* kLanguagesSupportedByLexendDeca[] = {
    "af", "ca", "cs", "da", "de", "en", "es", "et", "fi", "fil",
    "fr", "hr", "hu", "id", "it", "lt", "lv", "ms", "nl", "pl",
    "pt", "ro", "sk", "sl", "sv", "sw", "tr", "vi"};

const char* kLanguagesSupportedByEbGaramond[] = {
    "af", "bg", "ca", "cs", "da", "de", "en", "es", "et", "fi", "fil",
    "fr", "hr", "hu", "id", "it", "lt", "lv", "ms", "nl", "pl", "pt",
    "ro", "ru", "sk", "sl", "sr", "sv", "sw", "tr", "uk", "vi"};

const char* kLanguagesSupportedByStixTwoText[] = {
    "af", "bg", "ca", "cs", "da", "de", "en", "es", "et", "fi", "fil",
    "fr", "hr", "hu", "id", "it", "lt", "lv", "ms", "nl", "pl", "pt",
    "ro", "ru", "sk", "sl", "sr", "sv", "sw", "tr", "uk", "vi"};

const char* kLanguagesSupportedByAndika[] = {
    "af", "bg", "ca", "cs", "da", "de", "en", "es", "et", "fi", "fil", "fr",
    "hr", "hu", "id", "it", "kr", "lt", "lu", "lv", "ms", "nd", "nl",  "nr",
    "pl", "pt", "ro", "ru", "sk", "sl", "sr", "sv", "sw", "tr", "uk",  "vi"};

// Enum for logging when a text style setting is changed.
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
// TODO(crbug.com/1465029): Remove this enum once the views toolbar is removed.
enum class ReadAnythingSettingsChange {
  kFontChange = 0,
  kFontSizeChange = 1,
  kThemeChange = 2,
  kLineHeightChange = 3,
  kLetterSpacingChange = 4,
  kMaxValue = kLetterSpacingChange,
};

// Enum for logging the user-chosen font.
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum class ReadAnythingFont {
  kPoppins = 0,
  kSansSerif = 1,
  kSerif = 2,
  kComicNeue = 3,
  kLexendDeca = 4,
  kEbGaramond = 5,
  kStixTwoText = 6,
  kMaxValue = kStixTwoText,
};

// Enum for logging how a scroll occurs.
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum class ReadAnythingScrollEvent {
  kSelectedSidePanel = 0,
  kSelectedMainPanel = 1,
  kScrolledSidePanel = 2,
  kScrolledMainPanel = 3,
  kMaxValue = kScrolledMainPanel,
};

// Enum for logging when we show the empty state.
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum class ReadAnythingEmptyState {
  kEmptyStateShown = 0,
  kSelectionAfterEmptyStateShown = 1,
  kMaxValue = kSelectionAfterEmptyStateShown,
};

}  // namespace

#endif  // CHROME_COMMON_ACCESSIBILITY_READ_ANYTHING_CONSTANTS_H_
