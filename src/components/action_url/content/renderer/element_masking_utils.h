// Copyright 2025 The Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ACTION_URL_CONTENT_RENDERER_ELEMENT_MASKING_UTILS_H_
#define COMPONENTS_ACTION_URL_CONTENT_RENDERER_ELEMENT_MASKING_UTILS_H_

#include "third_party/blink/public/web/web_element.h"

namespace sensitive_masking {

// Utility class for element masking operations and loop prevention.
// Provides methods to safely mask elements and detect masking-related elements.
class ElementMaskingUtils {
 public:
  ElementMaskingUtils() = delete;
  ~ElementMaskingUtils() = delete;

  // Masks a single element with appropriate method based on element type
  static void MaskElement(const blink::WebElement& element);

  // Checks if an element was created or modified by our masking process
  static bool IsElementCreatedByMasking(const blink::WebElement& element);

  // Validates that an element is safe to process (not null, valid tag)
  static bool IsElementValid(const blink::WebElement& element);

 private:
  // Element type-specific masking methods
  static void MaskInputElement(const blink::WebElement& element);
  static void MaskContentElement(const blink::WebElement& element);

  // Adds masking marker attributes to prevent reprocessing
  static void MarkElementAsMasked(const blink::WebElement& element);

  // Detection methods for masking artifacts
  static bool HasMaskingAttributes(const blink::WebElement& element);
  static bool HasMaskingStyles(const blink::WebElement& element);

  // Enhanced masking with toggle functionality
  static void CreateMaskedContainer(const blink::WebElement& element);
  static void ToggleMaskedVisibility(const blink::WebElement& toggle_button);
  static blink::WebElement FindMaskedContainer(const blink::WebElement& element);
  
  // Clean input warning functionality
  static void CreateInputWarning(const blink::WebElement& element);
  
  // Constants for masking
  static constexpr char kMaskedAttribute[] = "data-sensitive-masked";
  static constexpr char kWarningAttribute[] = "data-sensitive-warning";
  static constexpr char kMaskedContainerAttr[] = "data-masked-container";
  static constexpr char kOriginalContentAttr[] = "data-original-content";
  static constexpr char kToggleButtonAttr[] = "data-toggle-button";
  static constexpr char kWarningContainerAttr[] = "data-warning-container";
  static constexpr char kMaskText[] = "XXX";
  static constexpr char kWarningText[] = "Enter sensitive info securely.";
};

}  // namespace sensitive_masking

#endif  // COMPONENTS_ACTION_URL_CONTENT_RENDERER_ELEMENT_MASKING_UTILS_H_