// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Download code which handles CRX files (extensions, themes, apps, ...).

#include "chrome/browser/download/download_crx_util.h"
#include "extensions/common/extension.h"
#include "components/download/public/common/download_item.h"
#include "net/http/http_response_headers.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/download_item_utils.h"
#include "base/strings/string_util.h"

namespace download_crx_util {

bool IsExtensionDownload(const download::DownloadItem& download_item) {
    // std::string content_disposition = download_item.GetContentDisposition();
    // if (content_disposition.find("filename=") != std::string::npos &&
    //     content_disposition.find(".crx") != std::string::npos) {
    //     return true;
    // }

    // if (download_item.GetMimeType() == extensions::Extension::kMimeType) {
    //     return true; 
    // }

    // std::string url = download_item.GetURL().spec();
    // if (url.ends_with(".crx")) {
    //     return true;
    // }

    // return false;


    std::string content_disposition = download_item.GetContentDisposition();
    LOG(INFO) << "IsExtensionDownload checking:";
    LOG(INFO) << "  Content disposition: " << content_disposition;
    
    std::string url = download_item.GetURL().spec();
    LOG(INFO) << "  Download URL: " << url;
    
    // Check the page URL where download was initiated
    content::WebContents* web_contents = 
        content::DownloadItemUtils::GetWebContents(&download_item);
    if (web_contents) {
        GURL page_url = web_contents->GetLastCommittedURL();
        LOG(INFO) << "  Page URL: " << page_url.spec();
        
        // Check if from trusted repo
        const char* TRUSTED_REPO = "github.com/wootzapp/ext-store";
        if (page_url.spec().find(TRUSTED_REPO) != std::string::npos) {
            LOG(INFO) << "  Found trusted repo";
            
            // Check for CRX file pattern in page URL
            if ((page_url.spec().find("/blob/main/") != std::string::npos || 
                 page_url.spec().find("/blob/master/") != std::string::npos) &&
                base::EndsWith(page_url.spec(), ".crx", 
                             base::CompareCase::INSENSITIVE_ASCII)) {
                LOG(INFO) << "  Detected trusted CRX download";
                return true;
            }
        }
    }

    // Fallback to regular extension checks
    if (content_disposition.find("filename=") != std::string::npos &&
        content_disposition.find(".crx") != std::string::npos) {
        return true;
    }

    if (download_item.GetMimeType() == extensions::Extension::kMimeType) {
        return true;
    }

    base::FilePath target_path = download_item.GetTargetFilePath();
    if (target_path.MatchesExtension(FILE_PATH_LITERAL(".crx"))) {
        LOG(INFO) << "Detected CRX via target path extension";
        return true;
    }

    LOG(INFO) << "Not a CRX file";
    return false;
}

bool IsTrustedExtensionDownload(Profile* profile,
                                const download::DownloadItem& item) {
  // Webstore exts are not supported
  return false;
}

}  // namespace download_crx_util