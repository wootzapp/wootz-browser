// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_SEARCH_ENGINES_MODEL_SEARCH_ENGINE_CHOICE_SERVICE_FACTORY_H_
#define IOS_CHROME_BROWSER_SEARCH_ENGINES_MODEL_SEARCH_ENGINE_CHOICE_SERVICE_FACTORY_H_

#include "base/no_destructor.h"
#include "components/keyed_service/ios/browser_state_keyed_service_factory.h"

class ChromeBrowserState;

namespace search_engines {
class SearchEngineChoiceService;
}

namespace ios {

class SearchEngineChoiceServiceFactory
    : public BrowserStateKeyedServiceFactory {
 public:
  SearchEngineChoiceServiceFactory(const SearchEngineChoiceServiceFactory&) =
      delete;
  SearchEngineChoiceServiceFactory& operator=(
      const SearchEngineChoiceServiceFactory&) = delete;

  static search_engines::SearchEngineChoiceService* GetForBrowserState(
      ChromeBrowserState* browser_state);

  static SearchEngineChoiceServiceFactory* GetInstance();

 private:
  friend class base::NoDestructor<SearchEngineChoiceServiceFactory>;

  SearchEngineChoiceServiceFactory();
  ~SearchEngineChoiceServiceFactory() override;

  // BrowserStateKeyedServiceFactory:
  std::unique_ptr<KeyedService> BuildServiceInstanceFor(
      web::BrowserState* context) const override;
};

}  // namespace ios

#endif  // IOS_CHROME_BROWSER_SEARCH_ENGINES_MODEL_SEARCH_ENGINE_CHOICE_SERVICE_FACTORY_H_
