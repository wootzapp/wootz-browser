/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

// This interface is for managing the global services of the application. Each
// service is lazily created when requested the first time. The service getters
// will return NULL if the service is not available, so callers must check for
// this condition.

#ifndef CHROME_BROWSER_WOOTZ_BROWSER_PROCESS_H_
#define CHROME_BROWSER_WOOTZ_BROWSER_PROCESS_H_

// #include "components/wootz_vpn/common/buildflags/buildflags.h"
// #include "components/greaselion/browser/buildflags/buildflags.h"
// #include "components/request_otr/common/buildflags/buildflags.h"
// #include "components/speedreader/common/buildflags/buildflags.h"
// #include "components/tor/buildflags/buildflags.h"
#include "build/build_config.h"
#include "extensions/buildflags/buildflags.h"

namespace wootz {
class WootzReferralsService;
class URLSanitizerComponentInstaller;
}  // namespace wootz

// #if BUILDFLAG(ENABLE_WOOTZ_VPN)
// namespace wootz_vpn {
// class WootzVPNConnectionManager;
// }  // namespace wootz_vpn
// #endif

namespace wootz_component_updater {
class LocalDataFilesService;
}  // namespace wootz_component_updater

// namespace wootz_shields {
// class AdBlockService;
// }  // namespace wootz_shields

namespace wootz_stats {
class WootzStatsUpdater;
}  // namespace wootz_stats

// namespace greaselion {
// #if BUILDFLAG(ENABLE_GREASELION)
// class GreaselionDownloadService;
// #endif
// }  // namespace greaselion

namespace debounce {
class DebounceComponentInstaller;
}  // namespace debounce

namespace https_upgrade_exceptions {
class HttpsUpgradeExceptionsService;
}  // namespace https_upgrade_exceptions

namespace localhost_permission {
class LocalhostPermissionComponent;
}  // namespace localhost_permission

namespace misc_metrics {
class ProcessMiscMetrics;
}  // namespace misc_metrics

// namespace request_otr {
// #if BUILDFLAG(ENABLE_REQUEST_OTR)
// class RequestOTRComponentInstallerPolicy;
// #endif
// }  // namespace request_otr

namespace ntp_background_images {
class NTPBackgroundImagesService;
}  // namespace ntp_background_images

namespace p3a {
class P3AService;
}  // namespace p3a

// namespace tor {
// class WootzTorClientUpdater;
// class WootzTorPluggableTransportUpdater;
// }  // namespace tor

// namespace speedreader {
// class SpeedreaderRewriterService;
// }

// namespace wootz_ads {
// class WootzStatsHelper;
// class ResourceComponent;
// }  // namespace wootz_ads

class WootzBrowserProcess {
 public:
  WootzBrowserProcess();
  virtual ~WootzBrowserProcess();
  virtual void StartWootzServices() = 0;
//   virtual wootz_shields::AdBlockService* ad_block_service() = 0;
  virtual https_upgrade_exceptions::HttpsUpgradeExceptionsService*
  https_upgrade_exceptions_service() = 0;
  virtual localhost_permission::LocalhostPermissionComponent*
  localhost_permission_component() = 0;
// #if BUILDFLAG(ENABLE_GREASELION)
//   virtual greaselion::GreaselionDownloadService*
//   greaselion_download_service() = 0;
// #endif
//   virtual debounce::DebounceComponentInstaller*
//   debounce_component_installer() = 0;
// #if BUILDFLAG(ENABLE_REQUEST_OTR)
//   virtual request_otr::RequestOTRComponentInstallerPolicy*
//   request_otr_component_installer() = 0;
// #endif
//   virtual wootz::URLSanitizerComponentInstaller*
//   URLSanitizerComponentInstaller() = 0;
//   virtual wootz_component_updater::LocalDataFilesService*
//   local_data_files_service() = 0;
// #if BUILDFLAG(ENABLE_TOR)
//   virtual tor::WootzTorClientUpdater* tor_client_updater() = 0;
//   virtual tor::WootzTorPluggableTransportUpdater*
//   tor_pluggable_transport_updater() = 0;
// #endif
//   virtual p3a::P3AService* p3a_service() = 0;
//   virtual wootz::WootzReferralsService* wootz_referrals_service() = 0;
//   virtual wootz_stats::WootzStatsUpdater* wootz_stats_updater() = 0;
// //   virtual wootz_ads::WootzStatsHelper* ads_wootz_stats_helper() = 0;
//   virtual ntp_background_images::NTPBackgroundImagesService*
//   ntp_background_images_service() = 0;
// #if BUILDFLAG(ENABLE_SPEEDREADER)
//   virtual speedreader::SpeedreaderRewriterService*
//   speedreader_rewriter_service() = 0;
// #endif
// #if BUILDFLAG(ENABLE_WOOTZ_VPN)
//   virtual wootz_vpn::WootzVPNConnectionManager*
//   wootz_vpn_connection_manager() = 0;
// #endif
//   virtual wootz_ads::ResourceComponent* resource_component() = 0;
//   virtual misc_metrics::ProcessMiscMetrics* process_misc_metrics() = 0;
};

extern WootzBrowserProcess* g_wootz_browser_process;

#endif  // CHROME_BROWSER_WOOTZ_BROWSER_PROCESS_H_