/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CHROME_BROWSER_PROFILES_WOOTZ_RENDERER_UPDATER_FACTORY_H_
#define CHROME_BROWSER_PROFILES_WOOTZ_RENDERER_UPDATER_FACTORY_H_

#include <memory>

#include "base/no_destructor.h"
#include "chrome/browser/profiles/profile_keyed_service_factory.h"

class Profile;
class WootzRendererUpdater;

namespace base {
template <typename T>
class NoDestructor;
}  // namespace base

// Singleton that creates/deletes WootzRendererUpdater as new Profiles are
// created/shutdown.
class WootzRendererUpdaterFactory : public ProfileKeyedServiceFactory {
 public:
  // Returns an instance of the WootzRendererUpdaterFactory singleton.
  static WootzRendererUpdaterFactory* GetInstance();

  // Returns the instance of RendererUpdater for the passed |profile|.
  static WootzRendererUpdater* GetForProfile(Profile* profile);

  WootzRendererUpdaterFactory(const WootzRendererUpdaterFactory&) = delete;
  WootzRendererUpdaterFactory& operator=(const WootzRendererUpdaterFactory&) =
      delete;

 protected:
  std::unique_ptr<KeyedService> BuildServiceInstanceForBrowserContext(
      content::BrowserContext* profile) const override;
  bool ServiceIsCreatedWithBrowserContext() const override;

 private:
  friend base::NoDestructor<WootzRendererUpdaterFactory>;

  WootzRendererUpdaterFactory();
  ~WootzRendererUpdaterFactory() override;
};

#endif  // CHROME_BROWSER_PROFILES_WOOTZ_RENDERER_UPDATER_FACTORY_H_
