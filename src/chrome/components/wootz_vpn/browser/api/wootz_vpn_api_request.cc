#include "chrome/components/wootz_vpn/browser/api/wootz_vpn_api_request.h"

#include <utility>

#include "base/debug/dump_without_crashing.h"
#include "base/json/json_writer.h"
#include "chrome/components/wootz_vpn/browser/api/wootz_vpn_api_helper.h"
#include "chrome/components/wootz_vpn/browser/api/vpn_response_parser.h"
#include "chrome/components/wootz_vpn/common/wootz_vpn_constants.h"
#include "chrome/components/skus/browser/skus_utils.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "url/gurl.h"
#include "url/url_constants.h"

namespace {

net::NetworkTrafficAnnotationTag GetNetworkTrafficAnnotationTag() {
  return net::DefineNetworkTrafficAnnotation("wootz_vpn_service", R"(
      semantics {
        sender: "Wootz VPN Service"
        description:
          "This service is used to communicate with Guardian VPN apis"
          "on behalf of the user interacting with the Wootz VPN."
        trigger:
          "Triggered by user connecting the Wootz VPN."
        data:
          "Servers, hosts and credentials for Wootz VPN"
        destination: WEBSITE
      }
      policy {
        cookies_allowed: NO
        policy_exception_justification:
          "Not implemented."
      }
    )");
}

GURL GetURLWithPath(const std::string& host, const std::string& path) {
  return GURL(std::string(url::kHttpsScheme) + "://" + host).Resolve(path);
}

std::string CreateJSONRequestBody(base::ValueView node) {
  std::string json;
  base::JSONWriter::Write(node, &json);
  return json;
}

}  // namespace

namespace wootz_vpn {

WootzVpnAPIRequest::WootzVpnAPIRequest(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
    : api_request_helper_(GetNetworkTrafficAnnotationTag(),
                          url_loader_factory) {}

WootzVpnAPIRequest::~WootzVpnAPIRequest() = default;

void WootzVpnAPIRequest::GetAllServerRegions(ResponseCallback callback) {
  auto internal_callback =
      base::BindOnce(&WootzVpnAPIRequest::OnGetResponse,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback));
  GURL base_url = GetURLWithPath(kVpnHost, kAllServerRegions);
  OAuthRequest(base_url, "GET", "", std::move(internal_callback));
}

void WootzVpnAPIRequest::GetTimezonesForRegions(ResponseCallback callback) {
  auto internal_callback =
      base::BindOnce(&WootzVpnAPIRequest::OnGetResponse,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback));
  GURL base_url = GetURLWithPath(kVpnHost, kTimezonesForRegions);
  OAuthRequest(base_url, "GET", "", std::move(internal_callback));
}

void WootzVpnAPIRequest::GetHostnamesForRegion(ResponseCallback callback,
                                               const std::string& region) {
  DCHECK(!region.empty());
  static bool dump_sent = false;
  if (!dump_sent && region.empty()) {
    base::debug::DumpWithoutCrashing();
    dump_sent = true;
  }

  auto internal_callback =
      base::BindOnce(&WootzVpnAPIRequest::OnGetResponse,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback));
  GURL base_url = GetURLWithPath(kVpnHost, kHostnameForRegion);
  base::Value::Dict dict;
  dict.Set("region", region);
  std::string request_body = CreateJSONRequestBody(dict);
  OAuthRequest(base_url, "POST", request_body, std::move(internal_callback));
}

void WootzVpnAPIRequest::GetProfileCredentials(
    ResponseCallback callback,
    const std::string& hostname) {
  auto internal_callback =
      base::BindOnce(&WootzVpnAPIRequest::OnGetResponse,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback));
  GURL base_url = GetURLWithPath(hostname, kProfileCredential);
  base::Value::Dict dict;
  std::string request_body = CreateJSONRequestBody(dict);
  OAuthRequest(base_url, "POST", request_body, std::move(internal_callback));
}

void WootzVpnAPIRequest::GetWireguardProfileCredentials(
    ResponseCallback callback,
    const std::string& public_key,
    const std::string& hostname) {
  auto internal_callback =
      base::BindOnce(&WootzVpnAPIRequest::OnGetResponse,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback));
  GURL base_url = GetURLWithPath(hostname, kCredential);
  base::Value::Dict dict;
  dict.Set("public-key", public_key);
  dict.Set("transport-protocol", "wireguard");
  std::string request_body = CreateJSONRequestBody(dict);
  OAuthRequest(base_url, "POST", request_body, std::move(internal_callback));
}

void WootzVpnAPIRequest::VerifyCredentials(
    ResponseCallback callback,
    const std::string& hostname,
    const std::string& client_id,
    const std::string& api_auth_token) {
  auto internal_callback =
      base::BindOnce(&WootzVpnAPIRequest::OnGetResponse,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback));
  GURL base_url =
      GetURLWithPath(hostname, kCredential + client_id + "/verify-credentials");
  base::Value::Dict dict;
  dict.Set("api-auth-token", api_auth_token);
  std::string request_body = CreateJSONRequestBody(dict);
  OAuthRequest(base_url, "POST", request_body, std::move(internal_callback));
}

void WootzVpnAPIRequest::InvalidateCredentials(
    ResponseCallback callback,
    const std::string& hostname,
    const std::string& client_id,
    const std::string& api_auth_token) {
  auto internal_callback =
      base::BindOnce(&WootzVpnAPIRequest::OnGetResponse,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback));
  GURL base_url = GetURLWithPath(
      hostname, kCredential + client_id + "/invalidate-credentials");
  base::Value::Dict dict;
  dict.Set("api-auth-token", api_auth_token);
  std::string request_body = CreateJSONRequestBody(dict);
  OAuthRequest(base_url, "POST", request_body, std::move(internal_callback));
}

void WootzVpnAPIRequest::GetSubscriberCredential(
    ResponseCallback callback,
    const std::string& product_type,
    const std::string& product_id,
    const std::string& validation_method,
    const std::string& bundle_id) {
  auto internal_callback =
      base::BindOnce(&WootzVpnAPIRequest::OnGetSubscriberCredential,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback));
  GURL base_url = GetURLWithPath(kVpnHost, kCreateSubscriberCredentialV12);
  base::Value::Dict dict;
  dict.Set("product-type", product_type);
  dict.Set("product-id", product_id);
  dict.Set("validation-method", validation_method);
  dict.Set("bundle-id", bundle_id);
  std::string request_body = CreateJSONRequestBody(dict);
  OAuthRequest(base_url, "POST", request_body, std::move(internal_callback));
}

void WootzVpnAPIRequest::GetSubscriberCredentialV12(
    ResponseCallback callback,
    const std::string& skus_credential,
    const std::string& environment) {
  auto internal_callback =
      base::BindOnce(&WootzVpnAPIRequest::OnGetSubscriberCredential,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback));

  const GURL base_url =
      GetURLWithPath(kVpnHost, kCreateSubscriberCredentialV12);
  base::Value::Dict dict;
  dict.Set("validation-method", "wootz-premium");
  dict.Set("wootz-vpn-premium-monthly-pass", skus_credential);
  std::string request_body = CreateJSONRequestBody(dict);
  OAuthRequest(base_url, "POST", request_body, std::move(internal_callback),
               {{"Wootz-Payments-Environment", environment}});
}

void WootzVpnAPIRequest::CreateSupportTicket(
    ResponseCallback callback,
    const std::string& email,
    const std::string& subject,
    const std::string& body) {
  auto internal_callback =
      base::BindOnce(&WootzVpnAPIRequest::OnCreateSupportTicket,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback));

  OAuthRequest(
      GetURLWithPath(kVpnHost, kCreateSupportTicket), "POST",
      CreateJSONRequestBody(GetValueWithTicketInfos(
          email, subject, body, GetTimeZoneName())),
      std::move(internal_callback));
}

void WootzVpnAPIRequest::OAuthRequest(
    const GURL& url,
    const std::string& method,
    const std::string& post_data,
    URLRequestCallback callback,
    const base::flat_map<std::string, std::string>& headers) {
  api_request_helper_.Request(method, url, post_data, "application/json",
                              std::move(callback), headers,
                              {.auto_retry_on_network_change = true});
}

void WootzVpnAPIRequest::OnGetResponse(
    ResponseCallback callback,
    api_request_helper::APIRequestResult result) {
  // NOTE: |api_request_helper_| uses JsonSanitizer to sanitize input made with
  // requests. |body| will be empty when the response from service is invalid
  // json.
  const bool success = result.response_code() == 200;
  std::move(callback).Run(result.SerializeBodyToString(), success);
}

void WootzVpnAPIRequest::OnCreateSupportTicket(
    ResponseCallback callback,
    APIRequestResult api_request_result) {
  bool success = api_request_result.response_code() == 200;
  VLOG(2) << "OnCreateSupportTicket success=" << success
          << "\nresponse_code=" << api_request_result.response_code();
  std::move(callback).Run(api_request_result.SerializeBodyToString(), success);
}

}  // namespace wootz_vpn
