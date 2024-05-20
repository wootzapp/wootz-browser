These design principles make it easier to write, debug, and maintain code in //chrome/browser.

## Caveats:
* These are recommendations, not requirements.
* These are not intended to be static. If you think a
  principle doesn't make sense, reach out to //chrome/OWNERS.
* These are intended to apply to new code and major refactors. We do not expect
  existing features to be refactored, except as need arises.

## Structure, modularity:
* Features should be modular with no dependency cycles.
    * This is a provisional design principle. We are currently investigating
      feasibility. TODO: canonical example
    * For most features, all business logic should live in some combination of
      //chrome/browser/<feature>, //chrome/browser/ui/<feature> or
      //component/<feature>.
    * WebUI resources are the only exception. They will continue to live in
      //chrome/browser/resources/<feature> alongside standalone BUILD.gn files.
    * This directory should have a standalone BUILD.gn and OWNERs file.
    * This directory may have its own namespace.
    * This directory should have a public/ subdirectory to enforce further
      encapsulation.
        * The main reason for this is to guard against future, unexpected usage
          of parts of the code that were intended to be private. This makes it
          difficult to change implementation details in the future.
        * The BUILD.gn should use public/sources separation.
    * Corollary: There are several global functions that facilitate dependency
      inversion. It will not be possible to call them from modularized features
      (no dependency cycles), and their usage in non-modularized features is
      considered a red flag:
        * `chrome::FindBrowserWithTab` (and everything in browser_finder.h)
        * `GetBrowserViewForNativeWindow`  (via browser_view.h)
        * `FindBrowserWindowWithWebContents` (via browser_window.h)

* Features should have a core controller with precise lifetime semantics. The
  core controller for most desktop features should be owned and instantiated by
  one of the following classes:
    * `TabFeatures` (member of `TabModel`)
        * This class should own all tab-centric features. e.g. print preview,
          lens overlay, compose, find-in-page, etc.
            * If the feature requires instantiation of
              `WebContents::SupportsUserData`, it should be done in this class.
        * For desktop chrome, `TabHelpers::AttachTabHelpers` will become a
          remove-only method. Clank/WebView may continue to use section 2 of
          `TabHelpers::AttachTabHelpers` (Clank/WebView only).
        * More complex features that also target mobile platforms or other
          supported embedders (e.g. android webview) will continue to use the
          layered components architecture.
            * We defer to //components/OWNERS for expertise and feedback on the
              architecture of these features, and encourage feature-owners to
              proactively reach out to them.
        * Lazy instantiation of `WebContents::SupportsUserData` is an
          anti-pattern.
    * `BrowserWindowFeatures` (member of `Browser`)
        * example: omnibox, security chip, bookmarks bar, side panel
    * `BrowserContextKeyedServiceFactory` (functionally a member of `Profile`)
    * `GlobalFeatures` (member of `BrowserProcess`)
    * The core controller should not be a `NoDestructor` singleton.
* Global functions should not access non-global state.
    * Pure functions do not access global state and are allowed. e.g. `base::UTF8ToWide()`
    * Global functions that wrap global state are allowed, e.g.
      `IsFooFeatureEnabled()` wraps the global variable
      `BASE_FEATURE(kFooFeature,...)`
    * Global functions that access non-global state are disallowed. e.g.
      static methods on `BrowserList`.

## UI
* Features should use WebUI and Views toolkit, which are x-platform.
    * Usage of underlying primitives is discouraged (aura, Cocoa, gtk, win32,
      etc.). This is usually a sign that either the feature is misusing the UI
      toolkits, or that the UI toolkits are missing important functionality.
* Features should use "MVC"
    * Clear separation between controller (control flow), view (presentation of
      UI) and model (storage of data).
    * For simple features that do not require data persistence, we only require
      separation of controller from view.
    * TODO: work with UI/toolkit team to come up with appropriate examples.
* Avoid subclassing Views/Widgets.
* Avoid self-owned objects/classes for views or controllers.

## General
* Code unrelated to building a web-browser should not live in //chrome.
    * See [chromeos/code.md](chromeos/code.md) for details on ChromeOS (non-browser) code.
    * We may move some modularized x-platform code into //components. The main
      distinction is that ChromeOS can depend on //components, but not on
      //chrome. This will be evaluated on a case-by-case basis.
* Avoid nested message loops.
* Threaded code should have DCHECKs asserting correct sequence.
    * Provides documentation and correctness checks.
    * See base/sequence_checker.h.
* Most features should be gated by base::Feature, API keys and/or gn build
  configuration, not C preprocessor conditionals e.g. `#if
  BUILDFLAG(FEATURE_FOO)`.
    * We ship a single product (Chrome) to multiple platforms. The purpose of
      proprocessor conditionals is:
        * (1) to allow platform-agnostic code to reference platform-specific
          code.
            * e.g. `#if BUILDFLAG(OS_WIN)`
        * (2) to glue src-internal into public //chromium/src.
            * e.g. `#if BUILDFLAG(GOOGLE_CHROME_BRANDING)`
        * (3) To make behavior changes to non-production binaries
            * e.g. `#if !defined(NDEBUG)`
            * e.g. `#if defined(ADDRESS_SANITIZER)`
    * (1) primarily serves as glue.
    * (2) turns Chromium into Chrome. We want the two to be as similar as
      possible. This preprocessor conditional should be used very sparingly.
      Almost all our tests are run against Chromium, so any logic behind this
      conditional will be mostly untested.
    * (3) is a last resort. The point of DEBUG/ASAN/... builds is to provide
      insight into problems that affect the production binaries we ship. By
      changing the production logic to do something different, we are no longer
      accomplishing this goal.
    * In all cases, large segments of code should not be gated behind
      preprocessor conditionals. Instead, they should be pulled into separate
      files via gn.
    * In the event that a feature does have large swathes of code in separate
      build files/translation units (e.g. extensions), using a custom feature
      flag (e.g. `BUILDFLAG(ENABLE_EXTENSIONS)`) to glue this into the main source
      is allowed. The glue code should be kept to a minimum.
* Avoid run-time channel checking.
* Avoid test only conditionals
    * This was historically common in unit_tests, because it was not possible to
      stub out dependencies due to lack of a clear API surface. By requiring
      modular features with clear API surfaces, it also becomes easy to perform
      dependency injection for tests, thereby removing the need for conditionals
      that can be nullptr in tests.
    * In the event that they are necessary, document and enforce via
      `CHECK_IS_TEST()`.
* Avoid unreachable branches.
    * We should have a semantic understanding of each path of control flow. How
      is this reached? If we don't know, then we should not have a conditional.
* For a given `base::Callback`, execution should either be always synchronous, or
always asynchronous. Mixing the two means callers have to deal with two distinct
control flows, which often leads to incorrect handling of one.
Avoid the following:
```cpp
if (result.cached) {
  RunCallbackSync())
} else {
  GetResultAndRunCallbackAsync()
}
```
* Avoid re-entrancy. Control flow should remain as localized as possible.
Bad (unnecessary delegation, re-entrancy)
```cpp
class CarSalesPerson : public Delegate {
  // Can return nullptr, in which case no car is shown
  std::unique_ptr<Car> ShowCar() {
    return car_factory_->CreateCar();
  }

  // Delegate overrides:
  // Whether the car should be shown, even if the factory is busy.
  bool ShouldShowCarIfFactoryIsBusy() override;

  CarFactory* car_factory_;
};

class CarFactory {
  std::unique_ptr<Car> CreateCar() {
    if (!CanCreateCar())
      return nullptr;
    if (FactoryIsBusy() && !delegate->ShouldShowCarIfFactoryIsBusy())
      return nullptr;
    return std::make_unique<Car>();
  }

  bool CanCreateCar();
  bool FactoryIsBusy();

  Delegate* delegate_;
};
```

Good, version 1: Remove delegation. Pass all relevant state to CarFactory so that CreateCar() does not depend on non-local state.
```cpp
class CarSalesPerson {
  // Can return nullptr, in which case no car is shown
  std::unique_ptr<Car> ShowCar() {
    return car_factory_->CreateCar(ShouldShowCarIfFactoryIsBusy());
  }

  // Whether the car should be shown, even if the factory is busy.
  bool ShouldShowCarIfFactoryIsBusy();

  CarFactory* car_factory_;
};

class CarFactory {
  std::unique_ptr<Car> CreateCar(bool show_even_if_factory_is_busy) {
    if (!CanCreateCar())
      return nullptr;
    if (FactoryIsBusy() && !show_even_if_factory_is_busy)
      return nullptr;
    return std::make_unique<Car>();
  }
  bool CanCreateCar();
  bool FactoryIsBusy();
};
```

Good, version 2: Remove delegation. CreateCar always create a car (fewer conditionals). State only flows from CarFactory to CarSalesPerson (and never backwards).
```cpp
class CarSalesPerson {
  // Can return nullptr, in which case no car is shown
  std::unique_ptr<Car> ShowCar() {
    if (!car_factory_->CanCreateCar())
      return nullptr;
    if (car_factory_->FactoryIsBusy() && !ShouldShowCarIfFactoryIsBusy())
      return nullptr;
    return car_factory_->CreateCar();
  }

  // Whether the car should be shown, even if the factory is busy.
  bool ShouldShowCarIfFactoryIsBusy();
  CarFactory* car_factory_;
};

class CarFactory {
  bool CanCreateCar();
  bool FactoryIsBusy();
  // never returns nullptr.
  std::unique_ptr<Car> CreateCar();
};
```


