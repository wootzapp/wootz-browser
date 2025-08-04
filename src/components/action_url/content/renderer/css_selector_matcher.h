// Copyright 2025 The Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_ACTION_URL_CONTENT_RENDERER_CSS_SELECTOR_MATCHER_H_
#define COMPONENTS_ACTION_URL_CONTENT_RENDERER_CSS_SELECTOR_MATCHER_H_

#include <string>
#include <vector>

#include "third_party/blink/public/web/web_element.h"

namespace sensitive_masking {

// Utility class for CSS selector pattern matching and validation.
// Provides efficient matching algorithms for common selector patterns
// and security validation for untrusted selector input.
class CssSelectorMatcher {
 public:
  CssSelectorMatcher() = default;
  ~CssSelectorMatcher() = default;

  // Disable copy and assignment
  CssSelectorMatcher(const CssSelectorMatcher&) = delete;
  CssSelectorMatcher& operator=(const CssSelectorMatcher&) = delete;

  // Security validation for CSS selectors to prevent XSS and injection attacks
  static bool IsValidSelector(const std::string& selector);

  // Fast pattern matching for simple selectors (tag, id, class, attribute)
  // Returns true if element matches the selector, false otherwise
  static bool DoesElementMatchSelector(const blink::WebElement& element, 
                                       const std::string& selector);

  // Validates and filters a list of selectors, removing invalid ones
  static std::vector<std::string> ValidateSelectors(
      const std::vector<std::string>& selectors);

  // Optimizes selectors by combining similar ones and removing duplicates
  static std::vector<std::string> OptimizeSelectors(
      const std::vector<std::string>& selectors);

 private:
  // Individual selector type matchers
  static bool MatchesTagSelector(const blink::WebElement& element, 
                                const std::string& selector);
  static bool MatchesIdSelector(const blink::WebElement& element, 
                               const std::string& selector);
  static bool MatchesClassSelector(const blink::WebElement& element, 
                                  const std::string& selector);
  static bool MatchesAttributeSelector(const blink::WebElement& element, 
                                      const std::string& selector);

  // Multi-layer security validation helpers
  static bool HasValidStructure(const std::string& selector);
  static bool ContainsDangerousPatterns(const std::string& selector);
  static bool HasValidCharacters(const std::string& selector);
  static bool IsWhitelistedSelectorType(const std::string& selector);

  // Security constants
  static constexpr size_t kMaxSelectorLength = 100;  // Reduced from 200
  static const std::vector<std::string> kDangerousPatterns;
};

}  // namespace sensitive_masking

#endif  // COMPONENTS_ACTION_URL_CONTENT_RENDERER_CSS_SELECTOR_MATCHER_H_