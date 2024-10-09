// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Download code which handles CRX files (extensions, themes, apps, ...).

#include "chrome/browser/download/download_crx_util.h"
#include "extensions/common/extension.h"
#include "components/download/public/common/download_item.h"

namespace download_crx_util {

bool IsExtensionDownload(const download::DownloadItem& download_item) {
  if (download_item.GetTargetDisposition() ==
      download::DownloadItem::TARGET_DISPOSITION_PROMPT) {
    return false;
  }

  return download_item.GetMimeType() == extensions::Extension::kMimeType;
}

bool IsTrustedExtensionDownload(Profile* profile,
                                const download::DownloadItem& item) {
  // Webstore exts are not supported
  return false;
}

}  // namespace download_crx_util
