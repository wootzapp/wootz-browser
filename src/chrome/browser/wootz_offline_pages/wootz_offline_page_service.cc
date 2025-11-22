// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/wootz_offline_pages/wootz_offline_page_service.h"

#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/hash/sha1.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "chrome/common/chrome_paths.h"
#include "content/public/common/mhtml_generation_params.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/filename_util.h"

// Generated JNI headers
#include "chrome/android/chrome_jni_headers/WootzOfflinePagePathUtils_jni.h"

WootzOfflinePageService::WootzOfflinePageService() = default;
WootzOfflinePageService::~WootzOfflinePageService() = default;

void WootzOfflinePageService::SavePage(
    content::WebContents* web_contents,
    const std::vector<GURL>& redirect_chain) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  GURL url = web_contents->GetLastCommittedURL();
  if (!url.is_valid() || !url.SchemeIsHTTPOrHTTPS())
    return;

  LOG(INFO) << "Kartik: WootzOfflinePageService::SavePage for " << url.spec();
  
  base::FilePath file_path = URLToFilePath(url);
  
  // Verify storage directory exists and is writable
  base::FilePath storage_dir = file_path.DirName();
  if (!base::DirectoryExists(storage_dir)) {
    LOG(ERROR) << "Kartik: Storage directory does not exist: " << storage_dir.value();
    return;
  }
  
  // Check if we have write permissions
  if (!base::PathIsWritable(storage_dir)) {
    LOG(ERROR) << "Kartik: Storage directory is not writable: " << storage_dir.value();
    return;
  }
  
  LOG(INFO) << "Kartik: Starting MHTML generation to: " << file_path.value();
  
  content::MHTMLGenerationParams params(file_path);
  // Set cache control to use cached resources (faster, more reliable)
  params.use_binary_encoding = true;  // More efficient encoding
  params.remove_popup_overlay = true;  // Remove popups for cleaner MHTML
  
  web_contents->GenerateMHTML(
      params, base::BindOnce(&WootzOfflinePageService::OnMHTMLGenerated,
                             weak_factory_.GetWeakPtr(), redirect_chain,
                             file_path));
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
  
  LOG(INFO) << "Kartik: MHTML saved successfully for " 
            << redirect_chain.back().spec()
            << " - file: " << file_path.value()
            << " - size: " << size << " bytes";
}

bool WootzOfflinePageService::GetOfflinePagePath(const GURL& url,
                                                 base::FilePath* path) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  *path = URLToFilePath(url);
  
  // First, check if the file exists at the direct path
  if (base::PathExists(*path)) {
    return true;
  }
  
  // If not found in the main directory, search subdirectories
  // This handles cases where users copy exported directories back
  LOG(INFO) << "Kartik: File not found at direct path, searching subdirectories...";
  
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
  
  LOG(INFO) << "Kartik: Offline page not found in main directory or subdirectories";
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

base::FilePath WootzOfflinePageService::URLToFilePath(const GURL& url) {
  std::string hash = base::SHA1HashString(url.spec());
  std::string hex_hash = base::HexEncode(hash.c_str(), hash.length());
  std::string filename = "wootz_offline_" + hex_hash + ".mhtml";
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
  
  // Generate MHTML content for the 404 page
  std::string mhtml_content = 
      "From: <Saved by Wootz Offline Pages>\r\n"
      "Subject: Page Not Available Offline\r\n"
      "Date: Mon, 01 Jan 2024 00:00:00 GMT\r\n"
      "MIME-Version: 1.0\r\n"
      "Content-Type: multipart/related;\r\n"
      "\ttype=\"text/html\";\r\n"
      "\tboundary=\"----MultipartBoundary--wootz404page----\"\r\n"
      "\r\n"
      "------MultipartBoundary--wootz404page----\r\n"
      "Content-Type: text/html\r\n"
      "Content-Transfer-Encoding: quoted-printable\r\n"
      "Content-Location: about:blank\r\n"
      "\r\n"
      "<!DOCTYPE html>\r\n"
      "<html>\r\n"
      "<head>\r\n"
      "    <meta charset=3D\"utf-8\">\r\n"
      "    <meta name=3D\"viewport\" content=3D\"width=3Ddevice-width, initial-scale=3D1.0\">\r\n"
      "    <title>Page Not Available Offline</title>\r\n"
      "    <style>\r\n"
      "        html, body {\r\n"
      "            height: 100vh;\r\n"
      "            width: 100vw;\r\n"
      "            margin: 0;\r\n"
      "            padding: 0;\r\n"
      "            overflow: hidden;\r\n"
      "        }\r\n"
      "        body {\r\n"
      "            font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto, sans-serif;\r\n"
      "            display: flex;\r\n"
      "            align-items: center;\r\n"
      "            justify-content: center;\r\n"
      "            background: #f5f5f5;\r\n"
      "        }\r\n"
      "        .container {\r\n"
      "            text-align: center;\r\n"
      "            max-width: 450px;\r\n"
      "            background: white;\r\n"
      "            padding: 24px 20px;\r\n"
      "            border-radius: 12px;\r\n"
      "            box-shadow: 0 2px 10px rgba(0,0,0,0.1);\r\n"
      "            margin: 0 16px;\r\n"
      "        }\r\n"
      "        .icon { font-size: 64px; margin: 0 0 16px 0; }\r\n"
      "        h2 { font-size: 22px; margin: 0 0 12px 0; color: #333; }\r\n"
      "        p { color: #666; line-height: 1.5; margin: 0 0 16px 0; font-size: 14px; }\r\n"
      "        .info {\r\n"
      "            background: #e3f2fd;\r\n"
      "            padding: 12px;\r\n"
      "            border-radius: 8px;\r\n"
      "            margin: 16px 0;\r\n"
      "            color: #1976d2;\r\n"
      "            font-size: 13px;\r\n"
      "            line-height: 1.4;\r\n"
      "        }\r\n"
      "    </style>\r\n"
      "</head>\r\n"
      "<body>\r\n"
      "    <div class=3D\"container\">\r\n"
      "        <div class=3D\"icon\">=F0=9F=93=AD</div>\r\n"
      "        <h2>Page Not Available Offline</h2>\r\n"
      "        <p>This page hasn't been saved for offline viewing yet.</p>\r\n"
      "        <div class=3D\"info\">\r\n"
      "            <strong>Tip:</strong> To view this page offline, visit it while online =\r\n"
      "with auto-save enabled. The page will be automatically saved for future =\r\n"
      "offline access.\r\n"
      "        </div>\r\n"
      "        <p style=3D\"color: #999; font-size: 11px; margin-top: 16px; margin-bottom: 0;\">\r\n"
      "            Wootz Offline Pages\r\n"
      "        </p>\r\n"
      "    </div>\r\n"
      "</body>\r\n"
      "</html>\r\n"
      "\r\n"
      "------MultipartBoundary--wootz404page------\r\n";
  
  // Call Java method to create the file
  JNIEnv* env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> j_content =
      base::android::ConvertUTF8ToJavaString(env, mhtml_content);
  
  bool success = wootz_offline_pages::Java_WootzOfflinePagePathUtils_create404Page(
      env, j_content);
  
  if (success) {
    LOG(INFO) << "Kartik: Successfully created offline 404 page";
  } else {
    LOG(ERROR) << "Kartik: Failed to create offline 404 page";
  }
}
