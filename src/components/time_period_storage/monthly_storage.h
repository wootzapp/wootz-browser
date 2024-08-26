/* Copyright 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_TIME_PERIOD_STORAGE_MONTHLY_STORAGE_H_
#define COMPONENTS_TIME_PERIOD_STORAGE_MONTHLY_STORAGE_H_

#include "components/time_period_storage/time_period_storage.h"

class PrefService;

class MonthlyStorage : public TimePeriodStorage {
 public:
  MonthlyStorage(PrefService* prefs, const char* pref_name);

  MonthlyStorage(const MonthlyStorage&) = delete;
  MonthlyStorage& operator=(const MonthlyStorage&) = delete;

  uint64_t GetMonthlySum() const;
  uint64_t GetHighestValueInMonth() const;
};

#endif  // COMPONENTS_TIME_PERIOD_STORAGE_MONTHLY_STORAGE_H_
