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
    private static final String NONCE_URL = "https://rfxzqjgv-3000.inc1.devtunnels.ms/nounce";
    private static final String ENROLLMENT_URL = "https://rfxzqjgv-3000.inc1.devtunnels.ms/enroll";
    private static final String BEARER_TOKEN = "Aoi3dkgpE905nvSiec";

    /**
     * Starts the device enrollment process by requesting a nonce from the server.
     * This method runs asynchronously and can be called from Java code.
     */
    public static void startDeviceEnrollment() {
        Log.i(TAG, "Starting device enrollment process");
        new EnrollmentNonceTask().execute();
    }

    /**
     * AsyncTask to handle the nonce request in the background.
     */
    private static class EnrollmentNonceTask extends AsyncTask<Void, Void, String> {
        @Override
        protected String doInBackground(Void... voids) {
            try {
                return requestNonce();
            } catch (Exception e) {
                Log.e(TAG, "Failed to request nonce: " + e.getClass().getSimpleName() + ": " + e.getMessage(), e);
                if (e.getCause() != null) {
                    Log.e(TAG, "Caused by: " + e.getCause().getClass().getSimpleName() + ": " + e.getCause().getMessage());
                }
                return null;
            }
        }

        @Override
        protected void onPostExecute(String response) {
            if (response != null) {
                Log.i(TAG, "Received nonce response: " + response);
                handleNonceResponse(response);
            } else {
                Log.e(TAG, "Failed to get nonce response");
            }
        }
    }

    /**
     * Makes HTTP GET request to the nonce endpoint with Bearer token authentication.
     * 
     * @return The response body as a string, or null if the request failed
     */
    private static String requestNonce() throws IOException {
        URL url = new URL(NONCE_URL);
        HttpURLConnection connection = (HttpURLConnection) url.openConnection();

        try {
            // Configure the connection
            connection.setRequestMethod("GET");
            connection.setRequestProperty("Authorization", "Bearer " + BEARER_TOKEN);
            connection.setRequestProperty("User-Agent", "Wootz-Browser/1.0");
            connection.setConnectTimeout(10000); // 10 seconds
            connection.setReadTimeout(30000);    // 30 seconds
            
            // Check response code
            int responseCode = connection.getResponseCode();
            
            if (responseCode != HttpURLConnection.HTTP_OK) {
                Log.e(TAG, "Nonce request failed: " + responseCode);
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
     * Handles the nonce response from the enrollment server.
     * Parses the nonce and initiates hardware key generation with attestation.
     * 
     * @param response The JSON response from the nonce endpoint
     */
    private static void handleNonceResponse(String response) {
        try {
            // Parse the JSON response - expecting {"nonce": "base64-encoded-nonce"}
            String nonceBase64 = WootzEnrollmentUtils.extractJsonValue(response, "nonce");
            
            if (nonceBase64 == null || nonceBase64.isEmpty()) {
                Log.e(TAG, "Missing nonce in response");
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
            
            // Generate hardware-backed key with attestation using the nonce
            boolean keyGenSuccess = WootzHardwareKeyStore.generateKeyWithAttestation(nonce);
            
            if (keyGenSuccess) {
                // Generate CSR using the hardware key
                String csr = WootzHardwareKeyStore.generateCSR();
                
                // Get the attestation certificate chain
                String attestationChain = WootzHardwareKeyStore.getAttestationChainAsPem();
                
                if (csr != null && !csr.isEmpty() && attestationChain != null && !attestationChain.isEmpty()) {
                    // Submit the enrollment with CSR
                    submitEnrollmentWithCSR(csr, nonceBase64, attestationChain);
                } else {
                    Log.e(TAG, "Failed to generate CSR or retrieve attestation chain");
                }
            } else {
                Log.e(TAG, "Hardware key generation failed");
            }
            
        } catch (Exception e) {
            Log.e(TAG, "Error handling nonce response", e);
        }
    }
    

    /**
     * Submits the device enrollment with CSR and attestation chain.
     * This is called after successful key generation and CSR creation.
     * 
     * @param csr The PEM-formatted Certificate Signing Request
     * @param nonce The base64-encoded nonce from the server
     * @param attestationChain The PEM-formatted attestation certificate chain
     */
    private static void submitEnrollmentWithCSR(String csr, String nonce, String attestationChain) {
        new EnrollmentSubmitTask().execute(csr, nonce, attestationChain);
    }

    /**
     * AsyncTask to handle the enrollment submission in the background.
     */
    private static class EnrollmentSubmitTask extends AsyncTask<Object, Void, Boolean> {
        @Override
        protected Boolean doInBackground(Object... params) {
            try {
                if (params.length >= 3 && params[0] instanceof String && 
                    params[1] instanceof String && params[2] instanceof String) {
                    // CSR-based enrollment submission
                    String csr = (String) params[0];
                    String nonce = (String) params[1];
                    String attestationChain = (String) params[2];
                    return submitEnrollmentRequestWithCSR(csr, nonce, attestationChain);
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
     * Makes HTTP POST request to submit the device enrollment with CSR.
     * 
     * @param csr The PEM-formatted Certificate Signing Request
     * @param nonce The base64-encoded nonce from the server
     * @param attestationChain The PEM-formatted attestation certificate chain
     * @return true if the submission was successful, false otherwise
     */
    private static boolean submitEnrollmentRequestWithCSR(String csr, String nonce, String attestationChain) throws IOException {
        URL url = new URL(ENROLLMENT_URL);
        HttpURLConnection connection = (HttpURLConnection) url.openConnection();

        try {
            // Configure the connection
            connection.setRequestMethod("POST");
            connection.setRequestProperty("Content-Type", "application/json");
            connection.setRequestProperty("Authorization", "Bearer " + BEARER_TOKEN);
            connection.setRequestProperty("User-Agent", "Wootz-Browser/1.0");
            connection.setDoOutput(true);
            connection.setConnectTimeout(10000); // 10 seconds
            connection.setReadTimeout(30000);    // 30 seconds

            // Create JSON request body for CSR-based enrollment
            String requestBody = WootzEnrollmentUtils.createCSREnrollmentRequestJson(
                csr, nonce, attestationChain);

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
            Log.e(TAG, "Enrollment response: " + response);
            
            // Extract values from JSON response (no private key needed)
            String deviceId = WootzEnrollmentUtils.extractJsonValue(response, "deviceId");
            String dicCertificate = WootzEnrollmentUtils.extractJsonValue(response, "dicCertificate");
            String expiresAt = WootzEnrollmentUtils.extractJsonValue(response, "expiresAt");
            String issuedAt = WootzEnrollmentUtils.extractJsonValue(response, "issuedAt");
            String stepCaUrl = WootzEnrollmentUtils.extractJsonValue(response, "stepCaUrl");
            
            // Store the DIC certificate and associate it with the hardware key
            boolean dicStored = WootzHardwareKeyStore.storeDicCertificate(
                deviceId, dicCertificate, expiresAt, issuedAt, stepCaUrl);
            
            if (!dicStored) {
                Log.e(TAG, "Failed to store DIC certificate");
            }
            
        } catch (Exception e) {
            Log.e(TAG, "Error handling enrollment success response", e);
        }
    }
}
