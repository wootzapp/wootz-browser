#ifndef COMPONENTS_ZK_PROOF_ZK_PROOF_H_
#define COMPONENTS_ZK_PROOF_ZK_PROOF_H_

#include <string>
#include <vector>
#include "components/zk_proof/rust/src/lib.rs.h"  // CXX-generated header

namespace zk_proof {

// Generate keys with real certificate and content parameters
// Returns a JSON string containing base64-encoded proving key and verification key JSON
std::string GenerateKeys(
    const std::vector<uint8_t>& cert_hash,
    const std::string& headers,
    const std::string& content);

// Extract just the public inputs in Garaga format
std::string ExtractPublicInputs(
    const std::vector<uint8_t>& cert_hash,
    const std::string& content);

// Generate a ZK proof with the provided proving key
std::string GenerateProofWithKey(
    const std::vector<uint8_t>& cert_hash,
    const std::string& headers,
    const std::string& content,
    const std::vector<uint8_t>& proving_key);

}  // namespace zk_proof

#endif  // COMPONENTS_ZK_PROOF_ZK_PROOF_H_
