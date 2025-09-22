// Copyright 2024 The Wootz Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.net;

import android.os.Handler;
import android.os.Looper;
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
import java.net.URLConnection;
import java.nio.charset.StandardCharsets;

/**
 * Java implementation of device enrollment HTTP client.
 * This handles communication with the enrollment server to obtain
 * challenge nonces and submit device attestations.
 */
@JNINamespace("net::android")
public class WootzDeviceEnrollment {
    private static final String TAG = "WootzDeviceEnrollment";
    private static final String NONCE_URL = "PROD_NONCE_URL";
    private static final String ENROLLMENT_URL = "PROD_ENROLLMENT_URL";
    private static final String BEARER_TOKEN = "PROD_BEARER_TOKEN";

    /**
     * Starts the device enrollment process by requesting a nonce from the server.
     * This method runs asynchronously on a dedicated thread to avoid AsyncTask issues.
     */
    public static void startDeviceEnrollment() {
        Log.e(TAG, "Starting device enrollment process");
        Thread networkThread = new Thread(() -> {
            try {
                String response = requestNonce();
                // Handle response on main thread
                new Handler(Looper.getMainLooper()).post(() -> {
                    if (response != null) {
                        Log.e(TAG, "Received nonce response successfully");
                        handleNonceResponse(response);
                    } else {
                        Log.e(TAG, "Failed to get nonce response");
                    }
                });
            } catch (Exception e) {
                Log.e(TAG, "Failed to request nonce: " + e.getClass().getSimpleName() + ": " + e.getMessage(), e);
                if (e.getCause() != null) {
                    Log.e(TAG, "Caused by: " + e.getCause().getClass().getSimpleName() + ": " + e.getCause().getMessage());
                }
            }
        });
        networkThread.setName("WootzEnrollmentNonce");
        networkThread.start();
    }


    /**
     * Makes HTTP GET request to the nonce endpoint with Bearer token authentication.
     * Uses Chromium's networking stack for better reliability and consistency.
     * 
     * @return The response body as a string, or null if the request failed
     */
    private static String requestNonce() throws IOException {
        URL url = new URL(NONCE_URL);
        
        // Create traffic annotation for privacy auditing
        NetworkTrafficAnnotationTag trafficAnnotation = NetworkTrafficAnnotationTag.createComplete(
            "wootz_device_enrollment_nonce",
            "semantics {\n" +
            "  sender: \"Wootz Device Enrollment\"\n" +
            "  description: \"Request nonce from enrollment server for device attestation\"\n" +
            "  trigger: \"User device enrollment process\"\n" +
            "  data: \"Bearer token for authentication\"\n" +
            "  destination: WEBSITE\n" +
            "}\n" +
            "policy {\n" +
            "  cookies_allowed: NO\n" +
            "  setting: \"This feature can be controlled by enterprise policy\"\n" +
            "}");
        
        // Use Chromium's networking stack instead of direct HttpURLConnection
        URLConnection urlConnection = ChromiumNetworkAdapter.openConnection(url, trafficAnnotation);
        HttpURLConnection connection = (HttpURLConnection) urlConnection;

        try {
            // Configure the connection
            connection.setRequestMethod("GET");
            connection.setRequestProperty("Authorization", "Bearer " + BEARER_TOKEN);
            connection.setRequestProperty("User-Agent", "Wootz-Browser/1.0");
            connection.setRequestProperty("Connection", "close"); // Force fresh connections
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
            // Parse the JSON response - expecting {"nonce": "hex-encoded-nonce"}
            String nonceHex = WootzEnrollmentUtils.extractJsonValue(response, "nonce");
            
            if (nonceHex == null || nonceHex.isEmpty()) {
                Log.e(TAG, "Missing nonce in response");
                return;
            }
            
            // Convert hex string to bytes
            byte[] nonce;
            try {
                nonce = hexStringToBytes(nonceHex);
            } catch (Exception e) {
                Log.e(TAG, "Failed to decode hex nonce", e);
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
                    submitEnrollmentWithCSR(csr, nonceHex, attestationChain);
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
     * Uses a dedicated thread to avoid AsyncTask threading issues.
     * 
     * @param csr The PEM-formatted Certificate Signing Request
     * @param nonce The base64-encoded nonce from the server
     * @param attestationChain The PEM-formatted attestation certificate chain
     */
    private static void submitEnrollmentWithCSR(String csr, String nonce, String attestationChain) {
        Thread networkThread = new Thread(() -> {
            try {
                boolean success = submitEnrollmentRequestWithCSR(csr, nonce, attestationChain);
                // Handle result on main thread
                new Handler(Looper.getMainLooper()).post(() -> {
                    if (success) {
                        Log.e(TAG, "Device enrollment completed successfully");
                    } else {
                        Log.e(TAG, "Device enrollment submission failed");
                    }
                });
            } catch (Exception e) {
                Log.e(TAG, "Failed to submit enrollment", e);
            }
        });
        networkThread.setName("WootzEnrollmentSubmit");
        networkThread.start();
    }

   
    /**
     * Makes HTTP POST request to submit the device enrollment with CSR.
     * Uses Chromium's networking stack for better reliability and consistency.
     * 
     * @param csr The PEM-formatted Certificate Signing Request
     * @param nonce The base64-encoded nonce from the server
     * @param attestationChain The PEM-formatted attestation certificate chain
     * @return true if the submission was successful, false otherwise
     */
    private static boolean submitEnrollmentRequestWithCSR(String csr, String nonce, String attestationChain) throws IOException {
        URL url = new URL(ENROLLMENT_URL);
        
        // Create traffic annotation for privacy auditing
        NetworkTrafficAnnotationTag trafficAnnotation = NetworkTrafficAnnotationTag.createComplete(
            "wootz_device_enrollment_submit",
            "semantics {\n" +
            "  sender: \"Wootz Device Enrollment\"\n" +
            "  description: \"Submit device enrollment with CSR and attestation chain\"\n" +
            "  trigger: \"Device enrollment process after successful nonce retrieval\"\n" +
            "  data: \"CSR, attestation chain, and bearer token\"\n" +
            "  destination: WEBSITE\n" +
            "}\n" +
            "policy {\n" +
            "  cookies_allowed: NO\n" +
            "  setting: \"This feature can be controlled by enterprise policy\"\n" +
            "}");
        
        // Use Chromium's networking stack instead of direct HttpURLConnection
        URLConnection urlConnection = ChromiumNetworkAdapter.openConnection(url, trafficAnnotation);
        HttpURLConnection connection = (HttpURLConnection) urlConnection;

        try {
            // Configure the connection
            connection.setRequestMethod("POST");
            connection.setRequestProperty("Content-Type", "application/json");
            connection.setRequestProperty("Authorization", "Bearer " + BEARER_TOKEN);
            connection.setRequestProperty("User-Agent", "Wootz-Browser/1.0");
            connection.setRequestProperty("Connection", "close"); // Force fresh connections
            connection.setDoOutput(true);
            connection.setConnectTimeout(10000); // 10 seconds
            connection.setReadTimeout(30000);    // 30 seconds

            // Create JSON request body for CSR-based enrollment
            String requestBody = WootzEnrollmentUtils.createCSREnrollmentRequestJson(
                csr, nonce, attestationChain);
            
            // Log basic enrollment info without exposing sensitive data
            Log.e(TAG, "Sending CSR enrollment request to API");

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
            Log.e(TAG, "Enrollment response received successfully");
            
            // Extract the certificate from the simplified JSON response
            String certificate = WootzEnrollmentUtils.extractJsonValue(response, "certificate");
            
            if (certificate == null || certificate.isEmpty()) {
                Log.e(TAG, "Missing certificate in enrollment response");
                return;
            }
            
            // Use existing storeDicCertificate method with available parameters
            String currentTimestamp = String.valueOf(System.currentTimeMillis());
            
            // Store the DIC certificate using the existing method
            boolean dicStored = WootzHardwareKeyStore.storeDicCertificate(
                null, certificate, null, currentTimestamp, null);
            
            if (dicStored) {
                Log.e(TAG, "Successfully stored DIC certificate");
            } else {
                Log.e(TAG, "Failed to store DIC certificate");
            }
            
        } catch (Exception e) {
            Log.e(TAG, "Error handling enrollment success response", e);
        }
    }
    
    /**
     * Convert hex string to byte array.
     * 
     * @param hexString The hex string to convert
     * @return The byte array
     */
    private static byte[] hexStringToBytes(String hexString) {
        if (hexString == null || hexString.length() % 2 != 0) {
            throw new IllegalArgumentException("Invalid hex string: " + hexString);
        }
        
        byte[] bytes = new byte[hexString.length() / 2];
        for (int i = 0; i < hexString.length(); i += 2) {
            bytes[i / 2] = (byte) ((Character.digit(hexString.charAt(i), 16) << 4) +
                                   Character.digit(hexString.charAt(i + 1), 16));
        }
        return bytes;
    }
}
