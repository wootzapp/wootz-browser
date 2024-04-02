// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_ASH_COMPONENTS_TETHER_TETHER_AVAILABILITY_OPERATION_H_
#define CHROMEOS_ASH_COMPONENTS_TETHER_TETHER_AVAILABILITY_OPERATION_H_

#include <map>
#include <vector>

#include "base/gtest_prod_util.h"
#include "base/memory/raw_ptr.h"
#include "base/observer_list.h"
#include "base/time/clock.h"
#include "base/time/time.h"
#include "chromeos/ash/components/multidevice/remote_device_ref.h"
#include "chromeos/ash/components/tether/message_transfer_operation.h"
#include "chromeos/ash/components/tether/scanned_device_info.h"

namespace ash::device_sync {
class DeviceSyncClient;
}

namespace ash::secure_channel {
class SecureChannelClient;
}

namespace ash::tether {

class ConnectionPreserver;
class HostScanDevicePrioritizer;
class MessageWrapper;
class TetherHostResponseRecorder;

// Operation used to perform a host scan. Attempts to connect to each of the
// devices passed and sends a TetherAvailabilityRequest to each connected device
// once an authenticated channel has been established; once a response has been
// received, TetherAvailabilityOperation alerts observers of devices which can provide
// a tethering connection.
class TetherAvailabilityOperation : public MessageTransferOperation {
 public:
  class Factory {
   public:
    static std::unique_ptr<TetherAvailabilityOperation> Create(
        const multidevice::RemoteDeviceRefList& devices_to_connect,
        device_sync::DeviceSyncClient* device_sync_client,
        secure_channel::SecureChannelClient* secure_channel_client,
        HostScanDevicePrioritizer* host_scan_device_prioritizer,
        TetherHostResponseRecorder* tether_host_response_recorder,
        ConnectionPreserver* connection_preserver);

    static void SetFactoryForTesting(Factory* factory);

   protected:
    virtual ~Factory();
    virtual std::unique_ptr<TetherAvailabilityOperation> CreateInstance(
        const multidevice::RemoteDeviceRefList& devices_to_connect,
        device_sync::DeviceSyncClient* device_sync_client,
        secure_channel::SecureChannelClient* secure_channel_client,
        HostScanDevicePrioritizer* host_scan_device_prioritizer,
        TetherHostResponseRecorder* tether_host_response_recorder,
        ConnectionPreserver* connection_preserver) = 0;

   private:
    static Factory* factory_instance_;
  };

  class Observer {
   public:
    // Invoked once with an empty list when the operation begins, then invoked
    // repeatedly once each result comes in. After all devices have been
    // processed, the callback is invoked one final time with
    // |is_final_scan_result| = true.
    virtual void OnTetherAvailabilityResponse(
        const std::vector<ScannedDeviceInfo>& scanned_device_list_so_far,
        const multidevice::RemoteDeviceRefList&
            gms_core_notifications_disabled_devices,
        bool is_final_scan_result) = 0;
  };

  TetherAvailabilityOperation(const TetherAvailabilityOperation&) = delete;
  TetherAvailabilityOperation& operator=(const TetherAvailabilityOperation&) = delete;

  ~TetherAvailabilityOperation() override;

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

 protected:
  TetherAvailabilityOperation(
      const multidevice::RemoteDeviceRefList& devices_to_connect,
      device_sync::DeviceSyncClient* device_sync_client,
      secure_channel::SecureChannelClient* secure_channel_client,
      HostScanDevicePrioritizer* host_scan_device_prioritizer,
      TetherHostResponseRecorder* tether_host_response_recorder,
      ConnectionPreserver* connection_preserver);

  void NotifyObserversOfScannedDeviceList(bool is_final_scan_result);

  // MessageTransferOperation:
  void OnDeviceAuthenticated(
      multidevice::RemoteDeviceRef remote_device) override;
  void OnMessageReceived(std::unique_ptr<MessageWrapper> message_wrapper,
                         multidevice::RemoteDeviceRef remote_device) override;
  void OnOperationStarted() override;
  void OnOperationFinished() override;
  MessageType GetMessageTypeForConnection() override;

  std::vector<ScannedDeviceInfo> scanned_device_list_so_far_;

 private:
  friend class TetherAvailabilityOperationTest;
  FRIEND_TEST_ALL_PREFIXES(TetherAvailabilityOperationTest,
                           DevicesArePrioritizedDuringConstruction);
  FRIEND_TEST_ALL_PREFIXES(TetherAvailabilityOperationTest, RecordsResponseDuration);
  FRIEND_TEST_ALL_PREFIXES(TetherAvailabilityOperationTest, ErrorResponses);
  FRIEND_TEST_ALL_PREFIXES(TetherAvailabilityOperationTest, NotificationsDisabled);
  FRIEND_TEST_ALL_PREFIXES(TetherAvailabilityOperationTest, TetherAvailable);
  FRIEND_TEST_ALL_PREFIXES(TetherAvailabilityOperationTest, LastProvisioningFailed);
  FRIEND_TEST_ALL_PREFIXES(TetherAvailabilityOperationTest, SetupRequired);
  FRIEND_TEST_ALL_PREFIXES(TetherAvailabilityOperationTest, TestMultipleDevices);

  using MessageTransferOperation::UnregisterDevice;

  void SetTestDoubles(base::Clock* clock_for_test,
                      scoped_refptr<base::TaskRunner> test_task_runner);
  void RecordTetherAvailabilityResponseDuration(const std::string device_id);

  raw_ptr<TetherHostResponseRecorder> tether_host_response_recorder_;
  raw_ptr<ConnectionPreserver> connection_preserver_;
  raw_ptr<base::Clock> clock_;
  scoped_refptr<base::TaskRunner> task_runner_;
  base::ObserverList<Observer>::Unchecked observer_list_;

  multidevice::RemoteDeviceRefList gms_core_notifications_disabled_devices_;

  std::map<std::string, base::Time>
      device_id_to_tether_availability_request_start_time_map_;

  base::WeakPtrFactory<TetherAvailabilityOperation> weak_ptr_factory_{this};
};

}  // namespace ash::tether

#endif  // CHROMEOS_ASH_COMPONENTS_TETHER_TETHER_AVAILABILITY_OPERATION_H_
