// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/password_manager/android/password_store_android_account_backend.h"

#include "base/android/build_info.h"
#include "base/metrics/histogram_functions.h"
#include "base/task/sequenced_task_runner.h"
#include "chrome/browser/password_manager/android/password_manager_eviction_util.h"
#include "chrome/browser/password_manager/android/password_manager_lifecycle_helper_impl.h"
#include "chrome/browser/password_manager/android/password_sync_controller_delegate_android.h"
#include "chrome/browser/password_manager/android/password_sync_controller_delegate_bridge_impl.h"
#include "components/password_manager/core/browser/affiliation/affiliated_match_helper.h"
#include "components/password_manager/core/browser/affiliation/affiliations_prefetcher.h"
#include "components/password_manager/core/browser/features/password_features.h"
#include "components/password_manager/core/browser/password_store/get_logins_with_affiliations_request_handler.h"
#include "components/password_manager/core/browser/password_store/password_store_backend_error.h"
#include "components/password_manager/core/browser/password_store/password_store_backend_metrics_recorder.h"
#include "components/password_manager/core/browser/password_store/split_stores_and_local_upm.h"
#include "components/password_manager/core/browser/password_sync_util.h"
#include "components/password_manager/core/common/password_manager_pref_names.h"
#include "components/sync/base/features.h"
#include "components/sync/model/proxy_model_type_controller_delegate.h"

namespace password_manager {

namespace {

constexpr char kUPMActiveHistogram[] =
    "PasswordManager.UnifiedPasswordManager.ActiveStatus2";

std::string GetSyncingAccount(const syncer::SyncService* sync_service) {
  CHECK(sync_service);
  // TODO(crbug.com/40067770): Migrate away from `ConsentLevel::kSync` on
  // Android.
  return sync_util::GetAccountEmailIfSyncFeatureEnabledIncludingPasswords(
      sync_service);
}

void LogUPMActiveStatus(syncer::SyncService* sync_service, PrefService* prefs) {
  // TODO(crbug.com/40067770): Migrate away from `ConsentLevel::kSync` on
  // Android.
  if (!sync_util::IsSyncFeatureEnabledIncludingPasswords(sync_service)) {
    base::UmaHistogramEnumeration(
        kUPMActiveHistogram,
        UnifiedPasswordManagerActiveStatus::kInactiveSyncOff);
    return;
  }

  // This check enrolls the client into "RemoveUPMUnenrollment" study allowing
  // us to understand the impact of removing unenrollemnt and percentage of user
  // left without Password Manager / unenrolled from UPM.
  if (prefs->GetBoolean(prefs::kUserReceivedGMSCoreError)) {
    PasswordStoreAndroidBackendDispatcherBridge::CanRemoveUnenrollment();
  }

  if (password_manager_upm_eviction::IsCurrentUserEvicted(prefs)) {
    base::UmaHistogramEnumeration(
        kUPMActiveHistogram,
        UnifiedPasswordManagerActiveStatus::kInactiveUnenrolledDueToErrors);
    return;
  }

  base::UmaHistogramEnumeration(kUPMActiveHistogram,
                                UnifiedPasswordManagerActiveStatus::kActive);
}

enum class ActionOnApiError {
  // See password_manager_upm_eviction::EvictCurrentUser().
  kEvict,
  // See prefs::kSavePasswordsSuspendedByError.
  kDisableSaving,
  // See syncer::SyncService::SendExplicitPassphraseToPlatformClient().
  kDisableSavingAndTryFixPassphraseError,
};

ActionOnApiError GetRecoveryActionForPassphraseRequiredError(
    bool supports_passphrase_error_fix) {
  if (supports_passphrase_error_fix) {
    return ActionOnApiError::kDisableSavingAndTryFixPassphraseError;
  }
  if (base::FeatureList::IsEnabled(
          features::kUnifiedPasswordManagerSyncOnlyInGMSCore)) {
    return ActionOnApiError::kDisableSaving;
  }
  return ActionOnApiError::kEvict;
}

ActionOnApiError GetRecoveryActionOnApiError(
    AndroidBackendAPIErrorCode api_error_code,
    bool does_gms_core_version_allow_for_removing_unenrollment,
    bool supports_passphrase_error_fix) {
  switch (api_error_code) {
    case AndroidBackendAPIErrorCode::kAuthErrorResolvable:
    case AndroidBackendAPIErrorCode::kAuthErrorUnresolvable:
      return ActionOnApiError::kDisableSaving;
    case AndroidBackendAPIErrorCode::kPassphraseRequired:
      return GetRecoveryActionForPassphraseRequiredError(
          supports_passphrase_error_fix);
    case AndroidBackendAPIErrorCode::kNetworkError:
    case AndroidBackendAPIErrorCode::kApiNotConnected:
    case AndroidBackendAPIErrorCode::kConnectionSuspendedDuringCall:
    case AndroidBackendAPIErrorCode::kReconnectionTimedOut:
    case AndroidBackendAPIErrorCode::kBackendGeneric:
    case AndroidBackendAPIErrorCode::kInternalError:
    case AndroidBackendAPIErrorCode::kDeveloperError:
    case AndroidBackendAPIErrorCode::kAccessDenied:
    case AndroidBackendAPIErrorCode::kBadRequest:
    case AndroidBackendAPIErrorCode::kBackendResourceExhausted:
    case AndroidBackendAPIErrorCode::kInvalidData:
    case AndroidBackendAPIErrorCode::kUnmappedErrorCode:
    case AndroidBackendAPIErrorCode::kUnexpectedError:
    case AndroidBackendAPIErrorCode::kKeyRetrievalRequired:
    case AndroidBackendAPIErrorCode::kChromeSyncAPICallError:
    case AndroidBackendAPIErrorCode::kErrorWhileDoingLeakServiceGRPC:
    case AndroidBackendAPIErrorCode::kRequiredSyncingAccountMissing:
    case AndroidBackendAPIErrorCode::kLeakCheckServiceAuthError:
    case AndroidBackendAPIErrorCode::kLeakCheckServiceResourceExhausted:
      break;
  }
  if (does_gms_core_version_allow_for_removing_unenrollment) {
    return ActionOnApiError::kDisableSaving;
  }
  return base::FeatureList::IsEnabled(
             features::kUnifiedPasswordManagerSyncOnlyInGMSCore)
             ? ActionOnApiError::kDisableSaving
             : ActionOnApiError::kEvict;
}

template <typename Response, typename CallbackType>
void ReplyWithEmptyList(CallbackType callback) {
  base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE, base::BindOnce(std::move(callback), Response()));
}

}  // namespace

PasswordStoreAndroidAccountBackend::PasswordStoreAndroidAccountBackend(
    PrefService* prefs,
    AffiliationsPrefetcher* affiliations_prefetcher,
    password_manager::IsAccountStore is_account_store)
    : PasswordStoreAndroidBackend(
          PasswordStoreAndroidBackendBridgeHelper::Create(is_account_store),
          std::make_unique<PasswordManagerLifecycleHelperImpl>(),
          prefs),
      affiliations_prefetcher_(affiliations_prefetcher) {
  sync_controller_delegate_ =
      std::make_unique<PasswordSyncControllerDelegateAndroid>(
          std::make_unique<PasswordSyncControllerDelegateBridgeImpl>());
  sync_controller_delegate_->SetSyncObserverCallbacks(
      base::BindRepeating(
          &PasswordStoreAndroidAccountBackend::OnPasswordsSyncStateChanged,
          weak_ptr_factory_.GetWeakPtr()),
      base::BindOnce(&PasswordStoreAndroidAccountBackend::SyncShutdown,
                     weak_ptr_factory_.GetWeakPtr()));
}

PasswordStoreAndroidAccountBackend::PasswordStoreAndroidAccountBackend(
    base::PassKey<class PasswordStoreAndroidAccountBackendTest> key,
    std::unique_ptr<PasswordStoreAndroidBackendBridgeHelper> bridge_helper,
    std::unique_ptr<PasswordManagerLifecycleHelper> lifecycle_helper,
    std::unique_ptr<PasswordSyncControllerDelegateAndroid>
        sync_controller_delegate,
    PrefService* prefs,
    AffiliationsPrefetcher* affiliations_prefetcher)
    : PasswordStoreAndroidBackend(std::move(bridge_helper),
                                  std::move(lifecycle_helper),
                                  prefs),
      affiliations_prefetcher_(affiliations_prefetcher) {
  sync_controller_delegate_ = std::move(sync_controller_delegate);
  sync_controller_delegate_->SetSyncObserverCallbacks(
      base::BindRepeating(
          &PasswordStoreAndroidAccountBackend::OnPasswordsSyncStateChanged,
          weak_ptr_factory_.GetWeakPtr()),
      base::BindOnce(&PasswordStoreAndroidAccountBackend::SyncShutdown,
                     weak_ptr_factory_.GetWeakPtr()));
}

PasswordStoreAndroidAccountBackend::~PasswordStoreAndroidAccountBackend() =
    default;

void PasswordStoreAndroidAccountBackend::InitBackend(
    AffiliatedMatchHelper* affiliated_match_helper,
    RemoteChangesReceived remote_form_changes_received,
    base::RepeatingClosure sync_enabled_or_disabled_cb,
    base::OnceCallback<void(bool)> completion) {
  Init(std::move(remote_form_changes_received));
  // The android backend doesn't currently support notifying the store of
  // sync changes. This currently only wired via the built-in backend being
  // notified by the `PasswordSyncBridge` and generally
  // applies to the account store. Support needs to be specifically implemented
  // if desired. See crbug.com/1004777.
  CHECK(!sync_enabled_or_disabled_cb);
  CHECK(completion);
  affiliated_match_helper_ = affiliated_match_helper;
  std::move(completion).Run(/*success*/ true);
}

void PasswordStoreAndroidAccountBackend::Shutdown(
    base::OnceClosure shutdown_completed) {
  affiliated_match_helper_ = nullptr;
  sync_service_ = nullptr;
  PasswordStoreAndroidBackend::Shutdown(std::move(shutdown_completed));
}

bool PasswordStoreAndroidAccountBackend::IsAbleToSavePasswords() {
  base::UmaHistogramBoolean(
      "PasswordManager.PasswordSavingDisabledDueToGMSCoreError",
      should_disable_saving_due_to_error_);
  return sync_service_ != nullptr && !should_disable_saving_due_to_error_;
}

void PasswordStoreAndroidAccountBackend::GetAllLoginsAsync(
    LoginsOrErrorReply callback) {
  if (!sync_util::IsSyncFeatureEnabledIncludingPasswords(sync_service_)) {
    ReplyWithEmptyList<LoginsResult>(std::move(callback));
    return;
  }
  GetAllLoginsInternal(GetSyncingAccount(sync_service_), std::move(callback));
}

void PasswordStoreAndroidAccountBackend::
    GetAllLoginsWithAffiliationAndBrandingAsync(LoginsOrErrorReply callback) {
  if (!sync_util::IsSyncFeatureEnabledIncludingPasswords(sync_service_)) {
    ReplyWithEmptyList<LoginsResult>(std::move(callback));
    return;
  }
  if (bridge_helper()->CanUseGetAllLoginsWithBrandingInfoAPI()) {
    GetAllLoginsWithAffiliationAndBrandingInternal(
        GetSyncingAccount(sync_service_), std::move(callback));
    return;
  }
  auto affiliation_injection =
      base::BindOnce(&PasswordStoreAndroidAccountBackend::
                         InjectAffiliationAndBrandingInformation,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback));
  GetAllLoginsInternal(GetSyncingAccount(sync_service_),
                       std::move(affiliation_injection));
}

void PasswordStoreAndroidAccountBackend::GetAutofillableLoginsAsync(
    LoginsOrErrorReply callback) {
  if (!sync_util::IsSyncFeatureEnabledIncludingPasswords(sync_service_)) {
    ReplyWithEmptyList<LoginsResult>(std::move(callback));
    return;
  }
  GetAutofillableLoginsInternal(GetSyncingAccount(sync_service_),
                                std::move(callback));
}

void PasswordStoreAndroidAccountBackend::GetAllLoginsForAccountAsync(
    std::string account,
    LoginsOrErrorReply callback) {
  CHECK(!account.empty());
  // This method is only used before the store split, to migrate non-syncable
  // data back to the built-in backend after password sync turns off.
  CHECK(!password_manager::UsesSplitStoresAndUPMForLocal(prefs()));
  GetAllLoginsInternal(std::move(account), std::move(callback));
}

void PasswordStoreAndroidAccountBackend::FillMatchingLoginsAsync(
    LoginsOrErrorReply callback,
    bool include_psl,
    const std::vector<PasswordFormDigest>& forms) {
  if (!sync_util::IsSyncFeatureEnabledIncludingPasswords(sync_service_)) {
    ReplyWithEmptyList<LoginsResult>(std::move(callback));
    return;
  }
  FillMatchingLoginsInternal(GetSyncingAccount(sync_service_),
                             std::move(callback), include_psl, forms);
}

void PasswordStoreAndroidAccountBackend::GetGroupedMatchingLoginsAsync(
    const PasswordFormDigest& form_digest,
    LoginsOrErrorReply callback) {
  if (!sync_util::IsSyncFeatureEnabledIncludingPasswords(sync_service_)) {
    ReplyWithEmptyList<LoginsResult>(std::move(callback));
    return;
  }
  if (bridge_helper()->CanUseGetAffiliatedPasswordsAPI()) {
    GetGroupedMatchingLoginsInternal(GetSyncingAccount(sync_service_),
                                     form_digest, std::move(callback));
    return;
  }

  GetLoginsWithAffiliationsRequestHandler(
      form_digest, this, affiliated_match_helper_.get(), std::move(callback));
}

void PasswordStoreAndroidAccountBackend::AddLoginAsync(
    const PasswordForm& form,
    PasswordChangesOrErrorReply callback) {
  CHECK(sync_util::IsSyncFeatureEnabledIncludingPasswords(sync_service_));
  AddLoginInternal(GetSyncingAccount(sync_service_), form, std::move(callback));
}

void PasswordStoreAndroidAccountBackend::UpdateLoginAsync(
    const PasswordForm& form,
    PasswordChangesOrErrorReply callback) {
  CHECK(sync_util::IsSyncFeatureEnabledIncludingPasswords(sync_service_));
  UpdateLoginInternal(GetSyncingAccount(sync_service_), form,
                      std::move(callback));
}

void PasswordStoreAndroidAccountBackend::RemoveLoginAsync(
    const PasswordForm& form,
    PasswordChangesOrErrorReply callback) {
  if (!sync_util::IsSyncFeatureEnabledIncludingPasswords(sync_service_)) {
    ReplyWithEmptyList<PasswordStoreChangeList>(std::move(callback));
    return;
  }
  RemoveLoginInternal(GetSyncingAccount(sync_service_), form,
                      std::move(callback));
}

void PasswordStoreAndroidAccountBackend::RemoveLoginsByURLAndTimeAsync(
    const base::RepeatingCallback<bool(const GURL&)>& url_filter,
    base::Time delete_begin,
    base::Time delete_end,
    base::OnceCallback<void(bool)> sync_completion,
    PasswordChangesOrErrorReply callback) {
  if (!sync_util::IsSyncFeatureEnabledIncludingPasswords(sync_service_)) {
    ReplyWithEmptyList<PasswordStoreChangeList>(std::move(callback));
    return;
  }
  RemoveLoginsByURLAndTimeInternal(GetSyncingAccount(sync_service_), url_filter,
                                   delete_begin, delete_end,
                                   std::move(callback));
}

void PasswordStoreAndroidAccountBackend::RemoveLoginsCreatedBetweenAsync(
    base::Time delete_begin,
    base::Time delete_end,
    PasswordChangesOrErrorReply callback) {
  if (!sync_util::IsSyncFeatureEnabledIncludingPasswords(sync_service_)) {
    ReplyWithEmptyList<PasswordStoreChangeList>(std::move(callback));
    return;
  }
  RemoveLoginsCreatedBetweenInternal(GetSyncingAccount(sync_service_),
                                     delete_begin, delete_end,
                                     std::move(callback));
}

void PasswordStoreAndroidAccountBackend::DisableAutoSignInForOriginsAsync(
    const base::RepeatingCallback<bool(const GURL&)>& origin_filter,
    base::OnceClosure completion) {
  CHECK(sync_util::IsSyncFeatureEnabledIncludingPasswords(sync_service_));
  DisableAutoSignInForOriginsInternal(GetSyncingAccount(sync_service_),
                                      origin_filter, std::move(completion));
}

std::unique_ptr<syncer::ProxyModelTypeControllerDelegate>
PasswordStoreAndroidAccountBackend::CreateSyncControllerDelegate() {
  return sync_controller_delegate_->CreateProxyModelControllerDelegate();
}

SmartBubbleStatsStore*
PasswordStoreAndroidAccountBackend::GetSmartBubbleStatsStore() {
  return nullptr;
}

base::WeakPtr<PasswordStoreBackend>
PasswordStoreAndroidAccountBackend::AsWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

PasswordStoreBackendErrorRecoveryType
PasswordStoreAndroidAccountBackend::RecoverOnErrorAndReturnResult(
    AndroidBackendAPIErrorCode error) {
  CHECK(sync_service_);
  switch (GetRecoveryActionOnApiError(
      error, bridge_helper()->CanRemoveUnenrollment(),
      sync_service_->SupportsExplicitPassphrasePlatformClient())) {
    case ActionOnApiError::kEvict: {
      // if `kUnifiedPasswordManagerSyncOnlyInGMSCore` is enabled eviction
      // should not happen.
      CHECK(!base::FeatureList::IsEnabled(
          password_manager::features::
              kUnifiedPasswordManagerSyncOnlyInGMSCore));
      if (!password_manager_upm_eviction::IsCurrentUserEvicted(prefs())) {
        password_manager_upm_eviction::EvictCurrentUser(static_cast<int>(error),
                                                        prefs());
      }
      return PasswordStoreBackendErrorRecoveryType::kUnrecoverable;
    }
    case ActionOnApiError::kDisableSavingAndTryFixPassphraseError:
      CHECK(sync_service_->SupportsExplicitPassphrasePlatformClient());
      sync_service_->SendExplicitPassphraseToPlatformClient();
      ABSL_FALLTHROUGH_INTENDED;
    case ActionOnApiError::kDisableSaving:
      should_disable_saving_due_to_error_ = true;
      return PasswordStoreBackendErrorRecoveryType::kRecoverable;
  }
}

void PasswordStoreAndroidAccountBackend::OnCallToGMSCoreSucceeded() {
  // Since the API call has succeeded, it's safe to reenable saving.
  should_disable_saving_due_to_error_ = false;
}

std::string PasswordStoreAndroidAccountBackend::GetAccountToRetryOperation() {
  CHECK(sync_service_);
  return GetSyncingAccount(sync_service_);
}

PasswordStoreBackendMetricsRecorder::PasswordStoreAndroidBackendType
PasswordStoreAndroidAccountBackend::GetStorageType() {
  return PasswordStoreBackendMetricsRecorder::PasswordStoreAndroidBackendType::
      kAccount;
}

void PasswordStoreAndroidAccountBackend::OnSyncServiceInitialized(
    syncer::SyncService* sync_service) {
  // TODO(crbug.com/1335387) Check if this might be called multiple times
  // without a need for it. If it is don't repeatedly initialize the sync
  // service to make it clear that it's not needed to do so for future readers
  // of the code.
  if (!sync_service_) {
    LogUPMActiveStatus(sync_service, prefs());
  }
  sync_service_ = sync_service;
  sync_controller_delegate_->OnSyncServiceInitialized(sync_service);

  // Stop fetching affiliations if AndroidBackend can be used and branding info
  // can be obtained directly from the GMS Core backend.
  if (!prefs()->GetBoolean(
          prefs::kUnenrolledFromGoogleMobileServicesDueToErrors) &&
      sync_util::IsSyncFeatureEnabledIncludingPasswords(sync_service_) &&
      bridge_helper()->CanUseGetAllLoginsWithBrandingInfoAPI()) {
    affiliations_prefetcher_->DisablePrefetching();
  }
}

void PasswordStoreAndroidAccountBackend::
    RecordAddLoginAsyncCalledFromTheStore() {
  base::UmaHistogramBoolean(
      "PasswordManager.PasswordStore.AccountBackend.AddLoginCalledOnStore",
      true);
}

void PasswordStoreAndroidAccountBackend::
    RecordUpdateLoginAsyncCalledFromTheStore() {
  base::UmaHistogramBoolean(
      "PasswordManager.PasswordStore.AccountBackend.UpdateLoginCalledOnStore",
      true);
}

void PasswordStoreAndroidAccountBackend::
    InjectAffiliationAndBrandingInformation(
        LoginsOrErrorReply callback,
        LoginsResultOrError forms_or_error) {
  if (!affiliated_match_helper_ ||
      absl::holds_alternative<PasswordStoreBackendError>(forms_or_error) ||
      absl::get<LoginsResult>(forms_or_error).empty()) {
    std::move(callback).Run(std::move(forms_or_error));
    return;
  }
  affiliated_match_helper_->InjectAffiliationAndBrandingInformation(
      std::move(absl::get<LoginsResult>(forms_or_error)), std::move(callback));
}

void PasswordStoreAndroidAccountBackend::OnPasswordsSyncStateChanged() {
  // Reply with a recoverable error, because this isn't a persistent issue,
  // only a transient state
  ClearAllTasksAndReplyWithReason(
      AndroidBackendError(
          AndroidBackendErrorType::kCancelledPwdSyncStateChanged),
      PasswordStoreBackendError(
          PasswordStoreBackendErrorType::kUncategorized,
          PasswordStoreBackendErrorRecoveryType::kRecoverable));
}

void PasswordStoreAndroidAccountBackend::SyncShutdown() {
  ClearAllTasksAndReplyWithReason(
      AndroidBackendError(
          AndroidBackendErrorType::kCancelledPwdSyncStateChanged),
      PasswordStoreBackendError(
          PasswordStoreBackendErrorType::kUncategorized,
          PasswordStoreBackendErrorRecoveryType::kRecoverable));
  sync_service_ = nullptr;
}

}  // namespace password_manager
