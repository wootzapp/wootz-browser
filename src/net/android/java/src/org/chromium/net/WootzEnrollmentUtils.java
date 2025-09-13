// Copyright 2024 The Wootz Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.net;

import java.security.cert.Certificate;

/**
 * Utility class for device enrollment operations.
 * This class provides helper methods for enrollment workflows,
 * certificate formatting, and device information handling.
 */
public class WootzEnrollmentUtils {
    private static final String TAG = "WootzEnrollmentUtils";

    /**
     * Simple JSON value extraction for enrollment response parsing.
     * This is a lightweight parser for the specific JSON structure used
     * in enrollment server responses.
     * 
     * @param json The JSON string to parse
     * @param key The key to extract
     * @return The extracted value or null if not found
     */
    public static String extractJsonValue(String json, String key) {
        String searchKey = "\"" + key + "\":";
        int startIndex = json.indexOf(searchKey);
        if (startIndex == -1) {
            return null;
        }
        
        startIndex += searchKey.length();
        
        // Skip whitespace
        while (startIndex < json.length() && Character.isWhitespace(json.charAt(startIndex))) {
            startIndex++;
        }
        
        if (startIndex >= json.length()) {
            return null;
        }
        
        // Check if the value is a string (starts with quote)
        if (json.charAt(startIndex) == '"') {
            startIndex++; // Skip opening quote
            int endIndex = findClosingQuote(json, startIndex);
            if (endIndex != -1) {
                String rawValue = json.substring(startIndex, endIndex);
                // Properly handle JSON escape sequences
                return unescapeJsonString(rawValue);
            }
        } else {
            // Handle non-string values (numbers, booleans)
            int endIndex = startIndex;
            while (endIndex < json.length() && 
                   json.charAt(endIndex) != ',' && 
                   json.charAt(endIndex) != '}' && 
                   json.charAt(endIndex) != ']') {
                endIndex++;
            }
            return json.substring(startIndex, endIndex).trim();
        }
        
        return null;
    }

    /**
     * Convert a public key to PEM format.
     * 
     * @param publicKeyBytes The encoded public key bytes
     * @return PEM-formatted public key string or null if conversion failed
     */
    public static String convertPublicKeyToPem(byte[] publicKeyBytes) {
        try {
            String base64 = android.util.Base64.encodeToString(publicKeyBytes, android.util.Base64.NO_WRAP);
            return "-----BEGIN PUBLIC KEY-----\n" +
                   insertLineBreaks(base64, 64) +
                   "\n-----END PUBLIC KEY-----";
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Convert a certificate to PEM format.
     * 
     * @param certificate The certificate to convert
     * @return PEM-formatted certificate string or null if conversion failed
     */
    public static String convertCertificateToPem(Certificate certificate) {
        try {
            byte[] certBytes = certificate.getEncoded();
            String base64 = android.util.Base64.encodeToString(certBytes, android.util.Base64.NO_WRAP);
            return "-----BEGIN CERTIFICATE-----\n" +
                   insertLineBreaks(base64, 64) +
                   "\n-----END CERTIFICATE-----";
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Convert a certificate chain to PEM format.
     * 
     * @param certChain Array of certificates to convert
     * @return PEM-formatted certificate chain string or null if conversion failed
     */
    public static String convertCertificateChainToPem(Certificate[] certChain) {
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
            return null;
        }
    }

    /**
     * Insert line breaks in a string at specified intervals.
     * Used for formatting PEM certificates and keys.
     * 
     * @param input The input string
     * @param lineLength The length at which to insert line breaks
     * @return The formatted string with line breaks
     */
    public static String insertLineBreaks(String input, int lineLength) {
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
     * Create an enrollment request JSON payload.
     * 
     * @param nonceId The nonce ID from the challenge response
     * @param attestationChainPem The PEM-formatted attestation certificate chain
     * @param devicePublicKeyPem The PEM-formatted device public key
     * @param manufacturer The device manufacturer
     * @param model The device model
     * @param osVersion The OS version
     * @return JSON request payload string
     */
    public static String createEnrollmentRequestJson(String nonceId, String attestationChainPem, 
            String devicePublicKeyPem, String manufacturer, String model, String osVersion) {
        return String.format(
            "{" +
            "\"nonceId\":\"%s\"," +
            "\"attestationChainPem\":\"%s\"," +
            "\"devicePublicKeyPem\":\"%s\"," +
            "\"deviceInfo\":{" +
                "\"manufacturer\":\"%s\"," +
                "\"model\":\"%s\"," +
                "\"osVersion\":\"Android %s\"" +
            "}" +
            "}",
            nonceId,
            attestationChainPem.replace("\n", "\\n").replace("\r", ""), // Escape newlines for JSON
            devicePublicKeyPem.replace("\n", "\\n").replace("\r", ""), // Escape newlines for JSON
            manufacturer,
            model,
            osVersion
        );
    }

    /**
     * Validate enrollment response status.
     * 
     * @param response The JSON response from enrollment server
     * @return true if the response indicates success
     */
    public static boolean isEnrollmentResponseSuccessful(String response) {
        String status = extractJsonValue(response, "status");
        return "success".equals(status);
    }

    /**
     * Check if enrollment response contains required DIC data.
     * 
     * @param response The JSON response from enrollment server
     * @return true if all required DIC fields are present
     */
    public static boolean hasRequiredDicData(String response) {
        String deviceId = extractJsonValue(response, "deviceId");
        String dicCertificate = extractJsonValue(response, "dicCertificate");
        String dicPrivateKey = extractJsonValue(response, "dicPrivateKey");
        
        return deviceId != null && dicCertificate != null && dicPrivateKey != null;
    }
    
    /**
     * Find the closing quote for a JSON string value, handling escaped quotes.
     * 
     * @param json The JSON string
     * @param startIndex The index to start searching from (after opening quote)
     * @return The index of the closing quote, or -1 if not found
     */
    private static int findClosingQuote(String json, int startIndex) {
        for (int i = startIndex; i < json.length(); i++) {
            char c = json.charAt(i);
            if (c == '"') {
                // Check if this quote is escaped
                int backslashCount = 0;
                for (int j = i - 1; j >= startIndex && json.charAt(j) == '\\'; j--) {
                    backslashCount++;
                }
                // If even number of backslashes (including 0), the quote is not escaped
                if (backslashCount % 2 == 0) {
                    return i;
                }
            }
        }
        return -1;
    }
    
    /**
     * Unescape a JSON string value by converting escape sequences to actual characters.
     * 
     * @param escaped The escaped JSON string value
     * @return The unescaped string
     */
    private static String unescapeJsonString(String escaped) {
        if (escaped == null || escaped.isEmpty()) {
            return escaped;
        }
        
        StringBuilder result = new StringBuilder();
        for (int i = 0; i < escaped.length(); i++) {
            char c = escaped.charAt(i);
            if (c == '\\' && i + 1 < escaped.length()) {
                char next = escaped.charAt(i + 1);
                switch (next) {
                    case 'n':
                        result.append('\n');
                        i++; // Skip the next character
                        break;
                    case 'r':
                        result.append('\r');
                        i++; // Skip the next character
                        break;
                    case 't':
                        result.append('\t');
                        i++; // Skip the next character
                        break;
                    case '\\':
                        result.append('\\');
                        i++; // Skip the next character
                        break;
                    case '"':
                        result.append('"');
                        i++; // Skip the next character
                        break;
                    case '/':
                        result.append('/');
                        i++; // Skip the next character
                        break;
                    default:
                        // Unknown escape sequence, keep as-is
                        result.append(c);
                        break;
                }
            } else {
                result.append(c);
            }
        }
        return result.toString();
    }
}
