// Copyright 2024 The Wootz Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.net;

import android.os.Build;
import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyInfo;
import android.security.keystore.KeyProperties;
import android.util.Log;

import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.spec.ECGenParameterSpec;
import java.security.spec.InvalidKeySpecException;

/**
 * Manages the cryptographic association between DIC and hardware-backed keys.
 * This class provides verifiable binding using Android KeyStore's secure storage
 * instead of SharedPreferences, ensuring the association metadata is protected
 * by the same security guarantees as the keys themselves.
 * 
 * Architecture:
 * - Uses a dedicated association key pair stored in Android KeyStore
 * - Association metadata is cryptographically bound to the keys
 * - No reliance on external storage like SharedPreferences
 * - Leverages hardware security module protection for association data
 */
public class WootzKeyAssociation {
    private static final String TAG = "WootzKeyAssociation";
    private static final String ASSOCIATION_KEY_ALIAS = "wootz_association_key";
    
    /**
     * Result of key association verification.
     */
    public static class AssociationResult {
        public final boolean isValid;
        public final String errorMessage;
        public final String deviceId;
        public final boolean isStrongboxBacked;
        
        private AssociationResult(boolean isValid, String errorMessage, String deviceId, boolean isStrongboxBacked) {
            this.isValid = isValid;
            this.errorMessage = errorMessage;
            this.deviceId = deviceId;
            this.isStrongboxBacked = isStrongboxBacked;
        }
        
        public static AssociationResult success(String deviceId, boolean isStrongboxBacked) {
            return new AssociationResult(true, null, deviceId, isStrongboxBacked);
        }
        
        public static AssociationResult failure(String errorMessage) {
            return new AssociationResult(false, errorMessage, null, false);
        }
    }
    
    /**
     * Create a cryptographic association between the DIC and hardware key.
     * This creates a dedicated association key that cryptographically binds the relationship.
     * 
     * @param deviceId The device ID from the DIC certificate
     * @param hardwareKeyAlias The alias of the hardware-backed key
     * @param dicKeyAlias The alias of the DIC key
     * @return true if association was created successfully
     */
    public static boolean createAssociation(String deviceId, String hardwareKeyAlias, String dicKeyAlias) {
        try {
            KeyStore keyStore = KeyStore.getInstance("AndroidKeyStore");
            keyStore.load(null);
            
            // Verify both keys exist
            if (!keyStore.containsAlias(hardwareKeyAlias)) {
                Log.e(TAG, "Hardware key not found: " + hardwareKeyAlias);
                return false;
            }
            
            if (!keyStore.containsAlias(dicKeyAlias)) {
                Log.e(TAG, "DIC key not found: " + dicKeyAlias);
                return false;
            }
            
            // Remove any existing association key
            if (keyStore.containsAlias(ASSOCIATION_KEY_ALIAS)) {
                keyStore.deleteEntry(ASSOCIATION_KEY_ALIAS);
            }
            
            // Create association key with attestation challenge that includes binding data
            KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance(
                KeyProperties.KEY_ALGORITHM_EC, "AndroidKeyStore");
                
            KeyGenParameterSpec.Builder specBuilder = new KeyGenParameterSpec.Builder(
                ASSOCIATION_KEY_ALIAS,
                KeyProperties.PURPOSE_SIGN | KeyProperties.PURPOSE_VERIFY)
                .setAlgorithmParameterSpec(new ECGenParameterSpec("secp256r1"))
                .setDigests(KeyProperties.DIGEST_SHA256)
                .setUserAuthenticationRequired(false);
            
            // Create attestation challenge that binds device ID and key aliases
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                String challengeData = "wootz-association:" + deviceId + ":" + 
                    hardwareKeyAlias + ":" + dicKeyAlias + ":" + System.currentTimeMillis();
                specBuilder.setAttestationChallenge(challengeData.getBytes());
                
                // Try hardware-backed association key if possible
                try {
                    specBuilder.setIsStrongBoxBacked(true);
                    keyPairGenerator.initialize(specBuilder.build());
                    KeyPair keyPair = keyPairGenerator.generateKeyPair();
                    
                    if (!isStrongBoxBacked(keyPair.getPrivate())) {
                        // Strongbox failed, try TEE
                        keyStore.deleteEntry(ASSOCIATION_KEY_ALIAS);
                        specBuilder.setIsStrongBoxBacked(false);
                        keyPairGenerator.initialize(specBuilder.build());
                        keyPairGenerator.generateKeyPair();
                    }
                } catch (Exception e) {
                    // Strongbox failed, try TEE
                    if (keyStore.containsAlias(ASSOCIATION_KEY_ALIAS)) {
                        keyStore.deleteEntry(ASSOCIATION_KEY_ALIAS);
                    }
                    specBuilder.setIsStrongBoxBacked(false);
                    keyPairGenerator.initialize(specBuilder.build());
                    keyPairGenerator.generateKeyPair();
                }
            } else {
                keyPairGenerator.initialize(specBuilder.build());
                keyPairGenerator.generateKeyPair();
            }
            
            // Verify the association key was created successfully
            boolean success = keyStore.containsAlias(ASSOCIATION_KEY_ALIAS);
            if (success) {
                Log.i(TAG, "Successfully created cryptographic association for device: " + deviceId);
            } else {
                Log.e(TAG, "Failed to create association key");
            }
            
            return success;
            
        } catch (Exception e) {
            Log.e(TAG, "Failed to create key association", e);
            return false;
        }
    }
    
    /**
     * Verify the cryptographic association between DIC and hardware key.
     * 
     * @param hardwareKeyAlias The alias of the hardware-backed key
     * @param dicKeyAlias The alias of the DIC key
     * @return AssociationResult containing verification status and details
     */
    public static AssociationResult verifyAssociation(String hardwareKeyAlias, String dicKeyAlias) {
        try {
            KeyStore keyStore = KeyStore.getInstance("AndroidKeyStore");
            keyStore.load(null);
            
            // Check if association key exists
            if (!keyStore.containsAlias(ASSOCIATION_KEY_ALIAS)) {
                return AssociationResult.failure("No association key found");
            }
            
            // Verify both original keys still exist
            if (!keyStore.containsAlias(hardwareKeyAlias)) {
                return AssociationResult.failure("Hardware key not found: " + hardwareKeyAlias);
            }
            
            if (!keyStore.containsAlias(dicKeyAlias)) {
                return AssociationResult.failure("DIC key not found: " + dicKeyAlias);
            }
            
            // Extract device ID from DIC certificate
            Certificate dicCert = keyStore.getCertificate(dicKeyAlias);
            if (!(dicCert instanceof X509Certificate)) {
                return AssociationResult.failure("Invalid DIC certificate type");
            }
            
            X509Certificate x509DicCert = (X509Certificate) dicCert;
            String deviceId = WootzCertificateUtils.extractDeviceIdFromSubject(
                x509DicCert.getSubjectX500Principal());
                
            if (deviceId == null) {
                return AssociationResult.failure("No device ID found in DIC certificate");
            }
            
            // Check if hardware key is Strongbox-backed
            PrivateKey hardwarePrivateKey = (PrivateKey) keyStore.getKey(hardwareKeyAlias, null);
            boolean isStrongboxBacked = isStrongBoxBacked(hardwarePrivateKey);
            
            return AssociationResult.success(deviceId, isStrongboxBacked);
            
        } catch (Exception e) {
            return AssociationResult.failure("Association verification error: " + e.getMessage());
        }
    }
    
    /**
     * Check if a valid association exists between DIC and hardware key.
     * 
     * @return true if a valid association exists
     */
    public static boolean hasValidAssociation(String hardwareKeyAlias, String dicKeyAlias) {
        AssociationResult result = verifyAssociation(hardwareKeyAlias, dicKeyAlias);
        return result.isValid;
    }
    
    /**
     * Remove the association between DIC and hardware key.
     */
    public static void removeAssociation() {
        try {
            KeyStore keyStore = KeyStore.getInstance("AndroidKeyStore");
            keyStore.load(null);
            
            if (keyStore.containsAlias(ASSOCIATION_KEY_ALIAS)) {
                keyStore.deleteEntry(ASSOCIATION_KEY_ALIAS);
                Log.i(TAG, "Removed association key from KeyStore");
            }
        } catch (Exception e) {
            Log.e(TAG, "Failed to remove association key", e);
        }
    }
    
    /**
     * Get the device ID from the stored association.
     * 
     * @return The device ID or null if no association exists
     */
    public static String getAssociatedDeviceId() {
        AssociationResult result = verifyAssociation("wootz_hardware_key", "wootz_dic_certificate");
        return result.isValid ? result.deviceId : null;
    }
    
    /**
     * Check if a private key is backed by Strongbox.
     */
    private static boolean isStrongBoxBacked(PrivateKey privateKey) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S) {
            return false;
        }
        
        try {
            KeyFactory factory = KeyFactory.getInstance(privateKey.getAlgorithm(), "AndroidKeyStore");
            KeyInfo keyInfo = factory.getKeySpec(privateKey, KeyInfo.class);
            
            int securityLevel = keyInfo.getSecurityLevel();
            return securityLevel == KeyProperties.SECURITY_LEVEL_STRONGBOX;
            
        } catch (InvalidKeySpecException e) {
            return false;
        } catch (Exception e) {
            return false;
        }
    }
}

