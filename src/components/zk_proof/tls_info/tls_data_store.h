#ifndef COMPONENTS_ZK_PROOF_TLS_INFO_TLS_DATA_STORE_H_
#define COMPONENTS_ZK_PROOF_TLS_INFO_TLS_DATA_STORE_H_

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "base/memory/singleton.h"

namespace zk_proof {

// Struct to hold TLS data for a navigation
struct TlsData {
  std::vector<uint8_t> cert_hash;
  std::string headers_json;
  std::string url;
};

// Singleton to store TLS data
class TlsDataStore {
 public:
  static TlsDataStore* GetInstance();
  
  // Store TLS data for a URL
  void StoreTlsData(const std::string& url, 
                   const std::vector<uint8_t>& cert_hash,
                   const std::string& headers_json);
  
  // Get TLS data for a URL (or most recent if url is empty)
  std::optional<TlsData> GetTlsData(const std::string& url = "");

 private:
  friend struct base::DefaultSingletonTraits<TlsDataStore>;
  TlsDataStore() = default;
  ~TlsDataStore() = default;

  std::mutex data_mutex_;
  std::map<std::string, TlsData> tls_data_by_url_;
  std::string most_recent_url_;
};

}  // namespace zk_proof

#endif  // COMPONENTS_ZK_PROOF_TLS_INFO_TLS_DATA_STORE_H_
