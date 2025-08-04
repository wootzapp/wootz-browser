// Copyright 2025 The Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/action_url/content/renderer/css_selector_matcher.h"

#include <algorithm>
#include <regex>
#include <set>

#include "base/logging.h"
#include "third_party/blink/public/platform/web_string.h"

namespace sensitive_masking {

// Comprehensive list of dangerous patterns for XSS prevention
const std::vector<std::string> CssSelectorMatcher::kDangerousPatterns = {
    // JavaScript execution vectors
    "javascript:", "expression(", "eval(", "function(", "return", "this.", "window.",
    "document.", "alert(", "confirm(", "prompt(", "console.", "location.",
    
    // CSS injection vectors  
    "@import", "@namespace", "@media", "@supports", "@document", "@page",
    "url(", "behavior:", "-moz-binding", "-webkit-binding", "binding:",
    
    // HTML/XML injection
    "<script", "</script", "<iframe", "<object", "<embed", "<link", "<meta",
    "<style", "</style", "<!doctype", "<?xml", "<![cdata[",
    
    // Protocol handlers and data URIs
    "data:", "blob:", "filesystem:", "chrome:", "chrome-extension:",
    "moz-extension:", "safari-extension:", "ms-browser-extension:",
    
    // Encoding bypasses
    "\\x", "\\u", "&#x", "&#", "%", "\\\\", "\\/", "\\r", "\\n", "\\t",
    
    // CSS property injection
    "content:", "counter(", "attr(", "calc(", "var(", "env(",
    
    // Comment injection
    "/*", "*/", "//", "<!--", "-->",
    
    // Quote escaping
    "\\'", "\\\"", "\\\\"
};

bool CssSelectorMatcher::IsValidSelector(const std::string& selector) {
  // Basic length and emptiness checks
  if (selector.empty() || selector.length() > kMaxSelectorLength) {
    LOG(WARNING) << "CSS selector rejected: invalid length";
    return false;
  }
  
  // Multi-layer security validation
  if (!HasValidStructure(selector)) {
    LOG(WARNING) << "CSS selector rejected: invalid structure";
    return false;
  }
  
  if (ContainsDangerousPatterns(selector)) {
    LOG(WARNING) << "CSS selector rejected: dangerous patterns detected";
    return false;
  }
  
  if (!HasValidCharacters(selector)) {
    LOG(WARNING) << "CSS selector rejected: invalid characters";
    return false;
  }
  
  if (!IsWhitelistedSelectorType(selector)) {
    LOG(WARNING) << "CSS selector rejected: not in whitelist";
    return false;
  }
  
  return true;
}

bool CssSelectorMatcher::DoesElementMatchSelector(
    const blink::WebElement& element, 
    const std::string& selector) {
  if (selector.empty() || element.IsNull()) {
    return false;
  }
  
  // Try different selector types in order of performance
  
  // 1. Simple tag selectors (fastest)
  if (MatchesTagSelector(element, selector)) {
    return true;
  }
  
  // 2. ID selectors
  if (MatchesIdSelector(element, selector)) {
    return true;
  }
  
  // 3. Class selectors
  if (MatchesClassSelector(element, selector)) {
    return true;
  }
  
  // 4. Attribute selectors (most complex)
  if (MatchesAttributeSelector(element, selector)) {
    return true;
  }
  
  // For complex selectors we can't handle manually, return false
  // (Caller should fall back to QuerySelectorAll)
  return false;
}

std::vector<std::string> CssSelectorMatcher::ValidateSelectors(
    const std::vector<std::string>& selectors) {
  // Reasonable limits to prevent abuse while allowing legitimate use cases
  static constexpr size_t kMaxSelectorsPerCall = 50;    // Increased for legitimate use
  static constexpr size_t kMaxTotalSelectors = 200;     // Total capacity per page
  
  // Check per-call limit (prevent single malicious call)
  if (selectors.size() > kMaxSelectorsPerCall) {
    LOG(ERROR) << "Too many selectors in single call: " << selectors.size() 
               << " (max per call: " << kMaxSelectorsPerCall << ")";
    // Don't reject completely - process up to the limit
    LOG(WARNING) << "Processing first " << kMaxSelectorsPerCall << " selectors only";
  }
  
  // Process selectors up to the per-call limit
  size_t selectors_to_process = std::min(selectors.size(), kMaxSelectorsPerCall);
  
  std::vector<std::string> validated_selectors;
  validated_selectors.reserve(selectors_to_process);
  
  size_t rejected_count = 0;
  for (size_t i = 0; i < selectors_to_process; ++i) {
    const auto& selector = selectors[i];
    
    if (IsValidSelector(selector)) {
      validated_selectors.push_back(selector);
    } else {
      rejected_count++;
      LOG(WARNING) << "Rejecting invalid CSS selector: " << selector;
    }
    
    // Check total capacity (across all calls for this page)
    if (validated_selectors.size() >= kMaxTotalSelectors) {
      LOG(WARNING) << "Reached maximum total selectors limit: " << kMaxTotalSelectors;
      break;
    }
  }
  
  // Log security and usage metrics
  if (rejected_count > 0) {
    LOG(WARNING) << "CSS selector validation rejected " << rejected_count 
                 << " out of " << selectors_to_process << " selectors processed";
  }
  
  DVLOG(1) << "Validated " << validated_selectors.size() << " CSS selectors";
  
  return OptimizeSelectors(validated_selectors);
}

std::vector<std::string> CssSelectorMatcher::OptimizeSelectors(
    const std::vector<std::string>& selectors) {
  if (selectors.empty()) {
    return selectors;
  }
  
  std::vector<std::string> optimized_selectors;
  std::set<std::string> seen_selectors;  // Remove duplicates
  
  // Group selectors by type for potential optimization
  std::vector<std::string> tag_selectors;
  std::vector<std::string> class_selectors;
  std::vector<std::string> id_selectors;
  std::vector<std::string> other_selectors;
  
  for (const auto& selector : selectors) {
    // Skip duplicates
    if (seen_selectors.count(selector)) {
      continue;
    }
    seen_selectors.insert(selector);
    
    // Categorize selectors for potential optimization
    if (selector.find_first_of("#.[:]") == std::string::npos) {
      // Pure tag selector
      tag_selectors.push_back(selector);
    } else if (selector.starts_with(".") && selector.find_first_of("#[:", 1) == std::string::npos) {
      // Pure class selector
      class_selectors.push_back(selector);
    } else if (selector.starts_with("#") && selector.find_first_of(".[:", 1) == std::string::npos) {
      // Pure ID selector
      id_selectors.push_back(selector);
    } else {
      // Complex selector
      other_selectors.push_back(selector);
    }
  }
  
  // For now, just combine all selectors back (future: could optimize further)
  // Priority order: IDs (most specific), classes, tags, others
  optimized_selectors.reserve(selectors.size());
  optimized_selectors.insert(optimized_selectors.end(), id_selectors.begin(), id_selectors.end());
  optimized_selectors.insert(optimized_selectors.end(), class_selectors.begin(), class_selectors.end());
  optimized_selectors.insert(optimized_selectors.end(), tag_selectors.begin(), tag_selectors.end());
  optimized_selectors.insert(optimized_selectors.end(), other_selectors.begin(), other_selectors.end());
  
  size_t removed_duplicates = selectors.size() - optimized_selectors.size();
  if (removed_duplicates > 0) {
    DVLOG(1) << "Removed " << removed_duplicates << " duplicate selectors";
  }
  
  return optimized_selectors;
}

bool CssSelectorMatcher::MatchesTagSelector(
    const blink::WebElement& element, 
    const std::string& selector) {
  // Pure tag selector has no special characters
  if (selector.find_first_of("#.[:]") != std::string::npos) {
    return false;
  }
  
  return element.HasHTMLTagName(blink::WebString::FromUTF8(selector));
}

bool CssSelectorMatcher::MatchesIdSelector(
    const blink::WebElement& element, 
    const std::string& selector) {
  if (!selector.starts_with("#") || selector.length() < 2) {
    return false;
  }
  
  std::string target_id = selector.substr(1);
  std::string element_id = element.GetAttribute("id").Utf8();
  return (element_id == target_id);
}

bool CssSelectorMatcher::MatchesClassSelector(
    const blink::WebElement& element, 
    const std::string& selector) {
  if (!selector.starts_with(".") || selector.length() < 2) {
    return false;
  }
  
  std::string target_class = selector.substr(1);
  std::string element_class = element.GetAttribute("class").Utf8();
  
  // Check if target class is present in the element's class list
  return (element_class.find(target_class) != std::string::npos);
}

bool CssSelectorMatcher::MatchesAttributeSelector(
    const blink::WebElement& element, 
    const std::string& selector) {
  // Must contain attribute selector brackets
  size_t bracket_start = selector.find('[');
  size_t bracket_end = selector.find(']');
  
  if (bracket_start == std::string::npos || bracket_end == std::string::npos ||
      bracket_end <= bracket_start) {
    return false;
  }
  
  // Check tag part if present
  if (bracket_start > 0) {
    std::string tag = selector.substr(0, bracket_start);
    if (!tag.empty() && !element.HasHTMLTagName(blink::WebString::FromUTF8(tag))) {
      return false;
    }
  }
  
  // Extract and parse attribute part
  std::string attr_part = selector.substr(bracket_start + 1, 
                                         bracket_end - bracket_start - 1);
  
  size_t equals_pos = attr_part.find('=');
  if (equals_pos != std::string::npos) {
    // Attribute value matching: [attr="value"]
    std::string attr_name = attr_part.substr(0, equals_pos);
    std::string attr_value = attr_part.substr(equals_pos + 1);
    
    // Remove quotes if present
    if (attr_value.length() >= 2 && 
        attr_value.front() == '"' && attr_value.back() == '"') {
      attr_value = attr_value.substr(1, attr_value.length() - 2);
    }
    
    std::string element_attr_value = 
        element.GetAttribute(blink::WebString::FromUTF8(attr_name)).Utf8();
    return element_attr_value == attr_value;
  } else {
    // Attribute existence check: [attr]
    std::string element_attr_value = 
        element.GetAttribute(blink::WebString::FromUTF8(attr_part)).Utf8();
    return !element_attr_value.empty();
  }
}

bool CssSelectorMatcher::ContainsDangerousPatterns(const std::string& selector) {
  std::string lower_selector = selector;
  std::transform(lower_selector.begin(), lower_selector.end(), 
                 lower_selector.begin(), ::tolower);
  
  for (const auto& pattern : kDangerousPatterns) {
    if (lower_selector.find(pattern) != std::string::npos) {
      return true;
    }
  }
  
  return false;
}

bool CssSelectorMatcher::HasValidStructure(const std::string& selector) {
  // Check for balanced brackets and quotes
  int square_brackets = 0;
  int parentheses = 0;
  bool in_quotes = false;
  char quote_char = '\0';
  
  for (size_t i = 0; i < selector.length(); ++i) {
    char c = selector[i];
    
    // Handle quotes
    if ((c == '"' || c == '\'') && !in_quotes) {
      in_quotes = true;
      quote_char = c;
    } else if (c == quote_char && in_quotes) {
      in_quotes = false;
      quote_char = '\0';
    }
    
    // Skip bracket counting inside quotes
    if (in_quotes) continue;
    
    // Count brackets
    switch (c) {
      case '[': square_brackets++; break;
      case ']': square_brackets--; break;
      case '(': parentheses++; break;
      case ')': parentheses--; break;
    }
    
    // Check for negative counts (malformed)
    if (square_brackets < 0 || parentheses < 0) {
      return false;
    }
  }
  
  // All brackets must be balanced and no unclosed quotes
  return square_brackets == 0 && parentheses == 0 && !in_quotes;
}

bool CssSelectorMatcher::IsWhitelistedSelectorType(const std::string& selector) {
  // Only allow specific, safe selector patterns
  static const std::vector<std::regex> whitelisted_patterns = {
    // Tag selectors: input, div, span, etc.
    std::regex(R"(^[a-zA-Z][a-zA-Z0-9]*$)"),
    
    // ID selectors: #my-id, #user_name
    std::regex(R"(^#[a-zA-Z][a-zA-Z0-9_-]*$)"),
    
    // Class selectors: .my-class, .form_field
    std::regex(R"(^\.[a-zA-Z][a-zA-Z0-9_-]*$)"),
    
    // Simple attribute selectors: [name="value"], [data-type]
    std::regex(R"(^[a-zA-Z][a-zA-Z0-9]*\[[a-zA-Z][a-zA-Z0-9_-]*\]$)"),
    std::regex(R"(^[a-zA-Z][a-zA-Z0-9]*\[[a-zA-Z][a-zA-Z0-9_-]*="[a-zA-Z0-9_-]+"\]$)"),
    
    // Tag with ID: input#password
    std::regex(R"(^[a-zA-Z][a-zA-Z0-9]*#[a-zA-Z][a-zA-Z0-9_-]*$)"),
    
    // Tag with class: input.form-control
    std::regex(R"(^[a-zA-Z][a-zA-Z0-9]*\.[a-zA-Z][a-zA-Z0-9_-]*$)"),
    
    // Simple descendant: form input, div span
    std::regex(R"(^[a-zA-Z][a-zA-Z0-9]*\s+[a-zA-Z][a-zA-Z0-9]*$)")
  };
  
  for (const auto& pattern : whitelisted_patterns) {
    if (std::regex_match(selector, pattern)) {
      return true;
    }
  }
  
  return false;
}

bool CssSelectorMatcher::HasValidCharacters(const std::string& selector) {
  // Strict character whitelist - only allow essential CSS selector characters
  static const std::regex allowed_pattern(
      R"(^[a-zA-Z0-9\-_\.\#\[\]="'\s]+$)");
  
  if (!std::regex_match(selector, allowed_pattern)) {
    return false;
  }
  
  // Additional checks for suspicious character sequences
  static const std::vector<std::string> suspicious_sequences = {
    "\\", "%", "&", "<", ">", "{", "}", "|", "^", "`", "~", 
    "++", "--", "**", "//", "..", "::", ";;", "!!", "??", "@@"
  };
  
  for (const auto& seq : suspicious_sequences) {
    if (selector.find(seq) != std::string::npos) {
      return false;
    }
  }
  
  return true;
}

}  // namespace sensitive_masking