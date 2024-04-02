// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/page_image_service/image_service_consent_helper.h"

#include "base/feature_list.h"
#include "base/metrics/field_trial_params.h"
#include "base/metrics/histogram_functions.h"
#include "components/page_image_service/features.h"
#include "components/page_image_service/metrics_util.h"
#include "components/sync/base/features.h"
#include "components/sync/service/sync_service.h"
#include "components/sync/service/sync_service_utils.h"
#include "components/unified_consent/consent_throttle.h"
#include "components/unified_consent/url_keyed_data_collection_consent_helper.h"

namespace page_image_service {

namespace {

void RunConsentThrottleCallback(
    base::OnceCallback<void(PageImageServiceConsentStatus)> callback,
    bool success) {
  std::move(callback).Run(success ? PageImageServiceConsentStatus::kSuccess
                                  : PageImageServiceConsentStatus::kFailure);
}

PageImageServiceConsentStatus ConsentStatusToUmaStatus(
    std::optional<bool> consent_status) {
  if (!consent_status) {
    return PageImageServiceConsentStatus::kTimedOut;
  }
  return consent_status.value() ? PageImageServiceConsentStatus::kSuccess
                                : PageImageServiceConsentStatus::kFailure;
}

}  // namespace

ImageServiceConsentHelper::ImageServiceConsentHelper(
    syncer::SyncService* sync_service,
    syncer::ModelType model_type)
    : sync_service_(sync_service),
      model_type_(model_type),
      timeout_duration_(base::Seconds(GetFieldTrialParamByFeatureAsInt(
          kImageServiceObserveSyncDownloadStatus,
          "timeout_seconds",
          10))) {
  if (base::FeatureList::IsEnabled(kImageServiceObserveSyncDownloadStatus)) {
    sync_service_observer_.Observe(sync_service);
  } else if (model_type == syncer::ModelType::BOOKMARKS) {
    consent_throttle_ = std::make_unique<unified_consent::ConsentThrottle>(
        unified_consent::UrlKeyedDataCollectionConsentHelper::
            NewPersonalizedBookmarksDataCollectionConsentHelper(
                sync_service,
                /*require_sync_feature_enabled=*/!base::FeatureList::IsEnabled(
                    syncer::kReplaceSyncPromosWithSignInPromos) &&
                    !base::FeatureList::IsEnabled(
                        syncer::kEnableBookmarkFoldersForAccountStorage)),
        timeout_duration_);
  } else if (model_type == syncer::ModelType::HISTORY_DELETE_DIRECTIVES) {
    consent_throttle_ = std::make_unique<unified_consent::ConsentThrottle>(
        unified_consent::UrlKeyedDataCollectionConsentHelper::
            NewPersonalizedDataCollectionConsentHelper(sync_service),
        timeout_duration_);
  } else {
    NOTREACHED();
  }
}

ImageServiceConsentHelper::~ImageServiceConsentHelper() = default;

void ImageServiceConsentHelper::EnqueueRequest(
    base::OnceCallback<void(PageImageServiceConsentStatus)> callback,
    mojom::ClientId client_id) {
  base::UmaHistogramBoolean("PageImageService.ConsentStatusRequestCount", true);
  if (consent_throttle_) {
    consent_throttle_->EnqueueRequest(
        base::BindOnce(&RunConsentThrottleCallback, std::move(callback)));
    return;
  }

  std::optional<bool> consent_status = GetConsentStatus();
  if (consent_status.has_value()) {
    std::move(callback).Run(*consent_status
                                ? PageImageServiceConsentStatus::kSuccess
                                : PageImageServiceConsentStatus::kFailure);
    return;
  }

  enqueued_request_callbacks_.emplace_back(std::move(callback), client_id);
  if (!request_processing_timer_.IsRunning()) {
    request_processing_timer_.Start(
        FROM_HERE, timeout_duration_,
        base::BindOnce(&ImageServiceConsentHelper::OnTimeoutExpired,
                       weak_ptr_factory_.GetWeakPtr()));
  }
}

void ImageServiceConsentHelper::OnStateChanged(
    syncer::SyncService* sync_service) {
  CHECK_EQ(sync_service_, sync_service);
  CHECK(base::FeatureList::IsEnabled(kImageServiceObserveSyncDownloadStatus));

  std::optional<bool> consent_status = GetConsentStatus();
  if (!consent_status.has_value()) {
    return;
  }

  request_processing_timer_.Stop();

  // The request callbacks can modify the vector while running. Swap the vector
  // onto the stack to prevent crashing. https://crbug.com/1472360.
  std::vector<std::pair<base::OnceCallback<void(PageImageServiceConsentStatus)>,
                        mojom::ClientId>>
      callbacks;
  std::swap(callbacks, enqueued_request_callbacks_);
  for (auto& request_callback_with_client_id : callbacks) {
    std::move(request_callback_with_client_id.first)
        .Run(*consent_status ? PageImageServiceConsentStatus::kSuccess
                             : PageImageServiceConsentStatus::kFailure);
  }
}

void ImageServiceConsentHelper::OnSyncShutdown(
    syncer::SyncService* sync_service) {
  CHECK_EQ(sync_service_, sync_service);
  CHECK(base::FeatureList::IsEnabled(kImageServiceObserveSyncDownloadStatus));

  sync_service_observer_.Reset();
  sync_service_ = nullptr;
}

std::optional<bool> ImageServiceConsentHelper::GetConsentStatus() {
  CHECK(base::FeatureList::IsEnabled(kImageServiceObserveSyncDownloadStatus));

  if (!sync_service_) {
    return false;
  }

  // If upload of the given ModelType is disabled (or inactive due to an
  // error), then consent must be assumed to be NOT given.
  // Note that the "INITIALIZING" state is good enough: It means the data
  // type is enabled in principle, Sync just hasn't fully finished
  // initializing yet. This case is handled by the DownloadStatus check
  // below.
  if (syncer::GetUploadToGoogleState(sync_service_, model_type_) ==
      syncer::UploadState::NOT_ACTIVE) {
    return false;
  }

  // Ensure Sync has downloaded all relevant updates (i.e. any deletions from
  // other devices are known).
  syncer::SyncService::ModelTypeDownloadStatus download_status =
      sync_service_->GetDownloadStatusFor(model_type_);
  switch (download_status) {
    case syncer::SyncService::ModelTypeDownloadStatus::kWaitingForUpdates:
      return std::nullopt;
    case syncer::SyncService::ModelTypeDownloadStatus::kUpToDate:
      return true;
    case syncer::SyncService::ModelTypeDownloadStatus::kError:
      return false;
  }
}

void ImageServiceConsentHelper::OnTimeoutExpired() {
  // The request callbacks can modify the vector while running. Swap the vector
  // onto the stack to prevent crashing. https://crbug.com/1472360.
  std::vector<std::pair<base::OnceCallback<void(PageImageServiceConsentStatus)>,
                        mojom::ClientId>>
      callbacks;
  std::swap(callbacks, enqueued_request_callbacks_);
  for (auto& request_callback_with_client_id : callbacks) {
    // Report consent status on timeout for each request to compare against the
    // number of all requests.
    PageImageServiceConsentStatus consent_status =
        ConsentStatusToUmaStatus(GetConsentStatus());
    base::UmaHistogramEnumeration("PageImageService.ConsentStatusOnTimeout",
                                  consent_status);
    if (sync_service_) {
      sync_service_->RecordReasonIfWaitingForUpdates(
          model_type_, kConsentTimeoutReasonHistogramName);
      sync_service_->RecordReasonIfWaitingForUpdates(
          model_type_,
          std::string(kConsentTimeoutReasonHistogramName) + "." +
              ClientIdToString(request_callback_with_client_id.second));
    }
    std::move(request_callback_with_client_id.first).Run(consent_status);
  }
}

}  // namespace page_image_service
