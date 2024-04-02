// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/wallpaper_handlers/mock_sea_pen_fetcher.h"

#include <optional>
#include <string>
#include <vector>

#include "ash/public/cpp/wallpaper/sea_pen_image.h"
#include "ash/webui/common/mojom/sea_pen.mojom.h"
#include "base/functional/callback.h"
#include "base/strings/stringprintf.h"
#include "base/task/sequenced_task_runner.h"
#include "components/manta/manta_status.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace wallpaper_handlers {

namespace {

std::vector<ash::SeaPenImage> MakeFakeImageResults() {
  std::vector<ash::SeaPenImage> image_results;
  for (uint32_t i = 1; i < 5; i++) {
    image_results.emplace_back(base::StringPrintf("fake_sea_pen_image_%d", i),
                               i);
  }
  return image_results;
}

}  // namespace

MockSeaPenFetcher::MockSeaPenFetcher() {
  ON_CALL(*this, FetchThumbnails)
      .WillByDefault(
          [](manta::proto::FeatureName feature_name,
             const ash::personalization_app::mojom::SeaPenQueryPtr& query,
             OnFetchThumbnailsComplete callback) {
            base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
                FROM_HERE,
                base::BindOnce(std::move(callback), MakeFakeImageResults(),
                               manta::MantaStatusCode::kOk));
          });

  ON_CALL(*this, FetchWallpaper)
      .WillByDefault(
          [](manta::proto::FeatureName feature_name,
             const ash::SeaPenImage& image,
             const ash::personalization_app::mojom::SeaPenQueryPtr& query,
             OnFetchWallpaperComplete callback) {
            std::move(callback).Run(
                ash::SeaPenImage(image.jpg_bytes, image.id));
          });
}

MockSeaPenFetcher::~MockSeaPenFetcher() = default;

}  // namespace wallpaper_handlers
