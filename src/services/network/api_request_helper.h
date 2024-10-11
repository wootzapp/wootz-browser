#ifndef SERVICES_NETWORK_API_REQUEST_HELPER_H_
#define SERVICES_NETWORK_API_REQUEST_HELPER_H_

#include <string>
#include <memory>
#include "base/functional/callback.h"
#include "base/memory/ref_counted.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "services/network/public/mojom/url_response_head.mojom.h"
#include "services/network/public/mojom/url_loader_factory.mojom.h" // Add this header

namespace network {

class APIRequestHelper {
 public:
  using ResponseCallback = base::OnceCallback<void(const std::string& response)>;

  // Makes a GET request to the specified URL.
  static void MakeGetRequest(const std::string& url, 
                             scoped_refptr<network::SharedURLLoaderFactory> loader_factory, 
                             ResponseCallback callback);

  // Makes a POST request to the specified URL with the provided body.
  static void MakePostRequest(const std::string& url, 
                              const std::string& body, 
                              scoped_refptr<network::SharedURLLoaderFactory> loader_factory, 
                              ResponseCallback callback);

 private:
  static void OnURLLoadComplete(std::unique_ptr<std::string> response_body, ResponseCallback callback);

  APIRequestHelper() = delete;
  ~APIRequestHelper() = delete;
};

}  // namespace network

#endif  // SERVICES_NETWORK_API_REQUEST_HELPER_H_
