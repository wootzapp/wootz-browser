// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Download code which handles CRX files (extensions, themes, apps, ...).

#include "chrome/browser/download/download_crx_util.h"

#include "extensions/buildflags/buildflags.h"

#include "base/strings/string_util.h"
#include "components/download/public/common/download_item.h"
#include "content/public/browser/download_item_utils.h"
#include "content/public/browser/web_contents.h"
#include "extensions/common/extension.h"
#include "net/http/http_response_headers.h"

// This file is used on non-desktop Android where extensions are not supported.
// static_assert(!BUILDFLAG(ENABLE_EXTENSIONS_CORE));

namespace download_crx_util {

bool IsExtensionDownload(const download::DownloadItem& download_item) {
  std::string content_disposition = download_item.GetContentDisposition();
  LOG(INFO) << "IsExtensionDownload checking:";
  LOG(INFO) << "  Content disposition: " << content_disposition;

  std::string url = download_item.GetURL().spec();
  LOG(INFO) << "  Download URL: " << url;

  // Only allow downloads from the trusted extension store
  content::WebContents* web_contents =
      content::DownloadItemUtils::GetWebContents(&download_item);
  if (!web_contents) {
    LOG(INFO) << "No web contents found, rejecting CRX";
    return false;
  }

  GURL page_url = web_contents->GetLastCommittedURL();
  LOG(INFO) << "  Page URL: " << page_url.spec();

  // Check if the page URL is from the trusted source
  const char* TRUSTED_SOURCE = "wootzapp://flow-store/";
  const char* TRUSTED_SOURCE_2 = "wootzapp://startup-crx-install/"; 
  if (page_url.spec() != TRUSTED_SOURCE && page_url.spec() != TRUSTED_SOURCE_2) {
    LOG(INFO) << "Not from trusted source, rejecting CRX";
    return false;
  }
  
  // Verify it's a CRX file from the download URL, not the page URL
  if (base::EndsWith(url, ".crx", base::CompareCase::INSENSITIVE_ASCII)) {
    LOG(INFO) << "Detected trusted CRX download";
    return true;
  }

  LOG(INFO) << "Not a valid CRX file path";
  return false;
}

}  // namespace download_crx_util
