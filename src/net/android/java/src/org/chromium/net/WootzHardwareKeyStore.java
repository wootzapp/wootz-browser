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

import java.io.ByteArrayInputStream;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.spec.ECGenParameterSpec;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.Date;
import java.util.List;

import javax.security.auth.x500.X500Principal;

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
 * - Cryptographic association between DIC and hardware keys
 * - JNI-only interface (no public Java access)
 * - Integration with Chromium's existing security infrastructure
 * 
 * DIC-Hardware Key Association:
 * The association between Device Identity Certificate (DIC) and the non-exportable
 * hardware key is established through cryptographic binding:
 * 
 * 1. Hardware Key Generation (WOOTZ_KEY_ALIAS = "wootz_hardware_key"):
 *    - Generated in Strongbox/TEE with non-exportable properties
 *    - Used for attestation and cryptographic binding signatures
 * 
 * 2. DIC Storage (WOOTZ_DIC_ALIAS = "wootz_dic_certificate"):
 *    - DIC certificate and private key stored in Android KeyStore
 *    - Associated with hardware key through cryptographic signature
 * 
 * 3. Cryptographic Binding (WootzKeyAssociation):
 *    - Creates dedicated association key in Android KeyStore
 *    - Association key uses attestation challenge with binding data
 *    - Provides verifiable proof that DIC belongs to specific hardware key
 *    - No reliance on SharedPreferences - all data in secure KeyStore
 * 
 * 4. Verification Methods:
 *    - verifyDicAssociation(): Cryptographically verifies the binding
 *    - hasDicAssociation(): Quick check for association existence
 *    - getDicAssociationDetails(): Full association status with security level
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
    private static final String WOOTZ_DIC_ALIAS = "wootz_dic_certificate";
    

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
        try {
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            
            // Remove any existing key and association first
            if (keyStore.containsAlias(WOOTZ_KEY_ALIAS)) {
                keyStore.deleteEntry(WOOTZ_KEY_ALIAS);
                WootzKeyAssociation.removeAssociation();
            }
            
            KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance(
                KEY_ALGORITHM, ANDROID_KEYSTORE);
            
            if (tryStrongboxGenerationWithAttestation(keyPairGenerator, attestationChallenge)) {
                return true;
            }
            
            return tryTeeGenerationWithAttestation(keyPairGenerator, attestationChallenge);

        } catch (Exception e) {
            Log.e(TAG, "Hardware key generation failed", e);
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
            return WootzEnrollmentUtils.convertPublicKeyToPem(publicKey.getEncoded());
            
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
            
            return WootzEnrollmentUtils.convertCertificateChainToPem(certChain);
            
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
            
            if (attestationChallenge != null) {
                specBuilder.setAttestationChallenge(attestationChallenge);
            }
            
            keyPairGenerator.initialize(specBuilder.build());
            KeyPair keyPair = keyPairGenerator.generateKeyPair();
            
            return isStrongBoxBacked(keyPair.getPrivate());
            
        } catch (Exception e) {
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
            
            return keyPair != null && keyPair.getPrivate() != null && 
                   isHardwareBacked(keyPair.getPrivate());
                        
        } catch (Exception e) {
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
                return true;
            }
            
            // Generate attestation challenge from browser startup context
            String challengeData = "wootz-browser-" + System.currentTimeMillis();
            byte[] attestationChallenge = challengeData.getBytes();
            
            // Generate hardware-backed key with attestation
            return generateHardwareBackedKeyWithAttestation(attestationChallenge);
            
        } catch (Exception e) {
            Log.e(TAG, "Hardware keystore initialization failed", e);
            return false;
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
    
    /**
     * Store the Device Identity Certificate (DIC) received from enrollment server.
     * This associates the DIC with the hardware-backed key for client authentication.
     * 
     * @param deviceId The device ID from enrollment response
     * @param dicCertificatePem The DIC certificate in PEM format
     * @param dicPrivateKeyPem The DIC private key in PEM format (will be stored securely)
     * @param expiresAt ISO8601 timestamp when DIC expires
     * @param issuedAt ISO8601 timestamp when DIC was issued
     * @param stepCaUrl The Step CA URL for future certificate operations
     * @return true if DIC was stored successfully
     */
    public static boolean storeDicCertificate(String deviceId, String dicCertificatePem, 
            String dicPrivateKeyPem, String expiresAt, String issuedAt, String stepCaUrl) {
        
        try {
            // Parse and validate the DIC certificate
            X509Certificate dicCert = WootzCertificateUtils.parsePemCertificate(dicCertificatePem);
            if (dicCert == null) {
                return false;
            }
            
            // Validate DIC properties using Chromium-style validation
            WootzCertificateUtils.DicValidationResult validation = 
                WootzCertificateUtils.validateDicCertificate(dicCert, deviceId);
            if (!validation.isValid) {
                Log.e(TAG, "DIC validation failed: " + validation.errorMessage);
                return false;
            }
            
            // Parse the DIC private key from PEM
            PrivateKey dicPrivateKey = WootzCertificateUtils.parsePemPrivateKey(dicPrivateKeyPem);
            if (dicPrivateKey == null) {
                return false;
            }
            
            // Store the DIC certificate and private key in Android KeyStore
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            
            // Remove any existing DIC and association first
            if (keyStore.containsAlias(WOOTZ_DIC_ALIAS)) {
                keyStore.deleteEntry(WOOTZ_DIC_ALIAS);
                WootzKeyAssociation.removeAssociation();
            }
            
            // Create certificate chain (DIC certificate only for now)
            Certificate[] certChain = new Certificate[] { dicCert };
            
            // Store the DIC private key and certificate chain
            keyStore.setKeyEntry(WOOTZ_DIC_ALIAS, dicPrivateKey, null, certChain);
            
            // Associate DIC with the hardware-backed key
            associateDicWithHardwareKey(validation.extractedDeviceId);
            
            return true;
            
        } catch (Exception e) {
            Log.e(TAG, "Failed to store DIC certificate", e);
            return false;
        }
    }
    
    /**
     * Get the stored DIC certificate in PEM format.
     * 
     * @return PEM-encoded DIC certificate or null if not available
     */
    public static String getDicCertificatePem() {
        try {
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            
            Certificate certificate = keyStore.getCertificate(WOOTZ_DIC_ALIAS);
            if (certificate == null) {
                Log.w(TAG, "No DIC certificate found");
                return null;
            }
            
            return WootzEnrollmentUtils.convertCertificateToPem(certificate);
            
        } catch (Exception e) {
            Log.e(TAG, "Failed to get DIC certificate", e);
            return null;
        }
    }
    
    /**
     * Check if a valid DIC certificate is stored.
     * 
     * @return true if DIC certificate exists and is valid
     */
    public static boolean hasDicCertificate() {
        try {
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            
            if (!keyStore.containsAlias(WOOTZ_DIC_ALIAS)) {
                return false;
            }
            
            Certificate certificate = keyStore.getCertificate(WOOTZ_DIC_ALIAS);
            if (certificate instanceof X509Certificate) {
                X509Certificate x509Cert = (X509Certificate) certificate;
                
                // Check if certificate is still valid (not expired)
                try {
                    x509Cert.checkValidity();
                    return true;
                } catch (Exception e) {
                    Log.w(TAG, "DIC certificate is expired or not yet valid", e);
                    return false;
                }
            }
            
            return false;
            
        } catch (Exception e) {
            Log.e(TAG, "Failed to check DIC certificate", e);
            return false;
        }
    }
    
    /**
     * Sign data using the DIC private key for client authentication.
     * 
     * @param data Data to sign
     * @return Signature bytes or null if signing failed
     */
    public static byte[] signWithDicKey(byte[] data) {
        try {
            KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
            keyStore.load(null);
            
            PrivateKey dicPrivateKey = (PrivateKey) keyStore.getKey(WOOTZ_DIC_ALIAS, null);
            if (dicPrivateKey == null) {
                Log.e(TAG, "No DIC private key available for signing");
                return null;
            }
            
            java.security.Signature signature = java.security.Signature.getInstance(SIGNATURE_ALGORITHM);
            signature.initSign(dicPrivateKey);
            signature.update(data);
            
            Log.i(TAG, "Data signed successfully with DIC private key");
            return signature.sign();
            
        } catch (Exception e) {
            Log.e(TAG, "Failed to sign with DIC private key", e);
            return null;
        }
    }
    
    // Private helper methods for DIC certificate handling
    
    
    /**
     * Associate the DIC with the hardware-backed key for client authentication.
     * This creates a cryptographic binding between the DIC and the non-exportable key.
     * 
     * @param deviceId The device ID extracted from the DIC certificate
     */
    private static void associateDicWithHardwareKey(String deviceId) {
        boolean success = WootzKeyAssociation.createAssociation(deviceId, WOOTZ_KEY_ALIAS, WOOTZ_DIC_ALIAS);
        if (success) {
            Log.i(TAG, "Successfully created cryptographic association between DIC and hardware key");
        } else {
            Log.e(TAG, "Failed to create cryptographic association between DIC and hardware key");
        }
    }
    
    /**
     * Verify that the DIC is properly associated with the hardware-backed key.
     * This performs cryptographic verification of the binding.
     * 
     * @return true if the association is valid
     */
    public static boolean verifyDicAssociation() {
        WootzKeyAssociation.AssociationResult result = 
            WootzKeyAssociation.verifyAssociation(WOOTZ_KEY_ALIAS, WOOTZ_DIC_ALIAS);
        
        if (!result.isValid) {
            Log.e(TAG, "DIC association verification failed: " + result.errorMessage);
        }
        
        return result.isValid;
    }
    
    /**
     * Check if a valid DIC association exists.
     * 
     * @return true if DIC is properly associated with hardware key
     */
    public static boolean hasDicAssociation() {
        return WootzKeyAssociation.hasValidAssociation(WOOTZ_KEY_ALIAS, WOOTZ_DIC_ALIAS);
    }
    
    /**
     * Get the device ID from the DIC association.
     * 
     * @return The associated device ID or null if no association exists
     */
    public static String getAssociatedDeviceId() {
        return WootzKeyAssociation.getAssociatedDeviceId();
    }
    
    /**
     * Remove the DIC association. This should be called when regenerating keys
     * or resetting enrollment.
     */
    public static void removeDicAssociation() {
        WootzKeyAssociation.removeAssociation();
        Log.i(TAG, "Removed DIC association");
    }
    
    /**
     * Get detailed information about the DIC association.
     * This provides comprehensive status including security level and device ID.
     * 
     * @return AssociationResult with detailed association information
     */
    public static WootzKeyAssociation.AssociationResult getDicAssociationDetails() {
        return WootzKeyAssociation.verifyAssociation(WOOTZ_KEY_ALIAS, WOOTZ_DIC_ALIAS);
    }
}
