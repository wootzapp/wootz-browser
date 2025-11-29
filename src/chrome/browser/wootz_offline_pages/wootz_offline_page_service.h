// Copyright 2024 Wootzapp Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_SERVICE_H_
#define CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_SERVICE_H_

#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "components/keyed_service/core/keyed_service.h"
#include "url/gurl.h"

namespace content {
class WebContents;
}

class WootzOfflinePageService : public KeyedService {
 public:
  WootzOfflinePageService();
  ~WootzOfflinePageService() override;

  void SavePage(content::WebContents* web_contents,
                const std::vector<GURL>& redirect_chain);
  bool GetOfflinePagePath(const GURL& url, base::FilePath* path);
  bool ClearAllPages();
  
  // 404 page management
  bool Does404PageExist();
  void EnsureOffline404PageExists();
  base::FilePath Get404PagePath();
  
  // No result found page management (for LinkedIn searches)
  bool DoesNoResultFoundPageExist();
  base::FilePath GetNoResultFoundPagePath();

 private:
  void OnMHTMLGenerated(const std::vector<GURL>& redirect_chain,
                        const base::FilePath& file_path,
                        int64_t size);
  base::FilePath GetStorageDir();
  base::FilePath URLToFilePath(const GURL& url);
  
  // URL canonicalization for consistent offline page lookup
  // Strips query parameters for non-search pages, normalizes trailing slashes
  GURL CanonicalizeUrlForOfflinePage(const GURL& url);
  
  // Manifest management
  void CreateManifest(const base::FilePath& html_path,
                     const std::vector<GURL>& redirect_chain,
                     int64_t size);
  bool FindPagePathFromManifest(const GURL& url, base::FilePath* path);
  base::FilePath GetManifestPath(const base::FilePath& html_path);
  
  // Subdomain variant generation for cross-subdomain matching
  // e.g., www.linkedin.com <-> in.linkedin.com <-> m.linkedin.com
  std::vector<GURL> GenerateSubdomainVariants(const GURL& url);

  base::WeakPtrFactory<WootzOfflinePageService> weak_factory_{this};
};

#endif  // CHROME_BROWSER_WOOTZ_OFFLINE_PAGES_WOOTZ_OFFLINE_PAGE_SERVICE_H_
