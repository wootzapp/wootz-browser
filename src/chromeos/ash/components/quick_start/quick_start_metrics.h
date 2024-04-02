// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_ASH_COMPONENTS_QUICK_START_QUICK_START_METRICS_H_
#define CHROMEOS_ASH_COMPONENTS_QUICK_START_QUICK_START_METRICS_H_

#include <optional>

#include "base/time/time.h"
#include "base/timer/elapsed_timer.h"
#include "chromeos/ash/components/quick_start/quick_start_response_type.h"

class GoogleServiceAuthError;

namespace ash::quick_start {

class QuickStartMetrics {
 public:
  // This enum is tied directly to a UMA enum defined in
  // //tools/metrics/histograms/metadata/quickstart/enums.xml, and should always
  // reflect it (do not change one without changing the other). Entries should
  // be never modified or deleted. Only additions possible.
  enum class ScreenName {
    kOther = 0,  // We don't expect this value to ever be emitted.
    kNone = 1,  // There is no previous screen when automatically resuming after
                // an update.
    kWelcomeScreen = 2,  // Quick Start entry point 1.
    kNetworkScreen = 3,  // Quick Start entry point 2, or in the middle of Quick
                         // Start when the host device is not connected to wifi.
    kGaiaScreen = 4,     // Quick Start entry point 3.
    kSetUpWithAndroidPhone = 5,  // Beginning of Quick Start flow.
    kConnectingToWifi = 6,       // Transferring wifi with Quick Start.
    kCheckingForUpdateAndDeterminingDeviceConfiguration = 7,
    kChooseChromebookSetup = 8,
    kInstallingLatestUpdate = 9,
    kResumingConnectionAfterUpdate = 10,
    kGettingGoogleAccountInfo = 11,
    kQuickStartComplete = 12,
    kSetupDevicePIN = 13,          // After Quick Start flow is complete.
    kAskForParentPermission = 14,  // Only for Unicorn accounts.
    kReviewPrivacyAndTerms = 15,   // Only for Unicorn accounts.
    kUnifiedSetup = 16,  // After Quick Start flow is complete, connect host
                         // phone to account.
    kMaxValue = kUnifiedSetup
  };

  enum class ExitReason {
    kAdvancedInFlow,
    kUserCancelled,
  };

  // This enum is tied directly to a UMA enum defined in
  // //tools/metrics/histograms/metadata/quickstart/enums.xml, and should always
  // reflect it (do not change one without changing the other). Entries should
  // be never modified or deleted. Only additions possible.
  enum class AdvertisingMethod {
    kQrCode = 0,
    kPin = 1,
    kMaxValue = kPin,
  };

  // This enum is tied directly to a UMA enum defined in
  // //tools/metrics/histograms/metadata/quickstart/enums.xml and should always
  // reflect it. The UMA enum cannot use
  // |device::BluetoothAdvertisement::ErrorCode| directly, because it is missing
  // the required |kMaxValue| field.
  enum class FastPairAdvertisingErrorCode {
    kUnsupportedPlatform = 0,
    kAdvertisementAlreadyExists = 1,
    kAdvertisementDoesNotExist = 2,
    kAdvertisementInvalidLength = 3,
    kStartingAdvertisement = 4,
    kResetAdvertising = 5,
    kAdapterPoweredOff = 6,
    kInvalidAdvertisementInterval = 7,
    kInvalidAdvertisementErrorCode = 8,
    kMaxValue = kInvalidAdvertisementErrorCode,
  };

  enum class NearbyConnectionsAdvertisingErrorCode {
    kFailedToStart,
  };

  // This enum is tied directly to a UMA enum defined in
  // //tools/metrics/histograms/metadata/quickstart/enums.xml, and should always
  // reflect it (do not change one without changing the other). Entries should
  // be never modified or deleted. Only additions possible.
  enum class HandshakeErrorCode {
    kFailedToReadResponse = 0,
    kFailedToParse = 1,
    kFailedToDecryptAuthPayload = 2,
    kFailedToParseAuthPayload = 3,
    kUnexpectedAuthPayloadRole = 4,
    kUnexpectedAuthPayloadAuthToken = 5,
    kInvalidHandshakeErrorCode = 6,
    kMaxValue = kInvalidHandshakeErrorCode,
  };

  // This enum is tied directly to a UMA enum defined in
  // //tools/metrics/histograms/metadata/quickstart/enums.xml, and should always
  // reflect it (do not change one without changing the other). Entries should
  // be never modified or deleted. Only additions possible.
  enum class MessageType {
    kWifiCredentials = 0,
    kBootstrapConfigurations = 1,
    kHandshake = 2,
    kNotifySourceOfUpdate = 3,
    kGetInfo = 4,
    kAssertion = 5,
    kBootstrapStateCancel = 6,
    kBootstrapStateComplete = 7,
    kMaxValue = kBootstrapStateComplete,
  };

  // This enum is tied directly to a UMA enum defined in
  // //tools/metrics/histograms/metadata/quickstart/enums.xml, and should always
  // reflect it (do not change one without changing the other). Entries should
  // be never modified or deleted. Only additions possible.
  enum class MessageReceivedErrorCode {
    kTimeOut = 0,
    kDeserializationFailure = 1,
    kUnknownError = 2,
    kMaxValue = kUnknownError,
  };

  // This enum is tied directly to a UMA enum defined in
  // //tools/metrics/histograms/metadata/quickstart/enums.xml, and should always
  // reflect it (do not change one without changing the other). Entries should
  // be never modified or deleted. Only additions possible.
  enum class AttestationCertificateRequestErrorCode {
    kUnknownError = 0,
    kBadRequest = 1,
    kAttestationNotSupportedOnDevice = 2,
    kMaxValue = kAttestationNotSupportedOnDevice,
  };

  // This enum is tied directly to a UMA enum defined in
  // //tools/metrics/histograms/metadata/quickstart/enums.xml, and should always
  // reflect it (do not change one without changing the other). Entries should
  // be never modified or deleted. Only additions possible.
  enum class GaiaAuthenticationResult {
    kUnknownError = 0,
    kSuccess = 1,
    kResponseParsingError = 2,
    kRejection = 3,
    kAdditionalChallengesOnSource = 4,
    kAdditionalChallengesOnTarget = 5,
    kMaxValue = kAdditionalChallengesOnTarget,
  };

  // This enum is tied directly to a UMA enum defined in
  // //tools/metrics/histograms/metadata/quickstart/enums.xml, and should always
  // reflect it (do not change one without changing the other). Entries should
  // be never modified or deleted. Only additions possible.
  enum class WifiTransferResultFailureReason {
    kConnectionDroppedDuringAttempt = 0,
    kEmptyResponseBytes = 1,
    kUnableToReadAsJSON = 2,
    kWifiNetworkInformationNotFound = 3,
    kSsidNotFound = 4,
    kEmptySsid = 5,
    kSecurityTypeNotFound = 6,
    kInvalidSecurityType = 7,
    kPasswordFoundAndOpenNetwork = 8,
    kPasswordNotFoundAndNotOpenNetwork = 9,
    kWifiHideStatusNotFound = 10,
    kMaxValue = kWifiHideStatusNotFound,
  };

  // This enum is tied directly to a UMA enum defined in
  // //tools/metrics/histograms/metadata/quickstart/enums.xml, and should always
  // reflect it (do not change one without changing the other). Entries should
  // be never modified or deleted. Only additions possible.
  enum class GaiaTransferResultFailureReason {
    kNoAccountsReceivedFromPhone = 0,
    kIneligibleAccount = 1,
    kFailedToSignIn = 2,
    kEmptyResponseBytes = 3,
    kUnableToReadAsJSON = 4,
    kUnexpectedResponseSize = 5,
    kUnsuccessfulCtapDeviceResponseStatus = 6,
    kCborDecodingError = 7,
    kInvalidCborDecodedValuesMap = 8,
    kEmptyCredentialId = 9,
    kEmptyAuthData = 10,
    kEmptySignature = 11,
    kEmptyEmail = 12,
    kMaxValue = kEmptyEmail,
  };

  enum class EntryPoint {
    kWelcome,
    kWifi,
    kGaia,
  };

  // Helper function that returns the MessageType equivalent of
  // QuickStartResponseType.
  static MessageType MapResponseToMessageType(
      QuickStartResponseType response_type);

  static void RecordScreenOpened(ScreenName screen);

  static void RecordScreenClosed(ScreenName screen,
                                 int32_t session_id,
                                 base::Time timestamp,
                                 std::optional<ScreenName> previous_screen);

  static void RecordWifiTransferResult(
      bool succeeded,
      std::optional<WifiTransferResultFailureReason> failure_reason);

  static void RecordGaiaTransferAttempted(bool attempted);

  static void RecordCapturePortalEncountered(int32_t session_id);

  static void RecordRedirectToEnterpriseEnrollment(int32_t session_id);

  static void RecordForcedUpdateRequired(int32_t session_id);

  static void RecordGaiaTransferResult(
      bool succeeded,
      std::optional<GaiaTransferResultFailureReason> failure_reason);

  static void RecordEntryPoint(EntryPoint entry_point);

  QuickStartMetrics();
  QuickStartMetrics(const QuickStartMetrics&) = delete;
  const QuickStartMetrics& operator=(const QuickStartMetrics&) = delete;
  virtual ~QuickStartMetrics();

  // Records the start of an attempt to fetch challenge bytes from Gaia.
  // Challenge bytes are later used to generate a Remote Attestation certificate
  // and a FIDO assertion.
  void RecordChallengeBytesRequested();

  // Records the end of an attempt to fetch challenge bytes from Gaia.
  // `status` is the overall status of the fetch. It is set to
  // `GoogleServiceAuthError::State::NONE` if the request was successful.
  void RecordChallengeBytesRequestEnded(const GoogleServiceAuthError& status);

  void RecordAttestationCertificateRequested();

  // Records the end of a Remote Attestation certificate request. `error_code`
  // is empty if the request was successful - otherwise it contains the details
  // of the error.
  void RecordAttestationCertificateRequestEnded(
      std::optional<AttestationCertificateRequestErrorCode> error_code);

  void RecordGaiaAuthenticationStarted();

  void RecordGaiaAuthenticationRequestEnded(
      const GaiaAuthenticationResult& result);

  void RecordFastPairAdvertisementStarted(AdvertisingMethod advertising_method);

  void RecordFastPairAdvertisementEnded(
      bool succeeded,
      std::optional<FastPairAdvertisingErrorCode> error_code);

  void RecordNearbyConnectionsAdvertisementStarted(
      int32_t session_id,
      AdvertisingMethod advertising_method);

  void RecordNearbyConnectionsAdvertisementEnded(
      bool succeeded,
      std::optional<NearbyConnectionsAdvertisingErrorCode> error_code);

  // TODO(b/308200138): Change the wording here to make this less confusing.
  void RecordHandshakeStarted(bool handshake_started);

  void RecordHandshakeResult(bool succeeded,
                             std::optional<HandshakeErrorCode> error_code);

  void RecordMessageSent(MessageType message_type);

  void RecordMessageReceived(
      MessageType desired_message_type,
      bool succeeded,
      std::optional<MessageReceivedErrorCode> error_code);

 private:
  // Timer to keep track of Fast Pair advertising duration. Should be
  // constructed when advertising starts and destroyed when advertising
  // finishes.
  std::unique_ptr<base::ElapsedTimer> fast_pair_advertising_timer_;
  std::optional<AdvertisingMethod> fast_pair_advertising_method_;

  // Timer to keep track of handshake duration. Should be constructed when
  // the handshake starts and destroyed when the handshake finishes.
  std::unique_ptr<base::ElapsedTimer> handshake_elapsed_timer_;

  // Timer to keep track of the duration of request/response pairs. Should be
  // constructed when the request is sent and destroyed when the response is
  // received.
  std::unique_ptr<base::ElapsedTimer> message_elapsed_timer_;

  // Timer to keep track of challenge bytes fetch requests. It should be set at
  // the start of a challenge bytes fetch and destroyed when a response is
  // received.
  std::unique_ptr<base::ElapsedTimer> challenge_bytes_fetch_timer_;

  // Timer to keep track of remote attestation certificate fetch requests. It
  // should be set at the start of a certificate fetch and destroyed when a
  // response is received.
  std::unique_ptr<base::ElapsedTimer> attestation_certificate_timer_;

  // Timer to keep track of Gaia authentication requests. It should be set at
  // the start of a Gaia authentication request and destroyed when a response is
  // received.
  std::unique_ptr<base::ElapsedTimer> gaia_authentication_timer_;
};

}  // namespace ash::quick_start

#endif  // CHROMEOS_ASH_COMPONENTS_QUICK_START_QUICK_START_METRICS_H_
