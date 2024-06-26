#include "build/buildflag.h"

namespace wootz_vpn {

inline constexpr char kManageUrlProd[] = "https://account.wootz.com/account/";
inline constexpr char kManageUrlStaging[] =
    "https://account.wootzsoftware.com/account/";
inline constexpr char kManageUrlDev[] =
    "https://account.wootz.software/account/";

// TODO(simonhong): Update when vpn feedback url is ready.
inline constexpr char kFeedbackUrl[] = "https://support.wootz.com/";
inline constexpr char kAboutUrl[] = "https://wootz.com/firewall-vpn/";

inline constexpr char kRegionContinentKey[] = "continent";
inline constexpr char kRegionNameKey[] = "name";
inline constexpr char kRegionNamePrettyKey[] = "name-pretty";
inline constexpr char kRegionCountryIsoCodeKey[] = "country-iso-code";
inline constexpr char kCreateSupportTicket[] =
    "api/v1.2/partners/support-ticket";
inline constexpr char kSupportTicketEmailKey[] = "email";
inline constexpr char kSupportTicketSubjectKey[] = "subject";
inline constexpr char kSupportTicketSupportTicketKey[] = "support-ticket";
inline constexpr char kSupportTicketPartnerClientIdKey[] = "partner-client-id";
inline constexpr char kSupportTicketTimezoneKey[] = "timezone";

inline constexpr char kVpnHost[] = "connect-api.guardianapp.com";

inline constexpr char kAllServerRegions[] = "api/v1/servers/all-server-regions";
inline constexpr char kTimezonesForRegions[] =
    "api/v1.1/servers/timezones-for-regions";
inline constexpr char kHostnameForRegion[] =
    "api/v1.2/servers/hostnames-for-region";
inline constexpr char kProfileCredential[] = "api/v1.1/register-and-create";
inline constexpr char kCredential[] = "api/v1.3/device/";
inline constexpr char kCreateSubscriberCredentialV12[] =
    "api/v1.2/subscriber-credential/create";
constexpr int kP3AIntervalHours = 24;

inline constexpr char kSubscriberCredentialKey[] = "credential";
inline constexpr char kSkusCredentialKey[] = "skus_credential";
inline constexpr char kRetriedSkusCredentialKey[] = "retried_skus_credential";
inline constexpr char kSubscriberCredentialExpirationKey[] = "expiration";

#if !BUILDFLAG(IS_ANDROID)
inline constexpr char kTokenNoLongerValid[] = "Token No Longer Valid";
#endif  // !BUILDFLAG(IS_ANDROID)

}  // namespace wootz_vpn
