// Copyright 2024 The Wootz Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.net;

import android.os.AsyncTask;
import android.util.Log;

import org.jni_zero.CalledByNative;
import org.jni_zero.JNINamespace;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;

/**
 * Java implementation of device enrollment HTTP client.
 * This handles communication with the enrollment server to obtain
 * challenge nonces and submit device attestations.
 */
@JNINamespace("net::android")
public class WootzDeviceEnrollment {
    private static final String TAG = "WootzDeviceEnrollment";
    private static final String ENROLLMENT_CHALLENGE_URL = "https://testingserver-production-776b.up.railway.app/api/attestation/get-enroll-nonce";
    private static final String ENROLLMENT_SUBMIT_URL = "https://testingserver-production-776b.up.railway.app/api/enrollment/enroll";

    /**
     * Starts the device enrollment process by requesting a challenge from the server.
     * This method runs asynchronously and can be called from Java code.
     */
    public static void startDeviceEnrollment() {
        Log.i(TAG, "Starting device enrollment process");
        new EnrollmentChallengeTask().execute();
    }

    /**
     * AsyncTask to handle the enrollment challenge request in the background.
     */
    private static class EnrollmentChallengeTask extends AsyncTask<Void, Void, String> {
        @Override
        protected String doInBackground(Void... voids) {
            try {
                return requestEnrollmentChallenge();
            } catch (Exception e) {
                Log.e(TAG, "Failed to request enrollment challenge: " + e.getClass().getSimpleName() + ": " + e.getMessage(), e);
                if (e.getCause() != null) {
                    Log.e(TAG, "Caused by: " + e.getCause().getClass().getSimpleName() + ": " + e.getCause().getMessage());
                }
                return null;
            }
        }

        @Override
        protected void onPostExecute(String response) {
            if (response != null) {
                Log.i(TAG, "Received enrollment challenge response: " + response);
                handleChallengeResponse(response);
            } else {
                Log.e(TAG, "Failed to get enrollment challenge response");
            }
        }
    }

    /**
     * Makes HTTP POST request to the enrollment challenge endpoint.
     * 
     * @return The response body as a string, or null if the request failed
     */
    private static String requestEnrollmentChallenge() throws IOException {
        Log.i(TAG, "Creating URL connection to: " + ENROLLMENT_CHALLENGE_URL);
        URL url = new URL(ENROLLMENT_CHALLENGE_URL);
        Log.i(TAG, "URL created successfully, opening connection...");
        HttpURLConnection connection = (HttpURLConnection) url.openConnection();
        Log.i(TAG, "Connection opened successfully");

        try {
            // Configure the connection to match the curl command exactly
            connection.setRequestMethod("POST");
            connection.setRequestProperty("Content-Type", "application/json");
            connection.setRequestProperty("User-Agent", "Wootz-Browser/1.0");
            connection.setConnectTimeout(10000); // 10 seconds
            connection.setReadTimeout(30000);    // 30 seconds
            
            // The curl command doesn't include a request body, so we don't set doOutput or send anything
            Log.i(TAG, "Making POST request to: " + ENROLLMENT_CHALLENGE_URL);
            Log.i(TAG, "Request method: POST");
            Log.i(TAG, "Content-Type: application/json");
            Log.i(TAG, "Connect timeout: " + connection.getConnectTimeout() + "ms");
            Log.i(TAG, "Read timeout: " + connection.getReadTimeout() + "ms");

            Log.i(TAG, "Attempting to get response code...");
            // Check response code
            int responseCode = connection.getResponseCode();
            Log.i(TAG, "Successfully got response code!");
            Log.i(TAG, "HTTP Response Code: " + responseCode);
            
            if (responseCode != HttpURLConnection.HTTP_OK) {
                Log.e(TAG, "HTTP request failed with response code: " + responseCode);
                
                // Try to read error response
                try (InputStream errorStream = connection.getErrorStream()) {
                    if (errorStream != null) {
                        BufferedReader errorReader = new BufferedReader(new InputStreamReader(errorStream, StandardCharsets.UTF_8));
                        StringBuilder errorResponse = new StringBuilder();
                        String line;
                        while ((line = errorReader.readLine()) != null) {
                            errorResponse.append(line);
                        }
                        Log.e(TAG, "Error response body: " + errorResponse.toString());
                    }
                } catch (Exception e) {
                    Log.e(TAG, "Error reading error stream", e);
                }
                return null;
            }

            Log.i(TAG, "HTTP request successful, reading response...");

            // Read the response
            Log.i(TAG, "Getting input stream...");
            try (InputStream inputStream = connection.getInputStream();
                 BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream, StandardCharsets.UTF_8))) {
                
                Log.i(TAG, "Input stream obtained, creating buffered reader...");
                
                StringBuilder response = new StringBuilder();
                String line;
                while ((line = reader.readLine()) != null) {
                    response.append(line);
                }
                
                String responseStr = response.toString();
                Log.i(TAG, "Response received, length: " + responseStr.length() + " characters");
                Log.i(TAG, "Response body: " + responseStr);
                Log.i(TAG, "Successfully completed request!");
                return responseStr;
            }

        } finally {
            connection.disconnect();
        }
    }

    /**
     * Handles the challenge response from the enrollment server.
     * Currently just logs the JSON response for testing purposes.
     * 
     * @param response The JSON response from the challenge endpoint
     */
    private static void handleChallengeResponse(String response) {
        try {
            Log.i(TAG, "=== ENROLLMENT SERVER RESPONSE ===");
            Log.i(TAG, "Raw JSON Response: " + response);
            Log.i(TAG, "Response length: " + (response != null ? response.length() : 0) + " characters");
            Log.i(TAG, "=== END RESPONSE ===");
            
            // Parse the JSON response manually (simple parsing for the specific format)
            String success = extractJsonValue(response, "status");
            String nonceId = extractJsonValue(response, "nonceId");
            String nonceBase64 = extractJsonValue(response, "nonceBase64");
            String expiresAt = extractJsonValue(response, "expiresAt");
            
            if (!"success".equals(success)) {
                Log.e(TAG, "Server returned success=false in challenge response");
                return;
            }
            
            if (nonceId == null || nonceBase64 == null) {
                Log.e(TAG, "Failed to parse nonce from response: missing nonceId or nonceBase64");
                return;
            }
            
            Log.i(TAG, "Challenge parsed successfully:");
            Log.i(TAG, "  NonceId: " + nonceId);
            Log.i(TAG, "  NonceBase64: " + nonceBase64);
            Log.i(TAG, "  Expires: " + expiresAt);
            
            // Generate hardware-backed key with attestation using the nonce
            Log.i(TAG, "Generating hardware-backed key with attestation...");
            
            // Decode the base64 nonce to bytes
            byte[] nonce;
            try {
                nonce = android.util.Base64.decode(nonceBase64, android.util.Base64.DEFAULT);
                Log.i(TAG, "Decoded nonce from base64, length: " + nonce.length + " bytes");
            } catch (Exception e) {
                Log.e(TAG, "Failed to decode base64 nonce: " + nonceBase64, e);
                return;
            }
            
            boolean keyGenSuccess = WootzHardwareKeyStore.generateKeyWithAttestation(nonce);
            
            if (keyGenSuccess) {
                Log.i(TAG, "Hardware key generation successful, retrieving certificate chain...");
                
                // Get the PEM certificate chain
                String pemChain = WootzHardwareKeyStore.getAttestationChainAsPem();
                
                if (pemChain != null && !pemChain.isEmpty()) {
                    Log.i(TAG, "Certificate chain retrieved successfully");
                    Log.i(TAG, "Certificate chain length: " + pemChain.length() + " characters");
                    
                    // Submit the enrollment with PEM certificate chain
                    submitEnrollmentWithPem(pemChain, nonceId);
                } else {
                    Log.e(TAG, "Failed to retrieve certificate chain after key generation");
                }
            } else {
                Log.e(TAG, "Hardware key generation failed");
            }
            
        } catch (Exception e) {
            Log.e(TAG, "Error handling challenge response", e);
        }
    }
    
    /**
     * Simple JSON value extraction for the specific response format.
     * This is a lightweight parser for our specific JSON structure.
     * 
     * @param json The JSON string to parse
     * @param key The key to extract
     * @return The extracted value or null if not found
     */
    private static String extractJsonValue(String json, String key) {
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
            int endIndex = json.indexOf('"', startIndex);
            if (endIndex != -1) {
                return json.substring(startIndex, endIndex);
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
     * Submits the device enrollment with the generated attestation certificate chain in PEM format.
     * This is called after successful key generation and attestation.
     * 
     * @param pemChain The PEM-formatted attestation certificate chain
     * @param nonceId The nonce ID from the challenge response
     */
    private static void submitEnrollmentWithPem(String pemChain, String nonceId) {
        new EnrollmentSubmitTask().execute(pemChain, nonceId);
    }

    /**
     * AsyncTask to handle the enrollment submission in the background.
     */
    private static class EnrollmentSubmitTask extends AsyncTask<Object, Void, Boolean> {
        @Override
        protected Boolean doInBackground(Object... params) {
            try {
                if (params.length >= 2 && params[0] instanceof String && params[1] instanceof String) {
                    // New PEM format submission
                    String pemChain = (String) params[0];
                    String nonceId = (String) params[1];
                    return submitEnrollmentRequestWithPem(pemChain, nonceId);
                } else {
                    Log.e(TAG, "Invalid parameters for enrollment submission");
                    return false;
                }
            } catch (Exception e) {
                Log.e(TAG, "Failed to submit enrollment", e);
                return false;
            }
        }

        @Override
        protected void onPostExecute(Boolean success) {
            if (success) {
                Log.i(TAG, "Device enrollment completed successfully");
            } else {
                Log.e(TAG, "Device enrollment submission failed");
            }
        }
    }
   
    /**
     * Makes HTTP POST request to submit the device enrollment with PEM certificate chain.
     * 
     * @param pemChain The PEM-formatted attestation certificate chain
     * @param nonceId The nonce ID from the challenge response
     * @return true if the submission was successful, false otherwise
     */
    private static boolean submitEnrollmentRequestWithPem(String pemChain, String nonceId) throws IOException {
        // Use the enrollment endpoint instead of submit
        URL url = new URL(ENROLLMENT_SUBMIT_URL);
        HttpURLConnection connection = (HttpURLConnection) url.openConnection();

        try {
            // Configure the connection
            connection.setRequestMethod("POST");
            connection.setRequestProperty("Content-Type", "application/json");
            connection.setRequestProperty("User-Agent", "Wootz-Browser/1.0");
            connection.setDoOutput(true);
            connection.setConnectTimeout(10000); // 10 seconds
            connection.setReadTimeout(30000);    // 30 seconds

            Log.i(TAG, "Submitting enrollment to: " + url.toString());
            Log.i(TAG, "Using nonceId: " + nonceId);
            Log.i(TAG, "PEM chain length: " + pemChain.length() + " characters");

            // Get device public key PEM (we need to extract this from the hardware key)
            String devicePublicKeyPem = WootzHardwareKeyStore.getPublicKeyAsPem();
            if (devicePublicKeyPem == null) {
                Log.e(TAG, "Failed to get device public key PEM");
                return false;
            }

            Log.i(TAG, "Device public key PEM length: " + devicePublicKeyPem.length() + " characters");

            // Create JSON request body matching the API specification
            String requestBody = String.format(
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
                pemChain.replace("\n", "\\n").replace("\r", ""), // Escape newlines for JSON
                devicePublicKeyPem.replace("\n", "\\n").replace("\r", ""), // Escape newlines for JSON
                android.os.Build.MANUFACTURER,
                android.os.Build.MODEL,
                android.os.Build.VERSION.RELEASE
            );

            Log.d(TAG, "Enrollment request body created, length: " + requestBody.length());

            // Send the request body
            try (OutputStream os = connection.getOutputStream()) {
                byte[] input = requestBody.getBytes(StandardCharsets.UTF_8);
                os.write(input, 0, input.length);
            }

            // Check response code
            int responseCode = connection.getResponseCode();
            Log.i(TAG, "Enrollment HTTP Response Code: " + responseCode);
            
            if (responseCode == HttpURLConnection.HTTP_OK || responseCode == HttpURLConnection.HTTP_CREATED) {
                Log.i(TAG, "Enrollment submission successful!");
                
                // Read the response
                try (InputStream inputStream = connection.getInputStream();
                     BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream, StandardCharsets.UTF_8))) {
                    
                    StringBuilder response = new StringBuilder();
                    String line;
                    while ((line = reader.readLine()) != null) {
                        response.append(line);
                    }
                    
                    Log.i(TAG, "=== ENROLLMENT SUCCESS RESPONSE ===");
                    Log.i(TAG, response.toString());
                    Log.i(TAG, "=== END SUCCESS RESPONSE ===");
                    // TODO: Parse response to store DIC (Device Identity Certificate) if provided
                }
                
                return true;
            } else {
                Log.e(TAG, "Enrollment submission failed with response code: " + responseCode);
                
                // Read error response
                try (InputStream errorStream = connection.getErrorStream();
                     BufferedReader reader = new BufferedReader(new InputStreamReader(errorStream, StandardCharsets.UTF_8))) {
                    
                    StringBuilder errorResponse = new StringBuilder();
                    String line;
                    while ((line = reader.readLine()) != null) {
                        errorResponse.append(line);
                    }
                    
                    Log.e(TAG, "=== ENROLLMENT ERROR RESPONSE ===");
                    Log.e(TAG, errorResponse.toString());
                    Log.e(TAG, "=== END ERROR RESPONSE ===");
                } catch (Exception e) {
                    Log.e(TAG, "Error reading error response", e);
                }
                
                return false;
            }

        } finally {
            connection.disconnect();
        }
    }
}
