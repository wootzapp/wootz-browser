// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/file_suggest/drive_recent_file_suggestion_provider.h"

#include <string>
#include <utility>
#include <vector>

#include "ash/constants/ash_features.h"
#include "base/barrier_closure.h"
#include "base/files/file_path.h"
#include "base/functional/callback.h"
#include "base/i18n/time_formatting.h"
#include "base/metrics/histogram_functions.h"
#include "base/ranges/algorithm.h"
#include "base/strings/string_util.h"
#include "base/time/time.h"
#include "chrome/browser/ash/app_list/search/files/justifications.h"
#include "chrome/browser/ash/drive/drive_integration_service.h"
#include "chrome/browser/ash/file_suggest/file_suggest_util.h"
#include "chromeos/ash/components/drivefs/drivefs_host.h"
#include "chromeos/ash/components/drivefs/drivefs_util.h"
#include "components/drive/file_errors.h"
#include "components/prefs/pref_service.h"
#include "mojo/public/cpp/bindings/callback_helpers.h"

namespace ash {
namespace {

constexpr char kBaseHistogramName[] = "Ash.Search.FileSuggestions.DriveRecents";

drivefs::mojom::QueryParametersPtr CreateRecentlyModifiedQuery(
    base::TimeDelta max_recency) {
  auto query = drivefs::mojom::QueryParameters::New();
  query->modified_time = base::Time::Now() - max_recency;
  query->modified_time_operator =
      drivefs::mojom::QueryParameters::DateComparisonOperator::kGreaterThan;
  query->viewed_time = base::Time::Now() - max_recency;
  query->viewed_time_operator =
      drivefs::mojom::QueryParameters::DateComparisonOperator::kGreaterThan;
  query->page_size = 15;
  query->query_source =
      drivefs::mojom::QueryParameters::QuerySource::kLocalOnly;
  query->sort_direction =
      drivefs::mojom::QueryParameters::SortDirection::kDescending;
  query->sort_field = drivefs::mojom::QueryParameters::SortField::kLastModified;
  return query;
}

drivefs::mojom::QueryParametersPtr CreateRecentlyViewedQuery(
    base::TimeDelta max_recency) {
  auto query = drivefs::mojom::QueryParameters::New();
  query->page_size = 15;
  query->query_source =
      drivefs::mojom::QueryParameters::QuerySource::kLocalOnly;
  query->sort_direction =
      drivefs::mojom::QueryParameters::SortDirection::kDescending;
  query->sort_field =
      drivefs::mojom::QueryParameters::SortField::kLastViewedByMe;
  query->viewed_time = base::Time::Now() - max_recency;
  query->viewed_time_operator =
      drivefs::mojom::QueryParameters::DateComparisonOperator::kGreaterThan;
  return query;
}

drivefs::mojom::QueryParametersPtr CreateSharedWithMeQuery() {
  auto query = drivefs::mojom::QueryParameters::New();
  query->page_size = 15;
  query->query_source =
      drivefs::mojom::QueryParameters::QuerySource::kLocalOnly;
  query->shared_with_me = true;
  query->sort_direction =
      drivefs::mojom::QueryParameters::SortDirection::kDescending;
  query->sort_field = drivefs::mojom::QueryParameters::SortField::kSharedWithMe;
  return query;
}

FileSuggestData CreateFileSuggestionWithJustification(
    const base::FilePath& path,
    app_list::JustificationType justification_type,
    const base::Time& timestamp,
    const drivefs::mojom::UserInfo* user_info) {
  // Use secondary timestamp for files suggested because they were shared with
  // the user, so they are ordered after suggestions for viewed/modified files.
  const bool shared_with_me_suggestion =
      justification_type == app_list::JustificationType::kShared;
  std::optional<base::Time> primary_timestamp;
  std::optional<base::Time> secondary_timestamp;
  if (shared_with_me_suggestion) {
    secondary_timestamp = timestamp;
  } else {
    primary_timestamp = timestamp;
  }

  return FileSuggestData(
      FileSuggestionType::kDriveFile, path,
      app_list::GetJustificationString(
          justification_type, timestamp,
          user_info ? user_info->display_name : std::string()),
      primary_timestamp, secondary_timestamp, /*new_score=*/std::nullopt);
}

std::optional<FileSuggestData> CreateFileSuggestion(
    const base::FilePath& path,
    const drivefs::mojom::FileMetadata& file_metadata,
    base::TimeDelta max_recency) {
  const base::Time& modified_time = file_metadata.modification_time;
  const base::Time& viewed_time = file_metadata.last_viewed_by_me_time;

  // If the file was shared with user, but not yet viewed by the user, surface
  // it as a shared file.
  if (const std::optional<base::Time>& shared_time =
          file_metadata.shared_with_me_time;
      shared_time && !shared_time->is_null() && viewed_time.is_null()) {
    if ((base::Time::Now() - *shared_time).magnitude() > max_recency) {
      return std::nullopt;
    }

    return CreateFileSuggestionWithJustification(
        path, app_list::JustificationType::kShared, *shared_time,
        features::IsShowSharingUserInLauncherContinueSectionEnabled()
            ? file_metadata.sharing_user.get()
            : nullptr);
  }

  // Viewed by the user more recently than the last modification.
  if (viewed_time > modified_time) {
    if ((base::Time::Now() - viewed_time).magnitude() > max_recency) {
      return std::nullopt;
    }
    return CreateFileSuggestionWithJustification(
        path, app_list::JustificationType::kViewed, viewed_time,
        /*user_info=*/nullptr);
  }

  if ((base::Time::Now() - modified_time).magnitude() > max_recency) {
    return std::nullopt;
  }

  base::UmaHistogramBoolean(
      base::JoinString({kBaseHistogramName, "ModifyingUserMetadataPresent"},
                       "."),
      !!file_metadata.last_modifying_user);

  // Last modification was by the user.
  if (file_metadata.modified_by_me_time &&
      !file_metadata.modified_by_me_time->is_null() &&
      file_metadata.modified_by_me_time >= modified_time) {
    return CreateFileSuggestionWithJustification(
        path, app_list::JustificationType::kModifiedByCurrentUser,
        *file_metadata.modified_by_me_time, /*user_info=*/nullptr);
  }

  // Last modification was by either by another user, or the last modifying user
  // information is not available.
  return CreateFileSuggestionWithJustification(
      path, app_list::JustificationType::kModified, modified_time,
      file_metadata.last_modifying_user.get());
}

}  // namespace

DriveRecentFileSuggestionProvider::DriveRecentFileSuggestionProvider(
    Profile* profile,
    base::RepeatingCallback<void(FileSuggestionType)> notify_update_callback)
    : FileSuggestionProvider(notify_update_callback),
      profile_(profile),
      max_recency_(GetMaxFileSuggestionRecency()) {
  auto* drive_integration_service =
      drive::DriveIntegrationServiceFactory::GetForProfile(profile);
  if (drive_integration_service) {
    drive::DriveIntegrationService::Observer::Observe(
        drive_integration_service);
    auto* drive_fs_host = drive_integration_service->GetDriveFsHost();
    if (drive_fs_host) {
      drivefs::DriveFsHost::Observer::Observe(drive_fs_host);
    }
  }
}

DriveRecentFileSuggestionProvider::~DriveRecentFileSuggestionProvider() =
    default;

void DriveRecentFileSuggestionProvider::GetSuggestFileData(
    GetSuggestFileDataCallback callback) {
  const bool has_active_request =
      !on_drive_results_ready_callback_list_.empty();

  // Return early if there is an active search request. `callback` will run when
  // the active search completes.
  if (has_active_request) {
    // Add `callback` to the waiting list, and return.
    on_drive_results_ready_callback_list_.AddUnsafe(std::move(callback));
    return;
  }

  drive::DriveIntegrationService* drive_service =
      drive::DriveIntegrationServiceFactory::FindForProfile(profile_);

  // If there is not any available drive service, return early.
  if (!drive_service || !drive_service->IsMounted()) {
    std::move(callback).Run(/*suggest_results=*/std::nullopt);
    return;
  }

  if (can_use_cache_) {
    std::move(callback).Run(GetSuggestionsFromLatestQueryResults());
    return;
  }

  on_drive_results_ready_callback_list_.AddUnsafe(std::move(callback));

  search_start_time_ = base::Time::Now();
  query_result_files_by_path_.clear();

  // Allow using cached results for requests that come in after the current one
  // finishes.
  can_use_cache_ = true;

  base::RepeatingClosure search_callback = base::BarrierClosure(
      3, base::BindOnce(
             &DriveRecentFileSuggestionProvider::OnRecentFilesSearchesCompleted,
             weak_factory_.GetWeakPtr()));
  PerformSearch(SearchType::kLastModified,
                CreateRecentlyModifiedQuery(max_recency_), drive_service,
                search_callback);
  PerformSearch(SearchType::kLastViewed,
                CreateRecentlyViewedQuery(max_recency_), drive_service,
                search_callback);
  PerformSearch(SearchType::kSharedWithUser, CreateSharedWithMeQuery(),
                drive_service, search_callback);
}

// static
std::string DriveRecentFileSuggestionProvider::GetHistogramSuffix(
    SearchType search_type) {
  switch (search_type) {
    case SearchType::kLastViewed:
      return "Viewed";
    case SearchType::kLastModified:
      return "Modified";
    case SearchType::kSharedWithUser:
      return "Shared";
  }
}

void DriveRecentFileSuggestionProvider::OnDriveIntegrationServiceDestroyed() {
  drive::DriveIntegrationService::Observer::Reset();
  drivefs::DriveFsHost::Observer::Reset();

  can_use_cache_ = false;
  query_result_files_by_path_.clear();
  on_drive_results_ready_callback_list_.Notify(
      /*suggest_results=*/std::nullopt);
}

void DriveRecentFileSuggestionProvider::OnFileSystemMounted() {
  if (!drivefs::DriveFsHost::Observer::GetHost()) {
    auto* drive_fs_host = drive::DriveIntegrationService::Observer::GetService()
                              ->GetDriveFsHost();
    if (drive_fs_host) {
      drivefs::DriveFsHost::Observer::Observe(drive_fs_host);
    }
  }
  NotifySuggestionUpdate(FileSuggestionType::kDriveFile);
}

void DriveRecentFileSuggestionProvider::OnHostDestroyed() {
  drivefs::DriveFsHost::Observer::Reset();
}

void DriveRecentFileSuggestionProvider::OnUnmounted() {
  can_use_cache_ = false;
  query_result_files_by_path_.clear();
  on_drive_results_ready_callback_list_.Notify(
      /*suggest_results=*/std::nullopt);

  NotifySuggestionUpdate(FileSuggestionType::kDriveFile);
}

void DriveRecentFileSuggestionProvider::OnFilesChanged(
    const std::vector<drivefs::mojom::FileChange>& changes) {
  can_use_cache_ = false;

  for (const auto& change : changes) {
    if (change.type == drivefs::mojom::FileChange::Type::kDelete) {
      query_result_files_by_path_.erase(change.path);
    }
  }
}

void DriveRecentFileSuggestionProvider::PerformSearch(
    SearchType search_type,
    drivefs::mojom::QueryParametersPtr query,
    drive::DriveIntegrationService* drive_service,
    base::RepeatingClosure callback) {
  drive_service->GetDriveFsHost()->PerformSearch(
      std::move(query),
      mojo::WrapCallbackWithDefaultInvokeIfNotRun(
          base::BindOnce(
              &DriveRecentFileSuggestionProvider::OnSearchRequestComplete,
              weak_factory_.GetWeakPtr(), search_type, std::move(callback)),
          drive::FileError::FILE_ERROR_ABORT, /*items=*/std::nullopt));
}

void DriveRecentFileSuggestionProvider::MaybeUpdateItemSuggestCache(
    base::PassKey<FileSuggestKeyedService>) {}

void DriveRecentFileSuggestionProvider::OnSearchRequestComplete(
    SearchType search_type,
    base::RepeatingClosure callback,
    drive::FileError error,
    std::optional<std::vector<drivefs::mojom::QueryItemPtr>> items) {
  // `error` is an enum, but has negative values, so UmaEnumeration does not
  // work - record it as a count instead.
  base::UmaHistogramCounts100(
      base::JoinString(
          {kBaseHistogramName, "QueryResult", GetHistogramSuffix(search_type)},
          "."),
      std::abs(error));

  if (error != drive::FileError::FILE_ERROR_OK &&
      error != drive::FileError::FILE_ERROR_INVALID_OPERATION &&
      error != drive::FileError::FILE_ERROR_OK_WITH_MORE_RESULTS) {
    can_use_cache_ = false;
  }

  if ((error == drive::FileError::FILE_ERROR_OK ||
       error == drive::FileError::FILE_ERROR_OK_WITH_MORE_RESULTS) &&
      items) {
    base::UmaHistogramTimes(
        base::JoinString({kBaseHistogramName, "DurationOnSuccess",
                          GetHistogramSuffix(search_type)},
                         "."),
        base::Time::Now() - search_start_time_);
    base::UmaHistogramCounts100(
        base::JoinString(
            {kBaseHistogramName, "ItemCount", GetHistogramSuffix(search_type)},
            "."),
        items->size());

    size_t folder_count = 0u;
    for (auto& item : *items) {
      if (item->metadata->type ==
          drivefs::mojom::FileMetadata::Type::kDirectory) {
        ++folder_count;
        continue;
      }
      query_result_files_by_path_.emplace(item->path,
                                          std::move(item->metadata));
    }
    base::UmaHistogramCounts100(
        base::JoinString({kBaseHistogramName, "FolderCount",
                          GetHistogramSuffix(search_type)},
                         "."),
        folder_count);

  } else {
    base::UmaHistogramTimes(
        base::JoinString({kBaseHistogramName, "DurationOnError",
                          GetHistogramSuffix(search_type)},
                         "."),
        base::Time::Now() - search_start_time_);
  }
  callback.Run();
}

std::vector<FileSuggestData>
DriveRecentFileSuggestionProvider::GetSuggestionsFromLatestQueryResults() {
  drive::DriveIntegrationService* const drive_service =
      drive::DriveIntegrationServiceFactory::FindForProfile(profile_);
  CHECK(drive_service);

  std::vector<FileSuggestData> results;
  for (const auto& item : query_result_files_by_path_) {
    base::FilePath root("/");
    base::FilePath path = drive_service->GetMountPointPath();
    if (!root.AppendRelativePath(item.first, &path)) {
      continue;
    }

    std::optional<FileSuggestData> suggestion =
        CreateFileSuggestion(path, *item.second, max_recency_);
    if (suggestion) {
      results.push_back(*suggestion);
    }
  }

  base::ranges::sort(results, [](const auto& lhs, const auto& rhs) {
    if (lhs.timestamp == rhs.timestamp) {
      return lhs.secondary_timestamp.value_or(base::Time()) >
             rhs.secondary_timestamp.value_or(base::Time());
    }
    return lhs.timestamp.value_or(base::Time()) >
           rhs.timestamp.value_or(base::Time());
  });

  return results;
}

void DriveRecentFileSuggestionProvider::OnRecentFilesSearchesCompleted() {
  drive::DriveIntegrationService* const drive_service =
      drive::DriveIntegrationServiceFactory::FindForProfile(profile_);
  if (!drive_service || !drive_service->IsMounted()) {
    can_use_cache_ = false;
    query_result_files_by_path_.clear();
    on_drive_results_ready_callback_list_.Notify(
        std::vector<FileSuggestData>());
    NotifySuggestionUpdate(FileSuggestionType::kDriveFile);
    return;
  }

  std::vector<FileSuggestData> results = GetSuggestionsFromLatestQueryResults();

  base::UmaHistogramTimes(
      base::JoinString({kBaseHistogramName, "DurationOnSuccess.Total"}, "."),
      base::Time::Now() - search_start_time_);
  base::UmaHistogramCounts100(
      base::JoinString({kBaseHistogramName, "ItemCount.Total"}, "."),
      results.size());

  for (size_t i = 0; i < results.size(); ++i) {
    if (!results[i].timestamp) {
      base::UmaHistogramCounts100(
          base::JoinString({kBaseHistogramName, "FirstSharedSuggestionIndex"},
                           "."),
          i);
      break;
    }
  }

  on_drive_results_ready_callback_list_.Notify(results);
  if (can_use_cache_) {
    NotifySuggestionUpdate(FileSuggestionType::kDriveFile);
  }
}

}  // namespace ash
