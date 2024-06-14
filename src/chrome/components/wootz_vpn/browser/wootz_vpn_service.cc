#include "chrome/components/wootz_vpn/browser/wootz_vpn_service.h"

#include <algorithm>
#include <optional>

#include "base/base64.h"
#include "base/check_is_test.h"
#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "chrome/components/wootz_vpn/browser/api/wootz_vpn_api_helper.h"
#include "chrome/components/wootz_vpn/browser/wootz_vpn_service_helper.h"
#include "chrome/components/wootz_vpn/browser/connection/wootz_vpn_region_data_manager.h"
#include "chrome/components/wootz_vpn/common/wootz_vpn_constants.h"
#include "chrome/components/wootz_vpn/common/wootz_vpn_utils.h"
#include "chrome/components/wootz_vpn/common/pref_names.h"
#include "chrome/components/p3a_utils/feature_usage.h"
#include "chrome/components/skus/browser/skus_utils.h"
#include "chrome/components/version_info/version_info.h"
#include "components/grit/wootz_components_strings.h"
#include "components/prefs/pref_service.h"
#include "components/version_info/version_info.h"
#include "net/cookies/cookie_inclusion_status.h"
#include "net/cookies/cookie_util.h"
#include "net/cookies/parsed_cookie.h"
#include "ui/base/l10n/l10n_util.h"
#include "url/url_util.h"

namespace wootz_vpn {

using ConnectionState = mojom::ConnectionState;
using PurchasedState = mojom::PurchasedState;

WootzVpnService::WootzVpnService(
    WootzVPNConnectionManager* connection_manager,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    PrefService* local_prefs,
    PrefService* profile_prefs,
    base::RepeatingCallback<mojo::PendingRemote<skus::mojom::SkusService>()>
        skus_service_getter)
    : local_prefs_(local_prefs),
      profile_prefs_(profile_prefs),
      skus_service_getter_(skus_service_getter),
      api_request_(new WootzVpnAPIRequest(url_loader_factory)) {
  DCHECK(IsWootzVPNFeatureEnabled());
#if !BUILDFLAG(IS_ANDROID)
  DCHECK(connection_manager);
  connection_manager_ = connection_manager;
  observed_.Observe(connection_manager_);

  policy_pref_change_registrar_.Init(profile_prefs_);
  policy_pref_change_registrar_.Add(
      prefs::kManagedWootzVPNDisabled,
      base::BindRepeating(&WootzVpnService::OnPreferenceChanged,
                          base::Unretained(this)));

#endif  // !BUILDFLAG(IS_ANDROID)

  CheckInitialState();
  InitP3A();
}

WootzVpnService::~WootzVpnService() = default;

bool WootzVpnService::IsWootzVPNEnabled() const {
  return ::wootz_vpn::IsWootzVPNEnabled(profile_prefs_);
}

std::string WootzVpnService::GetCurrentEnvironment() const {
  return local_prefs_->GetString(prefs::kWootzVPNEnvironment);
}

void WootzVpnService::BindInterface(
    mojo::PendingReceiver<mojom::ServiceHandler> receiver) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  receivers_.Add(this, std::move(receiver));
}

#if !BUILDFLAG(IS_ANDROID)
void WootzVpnService::OnConnectionStateChanged(mojom::ConnectionState state) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  VLOG(2) << __func__ << " " << state;
#if BUILDFLAG(IS_WIN)
  if (delegate_) {
    delegate_->WriteConnectionState(state);
  }
#endif
  // Ignore connection state change request for non purchased user.
  // This can be happened when user controls vpn via os settings.

  if (state == ConnectionState::CONNECTED) {
    // If user connected vpn from the system and launched the browser
    // we detected it was disabled by policies and disabling it.
    if (IsWootzVPNDisabledByPolicy(profile_prefs_)) {
      connection_manager_->Disconnect();
      return;
    }
#if BUILDFLAG(IS_WIN)
    // Run tray process each time we establish connection. System tray icon
    // manages self state to be visible/hidden due to settings.
    if (delegate_) {
      delegate_->ShowWootzVpnStatusTrayIcon();
    }
#endif
    RecordP3A(true);
  }

  for (const auto& obs : observers_)
    obs->OnConnectionStateChanged(state);
}

void WootzVpnService::OnRegionDataReady(bool success) {
  VLOG(2) << __func__ << " success - " << success << ", is waiting? "
          << wait_region_data_ready_;
  if (!wait_region_data_ready_) {
    return;
  }

  wait_region_data_ready_ = false;
}

void WootzVpnService::OnSelectedRegionChanged(const std::string& region_name) {
  const auto region_ptr = GetRegionPtrWithNameFromRegionList(
      region_name, connection_manager_->GetRegionDataManager().GetRegions());
  for (const auto& obs : observers_) {
    obs->OnSelectedRegionChanged(region_ptr.Clone());
  }
}

mojom::ConnectionState WootzVpnService::GetConnectionState() const {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  return connection_manager_->GetConnectionState();
}

bool WootzVpnService::IsConnected() const {
  if (!is_purchased_user()) {
    return false;
  }

  return GetConnectionState() == ConnectionState::CONNECTED;
}

void WootzVpnService::Connect() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!is_purchased_user()) {
    return;
  }

  connection_manager_->Connect();
}

void WootzVpnService::Disconnect() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!is_purchased_user()) {
    return;
  }

  connection_manager_->Disconnect();
}

void WootzVpnService::ToggleConnection() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  if (!is_purchased_user()) {
    return;
  }

  connection_manager_->ToggleConnection();
}

void WootzVpnService::GetConnectionState(GetConnectionStateCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  const auto state = connection_manager_->GetConnectionState();
  VLOG(2) << __func__ << " : " << state;
  std::move(callback).Run(state);
}

void WootzVpnService::GetAllRegions(GetAllRegionsCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  std::vector<mojom::RegionPtr> regions;
  for (const auto& region :
       connection_manager_->GetRegionDataManager().GetRegions()) {
    regions.push_back(region.Clone());
  }
  std::move(callback).Run(std::move(regions));
}

void WootzVpnService::GetSelectedRegion(GetSelectedRegionCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  VLOG(2) << __func__;

  auto region_name =
      connection_manager_->GetRegionDataManager().GetSelectedRegion();
  std::move(callback).Run(GetRegionPtrWithNameFromRegionList(
      region_name, connection_manager_->GetRegionDataManager().GetRegions()));
}

void WootzVpnService::SetSelectedRegion(mojom::RegionPtr region_ptr) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  VLOG(2) << __func__ << " : " << region_ptr->name_pretty;
  connection_manager_->SetSelectedRegion(region_ptr->name);
}

void WootzVpnService::GetProductUrls(GetProductUrlsCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  std::move(callback).Run(mojom::ProductUrls::New(
      kFeedbackUrl, kAboutUrl, GetManageUrl(GetCurrentEnvironment())));
}

void WootzVpnService::CreateSupportTicket(
    const std::string& email,
    const std::string& subject,
    const std::string& body,
    CreateSupportTicketCallback callback) {
  auto internal_callback =
      base::BindOnce(&WootzVpnService::OnCreateSupportTicket,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback));
  api_request_->CreateSupportTicket(
      std::move(internal_callback), email, subject, body,
      ::wootz_vpn::GetSubscriberCredential(local_prefs_));
}

void WootzVpnService::GetSupportData(GetSupportDataCallback callback) {
  std::string wootz_version =
      version_info::GetWootzVersionWithoutChromiumMajorVersion();

  std::move(callback).Run(wootz_version, std::string(version_info::GetOSType()),
                          connection_manager_->GetHostname(),
                          GetTimeZoneName());
}

void WootzVpnService::ResetConnectionState() {
  connection_manager_->ResetConnectionState();
}

void WootzVpnService::EnableOnDemand(bool enable) {
#if BUILDFLAG(IS_MAC)
  local_prefs_->SetBoolean(prefs::kWootzVPNOnDemandEnabled, enable);

  // If not connected, do nothing because on-demand bit will
  // be applied when new connection starts. Whenever new connection starts,
  // we create os vpn entry.
  if (IsConnected()) {
    VLOG(2) << __func__ << " : reconnect to apply on-demand config(" << enable
            << "> to current connection";
    Connect();
  }
#endif
}

void WootzVpnService::GetOnDemandState(GetOnDemandStateCallback callback) {
#if BUILDFLAG(IS_MAC)
  std::move(callback).Run(
      /*available*/ true,
      /*enabled*/ local_prefs_->GetBoolean(prefs::kWootzVPNOnDemandEnabled));
#else
  std::move(callback).Run(false, false);
#endif
}

// NOTE(bsclifton): Desktop uses API to create a ticket.
// Android and iOS directly send an email.
void WootzVpnService::OnCreateSupportTicket(
    CreateSupportTicketCallback callback,
    const std::string& ticket,
    bool success) {
  std::move(callback).Run(success, ticket);
}

void WootzVpnService::OnPreferenceChanged(const std::string& pref_name) {
  if (pref_name == prefs::kManagedWootzVPNDisabled) {
    if (IsWootzVPNDisabledByPolicy(profile_prefs_)) {
      connection_manager_->Disconnect();
    }
    return;
  }
}

void WootzVpnService::UpdatePurchasedStateForSessionExpired(
    const std::string& env) {
  // Double check that we don't set session expired state for fresh user.
  if (!connection_manager_->GetRegionDataManager().IsRegionDataReady()) {
    VLOG(1) << __func__ << " : Treat it as not purchased state for fresh user.";
    SetPurchasedState(env, PurchasedState::NOT_PURCHASED);
    return;
  }

  const auto session_expired_time =
      local_prefs_->GetTime(prefs::kWootzVPNSessionExpiredDate);
  // If it's not cached, it means this session expiration is first time since
  // last purchase because this cache is cleared when we get valid credential
  // summary.
  if (session_expired_time.is_null()) {
    local_prefs_->SetTime(prefs::kWootzVPNSessionExpiredDate,
                          base::Time::Now());
    SetPurchasedState(env, PurchasedState::SESSION_EXPIRED);
    return;
  }

  // Weird state. Maybe we don't see this condition.
  // Just checking for safe.
  if (session_expired_time > base::Time::Now()) {
    SetPurchasedState(env, PurchasedState::NOT_PURCHASED);
    return;
  }

  // Keep session expired state 30 days at most.
  constexpr int kSessionExpiredCheckingDurationInDays = 30;
  if ((base::Time::Now() - session_expired_time).InDays() >
      kSessionExpiredCheckingDurationInDays) {
    SetPurchasedState(env, PurchasedState::NOT_PURCHASED);
    return;
  }

  SetPurchasedState(env, PurchasedState::SESSION_EXPIRED);
}
#endif  // !BUILDFLAG(IS_ANDROID)

#if BUILDFLAG(IS_ANDROID)
void WootzVpnService::GetPurchaseToken(GetPurchaseTokenCallback callback) {
  std::string purchase_token_string = "";
  std::string package_string = "com.wootz.browser";
  std::string product_id_string = "wootz-firewall-vpn-premium";

  // Get the Android purchase token (for Google Play Store).
  // The value for this is validated on the account.wootz.com side
  auto* purchase_token =
      profile_prefs_->FindPreference(prefs::kWootzVPNPurchaseTokenAndroid);
  if (purchase_token && !purchase_token->IsDefaultValue()) {
    purchase_token_string =
        profile_prefs_->GetString(prefs::kWootzVPNPurchaseTokenAndroid);
  }

  // Package name is important; for real users, it'll be the Release package.
  // For testing we do have the ability to use the Nightly package.
  auto* package =
      profile_prefs_->FindPreference(prefs::kWootzVPNPackageAndroid);
  if (package && !package->IsDefaultValue()) {
    package_string = profile_prefs_->GetString(prefs::kWootzVPNPackageAndroid);
  }

  auto* product_id =
      profile_prefs_->FindPreference(prefs::kWootzVPNProductIdAndroid);
  if (product_id && !product_id->IsDefaultValue()) {
    product_id_string =
        profile_prefs_->GetString(prefs::kWootzVPNProductIdAndroid);
  }

  base::Value::Dict response;
  response.Set("type", "android");
  response.Set("raw_receipt", purchase_token_string);
  response.Set("package", package_string);
  response.Set("subscription_id", product_id_string);

  std::string response_json;
  base::JSONWriter::Write(response, &response_json);
  std::move(callback).Run(base::Base64Encode(response_json));
}
#endif  // BUILDFLAG(IS_ANDROID)

void WootzVpnService::AddObserver(
    mojo::PendingRemote<mojom::ServiceObserver> observer) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  observers_.Add(std::move(observer));
}

mojom::PurchasedInfo WootzVpnService::GetPurchasedInfoSync() const {
  return purchased_state_.value_or(
      mojom::PurchasedInfo(mojom::PurchasedState::NOT_PURCHASED, std::nullopt));
}

void WootzVpnService::GetPurchasedState(GetPurchasedStateCallback callback) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  std::move(callback).Run(GetPurchasedInfoSync().Clone());
}

void WootzVpnService::LoadPurchasedState(const std::string& domain) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (!skus::DomainIsForProduct(domain, "vpn")) {
    VLOG(2) << __func__ << ": LoadPurchasedState called for non-vpn product";
    return;
  }

  auto requested_env = skus::GetEnvironmentForDomain(domain);
  if (GetCurrentEnvironment() == requested_env &&
      GetPurchasedInfoSync().state == PurchasedState::LOADING) {
    VLOG(2) << __func__ << ": Loading in-progress";
    return;
  }

  SetPurchasedState(requested_env, PurchasedState::LOADING);

  if (HasValidSubscriberCredential(local_prefs_)) {
#if BUILDFLAG(IS_ANDROID)
    SetPurchasedState(requested_env, PurchasedState::PURCHASED);
#else
    if (connection_manager_->GetRegionDataManager().IsRegionDataReady()) {
      VLOG(2) << __func__
              << ": Set as a purchased user as we have valid subscriber "
                 "credentials & region data";
      SetPurchasedState(requested_env, PurchasedState::PURCHASED);
    } else {
      VLOG(2) << __func__ << ": Wait till we get valid region data.";
      // TODO(simonhong): Make purchases state independent from region data.
      wait_region_data_ready_ = true;
    }
    connection_manager_->GetRegionDataManager().FetchRegionDataIfNeeded();
#endif
    return;
  }

  if (HasValidSkusCredential(local_prefs_)) {
    // We can reach here if we fail to get subscriber credential from guardian.
    VLOG(2) << __func__
            << " Try to get subscriber credential with valid cached skus "
               "credential.";

    if (GetCurrentEnvironment() != requested_env) {
      SetCurrentEnvironment(requested_env);
    }

    api_request_->GetSubscriberCredentialV12(
        base::BindOnce(&WootzVpnService::OnGetSubscriberCredentialV12,
                       base::Unretained(this),
                       GetExpirationTimeForSkusCredential(local_prefs_)),
        GetSkusCredential(local_prefs_),
        GetWootzVPNPaymentsEnv(GetCurrentEnvironment()));
    return;
  }

  VLOG(2) << __func__
          << ": Checking purchased state as we doesn't have valid skus or "
             "subscriber credentials";

  RequestCredentialSummary(domain);
}

void WootzVpnService::RequestCredentialSummary(const std::string& domain) {
  // As we request new credential, clear cached value.
  ClearSubscriberCredential(local_prefs_);

  EnsureMojoConnected();
  skus_service_->CredentialSummary(
      domain, base::BindOnce(&WootzVpnService::OnCredentialSummary,
                             base::Unretained(this), domain));
}

void WootzVpnService::OnCredentialSummary(const std::string& domain,
                                          const std::string& summary_string) {
  if (!skus::DomainIsForProduct(domain, "vpn")) {
    VLOG(2) << __func__ << ": CredentialSummary called for non-vpn product";
    return;
  }

  auto env = skus::GetEnvironmentForDomain(domain);
  std::string summary_string_trimmed;
  base::TrimWhitespaceASCII(summary_string, base::TrimPositions::TRIM_ALL,
                            &summary_string_trimmed);
  if (summary_string_trimmed.length() == 0) {
    // no credential found; person needs to login
    VLOG(1) << __func__ << " : No credential found; user needs to login!";
    SetPurchasedState(env, PurchasedState::NOT_PURCHASED);
    return;
  }

  std::optional<base::Value> records_v = base::JSONReader::Read(
      summary_string, base::JSONParserOptions::JSON_PARSE_RFC);

  // Early return when summary is invalid or it's empty dict.
  if (!records_v || !records_v->is_dict()) {
    VLOG(1) << __func__ << " : Got invalid credential summary!";
    SetPurchasedState(env, PurchasedState::FAILED);
    return;
  }

  // Empty dict - clean user.
  if (records_v->GetDict().empty()) {
    SetPurchasedState(env, PurchasedState::NOT_PURCHASED);
    return;
  }

  if (IsValidCredentialSummary(*records_v)) {
    VLOG(1) << __func__ << " : Active credential found!";
    // if a credential is ready, we can present it
    EnsureMojoConnected();
    skus_service_->PrepareCredentialsPresentation(
        domain, "*",
        base::BindOnce(&WootzVpnService::OnPrepareCredentialsPresentation,
                       base::Unretained(this), domain));
#if !BUILDFLAG(IS_ANDROID)
    // Clear expired state data as we have active credentials.
    local_prefs_->SetTime(prefs::kWootzVPNSessionExpiredDate, {});
#endif
  } else if (IsValidCredentialSummaryButNeedActivation(*records_v)) {
    // Need to activate from account. Treat as not purchased till activated.
    VLOG(1) << __func__ << " : Need to activate vpn from account.";
    SetPurchasedState(env, PurchasedState::NOT_PURCHASED);
  } else {
    // When reaches here, remained_credential is zero. We can treat it as
    // user's current purchase is expired.
    VLOG(1) << __func__ << " : don't have remained credential.";
#if BUILDFLAG(IS_ANDROID)
    VLOG(1) << __func__ << " : Treat it as not purchased state in android.";
    SetPurchasedState(env, PurchasedState::NOT_PURCHASED);
#else
    VLOG(1) << __func__ << " : Treat it as session expired state in desktop.";
    UpdatePurchasedStateForSessionExpired(env);
#endif
  }
}

void WootzVpnService::OnPrepareCredentialsPresentation(
    const std::string& domain,
    const std::string& credential_as_cookie) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  auto env = skus::GetEnvironmentForDomain(domain);
  // Credential is returned in cookie format.
  net::CookieInclusionStatus status;
  net::ParsedCookie credential_cookie(credential_as_cookie,
                                      /*block_truncated=*/true, &status);

  // Credential value received needs to be URL decoded.
  // That leaves us with a Base64 encoded JSON blob which is the credential.
  const std::string encoded_credential = credential_cookie.Value();
  const auto time =
      net::cookie_util::ParseCookieExpirationTime(credential_cookie.Expires());
  url::RawCanonOutputT<char16_t> unescaped;
  url::DecodeURLEscapeSequences(
      encoded_credential, url::DecodeURLMode::kUTF8OrIsomorphic, &unescaped);
  std::string credential;
  base::UTF16ToUTF8(unescaped.data(), unescaped.length(), &credential);

  SetSkusCredential(local_prefs_, credential, time);

  if (GetCurrentEnvironment() != env) {
    // Change environment because we have successfully authorized with new one.
    SetCurrentEnvironment(env);
  }
}

// TODO(simonhong): Should move p3a to WootzVPNConnectionManager?
void WootzVpnService::InitP3A() {
  p3a_timer_.Start(FROM_HERE, base::Hours(kP3AIntervalHours), this,
                   &WootzVpnService::OnP3AInterval);
  RecordP3A(false);
}

void WootzVpnService::RecordP3A(bool new_usage) {
  if (new_usage) {
    p3a_utils::RecordFeatureUsage(local_prefs_, prefs::kWootzVPNFirstUseTime,
                                  prefs::kWootzVPNLastUseTime);
  }
  p3a_utils::RecordFeatureNewUserReturning(
      local_prefs_, prefs::kWootzVPNFirstUseTime, prefs::kWootzVPNLastUseTime,
      prefs::kWootzVPNUsedSecondDay, kNewUserReturningHistogramName);
  p3a_utils::RecordFeatureDaysInMonthUsed(
      local_prefs_, new_usage, prefs::kWootzVPNLastUseTime,
      prefs::kWootzVPNDaysInMonthUsed, kDaysInMonthUsedHistogramName);
  p3a_utils::RecordFeatureLastUsageTimeMetric(
      local_prefs_, prefs::kWootzVPNLastUseTime, kLastUsageTimeHistogramName);
}

#if BUILDFLAG(IS_ANDROID)
void WootzVpnService::RecordAndroidBackgroundP3A(int64_t session_start_time_ms,
                                                 int64_t session_end_time_ms) {
  if (session_start_time_ms < 0 || session_end_time_ms < 0) {
    RecordP3A(false);
    return;
  }
  base::Time session_start_time =
      base::Time::FromMillisecondsSinceUnixEpoch(
          static_cast<double>(session_start_time_ms))
          .LocalMidnight();
  base::Time session_end_time = base::Time::FromMillisecondsSinceUnixEpoch(
                                    static_cast<double>(session_end_time_ms))
                                    .LocalMidnight();
  for (base::Time day = session_start_time; day <= session_end_time;
       day += base::Days(1)) {
    bool is_last_day = day == session_end_time;
    // Call functions for each day in the last session to ensure
    // p3a_util functions produce the correct result
    p3a_utils::RecordFeatureUsage(local_prefs_, prefs::kWootzVPNFirstUseTime,
                                  prefs::kWootzVPNLastUseTime, day);
    p3a_utils::RecordFeatureNewUserReturning(
        local_prefs_, prefs::kWootzVPNFirstUseTime, prefs::kWootzVPNLastUseTime,
        prefs::kWootzVPNUsedSecondDay, kNewUserReturningHistogramName,
        is_last_day);
    p3a_utils::RecordFeatureDaysInMonthUsed(
        local_prefs_, day, prefs::kWootzVPNLastUseTime,
        prefs::kWootzVPNDaysInMonthUsed, kDaysInMonthUsedHistogramName,
        is_last_day);
  }
  p3a_utils::RecordFeatureLastUsageTimeMetric(
      local_prefs_, prefs::kWootzVPNLastUseTime, kLastUsageTimeHistogramName);
}
#endif

void WootzVpnService::OnP3AInterval() {
  RecordP3A(false);
}

void WootzVpnService::SetPurchasedState(
    const std::string& env,
    PurchasedState state,
    const std::optional<std::string>& description) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (GetPurchasedInfoSync().state == state || env != GetCurrentEnvironment()) {
    return;
  }

  VLOG(2) << __func__ << " : " << state;
  purchased_state_ = mojom::PurchasedInfo(state, description);

  for (const auto& obs : observers_)
    obs->OnPurchasedStateChanged(state, description);

#if !BUILDFLAG(IS_ANDROID)
  if (state == PurchasedState::PURCHASED) {
    connection_manager_->CheckConnection();

    // Some platform needs to install services to run vpn.
    connection_manager_->MaybeInstallSystemServices();
  }
#endif
}

void WootzVpnService::SetCurrentEnvironment(const std::string& env) {
  local_prefs_->SetString(prefs::kWootzVPNEnvironment, env);
  purchased_state_.reset();
}

void WootzVpnService::EnsureMojoConnected() {
  if (!skus_service_) {
    auto pending = skus_service_getter_.Run();
    skus_service_.Bind(std::move(pending));
  }
  DCHECK(skus_service_);
  skus_service_.set_disconnect_handler(base::BindOnce(
      &WootzVpnService::OnMojoConnectionError, base::Unretained(this)));
}

void WootzVpnService::OnMojoConnectionError() {
  skus_service_.reset();
  EnsureMojoConnected();
}

void WootzVpnService::Shutdown() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  skus_service_.reset();
  observers_.Clear();
  api_request_.reset();
  p3a_timer_.Stop();

#if !BUILDFLAG(IS_ANDROID)
  observed_.Reset();
  receivers_.Clear();
#endif  // !BUILDFLAG(IS_ANDROID)
}

void WootzVpnService::GetAllServerRegions(ResponseCallback callback) {
  api_request_->GetAllServerRegions(std::move(callback));
}

void WootzVpnService::GetTimezonesForRegions(ResponseCallback callback) {
  api_request_->GetTimezonesForRegions(std::move(callback));
}

void WootzVpnService::GetHostnamesForRegion(ResponseCallback callback,
                                            const std::string& region) {
  api_request_->GetHostnamesForRegion(std::move(callback), region);
}

void WootzVpnService::GetProfileCredentials(
    ResponseCallback callback,
    const std::string& hostname) {
  api_request_->GetProfileCredentials(std::move(callback),
                                       hostname);
}

void WootzVpnService::GetWireguardProfileCredentials(
    ResponseCallback callback,
    const std::string& public_key,
    const std::string& hostname) {
  api_request_->GetWireguardProfileCredentials(
      std::move(callback),  public_key, hostname);
}

void WootzVpnService::VerifyCredentials(
    ResponseCallback callback,
    const std::string& hostname,
    const std::string& client_id,
    const std::string& api_auth_token) {
  api_request_->VerifyCredentials(std::move(callback), hostname, client_id,
                                   api_auth_token);
}

void WootzVpnService::InvalidateCredentials(
    ResponseCallback callback,
    const std::string& hostname,
    const std::string& client_id,
    const std::string& api_auth_token) {
  api_request_->InvalidateCredentials(std::move(callback), hostname, client_id, api_auth_token);
}
}  // namespace wootz_vpn
