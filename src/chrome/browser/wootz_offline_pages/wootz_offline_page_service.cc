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
  content::MHTMLGenerationParams params(file_path);
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
    LOG(ERROR) << "Kartik: Failed to save MHTML for "
               << redirect_chain.back().spec();
    base::DeleteFile(file_path);
  } else {
    LOG(INFO) << "Kartik: Saved MHTML for " << redirect_chain.back().spec()
              << " to " << file_path.value();
    // Also create symlinks for the redirect chain.
    for (size_t i = 0; i < redirect_chain.size() - 1; ++i) {
      base::FilePath symlink_path = URLToFilePath(redirect_chain[i]);
      // Delete existing file/symlink to avoid creation failure
      if (base::PathExists(symlink_path)) {
        if (!base::DeleteFile(symlink_path)) {
          LOG(WARNING) << "Kartik: Failed to delete existing file at " 
                       << symlink_path.value();
          continue;
        }
      }
      // Create the symlink and log any failures
      if (!base::CreateSymbolicLink(file_path, symlink_path)) {
        LOG(WARNING) << "Kartik: Failed to create symlink from " 
                     << symlink_path.value() << " to " << file_path.value();
      }
    }
  }
}

bool WootzOfflinePageService::GetOfflinePagePath(const GURL& url,
                                                 base::FilePath* path) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  *path = URLToFilePath(url);
  return base::PathExists(*path);
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
