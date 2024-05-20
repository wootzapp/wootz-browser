// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/core_browser_context_keyed_service_factories.h"

#include "extensions/browser/api/web_request/web_request_event_router_factory.h"
#include "extensions/browser/app_window/app_window_geometry_cache.h"
#include "extensions/browser/app_window/app_window_registry.h"
#include "extensions/browser/event_router_factory.h"
#include "extensions/browser/extension_action_manager.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/browser/extension_prefs_helper_factory.h"
#include "extensions/browser/extension_protocols.h"
#include "extensions/browser/guest_view/mime_handler_view/mime_handler_stream_manager.h"
#include "extensions/browser/image_loader_factory.h"
#include "extensions/browser/process_manager_factory.h"
#include "extensions/browser/renderer_startup_helper.h"
#include "extensions/browser/service_worker/service_worker_keepalive.h"
#include "extensions/browser/service_worker/service_worker_task_queue_factory.h"
#include "extensions/browser/updater/update_service_factory.h"
#include "extensions/browser/user_script_world_configuration_manager.h"

namespace extensions {

void EnsureCoreBrowserContextKeyedServiceFactoriesBuilt() {
  AppWindowGeometryCache::Factory::GetInstance();
  AppWindowRegistry::Factory::GetInstance();
  EnsureExtensionURLLoaderFactoryShutdownNotifierFactoryBuilt();
  EventRouterFactory::GetInstance();
  ExtensionActionManager::EnsureFactoryBuilt();
  ExtensionFunction::EnsureShutdownNotifierFactoryBuilt();
  ExtensionPrefsFactory::GetInstance();
  ExtensionPrefsHelperFactory::GetInstance();
  ImageLoaderFactory::GetInstance();
  MimeHandlerStreamManager::EnsureFactoryBuilt();
  ProcessManagerFactory::GetInstance();
  RendererStartupHelperFactory::GetInstance();
  ServiceWorkerKeepalive::EnsureShutdownNotifierFactoryBuilt();
  ServiceWorkerTaskQueueFactory::GetInstance();
  UpdateServiceFactory::GetInstance();
  UserScriptWorldConfigurationManager::GetFactory();
  WebRequestEventRouterFactory::GetInstance();
}

}  // namespace extensions
