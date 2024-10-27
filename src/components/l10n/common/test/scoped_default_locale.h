/* Copyright (c) 2022 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMPONENTS_L10N_COMMON_TEST_SCOPED_DEFAULT_LOCALE_H_
#define COMPONENTS_L10N_COMMON_TEST_SCOPED_DEFAULT_LOCALE_H_

#include <string>

namespace wootz_l10n::test {

// Set the current default locale for testing, restoring the original locale
// upon destruction.
class ScopedDefaultLocale final {
 public:
  explicit ScopedDefaultLocale(const std::string& locale);

  ScopedDefaultLocale(const ScopedDefaultLocale& other) = delete;
  ScopedDefaultLocale& operator=(const ScopedDefaultLocale& other) = delete;

  ScopedDefaultLocale(ScopedDefaultLocale&& other) noexcept = delete;
  ScopedDefaultLocale& operator=(ScopedDefaultLocale&& other) noexcept = delete;

  ~ScopedDefaultLocale();

 private:
  std::string last_locale_;
};

}  // namespace wootz_l10n::test

#endif  // COMPONENTS_L10N_COMMON_TEST_SCOPED_DEFAULT_LOCALE_H_
