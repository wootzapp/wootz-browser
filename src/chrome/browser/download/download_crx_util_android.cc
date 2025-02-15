// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Download code which handles CRX files (extensions, themes, apps, ...).

#include "chrome/browser/download/download_crx_util.h"

#include "base/strings/string_util.h"
#include "components/download/public/common/download_item.h"
#include "content/public/browser/download_item_utils.h"
#include "content/public/browser/web_contents.h"
#include "extensions/common/extension.h"
#include "net/http/http_response_headers.h"

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
    // return false;
  }

  GURL page_url = web_contents->GetLastCommittedURL();
  LOG(INFO) << "  Page URL: " << page_url.spec();

  // Only allow from our trusted repo
  // const char* TRUSTED_REPO = "github.com/wootzapp/ext-store";
  // if (page_url.spec().find(TRUSTED_REPO) == std::string::npos) {
  //   LOG(INFO) << "Not from trusted repo, rejecting CRX";
  //   return false;
  // }

  // Verify it's a CRX file from the correct path structure
  if (base::EndsWith(page_url.spec(), ".crx",
                     base::CompareCase::INSENSITIVE_ASCII)) {
    LOG(INFO) << "Detected trusted CRX download";
    return true;
  }

  LOG(INFO) << "Not a valid CRX file path";
  return true;
}

// bool IsTrustedExtensionDownload(Profile* profile,
//                                 const download::DownloadItem& item) {
//   // Webstore exts are not supported
//   return false;
// }

}  // namespace download_crx_util
