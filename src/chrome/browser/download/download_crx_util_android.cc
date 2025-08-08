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
#include "chrome/browser/android/extension_developer_mode_settings_prefs.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "components/prefs/pref_service.h"
#include "chrome/browser/extensions/extension_install_prompt.h"
#include <regex>

namespace download_crx_util {

bool IsExtensionDownload(const download::DownloadItem& download_item) {
  std::string content_disposition = download_item.GetContentDisposition();
  std::string file_name = download_item.GetSuggestedFilename();
  LOG(INFO) << "IsExtensionDownload checking:";
  LOG(INFO) << "  Content disposition: " << content_disposition;
  
  // Only allow downloads from the trusted extension store
  content::WebContents* web_contents =
      content::DownloadItemUtils::GetWebContents(&download_item);
  if (!web_contents) {
    LOG(INFO) << "No web contents found, rejecting CRX";
    return false;
  }

  GURL page_url = web_contents->GetLastCommittedURL();
  LOG(INFO) << "  Page URL: " << page_url.spec();
  PrefService* prefs = ProfileManager::GetLastUsedProfile()->GetPrefs();
  bool is_developer_mode_enabled = prefs->GetBoolean(extension_developer_mode_settings::kExtensionDeveloperModeEnabledPref);
  LOG(INFO) << "  Developer mode enabled: " << is_developer_mode_enabled;
  std::string extension_file_name = prefs->GetString(extension_developer_mode_settings::kExtensionFileName);
  LOG(INFO) << "  Extension file name: " << extension_file_name;
  std::regex crx_regex(".*\\.crx$", std::regex::icase);
  if(is_developer_mode_enabled){
    if(std::regex_match(extension_file_name, crx_regex)){
      LOG(INFO) << "Developer mode is enabled, accepting CRX";
      
      ExtensionInstallPrompt::notifyExtensionInstalled();

      return true;
    }
    else{
      LOG(INFO) << "Developer mode is enabled, but not a CRX file, rejecting";
      return false;
    }
  }
  // Check if the page URL is from the trusted source
  const char* TRUSTED_SOURCE = "wootzapp://flow-store/";
  const char* TRUSTED_SOURCE_2 = "wootzapp://startup-crx-install/"; 
  if (page_url.spec() != TRUSTED_SOURCE && page_url.spec() != TRUSTED_SOURCE_2) {
    LOG(INFO) << "Not from trusted source, rejecting CRX";
    return false;
  }
  std::string url = download_item.GetURL().spec();
  
  // Verify it's a CRX file from the download URL, not the page URL
  if (base::EndsWith(url, ".crx", base::CompareCase::INSENSITIVE_ASCII)) {
    LOG(INFO) << "Detected trusted CRX download";
    
    ExtensionInstallPrompt::notifyExtensionInstalled();

    return true;
  }

  LOG(INFO) << "Not a valid CRX file path";
  return false;
}

}  // namespace download_crx_util