// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/accessibility/accessibility_features.h"

#include "base/feature_list.h"
#include "build/build_config.h"
#include "build/chromeos_buildflags.h"
#include "ui/accessibility/ax_features.mojom-features.h"

#if BUILDFLAG(IS_CHROMEOS_ASH)
#include "ash/constants/ash_features.h"
#endif

namespace features {

BASE_FEATURE(kAccessibilityFocusHighlight,
             "AccessibilityFocusHighlight",
             base::FEATURE_ENABLED_BY_DEFAULT);
bool IsAccessibilityFocusHighlightEnabled() {
  return base::FeatureList::IsEnabled(::features::kAccessibilityFocusHighlight);
}

BASE_FEATURE(kAccessibilityPdfOcrForSelectToSpeak,
             "kAccessibilityPdfOcrForSelectToSpeak",
#if BUILDFLAG(IS_CHROMEOS)
             base::FEATURE_ENABLED_BY_DEFAULT
#else
             base::FEATURE_DISABLED_BY_DEFAULT
#endif  // BUILDFLAG(IS_CHROMEOS)
);
bool IsAccessibilityPdfOcrForSelectToSpeakEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kAccessibilityPdfOcrForSelectToSpeak);
}

BASE_FEATURE(kAccessibilityPruneRedundantInlineText,
             "AccessibilityPruneRedundantInlineText",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAccessibilityPruneRedundantInlineTextEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kAccessibilityPruneRedundantInlineText);
}

BASE_FEATURE(kAugmentExistingImageLabels,
             "AugmentExistingImageLabels",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAugmentExistingImageLabelsEnabled() {
  return base::FeatureList::IsEnabled(::features::kAugmentExistingImageLabels);
}

BASE_FEATURE(kAutoDisableAccessibility,
             "AutoDisableAccessibility",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAutoDisableAccessibilityEnabled() {
  return base::FeatureList::IsEnabled(::features::kAutoDisableAccessibility);
}

BASE_FEATURE(kEnableAccessibilityAriaVirtualContent,
             "AccessibilityAriaVirtualContent",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAccessibilityAriaVirtualContentEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kEnableAccessibilityAriaVirtualContent);
}

BASE_FEATURE(kEnableAccessibilityExposeHTMLElement,
             "AccessibilityExposeHTMLElement",
             base::FEATURE_ENABLED_BY_DEFAULT);
bool IsAccessibilityExposeHTMLElementEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kEnableAccessibilityExposeHTMLElement);
}

BASE_FEATURE(kEnableAccessibilityLanguageDetection,
             "AccessibilityLanguageDetection",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAccessibilityLanguageDetectionEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kEnableAccessibilityLanguageDetection);
}

BASE_FEATURE(kEnableAccessibilityRestrictiveIA2AXModes,
             "AccessibilityRestrictiveIA2AXModes",
             base::FEATURE_ENABLED_BY_DEFAULT);
bool IsAccessibilityRestrictiveIA2AXModesEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kEnableAccessibilityRestrictiveIA2AXModes);
}

BASE_FEATURE(kEnableAccessibilityTreeForViews,
             "AccessibilityTreeForViews",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAccessibilityTreeForViewsEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kEnableAccessibilityTreeForViews);
}

BASE_FEATURE(kEnableAriaElementReflection,
             "EnableAriaElementReflection",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAriaElementReflectionEnabled() {
  return base::FeatureList::IsEnabled(::features::kEnableAriaElementReflection);
}

BASE_FEATURE(kTextBasedAudioDescription,
             "TextBasedAudioDescription",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsTextBasedAudioDescriptionEnabled() {
  return base::FeatureList::IsEnabled(::features::kTextBasedAudioDescription);
}

BASE_FEATURE(kUseAXPositionForDocumentMarkers,
             "UseAXPositionForDocumentMarkers",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsUseAXPositionForDocumentMarkersEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kUseAXPositionForDocumentMarkers);
}

#if BUILDFLAG(IS_WIN)
BASE_FEATURE(kIChromeAccessible,
             "IChromeAccessible",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsIChromeAccessibleEnabled() {
  return base::FeatureList::IsEnabled(::features::kIChromeAccessible);
}

BASE_FEATURE(kSelectiveUIAEnablement,
             "SelectiveUIAEnablement",
             base::FEATURE_ENABLED_BY_DEFAULT);
bool IsSelectiveUIAEnablementEnabled() {
  return base::FeatureList::IsEnabled(::features::kSelectiveUIAEnablement);
}

BASE_FEATURE(kUiaProvider, "UiaProvider", base::FEATURE_DISABLED_BY_DEFAULT);
bool IsUiaProviderEnabled() {
  return base::FeatureList::IsEnabled(kUiaProvider);
}
#endif  // BUILDFLAG(IS_WIN)

#if BUILDFLAG(IS_CHROMEOS_ASH)
bool IsDictationOfflineAvailable() {
  return base::FeatureList::IsEnabled(
      ash::features::kOnDeviceSpeechRecognition);
}

BASE_FEATURE(kAccessibilityDictationKeyboardImprovements,
             "AccessibilityDictationKeyboardImprovements",
             base::FEATURE_ENABLED_BY_DEFAULT);
bool IsAccessibilityDictationKeyboardImprovementsEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kAccessibilityDictationKeyboardImprovements);
}

BASE_FEATURE(kAccessibilityReducedAnimations,
             "AccessibilityReducedAnimations",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAccessibilityReducedAnimationsEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kAccessibilityReducedAnimations);
}

BASE_FEATURE(kAccessibilityFaceGaze,
             "AccessibilityFaceGaze",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAccessibilityFaceGazeEnabled() {
  return base::FeatureList::IsEnabled(::features::kAccessibilityFaceGaze);
}

BASE_FEATURE(kAccessibilitySelectToSpeakHoverTextImprovements,
             "AccessibilitySelectToSpeakHoverTextImprovements",
             base::FEATURE_ENABLED_BY_DEFAULT);
bool IsAccessibilitySelectToSpeakHoverTextImprovementsEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kAccessibilitySelectToSpeakHoverTextImprovements);
}

BASE_FEATURE(kExperimentalAccessibilityGoogleTtsHighQualityVoices,
             "ExperimentalAccessibilityGoogleTtsHighQualityVoices",
             base::FEATURE_ENABLED_BY_DEFAULT);
bool IsExperimentalAccessibilityGoogleTtsHighQualityVoicesEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kExperimentalAccessibilityGoogleTtsHighQualityVoices);
}

BASE_FEATURE(kExperimentalAccessibilityDictationContextChecking,
             "ExperimentalAccessibilityDictationContextChecking",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsExperimentalAccessibilityDictationContextCheckingEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kExperimentalAccessibilityDictationContextChecking);
}

BASE_FEATURE(kExperimentalAccessibilityGoogleTtsLanguagePacks,
             "ExperimentalAccessibilityGoogleTtsLanguagePacks",
             base::FEATURE_ENABLED_BY_DEFAULT);
bool IsExperimentalAccessibilityGoogleTtsLanguagePacksEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kExperimentalAccessibilityGoogleTtsLanguagePacks);
}

BASE_FEATURE(kAccessibilityExtraLargeCursor,
             "AccessibilityExtraLargeCursor",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAccessibilityExtraLargeCursorEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kAccessibilityExtraLargeCursor);
}

BASE_FEATURE(kAccessibilityMagnifierFollowsSts,
             "AccessibilityMagnifierFollowsSts",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAccessibilityMagnifierFollowsStsEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kAccessibilityMagnifierFollowsSts);
}

BASE_FEATURE(kAccessibilityMouseKeys,
             "AccessibilityMouseKeys",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAccessibilityMouseKeysEnabled() {
  return base::FeatureList::IsEnabled(::features::kAccessibilityMouseKeys);
}

BASE_FEATURE(kAccessibilityCaretBlinkIntervalSetting,
             "AccessibilityCaretBlinkIntervalSetting",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAccessibilityCaretBlinkIntervalSettingEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kAccessibilityCaretBlinkIntervalSetting);
}

#endif  // BUILDFLAG(IS_CHROMEOS_ASH)

#if BUILDFLAG(IS_ANDROID)
BASE_FEATURE(kAccessibilitySnapshotStressTests,
             "AccessibilitySnapshotStressTests",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAccessibilitySnapshotStressTestsEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kAccessibilitySnapshotStressTests);
}
#endif  // BUILDFLAG(IS_ANDROID)

#if !BUILDFLAG(IS_ANDROID)
bool IsScreenAIMainContentExtractionEnabled() {
  return base::FeatureList::IsEnabled(
      ax::mojom::features::kScreenAIMainContentExtractionEnabled);
}

bool IsScreenAIOCREnabled() {
  return base::FeatureList::IsEnabled(ax::mojom::features::kScreenAIOCREnabled);
}

BASE_FEATURE(kAccessibilityService,
             "AccessibilityService",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsAccessibilityServiceEnabled() {
  return base::FeatureList::IsEnabled(::features::kAccessibilityService);
}

// This feature is only used for generating training data for Screen2x and
// should never be used in any other circumstance, and should not be enabled by
// default.
BASE_FEATURE(kDataCollectionModeForScreen2x,
             "DataCollectionModeForScreen2x",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsDataCollectionModeForScreen2xEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kDataCollectionModeForScreen2x);
}

BASE_FEATURE(kLayoutExtraction,
             "LayoutExtraction",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsLayoutExtractionEnabled() {
  return base::FeatureList::IsEnabled(::features::kLayoutExtraction);
}

BASE_FEATURE(kPdfOcr,
             "PdfOcr",
#if BUILDFLAG(IS_CHROMEOS)
             base::FEATURE_ENABLED_BY_DEFAULT
#else
             base::FEATURE_DISABLED_BY_DEFAULT
#endif  // BUILDFLAG(IS_CHROMEOS)
);

bool IsPdfOcrEnabled() {
  return base::FeatureList::IsEnabled(::features::kPdfOcr);
}

BASE_FEATURE(kReadAnything, "ReadAnything", base::FEATURE_ENABLED_BY_DEFAULT);
bool IsReadAnythingEnabled() {
  return base::FeatureList::IsEnabled(::features::kReadAnything);
}

BASE_FEATURE(kReadAnythingLocalSidePanel,
             "ReadAnythingLocalSidePanel",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsReadAnythingLocalSidePanelEnabled() {
  return base::FeatureList::IsEnabled(
             ::features::kReadAnythingLocalSidePanel) &&
         base::FeatureList::IsEnabled(::features::kReadAnythingWebUIToolbar);
}

BASE_FEATURE(kReadAnythingOmniboxIcon,
             "ReadAnythingOmniboxIcon",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsReadAnythingOmniboxIconEnabled() {
  return base::FeatureList::IsEnabled(::features::kReadAnythingOmniboxIcon);
}

BASE_FEATURE(kReadAnythingDelaySidePanelLoad,
             "ReadAnythingDelaySidePanelLoad",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsReadAnythingDelaySidePanelLoadEnabled() {
  return base::FeatureList::IsEnabled(
      ::features::kReadAnythingDelaySidePanelLoad);
}

BASE_FEATURE(kReadAnythingReadAloud,
             "ReadAnythingReadAloud",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsReadAnythingReadAloudEnabled() {
  return base::FeatureList::IsEnabled(::features::kReadAnythingReadAloud);
}

BASE_FEATURE(kReadAnythingWebUIToolbar,
             "ReadAnythingWebUIToolbar",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsReadAnythingWebUIToolbarEnabled() {
  return base::FeatureList::IsEnabled(::features::kReadAnythingWebUIToolbar);
}

BASE_FEATURE(kReadAnythingWithScreen2x,
             "ReadAnythingWithScreen2x",
             base::FEATURE_ENABLED_BY_DEFAULT);
bool IsReadAnythingWithScreen2xEnabled() {
  return base::FeatureList::IsEnabled(::features::kReadAnythingWithScreen2x);
}

BASE_FEATURE(kReadAnythingWithAlgorithm,
             "ReadAnythingWithAlgorithm",
             base::FEATURE_ENABLED_BY_DEFAULT);
bool IsReadAnythingWithAlgorithmEnabled() {
  return base::FeatureList::IsEnabled(::features::kReadAnythingWithAlgorithm);
}

BASE_FEATURE(kReadAnythingImagesViaAlgorithm,
             "ReadAnythingImagesViaAlgorithm",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsReadAnythingImagesViaAlgorithmEnabled() {
  return base::FeatureList::IsEnabled(
             ::features::kReadAnythingImagesViaAlgorithm) &&
         IsReadAnythingWithAlgorithmEnabled();
}

// This feature is only for debug purposes and for security/privacy reasons,
// should be never enabled by default .
BASE_FEATURE(kScreenAIDebugMode,
             "ScreenAIDebugMode",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsScreenAIDebugModeEnabled() {
  return base::FeatureList::IsEnabled(::features::kScreenAIDebugMode);
}

// This feature is only used in tests and must not be enabled by default.
BASE_FEATURE(kScreenAITestMode,
             "ScreenAITestMode",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsScreenAITestModeEnabled() {
  return base::FeatureList::IsEnabled(::features::kScreenAITestMode);
}

// This feature is only used in tests and must not be enabled by default.
BASE_FEATURE(kMainNodeAnnotations,
             "MainNodeAnnotations",
             base::FEATURE_DISABLED_BY_DEFAULT);
bool IsMainNodeAnnotationsEnabled() {
  return base::FeatureList::IsEnabled(::features::kMainNodeAnnotations);
}
#endif  // !BUILDFLAG(IS_ANDROID)

#if BUILDFLAG(IS_MAC)
BASE_FEATURE(kAccessibilityRemoteUIApp,
             "AccessibilityRemoteUIApp",
             base::FEATURE_ENABLED_BY_DEFAULT);
bool IsAccessibilityRemoteUIAppEnabled() {
  return base::FeatureList::IsEnabled(::features::kAccessibilityRemoteUIApp);
}
#endif  // BUILDFLAG(IS_MAC)

}  // namespace features
