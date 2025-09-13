// Copyright 2024 The Wootz Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.net;

import android.util.Log;

import java.io.ByteArrayInputStream;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.List;

import javax.security.auth.x500.X500Principal;

/**
 * Utility class for certificate validation, parsing, and verification operations.
 * This class provides helper methods for DIC (Device Identity Certificate) handling
 * following Chromium's certificate verification patterns.
 */
public class WootzCertificateUtils {
    private static final String TAG = "WootzCertificateUtils";
    
    // Extended Key Usage OIDs
    private static final String OID_CLIENT_AUTH = "1.3.6.1.5.5.7.3.2";
    private static final String OID_DEVICE_IDENTITY = "1.3.6.1.4.1.11129.2.1.30"; // Custom OID for device identity

    /**
     * Result class for DIC certificate validation.
     * Contains validation status and extracted certificate properties.
     */
    public static class DicValidationResult {
        public final boolean isValid;
        public final String errorMessage;
        public final String extractedDeviceId;
        public final boolean hasClientAuth;
        public final boolean hasDeviceIdentityOid;
        
        private DicValidationResult(boolean isValid, String errorMessage, String extractedDeviceId,
                           boolean hasClientAuth, boolean hasDeviceIdentityOid) {
            this.isValid = isValid;
            this.errorMessage = errorMessage;
            this.extractedDeviceId = extractedDeviceId;
            this.hasClientAuth = hasClientAuth;
            this.hasDeviceIdentityOid = hasDeviceIdentityOid;
        }
        
        public static DicValidationResult failure(String errorMessage) {
            return new DicValidationResult(false, errorMessage, null, false, false);
        }
        
        public static DicValidationResult success(String deviceId, boolean hasClientAuth, boolean hasDeviceIdentityOid) {
            return new DicValidationResult(true, null, deviceId, hasClientAuth, hasDeviceIdentityOid);
        }
    }

    /**
     * Validate a DIC certificate following Chromium's certificate verification patterns.
     * Extracts and validates: CN=device_id, EKU:ClientAuth, custom OID DeviceIdentity
     * 
     * @param cert The X509Certificate to validate
     * @param expectedDeviceId The expected device ID to match against certificate CN
     * @return DicValidationResult containing validation status and extracted properties
     */
    public static DicValidationResult validateDicCertificate(X509Certificate cert, String expectedDeviceId) {
        try {
            // Check certificate validity period first (following Chromium X509Util pattern)
            try {
                cert.checkValidity();
            } catch (Exception e) {
                return DicValidationResult.failure("Certificate validity check failed: " + e.getMessage());
            }
            
            // Extract device ID from certificate subject
            String extractedDeviceId = extractDeviceIdFromSubject(cert.getSubjectX500Principal());
            if (extractedDeviceId == null) {
                return DicValidationResult.failure("No device ID found in certificate subject");
            }
            
            // Validate Extended Key Usage contains ClientAuth
            boolean hasClientAuth = validateClientAuthEku(cert);
            
            // Check for custom DeviceIdentity OID (optional)
            boolean hasDeviceIdentityOid = validateDeviceIdentityOid(cert);
            Log.e(TAG, "hasDeviceIdentityOid: " + hasDeviceIdentityOid);
            
            return DicValidationResult.success(extractedDeviceId, hasClientAuth, hasDeviceIdentityOid);
            
        } catch (Exception e) {
            return DicValidationResult.failure("Certificate validation error: " + e.getMessage());
        }
    }

    /**
     * Parse a PEM-formatted certificate into an X509Certificate.
     * 
     * @param pemCertificate The PEM-formatted certificate string
     * @return X509Certificate object or null if parsing failed
     */
    public static X509Certificate parsePemCertificate(String pemCertificate) {
        try {
            if (pemCertificate == null || pemCertificate.trim().isEmpty()) {
                Log.e(TAG, "PEM certificate is null or empty");
                return null;
            }
            
            Log.d(TAG, "Raw PEM certificate input: " + pemCertificate.substring(0, Math.min(100, pemCertificate.length())) + "...");
            
            // Clean up the PEM data - remove headers, footers, and normalize whitespace
            // (JSON escape sequences should already be handled by JSON extraction)
            String certData = pemCertificate.trim()
                .replace("-----BEGIN CERTIFICATE-----", "")
                .replace("-----END CERTIFICATE-----", "")
                .replaceAll("\\s+", ""); // Remove all whitespace including newlines
            
            if (certData.isEmpty()) {
                Log.e(TAG, "PEM certificate data is empty after cleanup");
                return null;
            }
            
            Log.d(TAG, "Base64 data length: " + certData.length() + ", first 50 chars: " + 
                  certData.substring(0, Math.min(50, certData.length())));
            
            // Use NO_WRAP flag to handle Base64 data without line breaks
            byte[] certBytes = android.util.Base64.decode(certData, android.util.Base64.NO_WRAP);
            
            CertificateFactory certFactory = CertificateFactory.getInstance("X.509");
            return (X509Certificate) certFactory.generateCertificate(new ByteArrayInputStream(certBytes));
            
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "Invalid Base64 in PEM certificate: " + e.getMessage(), e);
            Log.e(TAG, "Problematic Base64 data (first 100 chars): " + 
                  (pemCertificate != null ? pemCertificate.substring(0, Math.min(100, pemCertificate.length())) : "null"));
            return null;
        } catch (Exception e) {
            Log.e(TAG, "Failed to parse PEM certificate", e);
            return null;
        }
    }


    /**
     * Extract device ID from certificate subject (CN field).
     * Follows X.500 Distinguished Name parsing patterns.
     * 
     * @param subject The X500Principal subject from the certificate
     * @return The extracted device ID or null if not found
     */
    public static String extractDeviceIdFromSubject(X500Principal subject) {
        String subjectDN = subject.getName();
        Log.d(TAG, "Extracting device ID from subject DN: " + subjectDN);
        
        // Look for CN= pattern in the subject DN
        String[] parts = subjectDN.split(",");
        for (String part : parts) {
            part = part.trim();
            if (part.startsWith("CN=")) {
                String extractedCN = part.substring(3).trim();
                Log.d(TAG, "Found CN in certificate: " + extractedCN);
                return extractedCN;
            }
        }
        
        Log.w(TAG, "No CN found in certificate subject: " + subjectDN);
        return null;
    }

    /**
     * Validate that the certificate has ClientAuth in Extended Key Usage.
     * Following Chromium's EKU validation pattern.
     * 
     * @param cert The X509Certificate to validate
     * @return true if ClientAuth EKU is present
     */
    public static boolean validateClientAuthEku(X509Certificate cert) {
        try {
            List<String> ekuOids = cert.getExtendedKeyUsage();
            if (ekuOids == null) {
                return false; // ClientAuth EKU is required for DIC
            }
            
            return ekuOids.contains(OID_CLIENT_AUTH);
        } catch (Exception e) {
            return false;
        }
    }

    /**
     * Check if certificate contains custom DeviceIdentity OID.
     * This is optional but provides additional validation.
     * 
     * @param cert The X509Certificate to check
     * @return true if DeviceIdentity OID is present
     */
    public static boolean validateDeviceIdentityOid(X509Certificate cert) {
        try {
            List<String> ekuOids = cert.getExtendedKeyUsage();
            if (ekuOids == null) {
                return false;
            }
            
            return ekuOids.contains(OID_DEVICE_IDENTITY);
        } catch (Exception e) {
            return false;
        }
    }
}
