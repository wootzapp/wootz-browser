#include "components/zk_proof/zk_proof.h"

#include <base/logging.h>
#include <base/files/file_path.h>
#include <base/files/file.h>
#include <base/files/file_util.h>
#include <base/path_service.h>

namespace zk_proof {

std::string GenerateProofWithKey(
    const std::vector<uint8_t>& cert_hash,
    const std::string& headers,
    const std::string& content,
    const std::vector<uint8_t>& proving_key) {
    if (proving_key.empty()) {
        LOG(ERROR) << "Cannot generate proof: Empty proving key provided";
        return "";
    }
    
    LOG(INFO) << "Generating ZK proof with " << proving_key.size() 
              << " byte proving key";
    LOG(INFO) << "Certificate hash size: " << cert_hash.size() << " bytes";
    LOG(INFO) << "Headers JSON: " << headers;
    LOG(INFO) << "Content: " << (content.length() > 25 ? content.substr(0, 25) + "....." : content);
    
    // Call into Rust code without try-catch
    rust::String rust_result = generate_groth16_proof(
        rust::Slice<const uint8_t>(cert_hash.data(), cert_hash.size()),
        rust::Str(headers),
        rust::Str(content),
        rust::Slice<const uint8_t>(proving_key.data(), proving_key.size())
    );
    
    // Check if result is error message
    std::string result = std::string(rust_result.c_str(), rust_result.size());
    if (result.empty() || result.find("Failed to") == 0 || result.find("Error") == 0) {
        LOG(ERROR) << "Error generating proof: " << result;
        return "";
    }
    
    LOG(INFO) << "Successfully generated proof JSON";
    return result;
}

std::string GenerateKeys(
    const std::vector<uint8_t>& cert_hash,
    const std::string& headers,
    const std::string& content) {
    LOG(INFO) << "Generating ZK parameters with real values";
    
    // Call into Rust code to generate keys with real parameters
    rust::String keys_json = generate_keys(
        rust::Slice<const uint8_t>(cert_hash.data(), cert_hash.size()),
        rust::Str(headers),
        rust::Str(content)
    );
    
    // Convert rust::String to std::string
    std::string result = std::string(keys_json.c_str(), keys_json.size());
    if (result.empty() || result.find("Error:") == 0) {
        LOG(ERROR) << "Error generating ZK parameters: " << result;
        return "";
    }
    
    LOG(INFO) << "Successfully generated ZK parameters with real values";
    return result;
}

std::string ExtractPublicInputs(
    const std::vector<uint8_t>& cert_hash,
    const std::string& content) {
    LOG(INFO) << "Extracting public inputs";
    
    // Call into Rust code to extract public inputs
    rust::String public_inputs = extract_public_inputs(
        rust::Slice<const uint8_t>(cert_hash.data(), cert_hash.size()),
        rust::Str(content)
    );
    
    // Convert rust::String to std::string
    std::string result = std::string(public_inputs.c_str(), public_inputs.size());
    if (result.empty() || result.find("Error:") == 0) {
        LOG(ERROR) << "Error extracting public inputs: " << result;
        return "";
    }
    
    LOG(INFO) << "Successfully extracted public inputs";
    return result;
}

}  // namespace zk_proof
