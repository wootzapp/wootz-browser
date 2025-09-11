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
        URL url = new URL(ENROLLMENT_CHALLENGE_URL);
        HttpURLConnection connection = (HttpURLConnection) url.openConnection();

        try {
            // Configure the connection
            connection.setRequestMethod("POST");
            connection.setRequestProperty("Content-Type", "application/json");
            connection.setRequestProperty("User-Agent", "Wootz-Browser/1.0");
            connection.setConnectTimeout(10000); // 10 seconds
            connection.setReadTimeout(30000);    // 30 seconds
            
            // Check response code
            int responseCode = connection.getResponseCode();
            
            if (responseCode != HttpURLConnection.HTTP_OK) {
                Log.e(TAG, "Enrollment challenge request failed: " + responseCode);
                return null;
            }

            // Read the response
            try (InputStream inputStream = connection.getInputStream();
                 BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream, StandardCharsets.UTF_8))) {
                
                StringBuilder response = new StringBuilder();
                String line;
                while ((line = reader.readLine()) != null) {
                    response.append(line);
                }
                
                return response.toString();
            }

        } finally {
            connection.disconnect();
        }
    }

    /**
     * Handles the challenge response from the enrollment server.
     * Parses the nonce and initiates hardware key generation with attestation.
     * 
     * @param response The JSON response from the challenge endpoint
     */
    private static void handleChallengeResponse(String response) {
        try {
            // Parse the JSON response
            String success = WootzEnrollmentUtils.extractJsonValue(response, "status");
            String nonceId = WootzEnrollmentUtils.extractJsonValue(response, "nonceId");
            String nonceBase64 = WootzEnrollmentUtils.extractJsonValue(response, "nonceBase64");
            
            if (!"success".equals(success)) {
                Log.e(TAG, "Server returned failure in challenge response");
                return;
            }
            
            if (nonceId == null || nonceBase64 == null) {
                Log.e(TAG, "Missing nonce data in response");
                return;
            }
            
            // Decode the base64 nonce to bytes
            byte[] nonce;
            try {
                nonce = android.util.Base64.decode(nonceBase64, android.util.Base64.DEFAULT);
            } catch (Exception e) {
                Log.e(TAG, "Failed to decode nonce", e);
                return;
            }
            
            // Generate hardware-backed key with attestation
            boolean keyGenSuccess = WootzHardwareKeyStore.generateKeyWithAttestation(nonce);
            
            if (keyGenSuccess) {
                // Get the PEM certificate chain
                String pemChain = WootzHardwareKeyStore.getAttestationChainAsPem();
                
                if (pemChain != null && !pemChain.isEmpty()) {
                    // Submit the enrollment with PEM certificate chain
                    submitEnrollmentWithPem(pemChain, nonceId);
                } else {
                    Log.e(TAG, "Failed to retrieve certificate chain");
                }
            } else {
                Log.e(TAG, "Hardware key generation failed");
            }
            
        } catch (Exception e) {
            Log.e(TAG, "Error handling challenge response", e);
        }
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

            // Get device public key PEM
            String devicePublicKeyPem = WootzHardwareKeyStore.getPublicKeyAsPem();
            if (devicePublicKeyPem == null) {
                Log.e(TAG, "Failed to get device public key PEM");
                return false;
            }

            // Create JSON request body using utility method
            String requestBody = WootzEnrollmentUtils.createEnrollmentRequestJson(
                nonceId, pemChain, devicePublicKeyPem,
                android.os.Build.MANUFACTURER,
                android.os.Build.MODEL,
                android.os.Build.VERSION.RELEASE
            );

            // Send the request body
            try (OutputStream os = connection.getOutputStream()) {
                byte[] input = requestBody.getBytes(StandardCharsets.UTF_8);
                os.write(input, 0, input.length);
            }

            // Check response code
            int responseCode = connection.getResponseCode();
            
            if (responseCode == HttpURLConnection.HTTP_OK || responseCode == HttpURLConnection.HTTP_CREATED) {
                // Read the response
                try (InputStream inputStream = connection.getInputStream();
                     BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream, StandardCharsets.UTF_8))) {
                    
                    StringBuilder response = new StringBuilder();
                    String line;
                    while ((line = reader.readLine()) != null) {
                        response.append(line);
                    }
                    
                    // Parse and store DIC (Device Identity Certificate) from response
                    handleEnrollmentSuccessResponse(response.toString());
                }
                
                return true;
            } else {
                Log.e(TAG, "Enrollment submission failed: " + responseCode);
                return false;
            }

        } finally {
            connection.disconnect();
        }
    }
    
    /**
     * Handles the successful enrollment response by parsing and storing the DIC.
     * 
     * @param response The JSON response from the enrollment server
     */
    private static void handleEnrollmentSuccessResponse(String response) {
        try {
            // Validate enrollment response
            if (!WootzEnrollmentUtils.isEnrollmentResponseSuccessful(response)) {
                Log.e(TAG, "Enrollment response indicates failure");
                return;
            }
            
            if (!WootzEnrollmentUtils.hasRequiredDicData(response)) {
                Log.e(TAG, "Missing required DIC data in enrollment response");
                return;
            }
            
            // Extract values from JSON response
            String deviceId = WootzEnrollmentUtils.extractJsonValue(response, "deviceId");
            String dicCertificate = WootzEnrollmentUtils.extractJsonValue(response, "dicCertificate");
            String dicPrivateKey = WootzEnrollmentUtils.extractJsonValue(response, "dicPrivateKey");
            String expiresAt = WootzEnrollmentUtils.extractJsonValue(response, "expiresAt");
            String issuedAt = WootzEnrollmentUtils.extractJsonValue(response, "issuedAt");
            String stepCaUrl = WootzEnrollmentUtils.extractJsonValue(response, "stepCaUrl");
            
            // Store the DIC certificate and associate it with the hardware key
            boolean dicStored = WootzHardwareKeyStore.storeDicCertificate(
                deviceId, dicCertificate, dicPrivateKey, expiresAt, issuedAt, stepCaUrl);
            
            if (!dicStored) {
                Log.e(TAG, "Failed to store DIC certificate");
            }
            
        } catch (Exception e) {
            Log.e(TAG, "Error handling enrollment success response", e);
        }
    }
}
