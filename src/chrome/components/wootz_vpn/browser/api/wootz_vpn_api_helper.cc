#include "chrome/components/wootz_vpn/browser/api/wootz_vpn_api_helper.h"

#include <algorithm>
#include <optional>
#include <utility>

#include "base/base64.h"
#include "base/json/values_util.h"
#include "base/notreached.h"
#include "base/ranges/algorithm.h"
#include "base/strings/string_util.h"
#include "base/time/time.h"
#include "base/values.h"
#include "chrome/components/wootz_vpn/common/wootz_vpn_constants.h"
#include "chrome/components/wootz_vpn/common/wootz_vpn_data_types.h"
#include "chrome/components/wootz_vpn/common/pref_names.h"
#include "chrome/components/skus/browser/skus_utils.h"
#include "components/prefs/pref_service.h"
#include "third_party/icu/source/i18n/unicode/timezone.h"

namespace wootz_vpn {

std::unique_ptr<Hostname> PickBestHostname(
    const std::vector<Hostname>& hostnames) {
  std::vector<Hostname> filtered_hostnames;
  std::copy_if(hostnames.begin(), hostnames.end(),
               std::back_inserter(filtered_hostnames),
               [](const Hostname& hostname) { return !hostname.is_offline; });

  std::sort(filtered_hostnames.begin(), filtered_hostnames.end(),
            [](const Hostname& a, const Hostname& b) {
              return a.capacity_score > b.capacity_score;
            });

  if (filtered_hostnames.empty())
    return std::make_unique<Hostname>();

  // Pick highest capacity score.
  return std::make_unique<Hostname>(filtered_hostnames[0]);
}

std::vector<Hostname> ParseHostnames(const base::Value::List& hostnames_value) {
  std::vector<Hostname> hostnames;
  for (const auto& value : hostnames_value) {
    DCHECK(value.is_dict());
    if (!value.is_dict())
      continue;

    const auto& dict = value.GetDict();
    constexpr char kHostnameKey[] = "hostname";
    constexpr char kDisplayNameKey[] = "display-name";
    constexpr char kOfflineKey[] = "offline";
    constexpr char kCapacityScoreKey[] = "capacity-score";
    const std::string* hostname_str = dict.FindString(kHostnameKey);
    const std::string* display_name_str = dict.FindString(kDisplayNameKey);
    std::optional<bool> offline = dict.FindBool(kOfflineKey);
    std::optional<int> capacity_score = dict.FindInt(kCapacityScoreKey);

    if (!hostname_str || !display_name_str || !offline || !capacity_score)
      continue;

    hostnames.push_back(
        Hostname{*hostname_str, *display_name_str, *offline, *capacity_score});
  }

  return hostnames;
}

std::string GetTimeZoneName() {
  std::unique_ptr<icu::TimeZone> zone(icu::TimeZone::createDefault());
  icu::UnicodeString id;
  zone->getID(id);
  std::string current_time_zone;
  id.toUTF8String<std::string>(current_time_zone);
  return current_time_zone;
}

base::Value::Dict GetValueWithTicketInfos(
    const std::string& email,
    const std::string& subject,
    const std::string& body,
    const std::string& timezone) {
  base::Value::Dict dict;

  std::string email_trimmed, subject_trimmed, body_trimmed, body_encoded;

  base::TrimWhitespaceASCII(email, base::TRIM_ALL, &email_trimmed);
  base::TrimWhitespaceASCII(subject, base::TRIM_ALL, &subject_trimmed);
  
  // required fields
  dict.Set(kSupportTicketEmailKey, email_trimmed);
  dict.Set(kSupportTicketSubjectKey, subject_trimmed);
  dict.Set(kSupportTicketSupportTicketKey, base::Base64Encode(body_trimmed));
  dict.Set(kSupportTicketPartnerClientIdKey, "com.wootz.browser");
  dict.Set(kSupportTicketTimezoneKey, timezone);

  return dict;
}
}  // namespace wootz_vpn
