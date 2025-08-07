#ifndef CONTENT_BROWSER_UPLOAD_UPLOAD_BLOCKING_SERVICE_H_
#define CONTENT_BROWSER_UPLOAD_UPLOAD_BLOCKING_SERVICE_H_

#include <memory>
#include <string>

#include "build/build_config.h"

// Forward declarations
class PrefService;

#if BUILDFLAG(IS_ANDROID)
namespace content {
class UploadBlockedDialogBridge;
}
#endif

namespace ui {
class WindowAndroid;
}

namespace content {

class WebContents;

// Service to block file uploads on blacklisted websites using browser prefs
class UploadBlockingService {
 public:
  static UploadBlockingService* GetInstance();

  // Returns true if uploads should be blocked for the given URL
  bool ShouldBlockUpload(const std::string& url, PrefService* prefs);

#if BUILDFLAG(IS_ANDROID)

  // Shows notification using browser process method
  void ShowUploadBlockedNotification(WebContents* web_contents,
                                     const std::string& message);
#endif

 private:
  UploadBlockingService();
  ~UploadBlockingService();

  // Helper functions to check blocked status using prefs
  bool IsDomainBlocked(const std::string& domain, PrefService* prefs);

#if BUILDFLAG(IS_ANDROID)
  std::unique_ptr<content::UploadBlockedDialogBridge>
      upload_blocked_dialog_bridge_;
#endif

  UploadBlockingService(const UploadBlockingService&) = delete;
  UploadBlockingService& operator=(const UploadBlockingService&) = delete;
};

}  // namespace content

#endif  // CONTENT_BROWSER_UPLOAD_UPLOAD_BLOCKING_SERVICE_H_
