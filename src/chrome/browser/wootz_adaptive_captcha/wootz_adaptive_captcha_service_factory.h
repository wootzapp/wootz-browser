/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_WOOTZ_ADAPTIVE_CAPTCHA_WOOTZ_ADAPTIVE_CAPTCHA_SERVICE_FACTORY_H_
#define CHROME_BROWSER_WOOTZ_ADAPTIVE_CAPTCHA_WOOTZ_ADAPTIVE_CAPTCHA_SERVICE_FACTORY_H_

#include "components/wootz_adaptive_captcha/wootz_adaptive_captcha_delegate.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class Profile;

namespace base {
template <typename T>
class NoDestructor;
}  // namespace base

namespace wootz_adaptive_captcha {

class WootzAdaptiveCaptchaService;

class WootzAdaptiveCaptchaServiceFactory
    : public BrowserContextKeyedServiceFactory {
 public:
  static WootzAdaptiveCaptchaService* GetForProfile(Profile* profile);
  static WootzAdaptiveCaptchaServiceFactory* GetInstance();

 private:
  friend base::NoDestructor<WootzAdaptiveCaptchaServiceFactory>;

  WootzAdaptiveCaptchaServiceFactory();
  ~WootzAdaptiveCaptchaServiceFactory() override;

  WootzAdaptiveCaptchaServiceFactory(
      const WootzAdaptiveCaptchaServiceFactory&) = delete;
  WootzAdaptiveCaptchaServiceFactory& operator=(
      const WootzAdaptiveCaptchaServiceFactory&) = delete;

  // BrowserContextKeyedServiceFactory overrides:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
};

}  // namespace wootz_adaptive_captcha

#endif  // CHROME_BROWSER_WOOTZ_ADAPTIVE_CAPTCHA_WOOTZ_ADAPTIVE_CAPTCHA_SERVICE_FACTORY_H_