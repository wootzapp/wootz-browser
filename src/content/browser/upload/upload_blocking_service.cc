#include "content/browser/upload/upload_blocking_service.h"

#include <algorithm>
#include <cstdio>
#include <string>

#include "base/strings/stringprintf.h"
#include "build/build_config.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/upload_blocking_prefs.h"
#include "content/public/browser/web_contents.h"

#if BUILDFLAG(IS_ANDROID)
#include "content/browser/upload/upload_blocked_dialog_bridge.h"
#include "ui/android/window_android.h"
#endif

namespace content {

namespace {
// Global instance
UploadBlockingService* g_instance = nullptr;
}  // namespace

// static
UploadBlockingService* UploadBlockingService::GetInstance() {
  if (!g_instance) {
    g_instance = new UploadBlockingService();
  }
  return g_instance;
}

UploadBlockingService::UploadBlockingService() {
  // No initialization needed - all data comes from prefs now
  printf("AADI UPLOAD BLOCKING: Service initialized (using dialog)\n");

#if BUILDFLAG(IS_ANDROID)
  upload_blocked_dialog_bridge_ =
      std::make_unique<content::UploadBlockedDialogBridge>();
#endif
}

UploadBlockingService::~UploadBlockingService() = default;

#if BUILDFLAG(IS_ANDROID)

void UploadBlockingService::ShowUploadBlockedNotification(
    WebContents* web_contents,
    const std::string& message) {
  if (!web_contents) {
    printf(
        "AADI UPLOAD BLOCKING: Cannot show notification - missing "
        "WebContents\n");
    return;
  }

  printf("AADI UPLOAD BLOCKING: Showing Android dialog: %s\n", message.c_str());

  // Call the static Java method through JNI
  UploadBlockedDialogBridge::ShowDialog(web_contents, message);
}
#endif

bool UploadBlockingService::ShouldBlockUpload(const std::string& domain,
                                              PrefService* prefs) {
  if (!prefs) {
    printf("AADI UPLOAD BLOCKING: No prefs service available\n");
    return false;
  }

  bool is_blocked = IsDomainBlocked(domain, prefs);
  printf("AADI UPLOAD BLOCKING: Domain '%s' - %s\n", domain.c_str(),
         is_blocked ? "BLOCKED" : "ALLOWED");

  return is_blocked;
}

bool UploadBlockingService::IsDomainBlocked(const std::string& domain,
                                            PrefService* prefs) {
  LOG(INFO) << "[UPLOAD BLOCKING] Checking if domain is blocked: " << domain;

  if (!prefs) {
    LOG(WARNING) << "[UPLOAD BLOCKING] No PrefService provided - allowing "
                    "upload for domain: "
                 << domain;
    return false;
  }

  const auto& blocked_domains =
      prefs->GetList(upload_blocking_prefs::kBlockedUploadDomains);
  LOG(INFO) << "[UPLOAD BLOCKING] Found " << blocked_domains.size()
            << " blocked domains in preferences";
  std::string normalized_domain;
  if (domain.length() > 4 && domain.substr(0, 4) == "www.") {
    normalized_domain = domain.substr(4);
  } else {
    normalized_domain = domain;
  }

  // Check if the requested domain is in the blocked list
  for (const auto& domain_value : blocked_domains) {
    if (domain_value.is_string()) {
      if (domain_value.GetString() == normalized_domain) {
        LOG(WARNING) << "[UPLOAD BLOCKING] DOMAIN BLOCKED: " << domain
                     << " - Upload will be prevented";
        return true;
      }
    }
  }

  LOG(INFO) << "[UPLOAD BLOCKING] Domain allowed: " << domain
            << " - Upload will proceed";
  return false;
}

}  // namespace content
