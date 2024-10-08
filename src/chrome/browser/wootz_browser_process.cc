/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/browser/wootz_browser_process.h"

#include <cstddef>

WootzBrowserProcess* g_wootz_browser_process = nullptr;

WootzBrowserProcess::WootzBrowserProcess() = default;

WootzBrowserProcess::~WootzBrowserProcess() = default;