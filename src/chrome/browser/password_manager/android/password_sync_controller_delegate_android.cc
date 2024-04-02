// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/password_manager/android/password_sync_controller_delegate_android.h"

#include "base/functional/callback_forward.h"
#include "base/memory/weak_ptr.h"
#include "base/metrics/histogram_functions.h"
#include "base/task/sequenced_task_runner.h"
#include "components/password_manager/core/browser/password_store/android_backend_error.h"
#include "components/password_manager/core/browser/password_sync_util.h"
#include "components/signin/public/identity_manager/account_info.h"
#include "components/sync/engine/data_type_activation_response.h"
#include "components/sync/model/model_type_controller_delegate.h"
#include "components/sync/model/proxy_model_type_controller_delegate.h"
#include "components/sync/model/type_entities_count.h"
#include "components/sync/service/sync_service.h"

namespace password_manager {

namespace {

// TODO(crbug.com/40067770): Migrate away from `ConsentLevel::kSync` on Android.
using sync_util::IsSyncFeatureEnabledIncludingPasswords;

std::string BuildCredentialManagerNotificationMetricName(
    const std::string& suffix) {
  return "PasswordManager.SyncControllerDelegateNotifiesCredentialManager." +
         suffix;
}

}  // namespace

PasswordSyncControllerDelegateAndroid::PasswordSyncControllerDelegateAndroid(
    std::unique_ptr<PasswordSyncControllerDelegateBridge> bridge)
    : bridge_(std::move(bridge)) {
  DCHECK(bridge_);
  bridge_->SetConsumer(weak_ptr_factory_.GetWeakPtr());
}

PasswordSyncControllerDelegateAndroid::
    ~PasswordSyncControllerDelegateAndroid() = default;

void PasswordSyncControllerDelegateAndroid::SetSyncObserverCallbacks(
    base::RepeatingClosure on_pwd_sync_state_changed,
    base::OnceClosure on_sync_shutdown) {
  on_pwd_sync_state_changed_ = std::move(on_pwd_sync_state_changed);
  on_sync_shutdown_ = std::move(on_sync_shutdown);
}

std::unique_ptr<syncer::ProxyModelTypeControllerDelegate>
PasswordSyncControllerDelegateAndroid::CreateProxyModelControllerDelegate() {
  return std::make_unique<syncer::ProxyModelTypeControllerDelegate>(
      base::SequencedTaskRunner::GetCurrentDefault(),
      base::BindRepeating(
          &PasswordSyncControllerDelegateAndroid::GetWeakPtrToBaseClass,
          base::Unretained(this)));
}

void PasswordSyncControllerDelegateAndroid::OnSyncServiceInitialized(
    syncer::SyncService* sync_service) {
  sync_observation_.Observe(sync_service);
  // TODO(crbug.com/40067770): Migrate away from `ConsentLevel::kSync` on
  // Android.
  UpdateCredentialManagerSyncStatus(sync_service);
}

void PasswordSyncControllerDelegateAndroid::OnSyncStarting(
    const syncer::DataTypeActivationRequest& request,
    StartCallback callback) {
  // Set |skip_engine_connection| to true to indicate that, actually, this sync
  // datatype doesn't depend on the built-in SyncEngine to communicate changes
  // to/from the Sync server. Instead, Android specific functionality is
  // leveraged to achieve similar behavior.
  auto activation_response =
      std::make_unique<syncer::DataTypeActivationResponse>();
  activation_response->skip_engine_connection = true;
  std::move(callback).Run(std::move(activation_response));
}

void PasswordSyncControllerDelegateAndroid::OnSyncStopping(
    syncer::SyncStopMetadataFate metadata_fate) {
  switch (metadata_fate) {
    case syncer::KEEP_METADATA:
      // Sync got temporarily paused. Just ignore.
      break;
    case syncer::CLEAR_METADATA:
      // The user (or something equivalent like an enterprise policy)
      // permanently disrabled sync, either fully or specifically for passwords.
      // This also includes more advanced cases like the user having cleared all
      // sync data in the dashboard (birthday reset) or, at least in theory, the
      // sync server reporting that all sync metadata is obsolete (i.e.
      // CLIENT_DATA_OBSOLETE in the sync protocol).
      // TODO(crbug.com/1312392): Sync was disabled. Move passwords from syncing
      // storage to local storage.
      NOTIMPLEMENTED();
      break;
  }
}

void PasswordSyncControllerDelegateAndroid::GetAllNodesForDebugging(
    AllNodesCallback callback) {
  // This is not implemented because it's not worth the hassle just to display
  // debug information in chrome://sync-internals.
  std::move(callback).Run(syncer::PASSWORDS, base::Value::List());
}

void PasswordSyncControllerDelegateAndroid::GetTypeEntitiesCountForDebugging(
    base::OnceCallback<void(const syncer::TypeEntitiesCount&)> callback) const {
  // This is not implemented because it's not worth the hassle just to display
  // debug information in chrome://sync-internals.
  std::move(callback).Run(syncer::TypeEntitiesCount(syncer::PASSWORDS));
}

void PasswordSyncControllerDelegateAndroid::
    RecordMemoryUsageAndCountsHistograms() {
  // This is not implemented because it's not worth the hassle. Password sync
  // module on Android doesn't hold any password. Instead passwords are
  // requested on demand from the GMS Core.
}

void PasswordSyncControllerDelegateAndroid::OnStateChanged(
    syncer::SyncService* sync) {
  UpdateCredentialManagerSyncStatus(sync);
}

void PasswordSyncControllerDelegateAndroid::OnSyncShutdown(
    syncer::SyncService* sync) {
  sync_observation_.Reset();
  if (!on_sync_shutdown_)
    return;
  std::move(on_sync_shutdown_).Run();
}

void PasswordSyncControllerDelegateAndroid::OnCredentialManagerNotified() {
  base::UmaHistogramBoolean(
      BuildCredentialManagerNotificationMetricName("Success"), 1);
}

void PasswordSyncControllerDelegateAndroid::OnCredentialManagerError(
    const AndroidBackendError& error,
    int api_error_code) {
  base::UmaHistogramBoolean(
      BuildCredentialManagerNotificationMetricName("Success"), 0);
  base::UmaHistogramEnumeration(
      BuildCredentialManagerNotificationMetricName("ErrorCode"), error.type);
  if (error.type == AndroidBackendErrorType::kExternalError) {
    base::UmaHistogramSparse(
        BuildCredentialManagerNotificationMetricName("APIErrorCode"),
        api_error_code);
  }
}

void PasswordSyncControllerDelegateAndroid::UpdateCredentialManagerSyncStatus(
    syncer::SyncService* sync_service) {
  // TODO(crbug.com/40067770): Migrate away from `ConsentLevel::kSync` on
  // Android.
  IsPwdSyncEnabled is_enabled =
      IsPwdSyncEnabled(IsSyncFeatureEnabledIncludingPasswords(sync_service));
  if (credential_manager_sync_setting_.has_value() &&
      credential_manager_sync_setting_ == is_enabled) {
    return;
  }

  if (on_pwd_sync_state_changed_) {
    on_pwd_sync_state_changed_.Run();
  }

  credential_manager_sync_setting_ = is_enabled;
  if (is_enabled) {
    bridge_->NotifyCredentialManagerWhenSyncing(
        sync_service->GetAccountInfo().email);
  } else {
    bridge_->NotifyCredentialManagerWhenNotSyncing();
  }
}

base::WeakPtr<syncer::ModelTypeControllerDelegate>
PasswordSyncControllerDelegateAndroid::GetWeakPtrToBaseClass() {
  return weak_ptr_factory_.GetWeakPtr();
}

void PasswordSyncControllerDelegateAndroid::ClearMetadataIfStopped() {
  // No metadata is managed by PasswordSyncControllerDelegateAndroid.
}

void PasswordSyncControllerDelegateAndroid::ReportBridgeErrorForTest() {
  // Not supported for Android.
  NOTREACHED();
}

}  // namespace password_manager
