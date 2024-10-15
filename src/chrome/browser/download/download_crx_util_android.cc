// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Download code which handles CRX files (extensions, themes, apps, ...).

#include "chrome/browser/download/download_crx_util.h"
#include "extensions/common/extension.h"
#include "components/download/public/common/download_item.h"
#include "net/http/http_response_headers.h"

namespace download_crx_util {

bool IsExtensionDownload(const download::DownloadItem& download_item) {
    std::string content_disposition = download_item.GetContentDisposition();
    if (content_disposition.find("filename=") != std::string::npos &&
        content_disposition.find(".crx") != std::string::npos) {
        return true;
    }

    if (download_item.GetMimeType() == extensions::Extension::kMimeType) {
        return true; 
    }

    std::string url = download_item.GetURL().spec();
    if (url.ends_with(".crx")) {
        return true;
    }

    return false;
}

bool IsTrustedExtensionDownload(Profile* profile,
                                const download::DownloadItem& item) {
  // Webstore exts are not supported
  return false;
}

}  // namespace download_crx_util
