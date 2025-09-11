// Copyright 2024 The Wootz Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_ANDROID_WOOTZ_KEYSTORE_H_
#define NET_ANDROID_WOOTZ_KEYSTORE_H_

#include <jni.h>
#include <stdint.h>

#include <string>
#include <string_view>
#include <vector>

#include "base/android/scoped_java_ref.h"
#include "base/containers/span.h"

// Wootz hardware keystore functions for secure key operations.
// These functions provide access to hardware-backed keys (Strongbox/TEE)
// with proper attestation support for maximum security.

namespace net::android::wootz {

/**
 * Initialize the Wootz hardware keystore system.
 * This should be called once during browser startup to ensure
 * hardware-backed certificates are available.
 * 
 * @return true if initialization was successful
 */
bool InitializeHardwareKeystore();

/**
 * Generate hardware-backed P-256 key with attestation challenge.
 * The private key remains non-exportable in secure hardware.
 * 
 * @param attestation_challenge Challenge bytes for hardware attestation
 * @return true if key generation with attestation was successful
 */
bool GenerateHardwareBackedKeyWithAttestation(
    base::span<const uint8_t> attestation_challenge);

/**
 * Get the device public key in PEM format.
 * This corresponds to the non-exportable private key in hardware.
 * 
 * @return PEM-encoded public key string or empty if not available
 */
std::string GetDevicePublicKeyPem();

/**
 * Get the hardware attestation certificate chain in PEM format.
 * This provides cryptographic proof that the key is hardware-backed.
 * 
 * @return PEM-encoded attestation certificate chain or empty if not available
 */
std::string GetAttestationChainPem();

/**
 * Check if the current key is backed by Strongbox (highest security level).
 * 
 * @return true if key is Strongbox-backed
 */
bool IsKeyStrongboxBacked();

/**
 * Check if the current key is hardware-backed (TEE or Strongbox).
 * 
 * @return true if key is hardware-backed
 */
bool IsKeyHardwareBacked();

/**
 * Sign data using the non-exportable hardware private key.
 * The private key never leaves the secure hardware.
 * 
 * @param data Data to sign
 * @return Signature bytes or empty vector if signing failed
 */
std::vector<uint8_t> SignWithHardwareKey(base::span<const uint8_t> data);

}  // namespace net::android::wootz

#endif  // NET_ANDROID_WOOTZ_KEYSTORE_H_
