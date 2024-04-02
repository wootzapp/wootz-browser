// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/bluetooth/gatt_service.h"

#include "base/logging.h"
#include "device/bluetooth/bluetooth_adapter.h"
#include "device/bluetooth/public/cpp/bluetooth_uuid.h"

namespace bluetooth {

GattService::GattService(
    mojo::PendingReceiver<mojom::GattService> pending_gatt_service_receiver,
    mojo::PendingRemote<mojom::GattServiceObserver> pending_observer_remote,
    const device::BluetoothUUID& service_id,
    const scoped_refptr<device::BluetoothAdapter>& adapter)
    : service_id_(service_id),
      observer_remote_(std::move(pending_observer_remote)) {
  receiver_.Bind(std::move(pending_gatt_service_receiver));

  // Since a `GattService` corresponding to `service_id` is being created
  // here, one by this `service_id` should not exist yet.
  CHECK(!adapter->GetGattService(service_id.value()));
  adapter->CreateLocalGattService(
      /*uuid=*/service_id,
      /*is_primary=*/true,
      /*delegate=*/this);
}

GattService::~GattService() = default;

void GattService::OnCharacteristicReadRequest(
    const device::BluetoothDevice* device,
    const device::BluetoothLocalGattCharacteristic* characteristic,
    int offset,
    ValueCallback callback) {
  // TODO(b/311430390): Implement `OnCharacteristicReadRequest()` to notify
  // observers (which will be added as part of this TODO) to provide a
  // value for this read request. Only READ characteristics are supported for
  // the BLE V2 MVP use in Nearby Connections.
  NOTIMPLEMENTED();
}

void GattService::OnCharacteristicWriteRequest(
    const device::BluetoothDevice* device,
    const device::BluetoothLocalGattCharacteristic* characteristic,
    const std::vector<uint8_t>& value,
    int offset,
    base::OnceClosure callback,
    ErrorCallback error_callback) {
  NOTIMPLEMENTED();
}

void GattService::OnCharacteristicPrepareWriteRequest(
    const device::BluetoothDevice* device,
    const device::BluetoothLocalGattCharacteristic* characteristic,
    const std::vector<uint8_t>& value,
    int offset,
    bool has_subsequent_request,
    base::OnceClosure callback,
    ErrorCallback error_callback) {
  NOTIMPLEMENTED();
}

void GattService::OnDescriptorReadRequest(
    const device::BluetoothDevice* device,
    const device::BluetoothLocalGattDescriptor* descriptor,
    int offset,
    ValueCallback callback) {
  NOTIMPLEMENTED();
}

void GattService::OnDescriptorWriteRequest(
    const device::BluetoothDevice* device,
    const device::BluetoothLocalGattDescriptor* descriptor,
    const std::vector<uint8_t>& value,
    int offset,
    base::OnceClosure callback,
    ErrorCallback error_callback) {
  NOTIMPLEMENTED();
}

void GattService::OnNotificationsStart(
    const device::BluetoothDevice* device,
    device::BluetoothGattCharacteristic::NotificationType notification_type,
    const device::BluetoothLocalGattCharacteristic* characteristic) {
  NOTIMPLEMENTED();
}

void GattService::OnNotificationsStop(
    const device::BluetoothDevice* device,
    const device::BluetoothLocalGattCharacteristic* characteristic) {
  NOTIMPLEMENTED();
}

}  // namespace bluetooth
