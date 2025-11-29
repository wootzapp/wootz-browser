// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/wootz_offline_pages/wootz_offline_page_service.h"

#include <set>

#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/files/file_enumerator.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/hash/sha1.h"
#include "base/json/json_writer.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "base/values.h"
#include "chrome/browser/wootz_offline_pages/wootz_mhtml_to_html_converter.h"
#include "chrome/common/chrome_paths.h"
#include "content/public/common/mhtml_generation_params.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/filename_util.h"

// Generated JNI headers
#include "chrome/android/chrome_jni_headers/WootzOfflinePagePathUtils_jni.h"

WootzOfflinePageService::WootzOfflinePageService() {
  // Ensure 404 page exists when service is created
  EnsureOffline404PageExists();
}

WootzOfflinePageService::~WootzOfflinePageService() = default;

void WootzOfflinePageService::SavePage(
    content::WebContents* web_contents,
    const std::vector<GURL>& redirect_chain) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  GURL url = web_contents->GetLastCommittedURL();
  if (!url.is_valid() || !url.SchemeIsHTTPOrHTTPS())
    return;

  LOG(INFO) << "Kartik: WootzOfflinePageService::SavePage for " << url.spec();
  
  // Generate temporary MHTML path (will be converted to HTML)
  base::FilePath html_path = URLToFilePath(url);  // This returns .html path
  base::FilePath mhtml_temp_path = html_path.RemoveExtension().AddExtension(FILE_PATH_LITERAL("tmp"));
  
  // Verify storage directory exists and is writable
  base::FilePath storage_dir = html_path.DirName();
  if (!base::DirectoryExists(storage_dir)) {
    LOG(ERROR) << "Kartik: Storage directory does not exist: " << storage_dir.value();
    return;
  }
  
  // Check if we have write permissions
  if (!base::PathIsWritable(storage_dir)) {
    LOG(ERROR) << "Kartik: Storage directory is not writable: " << storage_dir.value();
    return;
  }
  
  LOG(INFO) << "Kartik: Starting MHTML generation to: " << mhtml_temp_path.value();
  
  content::MHTMLGenerationParams params(mhtml_temp_path);
  // Set cache control to use cached resources (faster, more reliable)
  params.use_binary_encoding = true;  // More efficient encoding
  params.remove_popup_overlay = true;  // Remove popups for cleaner MHTML
  
  web_contents->GenerateMHTML(
      params, base::BindOnce(&WootzOfflinePageService::OnMHTMLGenerated,
                             weak_factory_.GetWeakPtr(), redirect_chain,
                             mhtml_temp_path));
}

void WootzOfflinePageService::OnMHTMLGenerated(
    const std::vector<GURL>& redirect_chain,
    const base::FilePath& file_path,
    int64_t size) {
  if (size <= 0) {
    LOG(ERROR) << "Kartik: MHTML generation failed for "
               << redirect_chain.back().spec()
               << " - size: " << size 
               << " (this usually means the renderer process crashed or timed out)";
    
    // Check if file was partially created
    if (base::PathExists(file_path)) {
      int64_t actual_size = 0;
      if (base::GetFileSize(file_path, &actual_size)) {
        LOG(ERROR) << "Kartik: Partial file exists with size: " << actual_size << " bytes - deleting it";
      }
      base::DeleteFile(file_path);
    } else {
      LOG(ERROR) << "Kartik: Output file was not created at all: " << file_path.value();
    }
    return;
  }
  
  LOG(INFO) << "Kartik: MHTML generated successfully for " 
            << redirect_chain.back().spec()
            << " - file: " << file_path.value()
            << " - size: " << size << " bytes";
  
  // Convert MHTML to HTML for full JavaScript functionality
  // file_path is hash.tmp, so RemoveExtension gives hash, then add .html
  base::FilePath html_path = file_path.RemoveExtension().AddExtension(FILE_PATH_LITERAL("html"));
  
  LOG(INFO) << "Kartik: Converting MHTML to HTML: " << file_path.value() 
            << " -> " << html_path.value();
  
  bool conversion_success = wootz_offline_pages::WootzMHTMLToHTMLConverter::Convert(
      file_path, html_path);
  
  if (conversion_success) {
    LOG(INFO) << "Kartik: Successfully converted to HTML, deleting temporary MHTML file";
    
    // Delete the temporary MHTML file - we only keep the HTML
    if (!base::DeleteFile(file_path)) {
      LOG(WARNING) << "Kartik: Failed to delete temporary MHTML file: " << file_path.value();
    }
    
    // Get the actual file size after conversion
    int64_t html_size = 0;
    base::GetFileSize(html_path, &html_size);
    
    LOG(INFO) << "Kartik: Offline page saved as HTML: " << html_path.value()
              << " (size: " << html_size << " bytes)";
    
    // Create manifest.json with redirect chain and metadata
    CreateManifest(html_path, redirect_chain, html_size);
    
    // CRITICAL: Save the same HTML for ALL URLs in the redirect chain
    // This ensures that accessing any intermediate redirect URL will find the page
    if (redirect_chain.size() > 1) {
      LOG(INFO) << "Kartik: Creating copies for " << (redirect_chain.size() - 1) 
                << " redirect URLs in the chain";
      
      // The last URL in redirect_chain is the final destination (already saved above)
      // Create copies for all intermediate redirect URLs
      for (size_t i = 0; i < redirect_chain.size() - 1; i++) {
        const GURL& redirect_url = redirect_chain[i];
        base::FilePath redirect_path = URLToFilePath(redirect_url);
        
        // Copy the HTML file to the redirect URL's path
        if (base::CopyFile(html_path, redirect_path)) {
          LOG(INFO) << "Kartik: Saved redirect URL: " << redirect_url.spec()
                    << " -> " << redirect_path.value();
          
          // Also copy the manifest for each redirect URL
          base::FilePath redirect_manifest = GetManifestPath(redirect_path);
          base::FilePath source_manifest = GetManifestPath(html_path);
          if (base::CopyFile(source_manifest, redirect_manifest)) {
            LOG(INFO) << "Kartik: Copied manifest for redirect URL";
          }
        } else {
          LOG(ERROR) << "Kartik: Failed to save redirect URL: " << redirect_url.spec();
        }
      }
    }
  } else {
    LOG(ERROR) << "Kartik: Failed to convert MHTML to HTML, keeping MHTML file as fallback";
    // Keep the MHTML file if conversion fails
  }
}

bool WootzOfflinePageService::GetOfflinePagePath(const GURL& url,
                                                 base::FilePath* path) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  *path = URLToFilePath(url);
  
  // First, check if the file exists at the direct path
  if (base::PathExists(*path)) {
    LOG(INFO) << "Kartik: Found offline page at direct path: " << path->value();
    return true;
  }
  
  // Second, search manifest files for URL mappings (handles redirects efficiently)
  LOG(INFO) << "Kartik: File not found at direct path, searching manifests...";
  if (FindPagePathFromManifest(url, path)) {
    LOG(INFO) << "Kartik: Found page via manifest: " << path->value();
    return true;
  }
  
  // If not found in the main directory, search subdirectories
  // This handles cases where users copy exported directories back
  LOG(INFO) << "Kartik: Not found in manifests, searching subdirectories...";
  
  // Extract just the filename from the full path
  std::string filename = path->BaseName().AsUTF8Unsafe();
  
  JNIEnv* env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> j_filename =
      base::android::ConvertUTF8ToJavaString(env, filename);
  
  base::android::ScopedJavaLocalRef<jstring> j_found_path =
      wootz_offline_pages::Java_WootzOfflinePagePathUtils_findOfflinePageInSubdirectories(
          env, j_filename);
  
  if (!j_found_path.is_null()) {
    std::string found_path = base::android::ConvertJavaStringToUTF8(env, j_found_path);
    *path = base::FilePath(found_path);
    LOG(INFO) << "Kartik: Found offline page in subdirectory: " << found_path;
    return true;
  }
  
  LOG(INFO) << "Kartik: Offline page not found anywhere";
  return false;
}

bool WootzOfflinePageService::ClearAllPages() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  base::FilePath storage_dir = GetStorageDir();
  
  if (!base::DirectoryExists(storage_dir)) {
    LOG(INFO) << "Kartik: Storage directory does not exist, nothing to clear";
    return true;
  }
  
  // Delete all contents of the storage directory
  bool success = base::DeletePathRecursively(storage_dir);
  
  if (success) {
    LOG(INFO) << "Kartik: Successfully cleared all offline pages from " 
              << storage_dir.value();
    // Recreate the directory for future use
    base::CreateDirectory(storage_dir);
  } else {
    LOG(ERROR) << "Kartik: Failed to clear offline pages from " 
               << storage_dir.value();
  }
  
  return success;
}

base::FilePath WootzOfflinePageService::GetStorageDir() {
  // Get external storage path from Java
  JNIEnv* env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> j_path =
      wootz_offline_pages::Java_WootzOfflinePagePathUtils_getOfflinePageStoragePath(env);
  
  if (j_path.is_null()) {
    LOG(ERROR) << "Kartik: Failed to get external storage path from Java, "
               << "falling back to internal storage";
    // Fallback to internal storage if external is unavailable
    base::FilePath path;
    base::PathService::Get(chrome::DIR_USER_DATA, &path);
    path = path.Append(FILE_PATH_LITERAL("WootzOfflinePages"));
    if (!base::DirectoryExists(path))
      base::CreateDirectory(path);
    return path;
  }
  
  std::string path_str = base::android::ConvertJavaStringToUTF8(env, j_path);
  base::FilePath path(path_str);
  
  // Ensure directory exists
  if (!base::DirectoryExists(path)) {
    if (!base::CreateDirectory(path)) {
      LOG(ERROR) << "Kartik: Failed to create external storage directory: " 
                 << path.value();
    }
  }
  
  LOG(INFO) << "Kartik: Using external storage for offline pages: " << path.value();
  return path;
}

GURL WootzOfflinePageService::CanonicalizeUrlForOfflinePage(const GURL& url) {
  if (!url.is_valid() || !url.SchemeIsHTTPOrHTTPS()) {
    return url;
  }
  
  std::string path = url.path();
  
  // Step 1: Check if this is a search page (any site)
  // Search pages include: /search, /results, /find, etc.
  bool is_search_page = (path.find("/search") != std::string::npos ||
                         path.find("/results") != std::string::npos ||
                         path.find("/find") != std::string::npos);
  
  // Also check query parameters for common search indicators
  if (url.has_query() && !is_search_page) {
    std::string query = url.query();
    is_search_page = (query.find("q=") != std::string::npos ||
                     query.find("query=") != std::string::npos ||
                     query.find("search=") != std::string::npos ||
                     query.find("keyword=") != std::string::npos);
  }
  
  GURL::Replacements replacements;
  
  if (is_search_page) {
    // Keep query parameters for search pages - different searches are different pages
    // Only strip fragment/anchor
    replacements.ClearRef();
  } else {
    // Strip query parameters AND fragment for non-search pages
    // Example: /home?trk=a#section and /home?trk=b#other -> /home (same page)
    replacements.ClearQuery();
    replacements.ClearRef();
  }
  
  GURL canonical = url.ReplaceComponents(replacements);
  
  // Step 2: Normalize trailing slash AFTER stripping query params
  // This ensures consistent hashing whether URL has query params or not
  // Example: /home and /home?trk=a both become /home (no slash)
  // Example: /jobs/ and /jobs/?trk=a both become /jobs/ (with slash)
  std::string canonical_path = canonical.path();
  
  // Don't modify if path is empty, is root (/), or already ends with slash
  // Don't add slash if path looks like a file (has extension)
  bool looks_like_file = (canonical_path.find_last_of('.') != std::string::npos &&
                          canonical_path.find_last_of('.') > canonical_path.find_last_of('/'));
  
  // For consistency: if path doesn't end with / and is not a file, DON'T add slash
  // This matches most saved URLs which typically don't have trailing slashes
  // Just return as-is after query/fragment stripping
  
  return canonical;
}

base::FilePath WootzOfflinePageService::URLToFilePath(const GURL& url) {
  // Use canonical URL for consistent file naming
  GURL canonical = CanonicalizeUrlForOfflinePage(url);
  
  std::string hash = base::SHA1HashString(canonical.spec());
  std::string hex_hash = base::HexEncode(hash.c_str(), hash.length());
  // Changed from .mhtml to .html since we now convert and save as HTML
  std::string filename = "wootz_offline_" + hex_hash + ".html";
  
  LOG(INFO) << "Kartik: URL canonicalization: " << url.spec() 
            << " -> " << canonical.spec() << " -> " << filename;
  
  return GetStorageDir().Append(base::FilePath::FromUTF8Unsafe(filename));
}

base::FilePath WootzOfflinePageService::Get404PagePath() {
  JNIEnv* env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> j_path =
      wootz_offline_pages::Java_WootzOfflinePagePathUtils_get404PagePath(env);
  
  if (j_path.is_null()) {
    LOG(ERROR) << "Kartik: Failed to get 404 page path from Java";
    return base::FilePath();
  }
  
  std::string path_str = base::android::ConvertJavaStringToUTF8(env, j_path);
  return base::FilePath(path_str);
}

bool WootzOfflinePageService::Does404PageExist() {
  JNIEnv* env = base::android::AttachCurrentThread();
  return wootz_offline_pages::Java_WootzOfflinePagePathUtils_does404PageExist(env);
}

void WootzOfflinePageService::EnsureOffline404PageExists() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  
  // Check if 404 page already exists
  if (Does404PageExist()) {
    LOG(INFO) << "Kartik: 404 page already exists, skipping creation";
    return;
  }
  
  LOG(INFO) << "Kartik: Creating offline 404 page";
  
  // Generate HTML content for the 404 page (no longer using MHTML)
  std::string html_content = 
      "<!DOCTYPE html>\n"
      "<html>\n"
      "<head>\n"
      "    <meta charset=\"utf-8\">\n"
      "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
      "    <title>Page Not Available Offline</title>\n"
      "    <style>\n"
      "        html, body {\n"
      "            height: 100vh;\n"
      "            width: 100vw;\n"
      "            margin: 0;\n"
      "            padding: 0;\n"
      "            overflow: hidden;\n"
      "        }\n"
      "        body {\n"
      "            font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto, sans-serif;\n"
      "            display: flex;\n"
      "            align-items: center;\n"
      "            justify-content: center;\n"
      "            background: #f5f5f5;\n"
      "        }\n"
      "        .container {\n"
      "            text-align: center;\n"
      "            max-width: 450px;\n"
      "            background: white;\n"
      "            padding: 24px 20px;\n"
      "            border-radius: 12px;\n"
      "            box-shadow: 0 2px 10px rgba(0,0,0,0.1);\n"
      "            margin: 0 16px;\n"
      "        }\n"
      "        .icon { font-size: 64px; margin: 0 0 16px 0; }\n"
      "        h2 { font-size: 22px; margin: 0 0 12px 0; color: #333; }\n"
      "        p { color: #666; line-height: 1.5; margin: 0 0 16px 0; font-size: 14px; }\n"
      "        .info {\n"
      "            background: #e3f2fd;\n"
      "            padding: 12px;\n"
      "            border-radius: 8px;\n"
      "            margin: 16px 0;\n"
      "            color: #1976d2;\n"
      "            font-size: 13px;\n"
      "            line-height: 1.4;\n"
      "            text-align: left;\n"
      "        }\n"
      "    </style>\n"
      "</head>\n"
      "<body>\n"
      "    <div class=\"container\">\n"
      "        <div class=\"icon\">📭</div>\n"
      "        <h2>Page Not Available Offline</h2>\n"
      "        <p>This page hasn't been saved for offline viewing yet.</p>\n"
      "        <div class=\"info\">\n"
      "            <strong>Tip:</strong> To view this page offline, visit it while online "
      "with auto-save enabled. The page will be automatically saved for future "
      "offline access.\n"
      "        </div>\n"
      "        <p style=\"color: #999; font-size: 11px; margin-top: 16px; margin-bottom: 0;\">\n"
      "            Wootz Offline Pages\n"
      "        </p>\n"
      "    </div>\n"
      "</body>\n"
      "</html>\n";
  
  // Call Java method to create the file
  JNIEnv* env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> j_content =
      base::android::ConvertUTF8ToJavaString(env, html_content);
  
  bool success = wootz_offline_pages::Java_WootzOfflinePagePathUtils_create404Page(
      env, j_content);
  
  if (success) {
    LOG(INFO) << "Kartik: Successfully created offline 404 page as HTML";
  } else {
    LOG(ERROR) << "Kartik: Failed to create offline 404 page";
  }
}

base::FilePath WootzOfflinePageService::GetNoResultFoundPagePath() {
  JNIEnv* env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> j_path =
      wootz_offline_pages::Java_WootzOfflinePagePathUtils_getNoResultFoundPagePath(env);
  
  if (j_path.is_null()) {
    LOG(ERROR) << "Kartik: Failed to get no result found page path from Java";
    return base::FilePath();
  }
  
  std::string path_str = base::android::ConvertJavaStringToUTF8(env, j_path);
  return base::FilePath(path_str);
}

bool WootzOfflinePageService::DoesNoResultFoundPageExist() {
  JNIEnv* env = base::android::AttachCurrentThread();
  return wootz_offline_pages::Java_WootzOfflinePagePathUtils_doesNoResultFoundPageExist(env);
}

void WootzOfflinePageService::CreateManifest(
    const base::FilePath& html_path,
    const std::vector<GURL>& redirect_chain,
    int64_t size) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  
  if (redirect_chain.empty()) {
    LOG(WARNING) << "Kartik: Cannot create manifest with empty redirect chain";
    return;
  }
  
  LOG(INFO) << "Kartik: Creating manifest for " << html_path.value();
  
  // Build manifest JSON
  base::Value::Dict manifest;
  
  // Basic metadata
  std::string html_filename = html_path.BaseName().AsUTF8Unsafe();
  manifest.Set("page_id", html_filename);
  
  // Format timestamp as ISO 8601
  base::Time now = base::Time::Now();
  base::Time::Exploded exploded;
  now.UTCExplode(&exploded);
  std::string timestamp = base::StringPrintf(
      "%04d-%02d-%02dT%02d:%02d:%02dZ",
      exploded.year, exploded.month, exploded.day_of_month,
      exploded.hour, exploded.minute, exploded.second);
  
  manifest.Set("saved_at", timestamp);
  manifest.Set("primary_url", redirect_chain.back().spec());
  manifest.Set("size_bytes", static_cast<int>(size));
  manifest.Set("version", 1);
  
  // Store canonical primary URL for reference
  GURL canonical_primary = CanonicalizeUrlForOfflinePage(redirect_chain.back());
  manifest.Set("canonical_url", canonical_primary.spec());
  
  // Redirect chain (all intermediate URLs with original query parameters)
  base::Value::List redirect_list;
  for (const GURL& url : redirect_chain) {
    redirect_list.Append(url.spec());
  }
  manifest.Set("redirect_chain", std::move(redirect_list));
  
  // TRIPLE MAPPING: Store original URLs + canonical URLs + subdomain variants
  // This allows lookup by:
  // 1. Exact URL match (with query params)
  // 2. Canonical URL (without query params for non-search pages)
  // 3. Subdomain variants (www.linkedin.com <-> in.linkedin.com)
  base::Value::Dict url_mappings;
  std::set<std::string> processed_urls; // Avoid duplicate entries
  
  for (const GURL& url : redirect_chain) {
    // 1. Add original URL with all query parameters
    std::string original_spec = url.spec();
    if (processed_urls.find(original_spec) == processed_urls.end()) {
      url_mappings.Set(original_spec, html_filename);
      processed_urls.insert(original_spec);
      LOG(INFO) << "Kartik: Manifest mapping: " << original_spec;
    }
    
    // 2. Add canonical URL (normalized - without query params for non-search pages)
    GURL canonical = CanonicalizeUrlForOfflinePage(url);
    std::string canonical_spec = canonical.spec();
    if (processed_urls.find(canonical_spec) == processed_urls.end()) {
      url_mappings.Set(canonical_spec, html_filename);
      processed_urls.insert(canonical_spec);
      if (canonical_spec != original_spec) {
        LOG(INFO) << "Kartik: Manifest mapping (canonical): " << canonical_spec;
      }
    }
    
    // 3. Add subdomain variants (www, in, m, mobile, etc.)
    // This ensures in.linkedin.com and www.linkedin.com can find each other's pages
    std::vector<GURL> subdomain_variants = GenerateSubdomainVariants(canonical);
    for (const GURL& variant : subdomain_variants) {
      std::string variant_spec = variant.spec();
      if (processed_urls.find(variant_spec) == processed_urls.end()) {
        url_mappings.Set(variant_spec, html_filename);
        processed_urls.insert(variant_spec);
        if (variant_spec != canonical_spec && variant_spec != original_spec) {
          LOG(INFO) << "Kartik: Manifest mapping (subdomain variant): " << variant_spec;
        }
      }
    }
  }
  manifest.Set("url_mappings", std::move(url_mappings));
  
  // File info
  base::Value::Dict files;
  files.Set("html", html_filename);
  manifest.Set("files", std::move(files));
  
  // Metadata
  base::Value::Dict metadata;
  metadata.Set("has_scroll_fix", true);
  metadata.Set("has_viewport", true);
  metadata.Set("format", "html");
  manifest.Set("metadata", std::move(metadata));
  
  // Convert to JSON string
  std::string json_string;
  if (!base::JSONWriter::WriteWithOptions(
          manifest, base::JSONWriter::OPTIONS_PRETTY_PRINT, &json_string)) {
    LOG(ERROR) << "Kartik: Failed to serialize manifest JSON";
    return;
  }
  
  // Write to file
  base::FilePath manifest_path = GetManifestPath(html_path);
  if (base::WriteFile(manifest_path, json_string)) {
    LOG(INFO) << "Kartik: Manifest created: " << manifest_path.value()
              << " (tracking " << redirect_chain.size() << " URLs)";
  } else {
    LOG(ERROR) << "Kartik: Failed to write manifest file: " << manifest_path.value();
  }
}

bool WootzOfflinePageService::FindPagePathFromManifest(const GURL& url,
                                                       base::FilePath* path) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  
  base::FilePath storage_dir = GetStorageDir();
  if (!base::DirectoryExists(storage_dir)) {
    return false;
  }
  
  // Get all .json files in the storage directory
  base::FileEnumerator enumerator(storage_dir, false,
                                  base::FileEnumerator::FILES,
                                  FILE_PATH_LITERAL("*.json"));
  
  // Prepare both original and canonical URLs for lookup
  std::string original_url_spec = url.spec();
  GURL canonical_url = CanonicalizeUrlForOfflinePage(url);
  std::string canonical_url_spec = canonical_url.spec();
  
  LOG(INFO) << "Kartik: Searching manifests for: " << original_url_spec;
  if (original_url_spec != canonical_url_spec) {
    LOG(INFO) << "Kartik: Also trying canonical: " << canonical_url_spec;
  }
  
  for (base::FilePath manifest_path = enumerator.Next(); 
       !manifest_path.empty();
       manifest_path = enumerator.Next()) {
    
    // Skip special page manifests (404, no result found, etc.)
    std::string manifest_name = manifest_path.BaseName().AsUTF8Unsafe();
    if (manifest_name.find("404") != std::string::npos ||
        manifest_name.find("no_result") != std::string::npos) {
      continue;
    }
    
    // Read manifest file
    std::string manifest_content;
    if (!base::ReadFileToString(manifest_path, &manifest_content)) {
      continue;
    }
    
    // Parse JSON
    auto parsed = base::JSONReader::Read(manifest_content);
    if (!parsed || !parsed->is_dict()) {
      continue;
    }
    
    const base::Value::Dict& manifest = parsed->GetDict();
    
    // Check url_mappings for this URL
    const base::Value::Dict* url_mappings = manifest.FindDict("url_mappings");
    if (!url_mappings) {
      continue;
    }
    
    // TRIPLE LOOKUP: Try original URL, then canonical URL, then subdomain variants
    const std::string* html_filename = url_mappings->FindString(original_url_spec);
    std::string match_type = "original";
    
    if (!html_filename || html_filename->empty()) {
      // Try canonical URL if original not found
      html_filename = url_mappings->FindString(canonical_url_spec);
      match_type = "canonical";
    }
    
    if (!html_filename || html_filename->empty()) {
      // Try subdomain variants as last resort
      // This handles in.linkedin.com <-> www.linkedin.com matching
      std::vector<GURL> subdomain_variants = GenerateSubdomainVariants(canonical_url);
      for (const GURL& variant : subdomain_variants) {
        std::string variant_spec = variant.spec();
        if (variant_spec == original_url_spec || variant_spec == canonical_url_spec) {
          continue; // Already tried these
        }
        html_filename = url_mappings->FindString(variant_spec);
        if (html_filename && !html_filename->empty()) {
          match_type = "subdomain";
          LOG(INFO) << "Kartik: Subdomain variant matched: " << variant_spec;
          break;
        }
      }
    }
    
    if (html_filename && !html_filename->empty()) {
      // Found it! Return the HTML file path
      *path = storage_dir.Append(base::FilePath::FromUTF8Unsafe(*html_filename));
      
      if (base::PathExists(*path)) {
        LOG(INFO) << "Kartik: Found URL in manifest (" << match_type << " match): "
                  << manifest_path.value() << " -> " << *html_filename;
        return true;
      } else {
        LOG(WARNING) << "Kartik: Manifest points to non-existent file: " << *html_filename;
      }
    }
  }
  
  LOG(INFO) << "Kartik: URL not found in any manifest";
  return false;
}

base::FilePath WootzOfflinePageService::GetManifestPath(
    const base::FilePath& html_path) {
  return html_path.ReplaceExtension(FILE_PATH_LITERAL("json"));
}

std::vector<GURL> WootzOfflinePageService::GenerateSubdomainVariants(const GURL& url) {
  std::vector<GURL> variants;
  
  if (!url.is_valid() || !url.has_host()) {
    variants.push_back(url);
    return variants;
  }
  
  std::string host = url.host();
  
  // Common subdomain prefixes to generate variants for
  std::vector<std::string> common_subdomains = {"www", "m", "mobile", "in"};
  
  // Always add the original URL first
  variants.push_back(url);
  
  // Extract base domain (e.g., linkedin.com from www.linkedin.com)
  size_t first_dot = host.find('.');
  if (first_dot == std::string::npos) {
    // No subdomain, just domain.tld - return as-is
    return variants;
  }
  
  // Check if this already has a subdomain
  std::string potential_subdomain = host.substr(0, first_dot);
  std::string base_domain = host.substr(first_dot + 1);
  
  // If base domain doesn't have a dot, this is likely domain.tld (no subdomain)
  if (base_domain.find('.') == std::string::npos) {
    return variants;
  }
  
  // Generate variants by swapping subdomains
  for (const std::string& subdomain : common_subdomains) {
    if (subdomain == potential_subdomain) {
      continue; // Skip if it's the same as current
    }
    
    std::string variant_host = subdomain + "." + base_domain;
    GURL::Replacements replacements;
    replacements.SetHostStr(variant_host);
    GURL variant_url = url.ReplaceComponents(replacements);
    
    if (variant_url.is_valid()) {
      variants.push_back(variant_url);
    }
  }
  
  // Also add base domain without subdomain (e.g., linkedin.com)
  GURL::Replacements replacements;
  replacements.SetHostStr(base_domain);
  GURL base_url = url.ReplaceComponents(replacements);
  if (base_url.is_valid()) {
    variants.push_back(base_url);
  }
  
  return variants;
}
