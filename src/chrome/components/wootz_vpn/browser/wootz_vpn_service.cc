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
#include "chrome/components/p3a_utils/feature_usage.h"
#include "chrome/components/skus/browser/skus_utils.h"
#include "chrome/components/version_info/version_info.h"
#include "chrome/components/wootz_vpn/browser/api/wootz_vpn_api_helper.h"
#include "chrome/components/wootz_vpn/browser/connection/wootz_vpn_region_data_manager.h"
#include "chrome/components/wootz_vpn/browser/wootz_vpn_service_helper.h"
#include "chrome/components/wootz_vpn/common/pref_names.h"
#include "chrome/components/wootz_vpn/common/wootz_vpn_constants.h"
#include "chrome/components/wootz_vpn/common/wootz_vpn_utils.h"
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

  // Ignore connection state change request for non purchased user.
  // This can be happened when user controls vpn via os settings.

  if (state == ConnectionState::CONNECTED) {
    // If user connected vpn from the system and launched the browser
    // we detected it was disabled by policies and disabling it.
    if (IsWootzVPNDisabledByPolicy(profile_prefs_)) {
      connection_manager_->Disconnect();
      return;
    }

    RecordP3A(true);
  }

  for (const auto& obs : observers_) {
    obs->OnConnectionStateChanged(state);
  }
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
  return GetConnectionState() == ConnectionState::CONNECTED;
}

void WootzVpnService::Connect() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  connection_manager_->Connect();
}

void WootzVpnService::Disconnect() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

  connection_manager_->Disconnect();
}

void WootzVpnService::ToggleConnection() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);

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
  api_request_->CreateSupportTicket(std::move(internal_callback), email,
                                    subject, body);
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
#endif  // !BUILDFLAG(IS_ANDROID)

void WootzVpnService::AddObserver(
    mojo::PendingRemote<mojom::ServiceObserver> observer) {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  observers_.Add(std::move(observer));
}

void WootzVpnService::RequestCredentialSummary(const std::string& domain) {
  // As we request new credential, clear cached value.

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
    return;
  }

  std::optional<base::Value> records_v = base::JSONReader::Read(
      summary_string, base::JSONParserOptions::JSON_PARSE_RFC);

  // Early return when summary is invalid or it's empty dict.
  if (!records_v || !records_v->is_dict()) {
    VLOG(1) << __func__ << " : Got invalid credential summary!";
    return;
  }

  // Empty dict - clean user.
  if (records_v->GetDict().empty()) {
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
  } else {
    // When reaches here, remained_credential is zero. We can treat it as
    // user's current purchase is expired.
    VLOG(1) << __func__ << " : don't have remained credential.";
#if BUILDFLAG(IS_ANDROID)
    VLOG(1) << __func__ << " : Treat it as not purchased state in android.";
#else
    VLOG(1) << __func__ << " : Treat it as session expired state in desktop.";
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

void WootzVpnService::SetCurrentEnvironment(const std::string& env) {
  local_prefs_->SetString(prefs::kWootzVPNEnvironment, env);
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

void WootzVpnService::GetProfileCredentials(ResponseCallback callback,
                                            const std::string& hostname) {
  api_request_->GetProfileCredentials(std::move(callback), hostname);
}

void WootzVpnService::VerifyCredentials(ResponseCallback callback,
                                        const std::string& hostname,
                                        const std::string& client_id,
                                        const std::string& api_auth_token) {
  api_request_->VerifyCredentials(std::move(callback), hostname, client_id,
                                  api_auth_token);
}

void WootzVpnService::InvalidateCredentials(ResponseCallback callback,
                                            const std::string& hostname,
                                            const std::string& client_id,
                                            const std::string& api_auth_token) {
  api_request_->InvalidateCredentials(std::move(callback), hostname, client_id,
                                      api_auth_token);
}
}  // namespace wootz_vpn
