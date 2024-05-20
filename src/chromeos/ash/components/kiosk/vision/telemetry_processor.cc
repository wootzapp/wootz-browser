// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/ash/components/kiosk/vision/telemetry_processor.h"

#include <utility>
#include <vector>

#include "base/containers/circular_deque.h"
#include "base/containers/to_vector.h"
#include "media/capture/video/chromeos/mojom/cros_camera_service.mojom-forward.h"
#include "media/capture/video/chromeos/mojom/cros_camera_service.mojom.h"

namespace ash::kiosk_vision {

namespace {

// TODO(b/320669284): Reconsider once the telemetry API consumer is implemented.
constexpr int kMaxDequeSize = 10;

// Caps `deque` to `kMaxDequeSize` by removing elements from the front.
template <typename T>
void CapToMaxSize(base::circular_deque<T>& deque) {
  if (deque.size() <= kMaxDequeSize) {
    return;
  }
  int to_be_erased = kMaxDequeSize - deque.size();
  deque.erase(deque.begin(), deque.begin() + to_be_erased);
}

}  // namespace

TelemetryProcessor::TelemetryProcessor() = default;

TelemetryProcessor::~TelemetryProcessor() = default;

std::vector<int> TelemetryProcessor::TakeIdsProcessed() {
  return base::ToVector(std::exchange(latest_ids_processed_, {}));
}

std::vector<cros::mojom::KioskVisionError> TelemetryProcessor::TakeErrors() {
  return ToVector(std::exchange(latest_errors_, {}));
}

void TelemetryProcessor::OnDetection(
    const cros::mojom::KioskVisionDetection& detection) {
  for (const auto& a : detection.appearances) {
    latest_ids_processed_.push_back(a->person_id);
  }
  CapToMaxSize(latest_ids_processed_);
}

void TelemetryProcessor::OnError(cros::mojom::KioskVisionError error) {
  latest_errors_.push_back(error);
  CapToMaxSize(latest_errors_);
}

}  // namespace ash::kiosk_vision
