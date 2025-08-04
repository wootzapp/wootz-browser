#include "components/zk_proof/tls_info/tls_data_store.h"
#include "base/memory/singleton.h"
#include "base/logging.h"

namespace zk_proof {

// static
TlsDataStore* TlsDataStore::GetInstance() {
  return base::Singleton<TlsDataStore>::get();
}

void TlsDataStore::StoreTlsData(
    const std::string& url,
    const std::vector<uint8_t>& cert_hash,
    const std::string& headers_json) {
  LOG(INFO) << "Storing TLS data for URL: " << url;
  std::lock_guard<std::mutex> lock(data_mutex_);
  
  TlsData data;
  data.url = url;
  data.cert_hash = cert_hash;
  data.headers_json = headers_json;
  
  tls_data_by_url_[url] = data;
  most_recent_url_ = url;
  LOG(INFO) << "TLS data stored for URL: " << url;
}

std::optional<TlsData> TlsDataStore::GetTlsData(const std::string& url) {
  LOG(INFO) << "Getting TLS data for URL: " << url;
  std::lock_guard<std::mutex> lock(data_mutex_);
  
  std::string target_url = url.empty() ? most_recent_url_ : url;
  
  if (target_url.empty() || tls_data_by_url_.find(target_url) == tls_data_by_url_.end()) {
    return std::nullopt;
  }
  
  return tls_data_by_url_[target_url];
}

}  // namespace zk_proof
