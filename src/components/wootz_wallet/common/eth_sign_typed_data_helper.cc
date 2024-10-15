/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "components/wootz_wallet/common/eth_sign_typed_data_helper.h"

#include <limits>
#include <optional>
#include <utility>

#include "base/logging.h"
#include "base/strings/strcat.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "components/wootz_wallet/common/wootz_wallet_types.h"
#include "components/wootz_wallet/common/hash_utils.h"
#include "components/wootz_wallet/common/hex_utils.h"
#include "components/wootz_wallet/common/string_utils.h"

#include <sstream>
#include <iomanip>

namespace wootz_wallet {

// static
std::unique_ptr<EthSignTypedDataHelper> EthSignTypedDataHelper::Create(
    base::Value::Dict types,
    Version version) {
  return std::unique_ptr<EthSignTypedDataHelper>(
      new EthSignTypedDataHelper(std::move(types), version));
}

EthSignTypedDataHelper::EthSignTypedDataHelper(base::Value::Dict types,
                                               Version version)
    : types_(std::move(types)), version_(version) {}

EthSignTypedDataHelper::~EthSignTypedDataHelper() = default;

void EthSignTypedDataHelper::SetTypes(base::Value::Dict types) {
  types_ = std::move(types);
}

void EthSignTypedDataHelper::SetVersion(Version version) {
  version_ = version;
}

void EthSignTypedDataHelper::FindAllDependencyTypes(
    base::flat_map<std::string, base::Value>* known_types,
    const std::string& anchor_type_name) const {
  DCHECK(!anchor_type_name.empty());
  DCHECK(known_types);

  const auto* anchor_type = types_.FindList(anchor_type_name);
  if (!anchor_type) {
    return;
  }
  known_types->emplace(anchor_type_name, anchor_type->Clone());

  for (const auto& field : *anchor_type) {
    if (!field.is_dict()) {
      // EncodeType will fail for this anchor type, there is no need to continue
      // finding.
      return;
    }
    const std::string* type = field.GetDict().FindString("type");
    if (type) {
      auto type_split = base::SplitString(*type, "[", base::KEEP_WHITESPACE,
                                          base::SPLIT_WANT_ALL);
      std::string lookup_type = *type;
      if (type_split.size() == 2) {
        lookup_type = type_split[0];
      }

      if (!known_types->contains(lookup_type)) {
        FindAllDependencyTypes(known_types, lookup_type);
      }
    }
  }
}

std::string EthSignTypedDataHelper::EncodeType(
    const base::Value& type,
    const std::string& type_name) const {
  if (!type.is_list()) {
    return std::string();
  }
  std::string result = base::StrCat({type_name, "("});

  for (size_t i = 0; i < type.GetList().size(); ++i) {
    if (!type.GetList()[i].is_dict()) {
      return std::string();
    }
    const base::Value::Dict& root = type.GetList()[i].GetDict();
    const std::string* type_str = root.FindString("type");
    const std::string* name_str = root.FindString("name");
    if (!type_str || !name_str) {
      return std::string();
    }
    base::StrAppend(&result, {*type_str, " ", *name_str});
    if (i != type.GetList().size() - 1) {
      base::StrAppend(&result, {","});
    }
  }
  base::StrAppend(&result, {")"});
  return result;
}

std::string EthSignTypedDataHelper::EncodeTypes(
    const std::string& primary_type_name) const {
  std::string result;

  base::flat_map<std::string, base::Value> types_map;
  FindAllDependencyTypes(&types_map, primary_type_name);

  auto it = types_map.find(primary_type_name);
  if (it != types_map.end()) {
    base::StrAppend(&result, {EncodeType(it->second, primary_type_name)});
  }
  for (const auto& type : types_map) {
    if (type.first == primary_type_name) {
      continue;
    }
    base::StrAppend(&result, {EncodeType(type.second, type.first)});
  }
  return result;
}

std::vector<uint8_t> EthSignTypedDataHelper::GetTypeHash(
    const std::string primary_type_name) const {
  const std::string type_hash =
      KeccakHash(EncodeTypes(primary_type_name), false);
  return std::vector<uint8_t>(type_hash.begin(), type_hash.end());
}

std::optional<std::pair<std::vector<uint8_t>, base::Value::Dict>>
EthSignTypedDataHelper::HashStruct(const std::string primary_type_name,
                                   const base::Value::Dict& data) const {
  LOG(ERROR) << "JANGID: HashStruct started";
  LOG(ERROR) << "JANGID: Primary type name: " << primary_type_name;
  LOG(ERROR) << "JANGID: Input data: " << data.DebugString();

  auto encoded_data = EncodeData(primary_type_name, data);
  if (!encoded_data) {
    LOG(ERROR) << "JANGID: EncodeData failed";
    return std::nullopt;
  }

  LOG(ERROR) << "JANGID: Encoded data size: " << encoded_data->first.size();
  LOG(ERROR) << "JANGID: Encoded data (hex): " 
             << base::HexEncode(encoded_data->first.data(), encoded_data->first.size());
  LOG(ERROR) << "JANGID: Encoded dict: " << encoded_data->second.DebugString();

  auto keccak_hash = KeccakHash(encoded_data->first);
  LOG(ERROR) << "JANGID: Keccak hash size: " << keccak_hash.size();
  LOG(ERROR) << "JANGID: Keccak hash (hex): " 
             << base::HexEncode(keccak_hash.data(), keccak_hash.size());

  auto result = std::make_pair(std::move(keccak_hash),
                               std::move(encoded_data->second));

  LOG(ERROR) << "JANGID: HashStruct completed";
  return result;
}

// Encode the json data by the its type defined in json custom types starting
// from primary type. See unittests for some examples.
std::optional<std::pair<std::vector<uint8_t>, base::Value::Dict>>
EthSignTypedDataHelper::EncodeData(const std::string& primary_type_name,
                                   const base::Value::Dict& data) const {
  LOG(ERROR) << "JANGID: EncodeData started for primary_type_name: " << primary_type_name;
  LOG(ERROR) << "JANGID: Input data: " << data.DebugString();

  const auto* primary_type = types_.FindList(primary_type_name);
  if (!primary_type) {
    LOG(ERROR) << "JANGID: Primary type not found in types_";
    return std::nullopt;
  }
  LOG(ERROR) << "JANGID: Primary type found: " << primary_type->DebugString();

  std::vector<uint8_t> result;
  const std::vector<uint8_t> type_hash = GetTypeHash(primary_type_name);
  LOG(ERROR) << "JANGID: Type hash size: " << type_hash.size();
  LOG(ERROR) << "JANGID: Type hash (hex): " << base::HexEncode(type_hash.data(), type_hash.size());

  result.insert(result.end(), type_hash.begin(), type_hash.end());
  LOG(ERROR) << "JANGID: Result after inserting type hash, size: " << result.size();

  base::Value::Dict sanitized_data;

  for (const auto& item : *primary_type) {
    const auto& field = item.GetDict();
    const std::string* type_str = field.FindString("type");
    const std::string* name_str = field.FindString("name");
    if (!type_str || !name_str) {
      LOG(ERROR) << "JANGID: Invalid field, missing type or name";
      return std::nullopt;
    }
    LOG(ERROR) << "JANGID: Processing field - Name: " << *name_str << ", Type: " << *type_str;

    const base::Value* value = data.Find(*name_str);
    if (value) {
      LOG(ERROR) << "JANGID: Field value found: " << value->DebugString();
      auto encoded_field = EncodeField(*type_str, *value);
      if (!encoded_field) {
        LOG(ERROR) << "JANGID: Failed to encode field: " << *name_str;
        return std::nullopt;
      }
      LOG(ERROR) << "JANGID: Encoded field size: " << encoded_field->size();
      LOG(ERROR) << "JANGID: Encoded field (hex): " << base::HexEncode(encoded_field->data(), encoded_field->size());

      result.insert(result.end(), encoded_field->begin(), encoded_field->end());
      sanitized_data.Set(*name_str, value->Clone());
      LOG(ERROR) << "JANGID: Result size after inserting encoded field: " << result.size();
    } else {
      LOG(ERROR) << "JANGID: Field value not found: " << *name_str;
      if (version_ == Version::kV4) {
        for (size_t i = 0; i < 32; ++i) {
          result.push_back(0);
        }
        LOG(ERROR) << "JANGID: Added 32 zero bytes for missing field (V4)";
      }
    }
  }

  LOG(ERROR) << "JANGID: Final result size: " << result.size();
  LOG(ERROR) << "JANGID: Final result (hex): " << base::HexEncode(result.data(), result.size());
  LOG(ERROR) << "JANGID: Sanitized data: " << sanitized_data.DebugString();
  LOG(ERROR) << "JANGID: EncodeData completed";

  return std::make_pair(result, std::move(sanitized_data));
}

// Encode each field of a custom type, if a field is also a custom type it
// will call EncodeData recursively until it reaches an atomic type
std::optional<std::vector<uint8_t>> EthSignTypedDataHelper::EncodeField(
    const std::string& type,
    const base::Value& value) const {
  // ES6 section 20.1.2.6 Number.MAX_SAFE_INTEGER
  constexpr double kMaxSafeInteger = static_cast<double>(kMaxSafeIntegerUint64);
  std::vector<uint8_t> result;

  if (base::EndsWith(type, "]")) {
    if (version_ != Version::kV4) {
      VLOG(0) << "version has to be v4 to support array";
      return std::nullopt;
    }
    if (!value.is_list()) {
      return std::nullopt;
    }
    auto type_split = base::SplitString(type, "[", base::KEEP_WHITESPACE,
                                        base::SPLIT_WANT_ALL);
    if (type_split.size() != 2) {
      return std::nullopt;
    }
    const std::string array_type = type_split[0];
    std::vector<uint8_t> array_result;
    for (const auto& item : value.GetList()) {
      auto encoded_item = EncodeField(array_type, item);
      if (!encoded_item) {
        return std::nullopt;
      }
      array_result.insert(array_result.end(), encoded_item->begin(),
                          encoded_item->end());
    }
    auto array_hash = KeccakHash(array_result);
    result.insert(result.end(), array_hash.begin(), array_hash.end());
  } else if (type == "string") {
    const std::string* value_str = value.GetIfString();
    if (!value_str) {
      return std::nullopt;
    }
    const std::string encoded_value = KeccakHash(*value_str, false);
    const std::vector<uint8_t> encoded_value_bytes(encoded_value.begin(),
                                                   encoded_value.end());
    result.insert(result.end(), encoded_value_bytes.begin(),
                  encoded_value_bytes.end());
  } else if (type == "bytes") {
    const std::string* value_str = value.GetIfString();
    if (!value_str || (!value_str->empty() && !IsValidHexString(*value_str))) {
      return std::nullopt;
    }
    std::vector<uint8_t> bytes;
    if (!value_str->empty()) {
      CHECK(PrefixedHexStringToBytes(*value_str, &bytes));
    }
    const std::vector<uint8_t> encoded_value = KeccakHash(bytes);
    result.insert(result.end(), encoded_value.begin(), encoded_value.end());
  } else if (type == "bool") {
    std::optional<bool> value_bool = value.GetIfBool();
    if (!value_bool) {
      return std::nullopt;
    }
    uint256_t encoded_value = (uint256_t)*value_bool;
    // Append the encoded value to byte array result in big endian order
    for (int i = 256 - 8; i >= 0; i -= 8) {
      result.push_back(static_cast<uint8_t>((encoded_value >> i) & 0xFF));
    }
  } else if (type == "address") {
    LOG(ERROR) << "JANGID: Processing address type";
    const std::string* value_str = value.GetIfString();
    if (!value_str || !IsValidHexString(*value_str)) {
      LOG(ERROR) << "JANGID: Invalid address value: " << (value_str ? *value_str : "null");
      return std::nullopt;
    }
    LOG(ERROR) << "JANGID: Valid address string: " << *value_str;

    std::vector<uint8_t> address;
    bool conversion_success = PrefixedHexStringToBytes(*value_str, &address);
    CHECK(conversion_success);
    LOG(ERROR) << "JANGID: Address bytes size: " << address.size();

    if (address.size() != 20u) {
      LOG(ERROR) << "JANGID: Invalid address size: " << address.size() << " (expected 20)";
      return std::nullopt;
    }

    size_t padding_size = (256 - 160) / 8;
    LOG(ERROR) << "JANGID: Adding " << padding_size << " zero bytes as padding";
    for (size_t i = 0; i < padding_size; ++i) {
      result.push_back(0);
    }

    LOG(ERROR) << "JANGID: Result size before adding address: " << result.size();
    result.insert(result.end(), address.begin(), address.end());
    LOG(ERROR) << "JANGID: Result size after adding address: " << result.size();
    LOG(ERROR) << "JANGID: Encoded address (hex): " << base::HexEncode(result.data(), result.size());
  } else if (base::StartsWith(type, "bytes", base::CompareCase::SENSITIVE)) {
    unsigned num_bits;
    if (!base::StringToUint(type.data() + 5, &num_bits) || num_bits > 32) {
      return std::nullopt;
    }
    const std::string* value_str = value.GetIfString();
    if (!value_str || !IsValidHexString(*value_str)) {
      return std::nullopt;
    }
    std::vector<uint8_t> bytes;
    CHECK(PrefixedHexStringToBytes(*value_str, &bytes));
    if (bytes.size() > 32) {
      return std::nullopt;
    }
    result.insert(result.end(), bytes.begin(), bytes.end());
    for (size_t i = 0; i < 32u - bytes.size(); ++i) {
      result.push_back(0);
    }
  } else if (base::StartsWith(type, "uint", base::CompareCase::SENSITIVE)) {
    // uint8 to uint256 in steps of 8
    unsigned num_bits;
    if (!base::StringToUint(type.data() + 4, &num_bits) ||
        !ValidSolidityBits(num_bits)) {
      return std::nullopt;
    }

    std::optional<double> value_double = value.GetIfDouble();
    const std::string* value_str = value.GetIfString();
    uint256_t encoded_value = 0;
    if (value_double) {
      encoded_value = (uint256_t)(uint64_t)*value_double;
      if (encoded_value > (uint256_t)kMaxSafeInteger) {
        return std::nullopt;
      }
    } else if (value_str) {
      if (!value_str->empty() &&
          !HexValueToUint256(*value_str, &encoded_value) &&
          !Base10ValueToUint256(*value_str, &encoded_value)) {
        return std::nullopt;
      }
    } else {
      return std::nullopt;
    }

    std::optional<uint256_t> max_value = MaxSolidityUint(num_bits);
    if (max_value == std::nullopt || encoded_value > *max_value) {
      return std::nullopt;
    }

    // Append the encoded value to byte array result in big endian order
    for (int i = 256 - 8; i >= 0; i -= 8) {
      result.push_back(static_cast<uint8_t>((encoded_value >> i) & 0xFF));
    }
  } else if (base::StartsWith(type, "int", base::CompareCase::SENSITIVE)) {
    // int8 to int256 in steps of 8
    unsigned num_bits;
    if (!base::StringToUint(type.data() + 3, &num_bits) ||
        !ValidSolidityBits(num_bits)) {
      return std::nullopt;
    }
    std::optional<double> value_double = value.GetIfDouble();
    const std::string* value_str = value.GetIfString();
    int256_t encoded_value = 0;
    if (value_double) {
      encoded_value = (int256_t)(int64_t)*value_double;
      if (encoded_value > (int256_t)kMaxSafeInteger) {
        return std::nullopt;
      }
    } else if (value_str) {
      if (!value_str->empty() &&
          !HexValueToInt256(*value_str, &encoded_value) &&
          !Base10ValueToInt256(*value_str, &encoded_value)) {
        return std::nullopt;
      }
    } else {
      return std::nullopt;
    }

    std::optional<int256_t> min_value = MinSolidityInt(num_bits);
    std::optional<int256_t> max_value = MaxSolidityInt(num_bits);
    if (min_value == std::nullopt || max_value == std::nullopt ||
        encoded_value > *max_value || encoded_value < *min_value) {
      return std::nullopt;
    }

    // Append the encoded value to byte array result in big endian order
    for (int i = 256 - 8; i >= 0; i -= 8) {
      result.push_back(static_cast<uint8_t>((encoded_value >> i) & 0xFF));
    }
  } else {
    if (!value.is_dict()) {
      return std::nullopt;
    }
    auto encoded_data = EncodeData(type, value.GetDict());
    if (!encoded_data) {
      return std::nullopt;
    }
    std::vector<uint8_t> encoded_value = KeccakHash(encoded_data->first);

    result.insert(result.end(), encoded_value.begin(), encoded_value.end());
  }
  return result;
}

std::optional<std::pair<std::vector<uint8_t>, base::Value::Dict>>
EthSignTypedDataHelper::GetTypedDataDomainHash(
    const base::Value::Dict& domain_separator) const {
  LOG(ERROR) << "JANGID: GetTypedDataDomainHash started";
  LOG(ERROR) << "JANGID: Domain separator: " << domain_separator.DebugString();

  auto result = HashStruct("EIP712Domain", domain_separator);

  if (result) {
    LOG(ERROR) << "JANGID: Domain hash size: " << result->first.size();
    LOG(ERROR) << "JANGID: Domain hash (hex): " 
               << base::HexEncode(result->first.data(), result->first.size());
    LOG(ERROR) << "JANGID: Resulting dict: " << result->second.DebugString();
  } else {
    LOG(ERROR) << "JANGID: Failed to compute domain hash";
  }

  LOG(ERROR) << "JANGID: GetTypedDataDomainHash completed";
  return result;
}

std::optional<std::pair<std::vector<uint8_t>, base::Value::Dict>>
EthSignTypedDataHelper::GetTypedDataPrimaryHash(
    const std::string& primary_type_name,
    const base::Value::Dict& message) const {
  return HashStruct(primary_type_name, message);
}

// static
std::optional<std::vector<uint8_t>>
EthSignTypedDataHelper::GetTypedDataMessageToSign(
    const std::vector<uint8_t>& domain_hash,
    const std::vector<uint8_t>& primary_hash) {
  LOG(ERROR) << "JANGID: Entering GetTypedDataMessageToSign";

  // Log input hashes
  std::stringstream ss_domain, ss_primary;
  ss_domain << std::hex << std::setfill('0');
  ss_primary << std::hex << std::setfill('0');
  for (const auto& byte : domain_hash) {
    ss_domain << std::setw(2) << static_cast<int>(byte);
  }
  for (const auto& byte : primary_hash) {
    ss_primary << std::setw(2) << static_cast<int>(byte);
  }
  LOG(ERROR) << "JANGID: Domain hash: " << ss_domain.str();
  LOG(ERROR) << "JANGID: Primary hash: " << ss_primary.str();

  if (domain_hash.empty() || primary_hash.empty()) {
    LOG(ERROR) << "JANGID: Either domain_hash or primary_hash is empty. Returning nullopt.";
    return std::nullopt;
  }

  std::vector<uint8_t> encoded_data({0x19, 0x01});
  LOG(ERROR) << "JANGID: Initial encoded_data: 0x1901";

  encoded_data.insert(encoded_data.end(), domain_hash.begin(),
                      domain_hash.end());
  LOG(ERROR) << "JANGID: Encoded data after inserting domain_hash. Size: " << encoded_data.size();

  encoded_data.insert(encoded_data.end(), primary_hash.begin(),
                      primary_hash.end());
  LOG(ERROR) << "JANGID: Encoded data after inserting primary_hash. Size: " << encoded_data.size();

  // Log final encoded data
  std::stringstream ss_encoded;
  ss_encoded << std::hex << std::setfill('0');
  for (const auto& byte : encoded_data) {
    ss_encoded << std::setw(2) << static_cast<int>(byte);
  }
  LOG(ERROR) << "JANGID: Final encoded data: " << ss_encoded.str();

  auto result = KeccakHash(encoded_data);
  
  // Log the result
  if (!result.empty()) {
    std::stringstream ss_result;
    ss_result << std::hex << std::setfill('0');
    for (const auto& byte : result) {
      ss_result << std::setw(2) << static_cast<int>(byte);
    }
    LOG(ERROR) << "JANGID: Keccak hash result: " << ss_result.str();
  } else {
    LOG(ERROR) << "JANGID: KeccakHash returned nullopt";
  }

  LOG(ERROR) << "JANGID: Exiting GetTypedDataMessageToSign";
  return result;
}

}  // namespace wootz_wallet
