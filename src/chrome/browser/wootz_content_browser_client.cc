/* Copyright (c) 2019 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chrome/browser/wootz_content_browser_client.h"

#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "base/command_line.h"
#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/strings/strcat.h"
#include "base/system/sys_info.h"
#include "chrome/browser/chrome_browser_main.h"
#include "chrome/browser/chrome_browser_main_extra_parts.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_context_utils.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_provider_delegate_impl.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_service_factory.h"

#include "chrome/browser/ui/webui/wootz_wallet/android/android_wallet_page_ui.h"

#include "chrome/browser/profiles/wootz_renderer_updater.h"
#include "chrome/browser/profiles/wootz_renderer_updater_factory.h"

#include "components/wootz_wallet/browser/wootz_wallet_p3a_private.h"
#include "components/wootz_wallet/browser/wootz_wallet_service.h"
#include "components/wootz_wallet/browser/wootz_wallet_utils.h"
#include "components/wootz_wallet/browser/ethereum_provider_impl.h"
#include "components/wootz_wallet/browser/solana_provider_impl.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "components/wootz_wallet/common/common_utils.h"
#include "components/constants/pref_names.h"
#include "components/constants/webui_url_constants.h"
// #include "components/decentralized_dns/content/decentralized_dns_navigation_throttle.h"

#include "chrome/grit/wootz_generated_resources.h"
// #include "chrome/third_party/blink/renderer/wootz_farbling_constants.h"
#include "build/build_config.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/chrome_browser_interface_binders.h"
#include "chrome/browser/chrome_content_browser_client.h"
#include "chrome/browser/content_settings/host_content_settings_map_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_io_data.h"
#include "chrome/browser/search_engines/template_url_service_factory.h"
#include "chrome/common/url_constants.h"
#include "chrome/grit/branded_strings.h"
#include "components/content_settings/browser/page_specific_content_settings.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/embedder_support/switches.h"
#include "components/prefs/pref_service.h"
#include "components/services/heap_profiling/public/mojom/heap_profiling_client.mojom.h"
#include "components/user_prefs/user_prefs.h"
#include "components/version_info/version_info.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_task_traits.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/browser_url_handler.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/browser/weak_document_ptr.h"
#include "content/public/browser/web_ui_browser_interface_broker_registry.h"
#include "content/public/browser/web_ui_controller_interface_binder.h"
#include "content/public/common/content_client.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/url_constants.h"
#include "extensions/buildflags/buildflags.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/self_owned_receiver.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "net/cookies/site_for_cookies.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"
#include "third_party/blink/public/common/features.h"
#include "third_party/blink/public/common/loader/url_loader_throttle.h"
#include "third_party/blink/public/mojom/webpreferences/web_preferences.mojom.h"
#include "third_party/widevine/cdm/buildflags.h"
#include "ui/base/l10n/l10n_util.h"

using blink::web_pref::WebPreferences;

using content::BrowserThread;
using content::ContentBrowserClient;
using content::RenderFrameHost;
using content::WebContents;

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "chrome/browser/extensions/chrome_content_browser_client_extensions_part.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/constants.h"

using extensions::ChromeContentBrowserClientExtensionsPart;
#endif

#if BUILDFLAG(IS_ANDROID)
#include "chrome/browser/ui/webui/wootz_wallet/android/android_wallet_page_ui.h"
#endif  // BUILDFLAG(IS_ANDROID)

namespace {

bool HandleURLReverseOverrideRewrite(GURL* url,
                                     content::BrowserContext* browser_context) {
  if (WootzContentBrowserClient::HandleURLOverrideRewrite(url,
                                                          browser_context)) {
    return true;
  }

// For wallet pages, return true to update the displayed URL to react-routed
// URL rather than showing wootz://wallet for everything. This is needed
// because of a side effect from rewriting wootz:// to chrome:// in
// HandleURLRewrite handler which makes wootz://wallet the virtual URL here
// unless we return true to trigger an update of virtual URL here to the routed
// URL. For example, we will display wootz://wallet/send instead of
// wootz://wallet with this. This is Android only because currently both
// virtual and real URLs are chrome:// on desktop, so it doesn't have this
// issue.
// #if BUILDFLAG(IS_ANDROID)
//   if ((url->SchemeIs(content::kWootzUIScheme) ||
//        url->SchemeIs(content::kChromeUIScheme)) &&
//       url->host() == kWalletPageHost) {
//     if (url->SchemeIs(content::kChromeUIScheme)) {
//       GURL::Replacements replacements;
//       replacements.SetSchemeStr(content::kWootzUIScheme);
//       *url = url->ReplaceComponents(replacements);
//     }
//     return true;
//   }
// #endif

  return false;
}

bool HandleURLRewrite(GURL* url, content::BrowserContext* browser_context) {
  if (WootzContentBrowserClient::HandleURLOverrideRewrite(url,
                                                          browser_context)) {
    return true;
  }

// For wallet pages, return true so we can handle it in the reverse handler.
// Also update the real URL from wootz:// to chrome://.
// #if BUILDFLAG(IS_ANDROID)
//   if ((url->SchemeIs(content::kWootzUIScheme) ||
//        url->SchemeIs(content::kChromeUIScheme)) &&
//       url->host() == kWalletPageHost) {
//     if (url->SchemeIs(content::kWootzUIScheme)) {
//       GURL::Replacements replacements;
//       replacements.SetSchemeStr(content::kChromeUIScheme);
//       *url = url->ReplaceComponents(replacements);
//     }
//     return true;
//   }
// #endif

  return false;
}

// void MaybeBindWalletP3A(
//     content::RenderFrameHost* const frame_host,
//     mojo::PendingReceiver<wootz_wallet::mojom::WootzWalletP3A> receiver) {
//   auto* context = frame_host->GetBrowserContext();
//   if (wootz_wallet::IsAllowedForContext(frame_host->GetBrowserContext())) {
//     wootz_wallet::WootzWalletService* wallet_service =
//         wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(context);
//     DCHECK(wallet_service);
//     wallet_service->GetWootzWalletP3A()->Bind(std::move(receiver));
//   } else {
//     // Dummy API to avoid reporting P3A for OTR contexts
//     mojo::MakeSelfOwnedReceiver(
//         std::make_unique<wootz_wallet::WootzWalletP3APrivate>(),
//         std::move(receiver));
//   }
// }

void MaybeBindEthereumProvider(
    content::RenderFrameHost* const frame_host,
    mojo::PendingReceiver<wootz_wallet::mojom::EthereumProvider> receiver) {
  auto* wootz_wallet_service =
      wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(
          frame_host->GetBrowserContext());
  if (!wootz_wallet_service) {
    return;
  }

  content::WebContents* web_contents =
      content::WebContents::FromRenderFrameHost(frame_host);
  mojo::MakeSelfOwnedReceiver(
      std::make_unique<wootz_wallet::EthereumProviderImpl>(
          HostContentSettingsMapFactory::GetForProfile(
              Profile::FromBrowserContext(frame_host->GetBrowserContext())),
          wootz_wallet_service,
          std::make_unique<wootz_wallet::WootzWalletProviderDelegateImpl>(
              web_contents, frame_host),
          user_prefs::UserPrefs::Get(web_contents->GetBrowserContext())),
      std::move(receiver));
}

void MaybeBindSolanaProvider(
    content::RenderFrameHost* const frame_host,
    mojo::PendingReceiver<wootz_wallet::mojom::SolanaProvider> receiver) {
  auto* wootz_wallet_service =
      wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(
          frame_host->GetBrowserContext());
  if (!wootz_wallet_service) {
    return;
  }

  auto* json_rpc_service = wootz_wallet_service->json_rpc_service();
  CHECK(json_rpc_service);

  auto* keyring_service = wootz_wallet_service->keyring_service();
  CHECK(keyring_service);

  auto* tx_service = wootz_wallet_service->tx_service();
  CHECK(tx_service);

  auto* host_content_settings_map =
      HostContentSettingsMapFactory::GetForProfile(
          frame_host->GetBrowserContext());
  if (!host_content_settings_map) {
    return;
  }

  content::WebContents* web_contents =
      content::WebContents::FromRenderFrameHost(frame_host);
  mojo::MakeSelfOwnedReceiver(
      std::make_unique<wootz_wallet::SolanaProviderImpl>(
          *host_content_settings_map, wootz_wallet_service,
          std::make_unique<wootz_wallet::WootzWalletProviderDelegateImpl>(
              web_contents, frame_host)),
      std::move(receiver));
}

// void BindWootzSearchFallbackHost(
//     int process_id,
//     mojo::PendingReceiver<wootz_search::mojom::WootzSearchFallback> receiver) {
//   content::RenderProcessHost* render_process_host =
//       content::RenderProcessHost::FromID(process_id);
//   if (!render_process_host) {
//     return;
//   }

//   content::BrowserContext* context = render_process_host->GetBrowserContext();
//   mojo::MakeSelfOwnedReceiver(
//       std::make_unique<wootz_search::WootzSearchFallbackHost>(
//           context->GetDefaultStoragePartition()
//               ->GetURLLoaderFactoryForBrowserProcess()),
//       std::move(receiver));
// }

// void BindWootzSearchDefaultHost(
//     content::RenderFrameHost* const frame_host,
//     mojo::PendingReceiver<wootz_search::mojom::WootzSearchDefault> receiver) {
//   auto* context = frame_host->GetBrowserContext();
//   auto* profile = Profile::FromBrowserContext(context);
//   if (profile->IsRegularProfile()) {
//     auto* template_url_service =
//         TemplateURLServiceFactory::GetForProfile(profile);
//     const std::string host = frame_host->GetLastCommittedURL().host();
//     mojo::MakeSelfOwnedReceiver(
//         std::make_unique<wootz_search::WootzSearchDefaultHost>(
//             host, template_url_service, profile->GetPrefs()),
//         std::move(receiver));
//   } else {
//     // Dummy API which always returns false for private contexts.
//     mojo::MakeSelfOwnedReceiver(
//         std::make_unique<wootz_search::WootzSearchDefaultHostPrivate>(),
//         std::move(receiver));
//   }
// }

// void MaybeBindSkusSdkImpl(
//     content::RenderFrameHost* const frame_host,
//     mojo::PendingReceiver<skus::mojom::SkusService> receiver) {
//   auto* context = frame_host->GetBrowserContext();
//   skus::SkusServiceFactory::BindForContext(context, std::move(receiver));
// }

}  // namespace

WootzContentBrowserClient::WootzContentBrowserClient() = default;

WootzContentBrowserClient::~WootzContentBrowserClient() = default;

std::unique_ptr<content::BrowserMainParts>
WootzContentBrowserClient::CreateBrowserMainParts(bool is_integration_test) {
  std::unique_ptr<content::BrowserMainParts> main_parts =
      ChromeContentBrowserClient::CreateBrowserMainParts(is_integration_test);
  ChromeBrowserMainParts* chrome_main_parts =
      static_cast<ChromeBrowserMainParts*>(main_parts.get());
  chrome_main_parts->AddParts(std::make_unique<ChromeBrowserMainExtraParts>());
  return main_parts;
}

bool WootzContentBrowserClient::AreIsolatedWebAppsEnabled(
    content::BrowserContext* browser_context) {
  return false;
}

void WootzContentBrowserClient::BrowserURLHandlerCreated(
    content::BrowserURLHandler* handler) {

  handler->AddHandlerPair(&HandleURLRewrite, &HandleURLReverseOverrideRewrite);
  ChromeContentBrowserClient::BrowserURLHandlerCreated(handler);
}

void WootzContentBrowserClient::RenderProcessWillLaunch(
    content::RenderProcessHost* host) {
  Profile* profile = Profile::FromBrowserContext(host->GetBrowserContext());
  // The WootzRendererUpdater might be null for some irregular profiles, e.g.
  // the System Profile.
  if (WootzRendererUpdater* service =
          WootzRendererUpdaterFactory::GetForProfile(profile)) {
    service->InitializeRenderer(host);
  }

  ChromeContentBrowserClient::RenderProcessWillLaunch(host);
}

void WootzContentBrowserClient::
    RegisterAssociatedInterfaceBindersForRenderFrameHost(
        content::RenderFrameHost& render_frame_host,                // NOLINT
        blink::AssociatedInterfaceRegistry& associated_registry) {  // NOLINT
// #if BUILDFLAG(ENABLE_WIDEVINE)
//   associated_registry.AddInterface<
//       wootz_drm::mojom::WootzDRM>(base::BindRepeating(
//       [](content::RenderFrameHost* render_frame_host,
//          mojo::PendingAssociatedReceiver<wootz_drm::mojom::WootzDRM> receiver) {
//         WootzDrmTabHelper::BindWootzDRM(std::move(receiver), render_frame_host);
//       },
//       &render_frame_host));
// #endif  // BUILDFLAG(ENABLE_WIDEVINE)

#if !BUILDFLAG(IS_ANDROID)
  associated_registry.AddInterface<
      geolocation::mojom::WootzGeolocationPermission>(base::BindRepeating(
      [](content::RenderFrameHost* render_frame_host,
         mojo::PendingAssociatedReceiver<
             geolocation::mojom::WootzGeolocationPermission> receiver) {
        WootzGeolocationPermissionTabHelper::BindWootzGeolocationPermission(
            std::move(receiver), render_frame_host);
      },
      &render_frame_host));
#endif  // !BUILDFLAG(IS_ANDROID)

  // associated_registry.AddInterface<
  //     wootz_shields::mojom::WootzShieldsHost>(base::BindRepeating(
  //     [](content::RenderFrameHost* render_frame_host,
  //        mojo::PendingAssociatedReceiver<wootz_shields::mojom::WootzShieldsHost>
  //            receiver) {
  //       wootz_shields::WootzShieldsWebContentsObserver::BindWootzShieldsHost(
  //           std::move(receiver), render_frame_host);
  //     },
  //     &render_frame_host));


  ChromeContentBrowserClient::
      RegisterAssociatedInterfaceBindersForRenderFrameHost(render_frame_host,
                                                           associated_registry);
}

void WootzContentBrowserClient::RegisterWebUIInterfaceBrokers(
    content::WebUIBrowserInterfaceBrokerRegistry& registry) {
  ChromeContentBrowserClient::RegisterWebUIInterfaceBrokers(registry);

}

// std::optional<base::UnguessableToken>
// WootzContentBrowserClient::GetEphemeralStorageToken(
//     content::RenderFrameHost* render_frame_host,
//     const url::Origin& origin) {
//   DCHECK(render_frame_host);
//   auto* wc = content::WebContents::FromRenderFrameHost(render_frame_host);
//   if (!wc) {
//     return std::nullopt;
//   }

//   auto* es_tab_helper =
//       ephemeral_storage::EphemeralStorageTabHelper::FromWebContents(wc);
//   if (!es_tab_helper) {
//     return std::nullopt;
//   }

//   return es_tab_helper->GetEphemeralStorageToken(origin);
// }

// bool WootzContentBrowserClient::AllowWorkerFingerprinting(
//     const GURL& url,
//     content::BrowserContext* browser_context) {
//   return WorkerGetWootzFarblingLevel(url, browser_context) !=
//          WootzFarblingLevel::MAXIMUM;
// }

// uint8_t WootzContentBrowserClient::WorkerGetWootzFarblingLevel(
//     const GURL& url,
//     content::BrowserContext* browser_context) {
//   HostContentSettingsMap* host_content_settings_map =
//       HostContentSettingsMapFactory::GetForProfile(browser_context);
//   const bool shields_up =
//       wootz_shields::GetWootzShieldsEnabled(host_content_settings_map, url);
//   if (!shields_up) {
//     return WootzFarblingLevel::OFF;
//   }
//   auto fingerprinting_type = wootz_shields::GetFingerprintingControlType(
//       host_content_settings_map, url);
//   if (fingerprinting_type == ControlType::BLOCK) {
//     return WootzFarblingLevel::MAXIMUM;
//   }
//   if (fingerprinting_type == ControlType::ALLOW) {
//     return WootzFarblingLevel::OFF;
//   }
//   return WootzFarblingLevel::BALANCED;
// }

// content::ContentBrowserClient::AllowWebBluetoothResult
// WootzContentBrowserClient::AllowWebBluetooth(
//     content::BrowserContext* browser_context,
//     const url::Origin& requesting_origin,
//     const url::Origin& embedding_origin) {
//   if (!base::FeatureList::IsEnabled(blink::features::kWootzWebBluetoothAPI)) {
//     return ContentBrowserClient::AllowWebBluetoothResult::
//         BLOCK_GLOBALLY_DISABLED;
//   }
//   return ChromeContentBrowserClient::AllowWebBluetooth(
//       browser_context, requesting_origin, embedding_origin);
// }

bool WootzContentBrowserClient::CanCreateWindow(
    content::RenderFrameHost* opener,
    const GURL& opener_url,
    const GURL& opener_top_level_frame_url,
    const url::Origin& source_origin,
    content::mojom::WindowContainerType container_type,
    const GURL& target_url,
    const content::Referrer& referrer,
    const std::string& frame_name,
    WindowOpenDisposition disposition,
    const blink::mojom::WindowFeatures& features,
    bool user_gesture,
    bool opener_suppressed,
    bool* no_javascript_access) {
  // Check base implementation first
  bool can_create_window = ChromeContentBrowserClient::CanCreateWindow(
      opener, opener_url, opener_top_level_frame_url, source_origin,
      container_type, target_url, referrer, frame_name, disposition, features,
      user_gesture, opener_suppressed, no_javascript_access);

// #if BUILDFLAG(ENABLE_REQUEST_OTR)
//   // If the user has requested going off-the-record in this tab, don't allow
//   // opening new windows via script
//   if (content::WebContents* web_contents =
//           content::WebContents::FromRenderFrameHost(opener)) {
//     if (request_otr::
//             RequestOTRStorageTabHelper* request_otr_storage_tab_helper =
//                 request_otr::RequestOTRStorageTabHelper::FromWebContents(
//                     web_contents)) {
//       if (request_otr_storage_tab_helper->has_requested_otr()) {
//         *no_javascript_access = true;
//       }
//     }
//   }
// #endif

  return can_create_window;
}

// void WootzContentBrowserClient::ExposeInterfacesToRenderer(
//     service_manager::BinderRegistry* registry,
//     blink::AssociatedInterfaceRegistry* associated_registry,
//     content::RenderProcessHost* render_process_host) {
//   ChromeContentBrowserClient::ExposeInterfacesToRenderer(
//       registry, associated_registry, render_process_host);
//   registry->AddInterface(base::BindRepeating(&BindWootzSearchFallbackHost,
//                                              render_process_host->GetID()),
//                          content::GetUIThreadTaskRunner({}));
// }

void WootzContentBrowserClient::RegisterBrowserInterfaceBindersForFrame(
    content::RenderFrameHost* render_frame_host,
    mojo::BinderMapWithContext<content::RenderFrameHost*>* map) {
  ChromeContentBrowserClient::RegisterBrowserInterfaceBindersForFrame(
      render_frame_host, map);
  // map->Add<cosmetic_filters::mojom::CosmeticFiltersResources>(
  //     base::BindRepeating(&BindCosmeticFiltersResources));
  // if (wootz_search::IsDefaultAPIEnabled()) {
  //   map->Add<wootz_search::mojom::WootzSearchDefault>(
  //       base::BindRepeating(&BindWootzSearchDefaultHost));
  // }

  // map->Add<wootz_wallet::mojom::WootzWalletP3A>(
  //     base::BindRepeating(&MaybeBindWalletP3A));
  if (wootz_wallet::IsAllowedForContext(
          render_frame_host->GetBrowserContext())) {
    if (wootz_wallet::IsNativeWalletEnabled()) {
      map->Add<wootz_wallet::mojom::EthereumProvider>(
          base::BindRepeating(&MaybeBindEthereumProvider));
      map->Add<wootz_wallet::mojom::SolanaProvider>(
          base::BindRepeating(&MaybeBindSolanaProvider));
    }
  }

  // map->Add<skus::mojom::SkusService>(
  //     base::BindRepeating(&MaybeBindSkusSdkImpl));

#if BUILDFLAG(IS_ANDROID)
  content::RegisterWebUIControllerInterfaceBinder<
      wootz_wallet::mojom::PageHandlerFactory, AndroidWalletPageUI>(map);
#endif

#if !BUILDFLAG(IS_ANDROID)
  content::RegisterWebUIControllerInterfaceBinder<
      wootz_wallet::mojom::PageHandlerFactory, WalletPageUI>(map);
  content::RegisterWebUIControllerInterfaceBinder<
      wootz_wallet::mojom::PanelHandlerFactory, WalletPanelUI>(map);
  // content::RegisterWebUIControllerInterfaceBinder<
  //     wootz_private_new_tab::mojom::PageHandler, WootzPrivateNewTabUI>(map);
  // content::RegisterWebUIControllerInterfaceBinder<
  //     wootz_shields::mojom::PanelHandlerFactory, ShieldsPanelUI>(map);
  // if (base::FeatureList::IsEnabled(
  //         wootz_shields::features::kWootzAdblockCookieListOptIn)) {
  //   content::RegisterWebUIControllerInterfaceBinder<
  //       wootz_shields::mojom::CookieListOptInPageHandlerFactory,
  //       CookieListOptInUI>(map);
  // }
  content::RegisterWebUIControllerInterfaceBinder<
      wootz_rewards::mojom::RewardsPageHandlerFactory,
      wootz_rewards::RewardsPageTopUI>(map);
  content::RegisterWebUIControllerInterfaceBinder<
      wootz_rewards::mojom::PanelHandlerFactory, wootz_rewards::RewardsPanelUI>(
      map);
  content::RegisterWebUIControllerInterfaceBinder<
      wootz_rewards::mojom::TipPanelHandlerFactory, wootz_rewards::TipPanelUI>(
      map);
  if (base::FeatureList::IsEnabled(commands::features::kWootzCommands)) {
    content::RegisterWebUIControllerInterfaceBinder<
        commands::mojom::CommandsService, WootzSettingsUI>(map);
  }
#endif
}

bool WootzContentBrowserClient::HandleExternalProtocol(
    const GURL& url,
    content::WebContents::Getter web_contents_getter,
    int frame_tree_node_id,
    content::NavigationUIData* navigation_data,
    bool is_primary_main_frame,
    bool is_in_fenced_frame_tree,
    network::mojom::WebSandboxFlags sandbox_flags,
    ui::PageTransition page_transition,
    bool has_user_gesture,
    const std::optional<url::Origin>& initiating_origin,
    content::RenderFrameHost* initiator_document,
    mojo::PendingRemote<network::mojom::URLLoaderFactory>* out_factory) {
  return ChromeContentBrowserClient::HandleExternalProtocol(
      url, web_contents_getter, frame_tree_node_id, navigation_data,
      is_primary_main_frame, is_in_fenced_frame_tree, sandbox_flags,
      page_transition, has_user_gesture, initiating_origin, initiator_document,
      out_factory);
}

void WootzContentBrowserClient::AppendExtraCommandLineSwitches(
    base::CommandLine* command_line,
    int child_process_id) {
  ChromeContentBrowserClient::AppendExtraCommandLineSwitches(command_line,
                                                             child_process_id);
  // std::string process_type =
  //     command_line->GetSwitchValueASCII(switches::kProcessType);
  // if (process_type == switches::kRendererProcess) {
  //   uint64_t session_token =
  //       12345;  // the kinda thing an idiot would have on his luggage

  //   // Command line parameters from the browser process are propagated to the
  //   // renderers *after* ContentBrowserClient::AppendExtraCommandLineSwitches()
  //   // is called from RenderProcessHostImpl::AppendRendererCommandLine(). This
  //   // means we have to inspect the main browser process' parameters for the
  //   // |switches::kTestType| as it will be too soon to find it on command_line.
  //   const base::CommandLine& browser_command_line =
  //       *base::CommandLine::ForCurrentProcess();
  //   if (!browser_command_line.HasSwitch(switches::kTestType)) {
  //     content::RenderProcessHost* process =
  //         content::RenderProcessHost::FromID(child_process_id);
  //     Profile* profile =
  //         process ? Profile::FromBrowserContext(process->GetBrowserContext())
  //                 : nullptr;
  //     if (profile) {
  //       auto* wootz_farbling_service =
  //           wootz::WootzFarblingServiceFactory::GetForProfile(profile);
  //       if (wootz_farbling_service) {
  //         session_token = wootz_farbling_service->session_token();
  //       }
  //     }
  //     if (command_line->HasSwitch(switches::kEnableIsolatedWebAppsInRenderer)) {
  //       command_line->RemoveSwitch(switches::kEnableIsolatedWebAppsInRenderer);
  //     }
  //   }

  //   command_line->AppendSwitchASCII("chrome_session_token",
  //                                   base::NumberToString(session_token));

  //   // Switches to pass to render processes.
  //   static const char* const kSwitchNames[] = {
  //       translate::switches::kWootzTranslateUseGoogleEndpoint,
  //   };
  //   command_line->CopySwitchesFrom(browser_command_line, kSwitchNames);
  // }
}

// std::vector<std::unique_ptr<blink::URLLoaderThrottle>>
// WootzContentBrowserClient::CreateURLLoaderThrottles(
//     const network::ResourceRequest& request,
//     content::BrowserContext* browser_context,
//     const content::WebContents::Getter& wc_getter,
//     content::NavigationUIData* navigation_ui_data,
//     int frame_tree_node_id,
//     std::optional<int64_t> navigation_id) {
//   auto result = ChromeContentBrowserClient::CreateURLLoaderThrottles(
//       request, browser_context, wc_getter, navigation_ui_data,
//       frame_tree_node_id, navigation_id);
//   content::WebContents* contents = wc_getter.Run();

//   if (contents) {
//     const bool isMainFrame =
//         request.resource_type ==
//         static_cast<int>(blink::mojom::ResourceType::kMainFrame);

//     auto body_sniffer_throttle =
//         std::make_unique<body_sniffer::BodySnifferThrottle>(
//             base::SingleThreadTaskRunner::GetCurrentDefault());

//     // if (isMainFrame) {
//     //   // De-AMP
//     //   auto handler = de_amp::DeAmpBodyHandler::Create(request, wc_getter);
//     //   if (handler) {
//     //     body_sniffer_throttle->AddHandler(std::move(handler));
//     //   }
//     // }

//     result.push_back(std::move(body_sniffer_throttle));

//   }

//   return result;
// }

void WootzContentBrowserClient::WillCreateURLLoaderFactory(
    content::BrowserContext* browser_context,
    content::RenderFrameHost* frame,
    int render_process_id,
    URLLoaderFactoryType type,
    const url::Origin& request_initiator,
    const net::IsolationInfo& isolation_info,
    std::optional<int64_t> navigation_id,
    ukm::SourceIdObj ukm_source_id,
    network::URLLoaderFactoryBuilder& factory_builder,
    mojo::PendingRemote<network::mojom::TrustedURLLoaderHeaderClient>*
        header_client,
    bool* bypass_redirect_checks,
    bool* disable_secure_dns,
    network::mojom::URLLoaderFactoryOverridePtr* factory_override,
    scoped_refptr<base::SequencedTaskRunner> navigation_response_task_runner) {
  // TODO(iefremov): Skip proxying for certain requests?
  // WootzProxyingURLLoaderFactory::MaybeProxyRequest(
  //     browser_context, frame,
  //     type == URLLoaderFactoryType::kNavigation ? -1 : render_process_id,
  //     factory_builder, navigation_response_task_runner);

  ChromeContentBrowserClient::WillCreateURLLoaderFactory(
      browser_context, frame, render_process_id, type, request_initiator,
      isolation_info, std::move(navigation_id), ukm_source_id, factory_builder,
      header_client, bypass_redirect_checks, disable_secure_dns,
      factory_override, navigation_response_task_runner);
}

bool WootzContentBrowserClient::WillInterceptWebSocket(
    content::RenderFrameHost* frame) {
  return (frame != nullptr);
}

void WootzContentBrowserClient::CreateWebSocket(
    content::RenderFrameHost* frame,
    content::ContentBrowserClient::WebSocketFactory factory,
    const GURL& url,
    const net::SiteForCookies& site_for_cookies,
    const std::optional<std::string>& user_agent,
    mojo::PendingRemote<network::mojom::WebSocketHandshakeClient>
        handshake_client) {
  // auto* proxy = WootzProxyingWebSocket::ProxyWebSocket(
  //     frame, std::move(factory), url, site_for_cookies, user_agent,
  //     std::move(handshake_client));

  // if (ChromeContentBrowserClient::WillInterceptWebSocket(frame)) {
  //   ChromeContentBrowserClient::CreateWebSocket(
  //       frame, proxy->web_socket_factory(), url, site_for_cookies, user_agent,
  //       proxy->handshake_client().Unbind());
  // } else {
  //   proxy->Start();
  // }
}

void WootzContentBrowserClient::MaybeHideReferrer(
    content::BrowserContext* browser_context,
    const GURL& request_url,
    const GURL& document_url,
    blink::mojom::ReferrerPtr* referrer) {
  DCHECK(referrer && !referrer->is_null());
#if BUILDFLAG(ENABLE_EXTENSIONS)
  if (document_url.SchemeIs(extensions::kExtensionScheme) ||
      request_url.SchemeIs(extensions::kExtensionScheme)) {
    return;
  }
#endif
  if (document_url.SchemeIs(content::kChromeUIScheme) ||
      request_url.SchemeIs(content::kChromeUIScheme)) {
    return;
  }

  // Profile* profile = Profile::FromBrowserContext(browser_context);
  // const bool allow_referrers = wootz_shields::AreReferrersAllowed(
  //     HostContentSettingsMapFactory::GetForProfile(profile), document_url);
  // const bool shields_up = wootz_shields::GetWootzShieldsEnabled(
  //     HostContentSettingsMapFactory::GetForProfile(profile), document_url);

  // content::Referrer new_referrer;
  // if (wootz_shields::MaybeChangeReferrer(allow_referrers, shields_up,
  //                                        (*referrer)->url, request_url,
  //                                        &new_referrer)) {
  //   (*referrer)->url = new_referrer.url;
  //   (*referrer)->policy = new_referrer.policy;
  // }
}

GURL WootzContentBrowserClient::GetEffectiveURL(
    content::BrowserContext* browser_context,
    const GURL& url) {
  Profile* profile = Profile::FromBrowserContext(browser_context);
  if (!profile) {
    return url;
  }

#if BUILDFLAG(ENABLE_EXTENSIONS)
  if (extensions::ChromeContentBrowserClientExtensionsPart::
          AreExtensionsDisabledForProfile(profile)) {
    return url;
  }
  return ChromeContentBrowserClientExtensionsPart::GetEffectiveURL(profile,
                                                                   url);
#else
  return url;
#endif
}

// [static]
bool WootzContentBrowserClient::HandleURLOverrideRewrite(
    GURL* url,
    content::BrowserContext* browser_context) {
  // Some of these rewrites are for WebUI pages with URL that has moved.
  // After rewrite happens, GetWebUIFactoryFunction() will work as expected.
  // (see browser\ui\webui\wootz_web_ui_controller_factory.cc for more info)
  //
  // Scope of schema is intentionally narrower than content::HasWebUIScheme(url)
  // which also allows both `chrome-untrusted` and `chrome-devtools`.
  if (
      !url->SchemeIs(content::kChromeUIScheme)) {
    return false;
  }

  // wootz://sync => wootz://settings/wootzSync
  // if (url->host() == chrome::kChromeUISyncHost) {
  //   GURL::Replacements replacements;
  //   replacements.SetSchemeStr(content::kChromeUIScheme);
  //   replacements.SetHostStr(chrome::kChromeUISettingsHost);
  //   replacements.SetPathStr(kWootzSyncPath);
  //   *url = url->ReplaceComponents(replacements);
  //   return true;
  // }

  // no special win10 welcome page
  if (url->host() == chrome::kChromeUIWelcomeHost) {
    *url = GURL(chrome::kChromeUIWelcomeURL);
    return true;
  }

  return false;
}

// std::vector<std::unique_ptr<content::NavigationThrottle>>
// WootzContentBrowserClient::CreateThrottlesForNavigation(
//     content::NavigationHandle* handle) {
//   std::vector<std::unique_ptr<content::NavigationThrottle>> throttles =
//       ChromeContentBrowserClient::CreateThrottlesForNavigation(handle);

//   // inserting the navigation throttle at the fist position before any java
//   // navigation happens
//   throttles.insert(
//       throttles.begin(),
//       std::make_unique<wootz_rewards::RewardsProtocolNavigationThrottle>(
//           handle));

//   content::BrowserContext* context =
//       handle->GetWebContents()->GetBrowserContext();

//   std::unique_ptr<content::NavigationThrottle>
//       decentralized_dns_navigation_throttle =
//           decentralized_dns::DecentralizedDnsNavigationThrottle::
//               MaybeCreateThrottleFor(handle, g_browser_process->local_state(),
//                                      g_browser_process->GetApplicationLocale());
//   if (decentralized_dns_navigation_throttle) {
//     throttles.push_back(std::move(decentralized_dns_navigation_throttle));
//   }

//   // The HostContentSettingsMap might be null for some irregular profiles, e.g.
//   // the System Profile.
//   auto* host_content_settings_map =
//       HostContentSettingsMapFactory::GetForProfile(context);
//   if (host_content_settings_map) {
//     if (std::unique_ptr<content::NavigationThrottle>
//             domain_block_navigation_throttle = wootz_shields::
//                 DomainBlockNavigationThrottle::MaybeCreateThrottleFor(
//                     handle, g_wootz_browser_process->ad_block_service(),
//                     g_wootz_browser_process->ad_block_service()
//                         ->custom_filters_provider(),
//                     EphemeralStorageServiceFactory::GetForContext(context),
//                     host_content_settings_map,
//                     g_browser_process->GetApplicationLocale())) {
//       throttles.push_back(std::move(domain_block_navigation_throttle));
//     }
//   }


//   return throttles;
// }

// bool PreventDarkModeFingerprinting(WebContents* web_contents,
//                                    WebPreferences* prefs) {
//   Profile* profile =
//       Profile::FromBrowserContext(web_contents->GetBrowserContext());
//   // The HostContentSettingsMap might be null for some irregular profiles, e.g.
//   // the System Profile.
//   auto* host_content_settings_map =
//       HostContentSettingsMapFactory::GetForProfile(profile);
//   if (!host_content_settings_map) {
//     return false;
//   }
//   const GURL url = web_contents->GetLastCommittedURL();
//   const bool shields_up =
//       wootz_shields::GetWootzShieldsEnabled(host_content_settings_map, url);
//   auto fingerprinting_type = wootz_shields::GetFingerprintingControlType(
//       host_content_settings_map, url);
//   // https://github.com/wootz/wootz-browser/issues/15265
//   // Always use color scheme Light if fingerprinting mode strict
//   if (base::FeatureList::IsEnabled(
//           wootz_shields::features::kWootzDarkModeBlock) &&
//       shields_up && fingerprinting_type == ControlType::BLOCK &&
//       prefs->preferred_color_scheme !=
//           blink::mojom::PreferredColorScheme::kLight) {
//     prefs->preferred_color_scheme = blink::mojom::PreferredColorScheme::kLight;
//     return true;
//   }
//   return false;
// }

// bool WootzContentBrowserClient::OverrideWebPreferencesAfterNavigation(
//     WebContents* web_contents,
//     WebPreferences* prefs) {
//   bool changed =
//       ChromeContentBrowserClient::OverrideWebPreferencesAfterNavigation(
//           web_contents, prefs);
//   return PreventDarkModeFingerprinting(web_contents, prefs) || changed;
// }

// void WootzContentBrowserClient::OverrideWebkitPrefs(WebContents* web_contents,
//                                                     WebPreferences* web_prefs) {
//   ChromeContentBrowserClient::OverrideWebkitPrefs(web_contents, web_prefs);
//   PreventDarkModeFingerprinting(web_contents, web_prefs);
//   // This will stop NavigatorPlugins from returning fixed plugins data and will
//   // allow us to return our farbled data
//   web_prefs->allow_non_empty_navigator_plugins = true;

// }

// blink::UserAgentMetadata WootzContentBrowserClient::GetUserAgentMetadata() {
//   blink::UserAgentMetadata metadata =
//       ChromeContentBrowserClient::GetUserAgentMetadata();
//   // Expect the brand version lists to have brand version, chromium_version, and
//   // greased version.
//   base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
//   if (command_line->HasSwitch(embedder_support::kUserAgent)) {
//     return metadata;
//   }
//   DCHECK_EQ(3UL, metadata.brand_version_list.size());
//   DCHECK_EQ(3UL, metadata.brand_full_version_list.size());
//   // Zero out the last 3 version components in full version list versions.
//   for (auto& brand_version : metadata.brand_full_version_list) {
//     base::Version version(brand_version.version);
//     brand_version.version =
//         base::StrCat({base::NumberToString(version.components()[0]), ".0.0.0"});
//   }
//   // Zero out the last 3 version components in full version.
//   base::Version version(metadata.full_version);
//   metadata.full_version =
//       base::StrCat({base::NumberToString(version.components()[0]), ".0.0.0"});
//   return metadata;
// }

// GURL WootzContentBrowserClient::SanitizeURL(
//     content::RenderFrameHost* render_frame_host,
//     const GURL& url) {
//   // if (!base::FeatureList::IsEnabled(features::kWootzCopyCleanLinkFromJs)) {
//   //   return url;
//   // }
//   CHECK(render_frame_host);
//   CHECK(render_frame_host->GetBrowserContext());
//   auto* url_sanitizer_service =
//       wootz::URLSanitizerServiceFactory::GetForBrowserContext(
//           render_frame_host->GetBrowserContext());
//   CHECK(url_sanitizer_service);
//   if (!url_sanitizer_service->CheckJsPermission(
//           render_frame_host->GetLastCommittedURL())) {
//     return url;
//   }
//   return url_sanitizer_service->SanitizeURL(url);
// }

bool WootzContentBrowserClient::AllowSignedExchange(
    content::BrowserContext* context) {
  // This override has been introduced due to the deletion of the flag
  // `features::kSignedHTTPExchange`, which was being used to disable signed
  // exchanges.
  return false;
}
