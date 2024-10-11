#include "services/network/api_request_helper.h"

#include "base/logging.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "net/base/load_flags.h"
#include "url/gurl.h"

namespace network {
namespace{


void APIRequestHelper::MakeGetRequest(const std::string& url, 
                                      scoped_refptr<network::SharedURLLoaderFactory> loader_factory, 
                                      ResponseCallback callback) {
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(url);
  resource_request->method = "GET";
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
  resource_request->load_flags = net::LOAD_BYPASS_CACHE |
                                 net::LOAD_DISABLE_CACHE |
                                 net::LOAD_DO_NOT_SAVE_COOKIES;

  net::NetworkTrafficAnnotationTag traffic_annotation = net::DefineNetworkTrafficAnnotation("api_request_helper_get", R"(
      semantics {
        sender: "API Request Helper"
        description: "Making a GET request."
        trigger: "User initiated."
        data: "No user data."
        destination: OTHER
      }
      policy {
        cookies_allowed: NO
        setting: "This request cannot be disabled by settings."
        policy_exception_justification: "Not implemented."
      })");

  auto simple_loader = network::SimpleURLLoader::Create(std::move(resource_request), traffic_annotation);
  
  simple_loader->DownloadToString(
    loader_factory.get(),
    base::BindOnce(&APIRequestHelper::OnURLLoadComplete, std::move(callback)),
    1024 * 1024);
}

void APIRequestHelper::MakePostRequest(const std::string& url, 
                                       const std::string& body, 
                                       scoped_refptr<network::SharedURLLoaderFactory> loader_factory, 
                                       ResponseCallback callback) {
  auto resource_request = std::make_unique<network::ResourceRequest>();
  resource_request->url = GURL(url);
  resource_request->method = "POST";
  resource_request->credentials_mode = network::mojom::CredentialsMode::kOmit;
  resource_request->load_flags = net::LOAD_BYPASS_CACHE |
                                 net::LOAD_DISABLE_CACHE |
                                 net::LOAD_DO_NOT_SAVE_COOKIES;

  net::NetworkTrafficAnnotationTag traffic_annotation = net::DefineNetworkTrafficAnnotation("api_request_helper_post", R"(
      semantics {
        sender: "API Request Helper"
        description: "Making a POST request."
        trigger: "User initiated."
        data: "User provided data."
        destination: OTHER
      }
      policy {
        cookies_allowed: NO
        setting: "This request cannot be disabled by settings."
        policy_exception_justification: "Not implemented."
      })");

  auto simple_loader = network::SimpleURLLoader::Create(std::move(resource_request), traffic_annotation);

  simple_loader->AttachStringForUpload(body, "application/json");
  
  simple_loader->DownloadToString(
    loader_factory.get(),
    base::BindOnce(&APIRequestHelper::OnURLLoadComplete, std::move(callback)),
    1024 * 1024);
}

void APIRequestHelper::OnURLLoadComplete(std::unique_ptr<std::string> response_body, ResponseCallback callback) {
  std::string data = response_body ? std::move(*response_body) : "";
  std::move(callback).Run(data);
}
}
}  // namespace network
