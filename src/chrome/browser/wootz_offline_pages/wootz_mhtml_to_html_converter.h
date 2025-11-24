// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_MHTML_TO_HTML_CONVERTER_H_
#define CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_MHTML_TO_HTML_CONVERTER_H_

#include "base/files/file_path.h"
#include <string>
#include <vector>

namespace wootz_offline_pages {

// Represents a single part of an MHTML archive
struct MHTMLPart {
  std::string content_type;
  std::string content_location;
  std::string content_id;
  std::string transfer_encoding;
  std::string data;
};

// Converts MHTML files to single HTML files with embedded resources
class WootzMHTMLToHTMLConverter {
 public:
  // Converts an MHTML file to a single HTML file
  // Returns true on success, false on failure
  static bool Convert(const base::FilePath& mhtml_path,
                     const base::FilePath& html_path);

 private:
  // Parses MHTML content into individual parts
  static std::vector<MHTMLPart> ParseMHTML(const std::string& mhtml_content);
  
  // Extracts the boundary string from MHTML headers
  static std::string ExtractBoundary(const std::string& mhtml_content);
  
  // Decodes quoted-printable encoded content
  static std::string DecodeQuotedPrintable(const std::string& encoded);
  
  // Decodes base64 encoded content
  static std::string DecodeBase64(const std::string& encoded);
  
  // Builds a single HTML file from MHTML parts
  static std::string BuildHTML(const std::vector<MHTMLPart>& parts);
  
  // Creates a data URI from content type and data
  static std::string CreateDataURI(const std::string& content_type,
                                   const std::string& data,
                                   bool is_base64);
  
  // Converts a URL to the corresponding offline file name
  static std::string URLToFileName(const std::string& url);
  
  // Rewrites all href attributes in HTML to point to local files
  static void RewriteLinks(std::string& html);
};

}  // namespace wootz_offline_pages

#endif  // CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_MHTML_TO_HTML_CONVERTER_H_
