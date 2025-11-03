// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/wootz_offline_pages/wootz_offline_page_service.h"

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
      base::CreateSymbolicLink(file_path, symlink_path);
    }
  }
}

bool WootzOfflinePageService::GetOfflinePagePath(const GURL& url,
                                                 base::FilePath* path) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  *path = URLToFilePath(url);
  return base::PathExists(*path);
}

base::FilePath WootzOfflinePageService::GetStorageDir() {
  base::FilePath path;
  base::PathService::Get(chrome::DIR_USER_DATA, &path);
  path = path.Append(FILE_PATH_LITERAL("WootzOfflinePages"));
  if (!base::DirectoryExists(path))
    base::CreateDirectory(path);
  return path;
}

base::FilePath WootzOfflinePageService::URLToFilePath(const GURL& url) {
  std::string hash = base::SHA1HashString(url.spec());
  std::string hex_hash = base::HexEncode(hash.c_str(), hash.length());
  std::string filename = "wootz_offline_" + hex_hash + ".mhtml";
  return GetStorageDir().Append(base::FilePath::FromUTF8Unsafe(filename));
}
