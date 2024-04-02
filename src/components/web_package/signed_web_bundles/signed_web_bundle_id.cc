// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/web_package/signed_web_bundles/signed_web_bundle_id.h"

#include "base/containers/span.h"
#include "base/functional/bind.h"
#include "base/rand_util.h"
#include "base/ranges/algorithm.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/types/expected.h"
#include "components/base32/base32.h"

namespace web_package {

// static
base::expected<SignedWebBundleId, std::string> SignedWebBundleId::Create(
    base::StringPiece encoded_id) {
  if (encoded_id.size() != kEncodedIdLength) {
    return base::unexpected(
        base::StringPrintf("The signed web bundle ID must be exactly %zu "
                           "characters long, but was %zu characters long.",
                           kEncodedIdLength, encoded_id.size()));
  }

  for (const char c : encoded_id) {
    if (!(base::IsAsciiLower(c) || (c >= '2' && c <= '7'))) {
      return base::unexpected(
          "The signed web bundle ID must only contain lowercase ASCII "
          "characters and digits between 2 and 7 (without any padding).");
    }
  }

  // Base32 decode the ID as an array.
  const std::vector<uint8_t> decoded_id =
      base32::Base32Decode(base::ToUpperASCII(encoded_id));
  if (decoded_id.size() != kDecodedIdLength) {
    return base::unexpected(
        "The signed web bundle ID could not be decoded from its base32 "
        "representation.");
  }

  auto decoded_span = base::make_span(decoded_id);
  auto type_suffix = decoded_span.last<kTypeSuffixLength>();
  if (base::ranges::equal(type_suffix, kTypeDevelopment)) {
    return SignedWebBundleId(Type::kDevelopment, encoded_id,
                             decoded_span.first<kDecodedIdLength>());
  }
  if (base::ranges::equal(type_suffix, kTypeEd25519PublicKey)) {
    return SignedWebBundleId(Type::kEd25519PublicKey, encoded_id,
                             decoded_span.first<kDecodedIdLength>());
  }
  return base::unexpected("The signed web bundle ID has an unknown type.");
}

// static
SignedWebBundleId SignedWebBundleId::CreateForEd25519PublicKey(
    Ed25519PublicKey public_key) {
  std::array<uint8_t, kDecodedIdLength> decoded_id;
  base::ranges::copy(public_key.bytes(), decoded_id.begin());
  base::ranges::copy(kTypeEd25519PublicKey,
                     decoded_id.end() - kTypeSuffixLength);

  auto encoded_id_uppercase = base32::Base32Encode(
      decoded_id, base32::Base32EncodePolicy::OMIT_PADDING);
  auto encoded_id = base::ToLowerASCII(encoded_id_uppercase);
  return SignedWebBundleId(Type::kEd25519PublicKey, encoded_id, decoded_id);
}

// static
SignedWebBundleId SignedWebBundleId::CreateForDevelopment(
    base::span<const uint8_t, kDecodedIdLength - kTypeSuffixLength> data) {
  std::array<uint8_t, kDecodedIdLength> decoded_id;
  base::ranges::copy(data, decoded_id.begin());
  base::ranges::copy(kTypeDevelopment, decoded_id.end() - kTypeSuffixLength);

  auto encoded_id_uppercase = base32::Base32Encode(
      decoded_id, base32::Base32EncodePolicy::OMIT_PADDING);
  auto encoded_id = base::ToLowerASCII(encoded_id_uppercase);
  return SignedWebBundleId(Type::kDevelopment, encoded_id, decoded_id);
}

// static
SignedWebBundleId SignedWebBundleId::CreateRandomForDevelopment(
    base::RepeatingCallback<void(base::span<uint8_t>)> random_generator) {
  std::array<uint8_t, kDecodedIdLength - kTypeSuffixLength> random_bytes;
  random_generator.Run(random_bytes);
  return CreateForDevelopment(random_bytes);
}

SignedWebBundleId::SignedWebBundleId(
    Type type,
    base::StringPiece encoded_id,
    base::span<const uint8_t, kDecodedIdLength> decoded_id)
    : type_(type), encoded_id_(encoded_id) {
  base::ranges::copy(decoded_id, decoded_id_.begin());
}

SignedWebBundleId::SignedWebBundleId(const SignedWebBundleId& other) = default;

SignedWebBundleId::~SignedWebBundleId() = default;

// static
base::RepeatingCallback<void(base::span<uint8_t>)>
SignedWebBundleId::GetDefaultRandomGenerator() {
  // TODO(crbug.com/1490484): Remove the static_cast once all callers of
  // base::RandBytes are migrated to the span variant.
  return base::BindRepeating(
      static_cast<void (*)(base::span<uint8_t>)>(&base::RandBytes));
}

}  // namespace web_package
