// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/plus_addresses/coordinator/plus_address_bottom_sheet_mediator.h"

#import "base/functional/bind.h"
#import "base/memory/raw_ptr.h"
#import "base/strings/sys_string_conversions.h"
#import "components/plus_addresses/features.h"
#import "components/plus_addresses/plus_address_metrics.h"
#import "components/plus_addresses/plus_address_service.h"
#import "components/plus_addresses/plus_address_types.h"
#import "ios/chrome/browser/plus_addresses/ui/plus_address_bottom_sheet_constants.h"
#import "ios/chrome/browser/plus_addresses/ui/plus_address_bottom_sheet_consumer.h"
#import "ios/chrome/browser/url_loading/model/url_loading_browser_agent.h"
#import "ios/chrome/browser/url_loading/model/url_loading_params.h"
#import "url/gurl.h"
#import "url/origin.h"

@implementation PlusAddressBottomSheetMediator {
  // The service implementation that owns the data.
  raw_ptr<plus_addresses::PlusAddressService> _plusAddressService;
  // The origin to which all operations should be scoped.
  url::Origin _mainFrameOrigin;
  // The autofill callback to be run if the process completes via confirmation
  // on the bottom sheet.
  plus_addresses::PlusAddressCallback _autofillCallback;
  // The reserved plus address, which is then eligible for confirmation.
  NSString* _reservedPlusAddress;
  raw_ptr<UrlLoadingBrowserAgent> _urlLoader;
  BOOL _incognito;
}

- (instancetype)
    initWithPlusAddressService:(plus_addresses::PlusAddressService*)service
                     activeUrl:(GURL)activeUrl
              autofillCallback:(plus_addresses::PlusAddressCallback)callback
                     urlLoader:(UrlLoadingBrowserAgent*)urlLoader
                     incognito:(BOOL)incognito {
  // In order to have reached this point, the service should've been created. If
  // not, fail now, since something bad happened.
  CHECK(service);
  self = [super init];
  if (self) {
    _plusAddressService = service;
    _mainFrameOrigin = url::Origin::Create(activeUrl);
    _autofillCallback = std::move(callback);
    _urlLoader = urlLoader;
    _incognito = incognito;
  }
  return self;
}

#pragma mark - PlusAddressBottomSheetDelegate

- (void)reservePlusAddress {
  __weak __typeof(self) weakSelf = self;
  // Create the callback needed by the C++ `_plusAddressService` object,
  // notifying the consumer once the call returns.
  auto callback = base::BindOnce(^(
      const plus_addresses::PlusProfileOrError& maybe_plus_profile) {
    if (maybe_plus_profile.has_value()) {
      [weakSelf didReservePlusAddress:base::SysUTF8ToNSString(
                                          maybe_plus_profile->plus_address)];
    } else {
      [weakSelf.consumer notifyError:plus_addresses::PlusAddressMetrics::
                                         PlusAddressModalCompletionStatus::
                                             kReservePlusAddressError];
    }
  });
  _plusAddressService->ReservePlusAddress(_mainFrameOrigin,
                                          std::move(callback));
}

- (void)confirmPlusAddress {
  __weak __typeof(self) weakSelf = self;
  // Create the callback needed by the C++ `_plusAddressService` object,
  // notifying the consumer once the call returns.
  auto callback = base::BindOnce(
      ^(const plus_addresses::PlusProfileOrError& maybe_plus_profile) {
        if (maybe_plus_profile.has_value()) {
          [weakSelf runAutofillCallback:base::SysUTF8ToNSString(
                                            maybe_plus_profile->plus_address)];
        } else {
          [weakSelf.consumer notifyError:plus_addresses::PlusAddressMetrics::
                                             PlusAddressModalCompletionStatus::
                                                 kConfirmPlusAddressError];
        }
      });
  _plusAddressService->ConfirmPlusAddress(
      _mainFrameOrigin, base::SysNSStringToUTF8(_reservedPlusAddress),
      std::move(callback));
}

- (NSString*)primaryEmailAddress {
  std::optional<std::string> primaryAddress =
      _plusAddressService->GetPrimaryEmail();
  // TODO(crbug.com/40276862): determine the appropriate behavior in cases
  // without a primary email (or just switch the signature away from optional).
  if (!primaryAddress.has_value()) {
    return @"";
  }
  return base::SysUTF8ToNSString(primaryAddress.value());
}

- (void)openNewTab:(PlusAddressURLType)type {
  UrlLoadParams params = UrlLoadParams::InNewTab([self plusAddressURL:type]);
  params.append_to = OpenPosition::kCurrentTab;
  params.user_initiated = NO;
  params.in_incognito = _incognito;
  _urlLoader->Load(params);
}

#pragma mark - Private

// Runs the autofill callback and notifies the consumer of the successful
// confirmation.
- (void)runAutofillCallback:(NSString*)confirmedPlusAddress {
  std::move(_autofillCallback)
      .Run(base::SysNSStringToUTF8(confirmedPlusAddress));
  [_consumer didConfirmPlusAddress];
}

// Once a plus address is successfully reserved, store it and notify the
// consumer.
- (void)didReservePlusAddress:(NSString*)reservedPlusAddress {
  _reservedPlusAddress = reservedPlusAddress;
  [_consumer didReservePlusAddress:reservedPlusAddress];
}

- (GURL)plusAddressURL:(PlusAddressURLType)type {
  switch (type) {
    case PlusAddressURLType::kErrorReport:
      return GURL(plus_addresses::features::kPlusAddressErrorReportUrl.Get());
    case PlusAddressURLType::kManagement:
      return GURL(plus_addresses::features::kPlusAddressManagementUrl.Get());
  }
}
@end
