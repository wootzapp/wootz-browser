// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/wootz_offline_pages/wootz_mhtml_to_html_converter.h"

#include <string>
#include <vector>

#include "base/base64.h"
#include "base/files/file_util.h"
#include "base/hash/sha1.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"

namespace wootz_offline_pages {

namespace {

// Helper function to trim whitespace
std::string Trim(const std::string& str) {
  size_t start = str.find_first_not_of(" \t\r\n");
  if (start == std::string::npos)
    return "";
  size_t end = str.find_last_not_of(" \t\r\n");
  return str.substr(start, end - start + 1);
}

// Helper function to find header value
std::string FindHeader(const std::string& headers, const std::string& name) {
  size_t pos = headers.find(name + ":");
  if (pos == std::string::npos)
    return "";
  
  size_t start = pos + name.length() + 1;
  size_t end = headers.find('\n', start);
  if (end == std::string::npos)
    end = headers.length();
  
  std::string value = headers.substr(start, end - start);
  return Trim(value);
}

}  // namespace

// static
bool WootzMHTMLToHTMLConverter::Convert(const base::FilePath& mhtml_path,
                                        const base::FilePath& html_path) {
  LOG(INFO) << "Kartik: Converting MHTML to HTML: " << mhtml_path.value();
  
  // Read MHTML file
  std::string mhtml_content;
  if (!base::ReadFileToString(mhtml_path, &mhtml_content)) {
    LOG(ERROR) << "Kartik: Failed to read MHTML file: " << mhtml_path.value();
    return false;
  }
  
  // Parse MHTML parts
  std::vector<MHTMLPart> parts = ParseMHTML(mhtml_content);
  if (parts.empty()) {
    LOG(ERROR) << "Kartik: Failed to parse MHTML, no parts found";
    return false;
  }
  
  // LOG(INFO) << "Kartik: Parsed " << parts.size() << " MHTML parts";
  
  // Build single HTML file
  std::string html = BuildHTML(parts);
  if (html.empty()) {
    LOG(ERROR) << "Kartik: Failed to build HTML from MHTML parts";
    return false;
  }
  
  // Write HTML file TEMPORARILY (will be deleted after merging into flow file)
  if (!base::WriteFile(html_path, html)) {
    LOG(ERROR) << "Kartik: Failed to write temporary HTML file: " << html_path.value();
    return false;
  }
  
  LOG(INFO) << "Kartik: Saved temporary HTML: " << html_path.value()
            << " (size: " << html.length() << " bytes) - will be merged then deleted";
  return true;
}

// static
std::string WootzMHTMLToHTMLConverter::ExtractBoundary(
    const std::string& mhtml_content) {
  // Find boundary in Content-Type header
  size_t boundary_pos = mhtml_content.find("boundary=\"");
  if (boundary_pos == std::string::npos) {
    // Try without quotes
    boundary_pos = mhtml_content.find("boundary=");
    if (boundary_pos == std::string::npos)
      return "";
    
    size_t start = boundary_pos + 9;  // length of "boundary="
    size_t end = mhtml_content.find_first_of("\r\n;", start);
    if (end == std::string::npos)
      end = mhtml_content.length();
    return "--" + Trim(mhtml_content.substr(start, end - start));
  }
  
  size_t start = boundary_pos + 10;  // length of "boundary=\""
  size_t end = mhtml_content.find('"', start);
  if (end == std::string::npos)
    return "";
  
  return "--" + mhtml_content.substr(start, end - start);
}

// static
std::vector<MHTMLPart> WootzMHTMLToHTMLConverter::ParseMHTML(
    const std::string& mhtml_content) {
  std::vector<MHTMLPart> parts;
  
  // Extract boundary
  std::string boundary = ExtractBoundary(mhtml_content);
  if (boundary.empty()) {
    LOG(ERROR) << "Kartik: Failed to extract MHTML boundary";
    return parts;
  }
  
  LOG(INFO) << "Kartik: MHTML boundary: " << boundary;
  
  // Split by boundary
  size_t pos = 0;
  while ((pos = mhtml_content.find(boundary, pos)) != std::string::npos) {
    // Skip the boundary line
    pos += boundary.length();
    
    // Check for end boundary
    if (pos + 2 < mhtml_content.length() &&
        mhtml_content[pos] == '-' && mhtml_content[pos + 1] == '-') {
      break;  // End of MHTML
    }
    
    // Skip newlines after boundary
    while (pos < mhtml_content.length() &&
           (mhtml_content[pos] == '\r' || mhtml_content[pos] == '\n')) {
      pos++;
    }
    
    // Find next boundary or end
    size_t next_boundary = mhtml_content.find(boundary, pos);
    if (next_boundary == std::string::npos)
      next_boundary = mhtml_content.length();
    
    std::string part_content = mhtml_content.substr(pos, next_boundary - pos);
    
    // Split headers and body (separated by double newline)
    size_t body_start = part_content.find("\r\n\r\n");
    if (body_start == std::string::npos)
      body_start = part_content.find("\n\n");
    
    if (body_start == std::string::npos) {
      pos = next_boundary;
      continue;
    }
    
    std::string headers = part_content.substr(0, body_start);
    std::string body = part_content.substr(body_start + 4);  // Skip \r\n\r\n
    
    // Parse headers
    MHTMLPart part;
    part.content_type = FindHeader(headers, "Content-Type");
    part.content_location = FindHeader(headers, "Content-Location");
    part.content_id = FindHeader(headers, "Content-ID");
    part.transfer_encoding = FindHeader(headers, "Content-Transfer-Encoding");
    
    // Decode body based on transfer encoding
    if (part.transfer_encoding == "quoted-printable") {
      part.data = DecodeQuotedPrintable(body);
    } else if (part.transfer_encoding == "base64") {
      part.data = DecodeBase64(body);
    } else {
      part.data = body;
    }
    
    if (!part.content_type.empty()) {
      parts.push_back(part);
      // LOG(INFO) << "Kartik: Parsed part - Type: " << part.content_type
      //           << ", Size: " << part.data.length() << " bytes";
    }
    
    pos = next_boundary;
  }
  
  return parts;
}

// static
std::string WootzMHTMLToHTMLConverter::DecodeQuotedPrintable(
    const std::string& encoded) {
  std::string decoded;
  decoded.reserve(encoded.length());
  
  for (size_t i = 0; i < encoded.length(); i++) {
    if (encoded[i] == '=') {
      if (i + 2 < encoded.length()) {
        // Check for soft line break (= at end of line)
        if (encoded[i + 1] == '\r' || encoded[i + 1] == '\n') {
          // Skip soft line break
          if (encoded[i + 1] == '\r' && i + 2 < encoded.length() &&
              encoded[i + 2] == '\n') {
            i += 2;
          } else {
            i += 1;
          }
          continue;
        }
        
        // Decode =XX hex sequence
        char hex[3] = {encoded[i + 1], encoded[i + 2], 0};
        char* end;
        long value = std::strtol(hex, &end, 16);
        if (end == hex + 2) {
          decoded += static_cast<char>(value);
          i += 2;
        } else {
          decoded += encoded[i];
        }
      } else {
        decoded += encoded[i];
      }
    } else {
      decoded += encoded[i];
    }
  }
  
  return decoded;
}

// static
std::string WootzMHTMLToHTMLConverter::DecodeBase64(const std::string& encoded) {
  std::string cleaned;
  for (char c : encoded) {
    if (c != '\r' && c != '\n' && c != ' ' && c != '\t') {
      cleaned += c;
    }
  }
  
  std::string decoded;
  if (!base::Base64Decode(cleaned, &decoded)) {
    LOG(WARNING) << "Kartik: Failed to decode base64, using original";
    return encoded;
  }
  
  return decoded;
}

// static
std::string WootzMHTMLToHTMLConverter::CreateDataURI(
    const std::string& content_type,
    const std::string& data,
    bool is_base64) {
  std::string uri = "data:" + content_type;
  
  // Always encode to base64, since data might be binary
  // (even if it was originally base64, we decoded it in ParseMHTML)
  std::string encoded = base::Base64Encode(data);
  uri += ";base64," + encoded;
  
  return uri;
}

// static
std::string WootzMHTMLToHTMLConverter::URLToFileName(const std::string& url) {
  // Compute SHA1 hash of the URL
  std::string hash = base::SHA1HashString(url);
  std::string hex_hash = base::HexEncode(hash.c_str(), hash.length());
  return "wootz_offline_" + hex_hash + ".html";
}

// static
void WootzMHTMLToHTMLConverter::RewriteLinks(std::string& html) {
  LOG(INFO) << "Kartik: Rewriting links in HTML for offline navigation";
  
  size_t pos = 0;
  int rewritten_count = 0;
  
  // Find and replace all href attributes
  while ((pos = html.find("href=\"", pos)) != std::string::npos) {
    size_t url_start = pos + 6;  // After href="
    size_t url_end = html.find('"', url_start);
    
    if (url_end == std::string::npos) {
      pos = url_start;
      continue;
    }
    
    std::string url = html.substr(url_start, url_end - url_start);
    
    // Only process http and https URLs
    if (url.find("http://") == 0 || url.find("https://") == 0) {
      // Skip anchor links (they contain #)
      size_t hash_pos = url.find('#');
      std::string base_url = url;
      std::string fragment;
      
      if (hash_pos != std::string::npos) {
        base_url = url.substr(0, hash_pos);
        fragment = url.substr(hash_pos);  // Keep the # and fragment
      }
      
      // Convert URL to file name
      std::string new_href = URLToFileName(base_url);
      
      // Add fragment back if it exists
      if (!fragment.empty()) {
        new_href += fragment;
      }
      
      // Replace the URL in the HTML
      html.replace(url_start, url_end - url_start, new_href);
      
      rewritten_count++;
      pos = url_start + new_href.length();
    } else {
      pos = url_end;
    }
  }
  
  LOG(INFO) << "Kartik: Rewrote " << rewritten_count << " links for offline navigation";
}

// static
std::string WootzMHTMLToHTMLConverter::BuildHTML(
    const std::vector<MHTMLPart>& parts) {
  if (parts.empty()) {
    LOG(ERROR) << "Kartik: No parts to build HTML from";
    return "";
  }
  
  // First part should be HTML
  if (parts[0].content_type.find("text/html") == std::string::npos) {
    LOG(ERROR) << "Kartik: First part is not HTML: " << parts[0].content_type;
    return "";
  }
  
  std::string html = parts[0].data;
  LOG(INFO) << "Kartik: Base HTML size: " << html.length() << " bytes";
  
  // Process remaining parts
  for (size_t i = 1; i < parts.size(); i++) {
    const MHTMLPart& part = parts[i];
    
    if (part.content_type.find("text/css") != std::string::npos) {
      // Inline CSS
      std::string style_tag = "<style>\n" + part.data + "\n</style>\n";
      
      // Try to insert before </head>
      size_t head_end = html.find("</head>");
      if (head_end != std::string::npos) {
        html.insert(head_end, style_tag);
      } else {
        // If no </head>, insert at the beginning after <html>
        size_t html_start = html.find("<html");
        if (html_start != std::string::npos) {
          size_t insert_pos = html.find('>', html_start);
          if (insert_pos != std::string::npos) {
            html.insert(insert_pos + 1, "\n<head>\n" + style_tag + "</head>\n");
          }
        }
      }
      
      // Replace cid: references
      if (!part.content_id.empty()) {
        std::string cid = part.content_id;
        // Remove < and > if present
        if (cid.front() == '<')
          cid = cid.substr(1);
        if (cid.back() == '>')
          cid = cid.substr(0, cid.length() - 1);
        
        // This CSS is now inline, so remove the link reference
        std::string link_pattern = "href=\"cid:" + cid + "\"";
        size_t link_pos = html.find(link_pattern);
        if (link_pos != std::string::npos) {
          // Find and remove the entire <link> tag
          size_t link_start = html.rfind("<link", link_pos);
          if (link_start != std::string::npos) {
            size_t link_end = html.find(">", link_pos);
            if (link_end != std::string::npos) {
              html.erase(link_start, link_end - link_start + 1);
            }
          }
        }
      }
      
      // LOG(INFO) << "Kartik: Inlined CSS part " << i;
      
    } else if (part.content_type.find("image/") != std::string::npos) {
      // Convert images to data URIs
      // Images are already base64 encoded in MHTML
      bool is_base64 = (part.transfer_encoding == "base64");
      std::string data_uri = CreateDataURI(part.content_type, part.data, is_base64);
      
      // Replace references
      if (!part.content_location.empty()) {
        // Replace src="URL" with src="data:..."
        base::ReplaceSubstringsAfterOffset(&html, 0, part.content_location, data_uri);
        // LOG(INFO) << "Kartik: Replaced image URL: " << part.content_location;
      }
      
      if (!part.content_id.empty()) {
        std::string cid = part.content_id;
        if (cid.front() == '<')
          cid = cid.substr(1);
        if (cid.back() == '>')
          cid = cid.substr(0, cid.length() - 1);
        
        std::string cid_ref = "cid:" + cid;
        base::ReplaceSubstringsAfterOffset(&html, 0, cid_ref, data_uri);
        // LOG(INFO) << "Kartik: Replaced image CID: " << cid;
      }
    }
  }
  
  LOG(INFO) << "Kartik: Final HTML size: " << html.length() << " bytes";
  
  // Add scroll fixes and viewport for better mobile experience
  AddScrollFixCSS(html);
  AddViewportMetaTag(html);
  
  LOG(INFO) << "Kartik: Individual page saved with scroll fixes and viewport applied";
  
  return html;
}

// static
void WootzMHTMLToHTMLConverter::AddScrollFixCSS(std::string& html) {
  std::string scroll_fix_css = R"(
    <style>
      /* Force scrolling on all pages - fixes LinkedIn, ESPN, etc. */
      html, body {
        overflow: auto !important;
        -webkit-overflow-scrolling: touch !important;
        height: auto !important;
        position: static !important;
      }
      body {
        width: auto !important;
        min-height: 100vh !important;
      }
      /* Remove blocking iframes and overlays */
      iframe[src*="linkedin.com/app"],
      iframe[src*="install"],
      iframe[src*="download"],
      div[class*="modal-backdrop"],
      div[class*="overlay"],
      div[id*="overlay"] {
        display: none !important;
      }
      /* Ensure content is scrollable */
      #main, main, [role="main"], .content, #content {
        overflow: visible !important;
        height: auto !important;
      }
    </style>
  )";
  
  size_t head_end = html.find("</head>");
  if (head_end != std::string::npos) {
    html.insert(head_end, scroll_fix_css);
    LOG(INFO) << "Kartik: Added scroll-fix CSS before </head>";
  } else {
    // If no </head>, insert after <html> tag
    size_t html_tag = html.find("<html");
    if (html_tag != std::string::npos) {
      size_t insert_pos = html.find('>', html_tag);
      if (insert_pos != std::string::npos) {
        html.insert(insert_pos + 1, "\n<head>" + scroll_fix_css + "</head>\n");
        LOG(INFO) << "Kartik: Created <head> and added scroll-fix CSS";
      }
    }
  }
}

// static
void WootzMHTMLToHTMLConverter::AddViewportMetaTag(std::string& html) {
  // Check if viewport meta tag already exists
  if (html.find("name=\"viewport\"") != std::string::npos) {
    LOG(INFO) << "Kartik: Viewport meta tag already exists, skipping";
    return;
  }
  
  std::string viewport_meta = 
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">\n";
  
  size_t head_start = html.find("<head>");
  if (head_start != std::string::npos) {
    html.insert(head_start + 6, "\n" + viewport_meta);
    LOG(INFO) << "Kartik: Added viewport meta tag";
  } else {
    // Try to add after <html> if no <head>
    size_t html_tag = html.find("<html");
    if (html_tag != std::string::npos) {
      size_t insert_pos = html.find('>', html_tag);
      if (insert_pos != std::string::npos) {
        html.insert(insert_pos + 1, "\n<head>\n" + viewport_meta + "</head>\n");
        LOG(INFO) << "Kartik: Created <head> and added viewport meta tag";
      }
    }
  }
}

}  // namespace wootz_offline_pages
