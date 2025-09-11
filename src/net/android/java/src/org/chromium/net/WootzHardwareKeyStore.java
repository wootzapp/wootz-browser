// Copyright 2024 The Wootz Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.net;

import android.os.Build;
import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyInfo;
import android.security.keystore.KeyProperties;
import android.util.Log;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;

import java.security.InvalidAlgorithmParameterException;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.interfaces.ECPrivateKey;
import java.security.spec.ECGenParameterSpec;
import java.security.spec.InvalidKeySpecException;
import java.io.IOException;

/**
 * Wootz Hardware Key Store - Secure hardware-backed key operations with attestation
 * 
 * This class provides non-exportable key generation using Android's hardware security
 * modules (Strongbox or TEE) with proper attestation support. Keys generated here
 * are never exposed to Java application layer - only accessible via JNI to C++.
 * 
 * Security Features:
 * - Non-exportable P-256 keys in Strongbox/TEE
 * - Hardware attestation with X.509 certificate chains
 * - JNI-only interface (no public Java access)
 * - Integration with Chromium's existing security infrastructure
 * 
 * Architecture:
 * - Located in net/android (Chromium's trusted security layer)
 * - JNI interface for C++ integration only
 * - Follows AndroidKeyStore patterns for maximum security
 */
@JNINamespace("net::android::wootz")
public class WootzHardwareKeyStore {
    private static final String TAG = "WootzHardwareKeyStore";
    private static final String ANDROID_KEYSTORE = "AndroidKeyStore";
    private static final String KEY_ALGORITHM = KeyProperties.KEY_ALGORITHM_EC;
    private static final String SIGNATURE_ALGORITHM = "SHA256withECDSA";
    private static final String CURVE_NAME = "secp256r1"; // P-256
    private static final String WOOTZ_KEY_ALIAS = "wootz_hardware_key";
    
    // Track if the current key was generated with Strongbox intention
    private static boolean sCurrentKeyStrongboxIntended = false;

    /**
     * Generate hardware-backed P-256 key with attestation challenge.
     * This method creates a non-exportable key in Strongbox/TEE and returns
     * the device public key and attestation chain for verification.
     * 
     * @param attestationChallenge Challenge bytes for hardware attestation
     * @return true if key generation with attestation was successful
     */
    @CalledByNative
    private static boolean generateHardwareBackedKeyWithAttestation(byte[] attestationChallenge) {
        Log.i(TAG, "Generating hardware-backed P-256 key with attestation challenge");
        
        try {
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            
            // Remove any existing key first
            if (keyStore.containsAlias(WOOTZ_KEY_ALIAS)) {
                keyStore.deleteEntry(WOOTZ_KEY_ALIAS);
            }
            
            KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance(
                KEY_ALGORITHM, ANDROID_KEYSTORE);
            
            if (tryStrongboxGenerationWithAttestation(keyPairGenerator, attestationChallenge)) {
                return true;
            }
            
            return tryTeeGenerationWithAttestation(keyPairGenerator, attestationChallenge);

        } catch (Exception e) {
            Log.e(TAG, "Critical error in hardware key generation with attestation", e);
            return false;
        }
    }

    /**
     * Get the device public key in PEM format.
     * This public key corresponds to the non-exportable private key in hardware.
     * 
     * @return PEM-encoded public key or null if not available
     */
    @CalledByNative
    private static String getDevicePublicKeyPem() {
        try {
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            
            Certificate certificate = keyStore.getCertificate(WOOTZ_KEY_ALIAS);
            if (certificate == null) {
                Log.e(TAG, "No certificate found for hardware key");
                return null;
            }
            
            PublicKey publicKey = certificate.getPublicKey();
            return convertPublicKeyToPem(publicKey.getEncoded());
            
        } catch (Exception e) {
            Log.e(TAG, "Failed to get device public key", e);
            return null;
        }
    }

    /**
     * Get the hardware attestation certificate chain in PEM format.
     * This provides cryptographic proof that the key is hardware-backed.
     * 
     * @return PEM-encoded attestation certificate chain or null if not available
     */
    @CalledByNative
    private static String getAttestationChainPem() {
        try {
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            
            Certificate[] certChain = keyStore.getCertificateChain(WOOTZ_KEY_ALIAS);
            if (certChain == null || certChain.length == 0) {
                Log.e(TAG, "No attestation certificate chain found");
                return null;
            }
            
            return convertCertificateChainToPem(certChain);
            
        } catch (Exception e) {
            Log.e(TAG, "Failed to get attestation certificate chain", e);
            return null;
        }
    }

    /**
     * Check if the current key is backed by Strongbox (highest security level).
     * 
     * @return true if key is Strongbox-backed
     */
    @CalledByNative
    private static boolean isKeyStrongboxBacked() {
        try {
            PrivateKey privateKey = getPrivateKeyInternal();
            return privateKey != null && isStrongBoxBacked(privateKey);
        } catch (Exception e) {
            Log.e(TAG, "Failed to check Strongbox backing", e);
            return false;
        }
    }

    /**
     * Check if the current key is hardware-backed (TEE or Strongbox).
     * 
     * @return true if key is hardware-backed
     */
    @CalledByNative
    private static boolean isKeyHardwareBacked() {
        try {
            PrivateKey privateKey = getPrivateKeyInternal();
            return privateKey != null && isHardwareBacked(privateKey);
        } catch (Exception e) {
            Log.e(TAG, "Failed to check hardware backing", e);
            return false;
        }
    }

    /**
     * Sign data using the non-exportable hardware private key.
     * The private key never leaves the secure hardware.
     * 
     * @param data Data to sign
     * @return Signature bytes or null if signing failed
     */
    @CalledByNative
    private static byte[] signWithHardwareKey(byte[] data) {
        try {
            PrivateKey privateKey = getPrivateKeyInternal();
            if (privateKey == null) {
                Log.e(TAG, "No hardware private key available for signing");
                return null;
            }
            
            java.security.Signature signature = java.security.Signature.getInstance(SIGNATURE_ALGORITHM);
            signature.initSign(privateKey);
            signature.update(data);
            
            return signature.sign();
            
        } catch (Exception e) {
            Log.e(TAG, "Failed to sign with hardware key", e);
            return null;
        }
    }

    // Private implementation methods

    private static boolean tryStrongboxGenerationWithAttestation(
            KeyPairGenerator keyPairGenerator, byte[] attestationChallenge) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.P) {
            return false;
        }
        
        try {
            KeyGenParameterSpec.Builder specBuilder = new KeyGenParameterSpec.Builder(
                WOOTZ_KEY_ALIAS,
                KeyProperties.PURPOSE_SIGN | KeyProperties.PURPOSE_VERIFY)
                .setAlgorithmParameterSpec(new ECGenParameterSpec(CURVE_NAME))
                .setDigests(KeyProperties.DIGEST_SHA256)
                .setUserAuthenticationRequired(false)
                .setIsStrongBoxBacked(true);
            
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P && attestationChallenge != null) {
                specBuilder.setAttestationChallenge(attestationChallenge);
            }
            
            keyPairGenerator.initialize(specBuilder.build());
            KeyPair keyPair = keyPairGenerator.generateKeyPair();
            
            if (isStrongBoxBacked(keyPair.getPrivate())) {
                Log.i(TAG, "Successfully generated Strongbox-backed key with attestation");
                return true;
            } else {
                Log.e(TAG, "Strongbox requested but not achieved, trying TEE");
                return false;
            }
            
        } catch (Exception e) {
            Log.w(TAG, "Strongbox key generation with attestation failed, trying TEE: " + e.getMessage());
            return false;
        }
    }

    private static boolean tryTeeGenerationWithAttestation(
            KeyPairGenerator keyPairGenerator, byte[] attestationChallenge) {
        try {
            KeyGenParameterSpec.Builder specBuilder = new KeyGenParameterSpec.Builder(
                WOOTZ_KEY_ALIAS,
                KeyProperties.PURPOSE_SIGN | KeyProperties.PURPOSE_VERIFY)
                .setAlgorithmParameterSpec(new ECGenParameterSpec(CURVE_NAME))
                .setDigests(KeyProperties.DIGEST_SHA256)
                .setUserAuthenticationRequired(false);
            
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P && attestationChallenge != null) {
                specBuilder.setAttestationChallenge(attestationChallenge);
            }
            
            keyPairGenerator.initialize(specBuilder.build());
            KeyPair keyPair = keyPairGenerator.generateKeyPair();
            
            if (keyPair == null || keyPair.getPrivate() == null) {
                return false;
            }
            
            if (isHardwareBacked(keyPair.getPrivate())) {
                Log.i(TAG, "Successfully generated TEE hardware-backed key with attestation");
                return true;
            } else {
                return false;
            }
                        
        } catch (Exception e) {
            Log.e(TAG, "TEE key generation with attestation failed: " + e.getMessage());
            return false;
        }
    }

    private static PrivateKey getPrivateKeyInternal() {
        try {
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            return (PrivateKey) keyStore.getKey(WOOTZ_KEY_ALIAS, null);
        } catch (Exception e) {
            Log.e(TAG, "Failed to get private key", e);
            return null;
        }
    }

    private static boolean isStrongBoxBacked(PrivateKey privateKey) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S) { // API 31+ for getSecurityLevel in KeyInfo
            return false;
        }
        
        try {
            // Use the public KeyInfo API to get security level
            KeyFactory factory = KeyFactory.getInstance(privateKey.getAlgorithm(), ANDROID_KEYSTORE);
            KeyInfo keyInfo = factory.getKeySpec(privateKey, KeyInfo.class);
            
            int securityLevel = keyInfo.getSecurityLevel();
            return securityLevel == KeyProperties.SECURITY_LEVEL_STRONGBOX;
            
        } catch (InvalidKeySpecException e) {
            Log.w(TAG, "Key does not expose KeyInfo for Strongbox check", e);
            return false;
        } catch (Exception e) {
            Log.w(TAG, "Could not determine Strongbox backing: " + e.getMessage());
            return false;
        }
    }

    private static boolean isHardwareBacked(PrivateKey privateKey) {
        if (privateKey == null) {
            return false;
        }
        
        // Check if it's AndroidKeyStore key
        String className = privateKey.getClass().getName();
        if (!className.contains("AndroidKeyStore")) {
            return false;
        }
        
        // For API 31+, use KeyInfo.getSecurityLevel() - the proper public API
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            try {
                KeyFactory factory = KeyFactory.getInstance(privateKey.getAlgorithm(), ANDROID_KEYSTORE);
                KeyInfo keyInfo = factory.getKeySpec(privateKey, KeyInfo.class);
                
                int securityLevel = keyInfo.getSecurityLevel();
                return securityLevel == KeyProperties.SECURITY_LEVEL_STRONGBOX || 
                       securityLevel == KeyProperties.SECURITY_LEVEL_TRUSTED_ENVIRONMENT;
                       
            } catch (InvalidKeySpecException e) {
                Log.w(TAG, "Key does not expose KeyInfo for hardware backing check", e);
            } catch (Exception e) {
                Log.w(TAG, "Could not determine security level: " + e.getMessage());
            }
        }

        return false;
    }

    private static String convertPublicKeyToPem(byte[] publicKeyBytes) {
        try {
            String base64 = android.util.Base64.encodeToString(publicKeyBytes, android.util.Base64.NO_WRAP);
            return "-----BEGIN PUBLIC KEY-----\n" +
                   insertLineBreaks(base64, 64) +
                   "\n-----END PUBLIC KEY-----";
        } catch (Exception e) {
            Log.e(TAG, "Failed to convert public key to PEM", e);
            return null;
        }
    }

    private static String convertCertificateChainToPem(Certificate[] certChain) {
        try {
            StringBuilder pemChain = new StringBuilder();
            for (int i = 0; i < certChain.length; i++) {
                byte[] certBytes = certChain[i].getEncoded();
                String base64 = android.util.Base64.encodeToString(certBytes, android.util.Base64.NO_WRAP);
                
                pemChain.append("-----BEGIN CERTIFICATE-----\n");
                pemChain.append(insertLineBreaks(base64, 64));
                pemChain.append("\n-----END CERTIFICATE-----\n");
                
                if (i < certChain.length - 1) {
                    pemChain.append("\n");
                }
            }
            return pemChain.toString();
        } catch (Exception e) {
            Log.e(TAG, "Failed to convert certificate chain to PEM", e);
            return null;
        }
    }

    private static String insertLineBreaks(String input, int lineLength) {
        StringBuilder result = new StringBuilder();
        for (int i = 0; i < input.length(); i += lineLength) {
            result.append(input.substring(i, Math.min(i + lineLength, input.length())));
            if (i + lineLength < input.length()) {
                result.append("\n");
            }
        }
        return result.toString();
    }
    
    /**
     * Initialize hardware-backed keystore system.
     * This method should be called once during browser startup to ensure
     * hardware-backed certificates are available for the session.
     * 
     * @return true if initialization was successful
     */
    public static boolean initializeHardwareKeystore() {
        try {
            // Check if we already have a hardware-backed key
            if (isKeyHardwareBacked()) {
                Log.i(TAG, "Hardware-backed key already exists, skipping initialization");
                return true;
            }
            
            Log.i(TAG, "Initializing hardware-backed certificate system");
            
            // Generate attestation challenge from browser startup context
            String challengeData = "wootz-browser-" + System.currentTimeMillis();
            byte[] attestationChallenge = challengeData.getBytes();
            
            // Generate hardware-backed key with attestation
            boolean success = generateHardwareBackedKeyWithAttestation(attestationChallenge);
            
            if (success) {
                Log.i(TAG, "Hardware-backed certificate system initialized successfully");
                logSecurityLevel();
            } else {
                Log.e(TAG, "Failed to initialize hardware-backed certificate system");
            }
            
            return success;
        } catch (Exception e) {
            Log.e(TAG, "Exception during certificate system initialization", e);
            return false;
        }
    }
    
    /**
     * Logs the security level of the generated hardware key.
     */
    private static void logSecurityLevel() {
        if (isKeyStrongboxBacked()) {
            Log.i(TAG, "Certificate system using Strongbox security level");
        } else if (isKeyHardwareBacked()) {
            Log.i(TAG, "Certificate system using TEE security level");
        } else {
            Log.w(TAG, "Certificate system not hardware-backed");
        }
    }

    // Public wrapper methods for Java-to-Java calls (e.g., from device enrollment)
    
    /**
     * Public wrapper for generating hardware-backed key with attestation.
     * This method can be called from other Java classes for device enrollment.
     * 
     * @param attestationChallenge Challenge bytes for hardware attestation
     * @return true if key generation with attestation was successful
     */
    public static boolean generateKeyWithAttestation(byte[] attestationChallenge) {
        return generateHardwareBackedKeyWithAttestation(attestationChallenge);
    }

    /**
     * Public wrapper for getting the attestation certificate chain in PEM format.
     * This method can be called from other Java classes for device enrollment.
     * 
     * @return PEM-encoded attestation certificate chain or null if not available
     */
    public static String getAttestationChainAsPem() {
        return getAttestationChainPem();
    }
    
    /**
     * Public wrapper for getting device public key as PEM.
     * This method can be called from other Java classes for device enrollment.
     * 
     * @return PEM-encoded public key or null if not available
     */
    public static String getPublicKeyAsPem() {
        return getDevicePublicKeyPem();
    }
}
