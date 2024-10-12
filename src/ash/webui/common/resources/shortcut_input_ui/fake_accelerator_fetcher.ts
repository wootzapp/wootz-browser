// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import {AcceleratorAction} from './accelerator_actions.mojom-webui.js';
import {AcceleratorFetcherInterface, AcceleratorFetcherObserverInterface} from './accelerator_fetcher.mojom-webui.js';
import {StandardAcceleratorProperties} from './accelerator_info.mojom-webui.js';

export class FakeAcceleratorFetcher implements AcceleratorFetcherInterface {
  private observers: AcceleratorFetcherObserverInterface[] = [];

  mockAcceleratorsUpdated(
      actionId: AcceleratorAction,
      accelerators: StandardAcceleratorProperties[]): void {
    for (const observer of this.observers) {
      observer.onAcceleratorsUpdated(actionId, accelerators);
    }
  }

  observeAcceleratorChanges(
      _actionIds: AcceleratorAction[],
      observer: AcceleratorFetcherObserverInterface): void {
    this.observers.push(observer);
  }
}
