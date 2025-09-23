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

// mTLS Client Certificate Functions

/**
 * Check if DIC is available and ready for mTLS client authentication.
 * This verifies both certificate validity and hardware key association.
 * 
 * @return true if DIC can be used for mTLS
 */
bool IsDicAvailableForMTLS();

/**
 * Get the DIC certificate for mTLS client authentication.
 * Returns the certificate that should be presented to the server.
 * 
 * @return DER-encoded DIC certificate bytes or empty vector if not available
 */
std::vector<uint8_t> GetMTLSClientCertificate();

/**
 * Sign TLS handshake data using the hardware-backed key.
 * This is used during the mTLS handshake to prove possession of the private key
 * associated with the DIC certificate.
 * 
 * @param handshake_data The TLS handshake data to sign
 * @return Signature bytes or empty vector if signing failed
 */
std::vector<uint8_t> SignMTLSHandshake(base::span<const uint8_t> handshake_data);

/**
 * Get the DIC certificate chain for mTLS (if intermediate certificates exist).
 * Currently returns single certificate, but can be extended for full chain.
 * 
 * @return Vector of DER-encoded certificate byte vectors or empty if not available
 */
std::vector<std::vector<uint8_t>> GetMTLSCertificateChain();

/**
 * Get security information about the hardware key used for mTLS.
 * This provides details about the security level for logging/debugging.
 * 
 * @return JSON string with security information
 */
std::string GetMTLSSecurityInfo();

// CSR Generation Functions

/**
 * Generate a Certificate Signing Request (CSR) using the hardware-backed private key.
 * The CSR contains device information and is signed by the non-exportable hardware key.
 * Uses OpenSSL for industry-standard PKCS#10 compliance.
 * 
 * @param device_id The device identifier for CSR subject
 * @param public_key_bytes The encoded public key bytes
 * @param private_key_alias The Android KeyStore alias for the private key
 * @return PEM-encoded CSR string or empty if generation failed
 */
std::string GenerateCSR(const std::string& device_id,
                       base::span<const uint8_t> public_key_bytes,
                       const std::string& private_key_alias);

}  // namespace net::android::wootz

#endif  // NET_ANDROID_WOOTZ_KEYSTORE_H_
