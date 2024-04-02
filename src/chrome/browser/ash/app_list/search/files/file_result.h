// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ASH_APP_LIST_SEARCH_FILES_FILE_RESULT_H_
#define CHROME_BROWSER_ASH_APP_LIST_SEARCH_FILES_FILE_RESULT_H_

#include <iosfwd>
#include <optional>

#include "ash/public/cpp/style/color_mode_observer.h"
#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/memory/raw_ptr.h"
#include "chrome/browser/ash/app_list/search/chrome_search_result.h"

class Profile;
class SkBitmap;

namespace ash {
class ThumbnailLoader;
namespace string_matching {
class TokenizedString;
}  // namespace string_matching
}  // namespace ash

namespace app_list {

// This result class is shared between four file search providers:
// {drive,local} {zero-state,search}.
class FileResult : public ChromeSearchResult, public ash::ColorModeObserver {
 public:
  enum class Type { kFile, kDirectory, kSharedDirectory };

  FileResult(const std::string& id,
             const base::FilePath& filepath,
             const std::optional<std::u16string>& details,
             ResultType result_type,
             DisplayType display_type,
             float relevance,
             const std::u16string& query,
             Type type,
             Profile* profile);
  ~FileResult() override;

  FileResult(const FileResult&) = delete;
  FileResult& operator=(const FileResult&) = delete;

  // ChromeSearchResult overrides:
  void Open(int event_flags) override;
  std::optional<std::string> DriveId() const override;
  std::optional<GURL> url() const override;

  // Calculates file's match relevance score. Will return a default score if the
  // query is missing or the filename is empty.
  static double CalculateRelevance(
      const std::optional<ash::string_matching::TokenizedString>& query,
      const base::FilePath& filepath,
      const std::optional<base::Time>& last_accessed);

  // Depending on the file type and display type, request a thumbnail for this
  // result. If the request is successful, the current icon will be replaced by
  // the thumbnail.
  void RequestThumbnail(ash::ThumbnailLoader* thumbnail_loader);

  void set_drive_id(const std::optional<std::string>& drive_id) {
    drive_id_ = drive_id;
  }

  void set_url(const std::optional<GURL>& url) { url_ = url; }

 private:
  // ash::ColorModeObserver:
  void OnColorModeChanged(bool dark_mode_enabled) override;

  // Callback for the result of RequestThumbnail's call to the ThumbnailLoader.
  void OnThumbnailLoaded(const SkBitmap* bitmap, base::File::Error error);

  void UpdateIcon();

  const base::FilePath filepath_;
  const Type type_;
  const raw_ptr<Profile> profile_;

  std::optional<std::string> drive_id_;
  std::optional<GURL> url_;

  base::WeakPtrFactory<FileResult> weak_factory_{this};
};

::std::ostream& operator<<(::std::ostream& os, const FileResult& result);

}  // namespace app_list

#endif  // CHROME_BROWSER_ASH_APP_LIST_SEARCH_FILES_FILE_RESULT_H_
