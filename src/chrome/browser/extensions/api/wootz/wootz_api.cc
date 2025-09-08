// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/wootz/wootz_api.h"

#include <stddef.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "base/android/build_info.h"
#include "base/android/jni_string.h"
#include "base/base64.h"
#include "base/functional/bind.h"
#include "base/json/json_writer.h"
#include "base/lazy_instance.h"
#include "base/memory/ref_counted.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "build/build_config.h"
#include "chrome/android/chrome_jni_headers/WootzAppBackgroundContentService_jni.h"
#include "chrome/android/chrome_jni_headers/WootzBridge_jni.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/search_engines/template_url_service_factory.h"
#include "chrome/browser/wootz_wallet/wootz_wallet_service_factory.h"
#include "components/action_url/content/common/action_url_prefs.h"
#include "components/search_engines/template_url_service.h"
#include "components/subresource_filter/content/browser/content_subresource_filter_throttle_manager.h"
#include "components/subresource_filter/core/browser/subresource_filter_prefs.h"
#include "components/wootz_wallet/browser/eth_tx_manager.h"
#include "components/wootz_wallet/browser/tx_meta.h"
#include "components/wootz_wallet/browser/tx_service.h"
#include "components/wootz_wallet/browser/wootz_wallet_service.h"
#include "components/zk_proof/zk_proof.h"
#include "components/zk_proof/tls_info/tls_data_store.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_event_histogram_value.h"
#include "chrome/browser/extensions/extension_tab_util.h"
#include "chrome/browser/ui/android/tab_model/tab_model_list.h"
#include "chrome/browser/ui/android/tab_model/tab_model.h"
#include "components/action_url/content/browser/content_sensitive_masking_driver_factory.h"
#include "components/action_url/content/browser/content_sensitive_masking_driver.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_prefs.h"
#include "extensions/browser/extension_prefs_factory.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/extension_id.h"
#include "net/base/filename_util.h"
#include "third_party/jni_zero/jni_zero.h"
#include "ui/android/window_android.h"
#include "ui/gfx/image/image.h"
#include "chrome/android/chrome_jni_headers/OpenExtensionsById_jni.h"
#include "base/android/shared_preferences/shared_preferences_manager.h"
#include "chrome/browser/preferences/android/chrome_shared_preferences.h"
#include "base/json/json_reader.h"
#include "base/json/values_util.h"
#include "base/time/time.h"
#include "extensions/common/permissions/permissions_data.h"
#include "extensions/common/permissions/api_permission.h"
#include "extensions/common/mojom/api_permission_id.mojom.h"
#include "base/logging.h"
#include "components/zk_proof/zk_proof.h"
#include "components/zk_proof/tls_info/tls_data_store.h"
#include "components/subresource_filter/core/browser/subresource_filter_prefs.h"
#include "components/subresource_filter/content/browser/content_subresource_filter_throttle_manager.h"
#include "components/automation_agent/content/browser/automation_controller.h"
#include "components/automation_agent/content/browser/automation_controller_factory.h"
#include "chrome/browser/ui/android/tab_model/tab_model_list.h"
#include "chrome/browser/ui/android/tab_model/tab_model.h"
#include "components/prefs/pref_service.h"
#include "chrome/browser/prefs/blocked_domains_prefs.h"
#include "chrome/browser/prefs/saml_prefs.h"
#include "components/saml_verifier/saml_verifier.h"
#include "content/public/browser/copy_paste_blocker_prefs.h"
#include "content/public/browser/render_frame_host.h"
#include "components/action_url/content/common/mojom/sensitive_element_masking.mojom.h"
#include "mojo/public/cpp/bindings/associated_remote.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"



namespace extensions {

const char kWootzAppSearchAIModelName[] = "wootzapp_search_ai_model_name";
const char kWootzAppSearchAIModelAPIKey[] = "wootzapp_search_ai_model_api_key";

wootz_wallet::KeyringService* GetKeyringService(
    content::BrowserContext* context) {
  auto* profile = Profile::FromBrowserContext(context);
  return wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(profile)
      ->keyring_service();
}

content::WebContents* WebContentsIdToJavaWebContents(int webContentsId) {
  JNIEnv* env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jobject> receiver_from_native = Java_WootzAppBackgroundContentService_getBackgroundWebContents(
      env, webContentsId);

  if(receiver_from_native.is_null()) {
    return nullptr;
  }

  content::WebContents* web_contents = content::WebContents::FromJavaWebContents(receiver_from_native);
  if (web_contents) {
    // Ensure AutomationControllerFactory exists for this WebContents
    if (!automation::AutomationControllerFactory::FromWebContents(web_contents)) {
      automation::AutomationControllerFactory::CreateForWebContents(web_contents);
      LOG(INFO) << "Created AutomationControllerFactory for existing WebContents ID: " << webContentsId;
    }
  }

  return web_contents;
}

void OpenExtensionsById(const std::string& extensionId) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_OpenExtensionsById_openExtensionByIdNative(
      env, 
      base::android::ConvertUTF8ToJavaString(env, extensionId));
}

void closeExtensionBottomSheet() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_OpenExtensionsById_closeExtensionBottomSheetNative(
      env);
}

ExtensionId GetWootzWalletExtensionId(content::BrowserContext* context) {
  const char* kWootzWalletExtensionName = "Wootz Wallet";
  LOG(ERROR) << "JANGID: Searching for extension with name: "
             << kWootzWalletExtensionName;

  ExtensionRegistry* registry = ExtensionRegistry::Get(context);
  if (!registry) {
    LOG(ERROR) << "JANGID: Failed to get ExtensionRegistry";
    return ExtensionId();
  }

  LOG(ERROR) << "JANGID: Iterating through enabled extensions";
  int count = 0;
  for (const auto& extension : registry->enabled_extensions()) {
    count++;
    LOG(ERROR) << "JANGID: Checking extension " << count << ":";
    LOG(ERROR) << "  ID: " << extension->id();
    LOG(ERROR) << "  Name: " << extension->name();
    LOG(ERROR) << "  Version: " << extension->version().GetString();

    if (extension->name() == kWootzWalletExtensionName) {
      LOG(ERROR) << "JANGID: Found matching extension with ID: "
                 << extension->id();
      return extension->id();
    }
  }

  LOG(ERROR)<< "JANGID: No matching extension found. Total extensions checked: "<< count;
  
  return ExtensionId();
}

static base::LazyInstance<BrowserContextKeyedAPIFactory<WootzAPI>>::
    DestructorAtExit g_wootz_api_factory = LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<WootzAPI>* WootzAPI::GetFactoryInstance() {
  LOG(ERROR)<<"Jangid_Observer GetFactoryInstance";
    return g_wootz_api_factory.Pointer();
}

WootzAPI::WootzAPI(content::BrowserContext* context)
    : browser_context_(context),
      observer_receiver_(this) {
    LOG(ERROR) << "Jangid_Observer Creating WootzAPI";
    StartObserving();  // Start observing immediately
}

WootzAPI::~WootzAPI() {
    LOG(ERROR) << "Jangid_Observer Destroying WootzAPI";
}

void WootzAPI::StartObserving() {
    LOG(ERROR) << "jangid_observer: Starting to observe transactions";

    Profile* profile = Profile::FromBrowserContext(browser_context_);
    auto* service = 
        wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(profile);
    if (!service) {
        LOG(ERROR) << "jangid_observer: Failed to get WootzWalletService for profile";
        return;
    }
    LOG(ERROR) << "jangid_observer: Successfully retrieved WootzWalletService";

    mojo::PendingRemote<wootz_wallet::mojom::TxServiceObserver> observer;
    observer_receiver_.Bind(observer.InitWithNewPipeAndPassReceiver());
    service->tx_service()->AddObserver(std::move(observer));
}

void WootzAPI::OnNewUnapprovedTx(
    wootz_wallet::mojom::TransactionInfoPtr tx_info) {

  Profile* profile = Profile::FromBrowserContext(browser_context_);

  ExtensionId wootz_wallet_extension_id = GetWootzWalletExtensionId(profile);
  LOG(ERROR) << "jangid_sign: Wootz wallet extension ID: " << wootz_wallet_extension_id;

  ExtensionRegistry* registry = ExtensionRegistry::Get(profile);
  const Extension* extension = registry->GetExtensionById(
      wootz_wallet_extension_id, ExtensionRegistry::ENABLED);

  if (extension) {
    LOG(ERROR) << "jangid_sign: Extension details:";
    LOG(ERROR) << "jangid_sign: Name: " << extension->name();
    LOG(ERROR) << "jangid_sign: Version: " << extension->version().GetString();
    LOG(ERROR) << "jangid_sign: Description: " << extension->description();
  } else {
    LOG(ERROR) << "jangid_sign: Extension not found or not enabled";
  }

  auto* event_router = EventRouter::Get(profile);
  if (!event_router) {
    LOG(ERROR) << "jangid_sign: Event router not available";
    return;
  }
  LOG(ERROR) << "jangid_sign: Got event router";

  base::Value::List event_args;
  base::Value::Dict tx_details;
  tx_details.Set("txMetaId", tx_info->id);
  tx_details.Set("from", tx_info->from_account_id->address);
  tx_details.Set("tx_hash", tx_info->tx_hash);
  tx_details.Set("chainId", tx_info->chain_id);
  tx_details.Set("coinType", static_cast<int>(tx_info->from_account_id->coin));
  tx_details.Set("origin", tx_info->origin_info ? tx_info->origin_info->origin_spec : "");

  // Solana transaction data
  if (tx_info->from_account_id->coin == wootz_wallet::mojom::CoinType::SOL) {
    LOG(ERROR) << "jangid_sign: Processing Solana transaction data of coin type: " << static_cast<int>(tx_info->from_account_id->coin);
    const auto& solana_tx_data = tx_info->tx_data_union->get_solana_tx_data();

    const auto& instructions = solana_tx_data->instructions;
    
    // Loop through all instructions
    for(size_t i = 0; i < instructions.size(); i++) {
        LOG(ERROR) << "Instruction " << i + 1 << " of " << instructions.size();
        
        // Check if instruction has decoded data
        if (instructions[i]->decoded_data) {
            auto& decoded_data = instructions[i]->decoded_data;
            
            // Log instruction type
            LOG(ERROR) << "Instruction Type: " << decoded_data->instruction_type;
            
            // Loop through all parameters in the instruction
            for(size_t j = 0; j < decoded_data->params.size(); j++) {
                auto& param = decoded_data->params[j];
                LOG(ERROR) << "Parameter " << j + 1 << ":";
                LOG(ERROR) << "  Name: " << param->name;
                LOG(ERROR) << "  Value: " << param->value;

                if(param->name == "lamports") {
                  LOG(ERROR) << "jangid_sign: Solana lamports: " << param->value;
                  tx_details.Set("transaction_amount", base::NumberToString(std::stod(param->value)));
                }

                LOG(ERROR) << "  Type: " << param->type;
                LOG(ERROR) << "  Localized Name: " << param->localized_name;
            }
        } else {
            LOG(ERROR) << "No decoded data for instruction " << i + 1;
        }
    }
    
    base::Value::List fee_list;
    base::Value::Dict fee_dict;

    if (solana_tx_data->fee_estimation) {
      fee_dict.Set("baseFee", static_cast<double>(solana_tx_data->fee_estimation->base_fee));
      fee_dict.Set("computeUnits", static_cast<double>(solana_tx_data->fee_estimation->compute_units));
      fee_dict.Set("feePerComputeUnit", static_cast<double>(solana_tx_data->fee_estimation->fee_per_compute_unit));

      fee_list.Append(std::move(fee_dict));
      
      LOG(ERROR) << "jangid_sign: Solana fee estimation: base_fee=" 
                << solana_tx_data->fee_estimation->base_fee
                << ", compute_units=" << solana_tx_data->fee_estimation->compute_units
                << ", fee_per_compute_unit=" << solana_tx_data->fee_estimation->fee_per_compute_unit;
    }
    
    tx_details.Set("feeEstimation", std::move(fee_list));
    LOG(ERROR) << "jangid_sign: Solana fee estimation: " << tx_details.Find("feeEstimation");
  }
  // Ethereum transaction data
  else if (tx_info->from_account_id->coin == wootz_wallet::mojom::CoinType::ETH) {
    LOG(ERROR) << "jangid_sign: Processing Ethereum transaction data of coin type: " << static_cast<int>(tx_info->from_account_id->coin);

    // Ethereum transaction data
    if(tx_info->tx_data_union->is_eth_tx_data()) {
      const auto& eth_tx_data = tx_info->tx_data_union->get_eth_tx_data();
    
      LOG(ERROR) << "jangid_sign: Ethereum transaction amount: " << eth_tx_data->value;
      tx_details.Set("transaction_amount", eth_tx_data->value);

      base::Value::List fee_list;
      base::Value::Dict fee_dict;

      LOG(ERROR) << "jangid_sign: Ethereum transaction gas price: " << eth_tx_data->gas_price;
      fee_dict.Set("baseFee", std::stod(eth_tx_data->gas_price));

      LOG(ERROR) << "jangid_sign: Ethereum transaction gas limit: " << eth_tx_data->gas_limit;
      fee_dict.Set("computeUnits", std::stod(eth_tx_data->gas_limit));

      LOG(ERROR) << "jangid_sign: Ethereum transaction gas price: " << eth_tx_data->gas_price;
      fee_dict.Set("feePerComputeUnit", std::stod(eth_tx_data->gas_price));

      fee_list.Append(std::move(fee_dict));
      tx_details.Set("feeEstimation", std::move(fee_list));

      LOG(ERROR) << "jangid_sign: Ethereum fee estimation: " << tx_details.Find("feeEstimation");
    }
    // Ethereum transaction data 1559
    else if(tx_info->tx_data_union->is_eth_tx_data_1559()) {
      const auto& eth_tx_data_1559 = tx_info->tx_data_union->get_eth_tx_data_1559();

      LOG(ERROR) << "jangid_sign: Ethereum transaction amount: " << eth_tx_data_1559->base_data->value;
      tx_details.Set("transaction_amount", eth_tx_data_1559->base_data->value);

      base::Value::List fee_list;
      base::Value::Dict fee_dict; 

      LOG(ERROR) << "jangid_sign: Ethereum transaction gas price: " << eth_tx_data_1559->base_data->gas_price;
      fee_dict.Set("baseFee", std::stod(eth_tx_data_1559->base_data->gas_price));

      LOG(ERROR) << "jangid_sign: Ethereum transaction gas limit: " << eth_tx_data_1559->base_data->gas_limit;
      fee_dict.Set("computeUnits", std::stod(eth_tx_data_1559->base_data->gas_limit));  

      LOG(ERROR) << "jangid_sign: Ethereum transaction gas price: " << eth_tx_data_1559->base_data->gas_price;
      fee_dict.Set("feePerComputeUnit", std::stod(eth_tx_data_1559->base_data->gas_price));

      fee_list.Append(std::move(fee_dict));
      tx_details.Set("feeEstimation", std::move(fee_list));

      LOG(ERROR) << "jangid_sign: Ethereum fee estimation: " << tx_details.Find("feeEstimation");
    }
  }

  LOG(ERROR) << "jangid_sign: Transaction details: " << tx_details;
  event_args.Append(std::move(tx_details));
  LOG(ERROR) << "jangid_sign: Event arguments: " << event_args;

  std::unique_ptr<Event> event = std::make_unique<Event>(
      events::WOOTZ_ON_NEW_UNAPPROVED_TX,
      "wootz.OnNewUnapprovedTxAPI",
      std::move(event_args), 
      profile,
      std::nullopt,
      GURL(),
      EventRouter::USER_GESTURE_UNKNOWN,
      mojom::EventFilteringInfo::New());

  LOG(ERROR) << "jangid_sign: Dispatching event to extension: " << wootz_wallet_extension_id;
  
  event_router->DispatchEventToExtension(wootz_wallet_extension_id, std::move(event));
  OpenExtensionsById(wootz_wallet_extension_id);
}

void WootzAPI::OnTransactionStatusChanged(
    wootz_wallet::mojom::TransactionInfoPtr tx_info) {
  LOG(ERROR) << "jangid_sign: OnTransactionStatusChanged called";
  LOG(ERROR) << "jangid_sign: Transaction ID: " << tx_info->id;
  LOG(ERROR) << "jangid_sign: Current status: " << static_cast<int>(tx_info->tx_status);

  base::Value::List event_args;
  base::Value::Dict status_info;
  status_info.Set("txMetaId", tx_info->id);
  
  std::string status_str;
  if (tx_info->tx_status == wootz_wallet::mojom::TransactionStatus::Unapproved) {
    status_str = "unapproved";
  } else if (tx_info->tx_status == wootz_wallet::mojom::TransactionStatus::Approved) {
    status_str = "approved";
  } else if (tx_info->tx_status == wootz_wallet::mojom::TransactionStatus::Rejected) {
    status_str = "rejected";
  } else if (tx_info->tx_status == wootz_wallet::mojom::TransactionStatus::Submitted) {
    status_str = "submitted";
  } else if (tx_info->tx_status == wootz_wallet::mojom::TransactionStatus::Confirmed) {
    status_str = "confirmed";
  } else if (tx_info->tx_status == wootz_wallet::mojom::TransactionStatus::Error) {
    status_str = "error";
  } else if (tx_info->tx_status == wootz_wallet::mojom::TransactionStatus::Dropped) {
    status_str = "dropped";
  } else if (tx_info->tx_status == wootz_wallet::mojom::TransactionStatus::Signed) {
    status_str = "signed";
  } else {
    status_str = "unknown";
  }
  
  LOG(ERROR) << "jangid_sign: Status string: " << status_str;
  
  status_info.Set("status", status_str);
  if (!tx_info->tx_hash.empty()) {
    status_info.Set("hash", tx_info->tx_hash);
    LOG(ERROR) << "jangid_sign: Transaction hash: " << tx_info->tx_hash;
  }
  event_args.Append(std::move(status_info));

  Profile* profile = Profile::FromBrowserContext(browser_context_);
  ExtensionId wootz_wallet_extension_id = GetWootzWalletExtensionId(profile);
  LOG(ERROR) << "jangid_sign: Extension ID for status change: " << wootz_wallet_extension_id;

  auto* event_router = EventRouter::Get(profile);
  if (!event_router) {
    LOG(ERROR) << "jangid_sign: Event router not available for status change";
    return;
  }

  std::unique_ptr<Event> event = std::make_unique<Event>(
      events::WOOTZ_ON_TRANSACTION_STATUS_CHANGED,
      "wootz.onTransactionStatusChangedAPI",
      std::move(event_args), 
      profile,
      std::nullopt,
      GURL(), 
      EventRouter::USER_GESTURE_UNKNOWN,
      mojom::EventFilteringInfo::New());
  
  LOG(ERROR) << "jangid_sign: Dispatching status change event";
  event_router->DispatchEventToExtension(
      wootz_wallet_extension_id, 
      std::move(event));
  LOG(ERROR) << "jangid_sign: Status change event dispatched successfully";
}

void WootzAPI::OnUnapprovedTxUpdated(wootz_wallet::mojom::TransactionInfoPtr tx_info) {
    LOG(ERROR) << "Unapproved transaction updated: ";
}


void WootzAPI::OnTxServiceReset() {
    LOG(ERROR) << "Transaction service reset";
}

void WootzAPI::OnDropdownButtonClicked(const std::string& selectedFeature, const std::string& extensionId, const std::string& extensionName) {  
    
    Profile* profile = Profile::FromBrowserContext(browser_context_);
    if (extensionId.empty()) {
        LOG(ERROR) << "Extension ID is empty, cannot dispatch event";
        return;
    }
    ExtensionRegistry* extension_registry = ExtensionRegistry::Get(profile);
    if (!extension_registry) {
        LOG(ERROR) << "Extension registry not available";
        return;
    }

    const Extension* extension = extension_registry->GetExtensionById(
        extensionId, ExtensionRegistry::ENABLED);
    if (!extension) {
        LOG(ERROR) << "Extension not found " << extensionId;
        return;
    }
    
    LOG(ERROR) << "Extension found and enabled: " << extension->name() 
               << " (ID: " << extensionId << ")";

    auto* event_router = EventRouter::Get(profile);
    if (!event_router) {
        LOG(ERROR) << "Event router not available";
        return;
    }
    
    base::Value::List event_args;
    base::Value::Dict dropdown_info;
    dropdown_info.Set("selectedFeature", selectedFeature);
    dropdown_info.Set("extensionId", extensionId);
    dropdown_info.Set("extensionName", extensionName);
    dropdown_info.Set("timestamp", base::Time::Now().InMillisecondsFSinceUnixEpoch());
    
    event_args.Append(std::move(dropdown_info));
    
  
    std::unique_ptr<Event> event = std::make_unique<Event>(
        events::WOOTZ_ON_DROPDOWN_BUTTON_CLICKED,
        "wootz.onDropdownButtonClicked",
        std::move(event_args), 
        profile,
        std::nullopt,
        GURL(), 
        EventRouter::USER_GESTURE_UNKNOWN,
        mojom::EventFilteringInfo::New());
    if (!event_router->ExtensionHasEventListener(extensionId, "wootz.onDropdownButtonClicked")) {
        LOG(WARNING) << "Extension is not listening for wootz.onDropdownButtonClicked events: " << extensionId;
    }
    
    event_router->DispatchEventToExtension(extensionId, std::move(event));

    LOG(ERROR) << "Opening extension: " << extensionId;
    OpenExtensionsById(extensionId);
}

ExtensionFunction::ResponseAction WootzInfoFunction::Run() {
  const base::android::BuildInfo* build_info =
      base::android::BuildInfo::GetInstance();

  // Create a dictionary to store the relevant build information
  base::Value::Dict build_info_dict;
  build_info_dict.Set("device", build_info->device());
  build_info_dict.Set("manufacturer", build_info->manufacturer());
  build_info_dict.Set("model", build_info->model());
  build_info_dict.Set("brand", build_info->brand());
  build_info_dict.Set("sdk_int", build_info->sdk_int());
  build_info_dict.Set("android_build_id", build_info->android_build_id());
  build_info_dict.Set("android_build_fp", build_info->android_build_fp());
  build_info_dict.Set("gms_version_code", build_info->gms_version_code());
  build_info_dict.Set("host_package_name", build_info->host_package_name());
  build_info_dict.Set("package_name", build_info->package_name());
  build_info_dict.Set("package_version_code",
                      build_info->package_version_code());
  build_info_dict.Set("package_version_name",
                      build_info->package_version_name());
  build_info_dict.Set("abi_name", build_info->abi_name());
  build_info_dict.Set("is_tv", build_info->is_tv());
  build_info_dict.Set("is_automotive", build_info->is_automotive());
  build_info_dict.Set("is_foldable", build_info->is_foldable());

  std::string json_string;
  base::JSONWriter::Write(build_info_dict, &json_string);

  return RespondNow(WithArguments(json_string));
}

ExtensionFunction::ResponseAction WootzHelloWorldFunction::Run() {
  base::Value::Dict result;
  result.Set("message", "Hello, World!");

  std::string json_string;
  base::JSONWriter::Write(result, &json_string);

  return RespondNow(WithArguments(json_string));
}

ExtensionFunction::ResponseAction WootzLogFunction::Run() {
  if (args().size() < 1) {
    return RespondNow(NoArguments());
  }
  LOG(ERROR) << "CONSOLE.LOG: " << args()[0];
  return RespondNow(NoArguments());
}

[[maybe_unused]]
ExtensionFunction::ResponseAction WootzShowDialogFunction::Run() {
#if 0
    JNIEnv* env = base::android::AttachCurrentThread();
    
    content::WebContents* web_contents = GetSenderWebContents();
    if (!web_contents) {
        return RespondNow(Error("Unable to get WebContents"));
    }

    Java_WootzBridge_showDialog(env, web_contents->GetJavaWebContents());
#endif
    return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WootzSetSelectedChainsFunction::Run() {
  if (args().empty() || !args()[0].is_list()) {
    return RespondNow(Error("Invalid arguments"));
  }

  const base::Value::List& chains_list = args()[0].GetList();
  std::vector<std::string> selected_chains;

  for (const auto& chain : chains_list) {
    if (!chain.is_string()) {
      continue;
    }
    const std::string& chain_str = chain.GetString();
    if (chain_str == "ethereum" || chain_str == "solana") {
      selected_chains.push_back(chain_str);
    }
  }

  // Ensure at least one chain is selected
  if (selected_chains.empty()) {
    return RespondNow(Error("At least one chain must be selected"));
  }

  auto* keyring_service = GetKeyringService(browser_context());
  if (!keyring_service) {
    return RespondNow(Error("KeyringService not available"));
  }

  keyring_service->SetSelectedChains(selected_chains);

  base::Value::Dict result;
  result.Set("success", true);
  return RespondNow(WithArguments(std::move(result)));
}

ExtensionFunction::ResponseAction WootzCreateWalletFunction::Run() {
  if (args().empty() || !args()[0].is_string()) {
    return RespondNow(Error("Invalid arguments"));
  }

  std::string password = args()[0].GetString();
  if (password.empty()) {
    return RespondNow(Error("Password cannot be empty"));
  }
  auto* keyring_service = GetKeyringService(browser_context());

  keyring_service->SetPassword(password);
    
    LOG(ERROR)<<"JANGID: Password"<<password;
    
  if (!keyring_service) {
    return RespondNow(Error("KeyringService not available"));
  }

  keyring_service->CreateWallet(
      password,
      base::BindOnce(&WootzCreateWalletFunction::OnWalletCreated, this));

  return RespondLater();
}

void WootzCreateWalletFunction::OnWalletCreated(
    const std::optional<std::string>& recovery_phrase) {
  base::Value::Dict result;
  result.Set("success", recovery_phrase.has_value());
  if (recovery_phrase) {
    result.Set("recoveryPhrase", *recovery_phrase);
  } else {
    result.Set("error", "Failed to create wallet");
  }

  base::Value::List result_list;
  result_list.Append(std::move(result));
  Respond(ArgumentList(std::move(result_list)));
}

ExtensionFunction::ResponseAction WootzIsWalletCreatedFunction::Run() {
  auto* keyring_service = GetKeyringService(browser_context());

  if (!keyring_service) {
    return RespondNow(Error("KeyringService not available"));
  }

  base::Value::Dict result;
  result.Set("isCreated", keyring_service->IsWalletCreatedSync());

  return RespondNow(WithArguments(std::move(result)));
}

ExtensionFunction::ResponseAction WootzUnlockWalletFunction::Run() {
  if (args().empty() || !args()[0].is_string()) {
    return RespondNow(Error("Invalid arguments"));
  }

  auto* keyring_service = GetKeyringService(browser_context());

  if (!keyring_service) {
    return RespondNow(Error("KeyringService not available"));
  }

   std::string input_password = args()[0].GetString();

  keyring_service->Unlock(
      input_password, base::BindOnce(&WootzUnlockWalletFunction::OnUnlocked, this));

  // Set the password again after unlocking
  keyring_service->SetPassword(input_password);

  return RespondLater();
}

void WootzUnlockWalletFunction::OnUnlocked(bool success) {
  base::Value::Dict result;
  result.Set("success", success);
  if (!success) {
    result.Set("error", "Failed to unlock wallet");
  }

  base::Value::List result_list;
  result_list.Append(std::move(result));
  Respond(ArgumentList(std::move(result_list)));
}

ExtensionFunction::ResponseAction WootzLockWalletFunction::Run() {
  auto* keyring_service = GetKeyringService(browser_context());

  if (!keyring_service) {
    return RespondNow(Error("KeyringService not available"));
  }

  keyring_service->Lock();
  
  base::Value::Dict result;
  result.Set("success", true);

  return RespondNow(WithArguments(std::move(result)));
}

ExtensionFunction::ResponseAction WootzIsLockedFunction::Run() {
  auto* keyring_service = GetKeyringService(browser_context());

  if (!keyring_service) {
    return RespondNow(Error("KeyringService not available"));
  }

  keyring_service->IsLocked(
      base::BindOnce(&WootzIsLockedFunction::OnIsLocked, this));

  return RespondLater();
}

void WootzIsLockedFunction::OnIsLocked(bool is_locked) {
  base::Value::Dict result;
  result.Set("isLocked", is_locked);

  base::Value::List result_list;
  result_list.Append(std::move(result));
  Respond(ArgumentList(std::move(result_list)));
}


ExtensionFunction::ResponseAction WootzGetAllAccountsFunction::Run() {
  auto* keyring_service = GetKeyringService(browser_context());

  if (!keyring_service) {
    return RespondNow(Error("KeyringService not available"));
  }

  keyring_service->GetAllAccounts(
      base::BindOnce(&WootzGetAllAccountsFunction::OnGetAllAccounts,
                     weak_factory_.GetWeakPtr()));

  return RespondLater();
}

void WootzGetAllAccountsFunction::OnGetAllAccounts(
    wootz_wallet::mojom::AllAccountsInfoPtr all_accounts_info) {

  base::Value::Dict result;
  base::Value::List accounts_list;

  for (const auto& account : all_accounts_info->accounts) {
    base::Value::Dict account_dict;
    account_dict.Set("address", account->address);
    account_dict.Set("name", account->name);
    account_dict.Set("coin", static_cast<int>(account->account_id->coin));
    accounts_list.Append(std::move(account_dict));
  }

  result.Set("accounts", std::move(accounts_list));
  result.Set("success", true);

  base::Value::List result_list;
  result_list.Append(std::move(result));
  Respond(ArgumentList(std::move(result_list)));
}

// static

void WootzSignMessageFunction::NotifyExtensionOfPendingRequest(
    content::BrowserContext* context) {

  auto* service = wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(
      Profile::FromBrowserContext(context));

  if (!service) {
    LOG(ERROR) << "JANGID: NotifyExtensionOfPendingRequest: Failed to get WootzWalletService";
    return;
  }

  service->GetPendingSignMessageRequests(
      base::BindOnce(&WootzSignMessageFunction::OnGetPendingRequests,
                     base::Unretained(context)));
}

// static
void WootzSignMessageFunction::OnGetPendingRequests(
    content::BrowserContext* context,
    std::vector<wootz_wallet::mojom::SignMessageRequestPtr> requests) {
  if (requests.empty()) {
    LOG(ERROR) << "JANGID: OnGetPendingRequests: No pending requests, returning";
    return;
  }

  const auto& request = requests[0];
  base::Value::Dict request_dict;
  request_dict.Set("id", request->id);
  request_dict.Set("address", request->account_id->address);
  request_dict.Set("origin", request->origin_info->origin_spec);
  request_dict.Set("chainId", request->chain_id);
  request_dict.Set("isEip712", request->sign_data->is_eth_sign_typed_data());

  // Log the request to console
  std::string json_string;
  base::JSONWriter::Write(request_dict, &json_string);
  LOG(ERROR) << "JANGID: OnGetPendingRequests: Pending sign message request: "
             << json_string;

  // Pass the request to the extension using an event
  auto* event_router = EventRouter::Get(context);
  if (event_router) {
    LOG(ERROR) << "JANGID: OnGetPendingRequests: Broadcasting event to extension";
    base::Value::List event_args;
    event_args.Append(std::move(request_dict));

    // Fetch the Wootz Wallet extension ID
    ExtensionId wootz_wallet_extension_id = GetWootzWalletExtensionId(context);

    LOG(ERROR) << "JANGID: Wootz Wallet extension ID: " << wootz_wallet_extension_id;

    if (!wootz_wallet_extension_id.empty()) {
      // Get more information about the extension
      ExtensionRegistry* registry = ExtensionRegistry::Get(context);
      const Extension* extension = registry->GetExtensionById(
          wootz_wallet_extension_id, ExtensionRegistry::ENABLED);

      if (extension) {
        LOG(ERROR) << "JANGID: Extension details:";
        LOG(ERROR) << "  Name: " << extension->name();
        LOG(ERROR) << "  Version: " << extension->version().GetString();
        LOG(ERROR) << "  Description: " << extension->description();
      } else {
        LOG(ERROR) << "JANGID: Extension found but details not available";
      }

      std::unique_ptr<Event> event = std::make_unique<Event>(
          events::WOOTZ_ON_SIGN_MESSAGE_REQUESTED,
          "wootz.onSignMessageRequested",
          std::move(event_args), 
          context,
          std::nullopt,
          GURL(), 
          EventRouter::USER_GESTURE_UNKNOWN,
          mojom::EventFilteringInfo::New());

      LOG(ERROR) << "JANGID: Dispatching event to extension: "
                 << wootz_wallet_extension_id;
      event_router->DispatchEventToExtension(wootz_wallet_extension_id,
                                             std::move(event));

      
      // For opening Extension Automatically
      OpenExtensionsById(wootz_wallet_extension_id);
      
      LOG(ERROR) << "JANGID: Event dispatched successfully";
    } else {
      LOG(ERROR) << "JANGID: Wootz Wallet extension ID not found";

      // Log all enabled extensions
      ExtensionRegistry* registry = ExtensionRegistry::Get(context);
      for (const auto& extension : registry->enabled_extensions()) {
        LOG(ERROR) << "  ID: " << extension->id()
                   << ", Name: " << extension->name();
      }
    }
  } else {
    LOG(ERROR) << "JANGID: OnGetPendingRequests: Failed to get EventRouter";
  }
}
ExtensionFunction::ResponseAction WootzSignMessageFunction::Run() {

  // Check if we have the correct number of arguments
  if (args().size() < 2 || args().size() > 3) {
    return RespondNow(Error("Incorrect number of arguments"));
  }

  // Validate argument types
  if (!args()[0].is_int() || !args()[1].is_bool()) {
    return RespondNow(Error("Invalid argument types"));
  }

  int request_id = args()[0].GetInt();
  bool approved = args()[1].GetBool();
  std::string signature;
  if (args().size() == 3 && args()[2].is_string()) {
    signature = args()[2].GetString();
  }

  auto* service = wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(
      Profile::FromBrowserContext(browser_context()));

  if (!service) {
    return RespondNow(Error("WootzWalletService not available"));
  }

  wootz_wallet::mojom::ByteArrayStringUnionPtr signature_ptr = nullptr;

  std::optional<std::string> error =
      approved ? std::nullopt
               : std::make_optional<std::string>("User rejected");

  service->NotifySignMessageRequestProcessed(approved, request_id,
                                             std::move(signature_ptr), error);

  closeExtensionBottomSheet();

  return RespondNow(WithArguments(base::Value(true)));
}

ExtensionFunction::ResponseAction WootzSignTransactionFunction::Run() {
  LOG(ERROR) << "jangid_sign: Args: " << args().size();
  
  // Validate arguments
  if (args().empty() || !args()[0].is_string() || !args()[1].is_string() ||
      !args()[2].is_int() || !args()[3].is_bool()) {
    LOG(ERROR) << "jangid_sign: Invalid arguments provided";
    return RespondNow(Error("Invalid arguments"));
  }
  
  std::string tx_meta_id = args()[0].GetString();
  std::string chain_id = args()[1].GetString();
  int coin = args()[2].GetInt();
  bool approved = args()[3].GetBool();

  wootz_wallet::mojom::CoinType coin_type = static_cast<wootz_wallet::mojom::CoinType>(coin);
    
  // Get services
  Profile* profile = Profile::FromBrowserContext(browser_context());

  auto* service = wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(profile);
  if (!service || !service->tx_service()) {
    LOG(ERROR) << "jangid_sign: Service not available";
    return RespondNow(Error("Service not available"));
  }

  auto* tx_service = service->tx_service();
  if (!tx_service) {
    LOG(ERROR) << "jangid_sign: Transaction service not available";
    return RespondNow(Error("Transaction service not available"));
  }

  if (approved) {
    LOG(ERROR) << "jangid_sign: Approving transaction...";
    tx_service->ApproveTransaction(
        coin_type,
        chain_id,
        tx_meta_id,
        base::BindOnce(&WootzSignTransactionFunction::OnTransactionSigned,
                       this));
  } else {
    LOG(ERROR) << "jangid_sign: Rejecting transaction...";
    tx_service->RejectTransaction(
        coin_type,
        chain_id,
        tx_meta_id,
        base::BindOnce(&WootzSignTransactionFunction::OnTransactionRejected,
                       this));
  }

  LOG(ERROR) << "jangid_sign: Waiting for callback...";
  closeExtensionBottomSheet();
  return RespondLater();
}

void WootzSignTransactionFunction::OnTransactionSigned(
    bool success,
    wootz_wallet::mojom::ProviderErrorUnionPtr error,
    const std::string& error_message) {
  
  if (!success) {
    LOG(ERROR) << "jangid_sign: Transaction signing failed: " << error_message;
    Respond(Error(error_message));
    return;
  }
  
  LOG(ERROR) << "jangid_sign: Transaction signed successfully";
  closeExtensionBottomSheet();
  Respond(NoArguments());
}

void WootzSignTransactionFunction::OnTransactionRejected(bool success) {  
  if (!success) {
    LOG(ERROR) << "jangid_sign: Transaction rejection failed";
    Respond(Error("Failed to reject transaction"));
    return;
  }
  
  LOG(ERROR) << "jangid_sign: Transaction rejected successfully";
  Respond(NoArguments());
}


void WootzSignSolanaTransactionFunction::NotifyExtensionOfPendingRequest(
    content::BrowserContext* context) {

  auto* service = wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(
      Profile::FromBrowserContext(context));

  if (!service) {
    LOG(ERROR) << "JANGID: NotifyExtensionOfPendingRequest: Failed to get WootzWalletService";
    return;
  }

  service->GetPendingSignTransactionRequests(
      base::BindOnce(&WootzSignSolanaTransactionFunction::OnGetPendingRequests,
                     base::Unretained(context)));
}

void WootzSignSolanaTransactionFunction::OnGetPendingRequests(
    content::BrowserContext* context,
    std::vector<wootz_wallet::mojom::SignTransactionRequestPtr> requests) {

  auto* event_router = EventRouter::Get(context);
  if (!event_router) {
    LOG(ERROR) << "jangid_sign: Event router not available";
    return;
  }

  ExtensionId wootz_wallet_extension_id = GetWootzWalletExtensionId(context);

  base::Value::Dict request_dict;
  request_dict.Set("id", requests[0]->id);
  request_dict.Set("address", requests[0]->from_address);
  request_dict.Set("origin", requests[0]->origin_info->origin_spec);
  request_dict.Set("chainId", requests[0]->chain_id);
  request_dict.Set("encodedMessage", base::Base64Encode(requests[0]->tx_data->get_solana_tx_data()->recent_blockhash));

  base::Value::List event_args;
  event_args.Append(std::move(request_dict));

  std::unique_ptr<Event> event = std::make_unique<Event>(
      events::WOOTZ_ON_SOLANA_SIGN_TRANSACTION_REQUESTED,
      "wootz.onSolanaSignTransactionRequested",
      std::move(event_args), 
      context,
      std::nullopt,
      GURL(), 
      EventRouter::USER_GESTURE_UNKNOWN,
      mojom::EventFilteringInfo::New());

  event_router->DispatchEventToExtension(
      wootz_wallet_extension_id,
      std::move(event));
  
  OpenExtensionsById(wootz_wallet_extension_id);
}

ExtensionFunction::ResponseAction WootzSignSolanaTransactionFunction::Run() {
  LOG(ERROR) << "JANGID: WootzSignSolanaTransactionFunction::Run started";

  int request_id = args()[0].GetInt();
  bool approved = args()[1].GetBool();

  wootz_wallet::mojom::ByteArrayStringUnionPtr signature_ptr = nullptr;

  std::optional<std::string> error =
    approved ? std::nullopt
              : std::make_optional<std::string>("User rejected");

  auto* service = wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(
      Profile::FromBrowserContext(browser_context()));

  if (!service) {
    LOG(ERROR) << "JANGID: Service not available";
    return RespondNow(Error("Service not available"));
  }

  service->NotifySignTransactionRequestProcessed(approved, request_id,
                                             std::move(signature_ptr), error);

  closeExtensionBottomSheet();

  return RespondNow(WithArguments(base::Value(true)));
}

// background worker

const char kWootzJobsListKey[] = "Chrome.Wootzapp.Jobs";
const char kWootzJobResultsKey[] = "Chrome.Wootzapp.JobsResult";

ExtensionFunction::ResponseAction WootzSetJobFunction::Run() {
  if (!args()[0].GetIfString())
    return RespondNow(Error("URL must be a string"));
  std::string url = *args()[0].GetIfString();

  auto prefs = android::shared_preferences::GetChromeSharedPreferences();
  
  std::string jobs_json = prefs.ReadString(kWootzJobsListKey, "[]");
  LOG(ERROR) << "WOOTZ JOBS: " << jobs_json;
  std::optional<base::Value> parsed = base::JSONReader::Read(jobs_json);
  base::Value::List* jobs = parsed->GetIfList();
  if (!jobs) {
    jobs = new base::Value::List();
  }

  jobs->Append(url);

  std::string new_jobs_json;
  base::JSONWriter::Write(base::Value(std::move(*jobs)), &new_jobs_json);
  prefs.WriteString(kWootzJobsListKey, new_jobs_json);

  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WootzRemoveJobFunction::Run() {
  if (!args()[0].GetIfString())
    return RespondNow(Error("URL must be a string"));
  std::string url = *args()[0].GetIfString();

  auto prefs = android::shared_preferences::GetChromeSharedPreferences();
  
  std::string jobs_json = prefs.ReadString(kWootzJobsListKey, "[]");

  LOG(ERROR) << "WOOTZ JOBS: " << jobs_json;

  absl::optional<base::Value> parsed = base::JSONReader::Read(jobs_json);
  base::Value::List* jobs = parsed->GetIfList();
  if (!jobs) return RespondNow(NoArguments());

  for (auto it = jobs->begin(); it != jobs->end(); ) {
    if (it->GetIfString() && *it->GetIfString() == url) {
      it = jobs->erase(it);
    } else {
      ++it;
    }
  }

  std::string new_jobs_json;
  base::JSONWriter::Write(base::Value(std::move(*jobs)), &new_jobs_json);
  prefs.WriteString(kWootzJobsListKey, new_jobs_json);

  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WootzGetJobsFunction::Run() {
  auto prefs = android::shared_preferences::GetChromeSharedPreferences();
  std::string results_json = prefs.ReadString(kWootzJobResultsKey, "[]");
  
  absl::optional<base::Value> parsed = base::JSONReader::Read(results_json);
  if (!parsed || !parsed->is_list()) {
    // Return empty array rather than error
    base::Value::List empty;
    return RespondNow(WithArguments(base::Value(std::move(empty))));
  }

  return RespondNow(WithArguments(std::move(*parsed)));
}

ExtensionFunction::ResponseAction WootzListJobsFunction::Run() {
  auto prefs = android::shared_preferences::GetChromeSharedPreferences();
  std::string jobs_json = prefs.ReadString(kWootzJobsListKey, "[]");
  
  LOG(ERROR) << "WOOTZ JOBS LIST JSON: " << jobs_json;

  absl::optional<base::Value> parsed = base::JSONReader::Read(jobs_json);
  if (!parsed || !parsed->is_list()) {
    base::Value::List empty;
    return RespondNow(WithArguments(base::Value(std::move(empty))));
  }

  return RespondNow(WithArguments(std::move(*parsed)));
}

ExtensionFunction::ResponseAction WootzCleanJobsFunction::Run() {
  auto prefs = android::shared_preferences::GetChromeSharedPreferences();
  
  // Clear both jobs and results
  prefs.RemoveKey(kWootzJobsListKey);
  prefs.RemoveKey(kWootzJobResultsKey);

  return RespondNow(NoArguments());
}


// Implementation of the new WootzGetBrowserInfoFunction
ExtensionFunction::ResponseAction WootzGetBrowserInfoFunction::Run() {
  // Create a dictionary to store the browser information
  base::Value::Dict browser_info;
  
  // Set hardcoded values that identify this as Wootzapp Browser
  browser_info.Set("name", "Wootzapp Browser");
  browser_info.Set("vendor", "Wootzapp Inc.");
  
  // Get the Chrome version information
  const base::android::BuildInfo* build_info = base::android::BuildInfo::GetInstance();
  std::string version = build_info->package_version_name();
  browser_info.Set("version", version);
  
  // Generate a unique build identifier
  // This could be a combination of build date, channel, and other non-spoofable information
  std::string build_id = version + "-wootz-" + build_info->android_build_id();
  browser_info.Set("buildId", build_id);
  
  return RespondNow(WithArguments(std::move(browser_info)));
}

ExtensionFunction::ResponseAction WootzSetBlinksEnabledFunction::Run() {
  // Validate arguments
  if (args().empty() || !args()[0].is_bool()) {
    return RespondNow(Error("Missing or invalid 'enabled' argument"));
  }
  bool enabled = args()[0].GetBool();

  Profile* profile = Profile::FromBrowserContext(browser_context());
  if (!profile) {
    return RespondNow(Error("No profile found"));
  }

  profile->GetPrefs()->SetBoolean(action_url::prefs::kBlinksEnabled, enabled);

  base::Value::Dict result;
  result.Set("success", true);
  return RespondNow(WithArguments(std::move(result)));
}

// ExtensionFunction::ResponseAction
// WootzShowConsentDialogAndMaybeStartServiceFunction::Run() {
//   JNIEnv* env = base::android::AttachCurrentThread();

//     content::WebContents* web_contents = GetSenderWebContents();
//     if (!web_contents) {
//         return RespondNow(Error("Unable to get WebContents"));
//     }

//     // First, check if we already have consent
//     if (!Java_WootzBridge_hasUserConsent(env)) {
//         // If not, show the consent dialog
//         // auto* callback_ptr = new base::OnceCallback<void(bool)>(
//         //     base::BindOnce(&WootzShowConsentDialogAndMaybeStartServiceFunction::OnConsentResult,
//         //                    this));

//         Java_WootzBridge_showConsentDialog(env, reinterpret_cast<jlong>(this),
//                                            web_contents->GetJavaWebContents());
//         return RespondLater();
//     } else {
//         // If we already have consent, start the service directly
//         Java_WootzBridge_startBrowsingDataService(env);
//         return RespondNow(NoArguments());
//     }
// //   base::Value::Dict result;
// //   result.Set("message", "Consent dialog shown and service started");

// //   std::string json_string;
// //   base::JSONWriter::Write(result, &json_string);

// //   return RespondNow(WithArguments(json_string));
// }
// void WootzShowConsentDialogAndMaybeStartServiceFunction::OnConsentDialogResult(JNIEnv* env, jboolean consented) {
//     if (consented) {
//         Java_WootzBridge_startBrowsingDataService(env);
//     }
//     Respond(NoArguments());
// }

std::optional<zk_proof::TlsData> GetBaseDomainTlsData(const std::string& url) {
  GURL gurl(url);
  if (!gurl.is_valid() || !gurl.has_host()) {
    LOG(ERROR) << "Invalid URL format or missing host: " << url;
    return std::nullopt;
  }

  // Vector to store all URLs to try, from most specific to least specific
  std::vector<std::string> urls_to_try;

  // Build base URL (scheme + host)
  std::string base_url = gurl.scheme() + "://" + gurl.host();
  
  // Get the path components
  std::string path = gurl.path();
  std::vector<std::string> components;
  
  // Split path into components, ignoring empty parts
  if (!path.empty()) {
    size_t start = (path[0] == '/') ? 1 : 0;
    size_t end = 0;
    
    while ((end = path.find('/', start)) != std::string::npos) {
      if (end > start) {
        components.push_back(path.substr(start, end - start));
      }
      start = end + 1;
    }
    if (start < path.length()) {
      components.push_back(path.substr(start));
    }
  }

  // Add URLs from most specific to least specific
  // First try the full URL
  urls_to_try.push_back(url);
  if (!base::EndsWith(url, "/")) {
    urls_to_try.push_back(url + "/");
  }

  // Then try base domain
  urls_to_try.push_back(base_url);
  urls_to_try.push_back(base_url + "/");

  // Then try each intermediate path
  std::string cumulative_path = base_url;
  for (const auto& component : components) {
    cumulative_path += "/" + component;
    LOG(ERROR) << "Adding intermediate path: " << cumulative_path;
    urls_to_try.push_back(cumulative_path);
    urls_to_try.push_back(cumulative_path + "/");
  }

  // Try each URL
  for (const auto& try_url : urls_to_try) {
    LOG(ERROR) << "Trying URL for TLS data: " << try_url;
    auto tls_data = zk_proof::TlsDataStore::GetInstance()->GetTlsData(try_url);
    if (tls_data) {
      LOG(ERROR) << "Found TLS data for URL: " << try_url;
      return tls_data;
    }
  }

  LOG(ERROR) << "No TLS data found for any parent URL of: " << url;
  return std::nullopt;
}

ExtensionFunction::ResponseAction WootzGenerateZKProofFunction::Run() {
  // Validate arguments
  LOG(INFO) << "Validating arguments for ZK proof generation.";
  if (args().size() != 2 || !args()[0].is_string() || !args()[1].is_string()) {
    LOG(ERROR) << "Invalid arguments received. Expected URL and content strings.";
    return RespondNow(Error("Invalid arguments. Expected URL and content strings."));
  }

  std::string url = args()[0].GetString();
  std::string content = args()[1].GetString();

  LOG(INFO) << "Starting ZK proof generation for URL: " << url;
  
  // Get TLS data
  auto tls_data = GetBaseDomainTlsData(url);
  if (!tls_data) {
    LOG(ERROR) << "No TLS data found for URL: " << url;
    base::Value::Dict result;
    result.Set("success", false);
    result.Set("error", "No TLS data found for the specified URL");
    
    base::Value::List result_list;
    result_list.Append(std::move(result));
    return RespondNow(WithArguments(std::move(result_list)));
  }
  
  LOG(INFO) << "Generating ZK keys...";
  base::Value::Dict result;
  
  // Generate keys directly
  std::string keys_json = zk_proof::GenerateKeys(
    tls_data->cert_hash,
    tls_data->headers_json,
    content
  );
  
  // Parse the keys JSON
  absl::optional<base::Value> parsed_keys = base::JSONReader::Read(keys_json);
  if (!parsed_keys || !parsed_keys->is_dict()) {
    LOG(ERROR) << "Failed to parse keys JSON";
    result.Set("success", false);
    result.Set("error", "Failed to generate ZK keys");
    
    return RespondNow(WithArguments(std::move(result)));
  }
  
  const base::Value::Dict& keys_dict = parsed_keys->GetDict();
  
  // Extract proving key
  const std::string* pk_b64 = keys_dict.FindString("proving_key_base64");
  if (!pk_b64) {
    LOG(ERROR) << "Proving key not found in response";
    result.Set("success", false);
    result.Set("error", "Proving key not found in generated keys");
  
    return RespondNow(WithArguments(std::move(result)));
  }
  
  // Decode the base64 proving key
  std::string pk_bytes;
  if (!base::Base64Decode(*pk_b64, &pk_bytes)) {
    LOG(ERROR) << "Failed to decode proving key from base64";
    result.Set("success", false);
    result.Set("error", "Failed to decode proving key");
    
    return RespondNow(WithArguments(std::move(result)));
  }
  
  LOG(INFO) << "Generating proof...";
  std::vector<uint8_t> proving_key(pk_bytes.begin(), pk_bytes.end());
  
  // Generate proof
  std::string proof_json = zk_proof::GenerateProofWithKey(
    tls_data->cert_hash,
    tls_data->headers_json,
    content,
    proving_key
  );
  
  // Extract public inputs
  std::string public_inputs_json = zk_proof::ExtractPublicInputs(
    tls_data->cert_hash,
    content
  );
  
  // Get verification key
  const std::string* vk_json = keys_dict.FindString("verification_key_json");
  if (!vk_json) {
    LOG(ERROR) << "Verification key not found in response";
    result.Set("success", false);
    result.Set("error", "Verification key not found in generated keys");
 
    return RespondNow(WithArguments(std::move(result)));
  }
  
  // Create final result
  result.Set("success", !proof_json.empty() && !public_inputs_json.empty());
  
  if (!proof_json.empty() && !public_inputs_json.empty()) {
    result.Set("proof", proof_json);
    result.Set("verificationKey", *vk_json);
    result.Set("publicInputs", public_inputs_json);
    LOG(INFO) << "ZK proof generation completed successfully.";
  } else {
    result.Set("error", "Failed to generate proof or public inputs");
    LOG(ERROR) << "Failed to generate proof or public inputs.";
  }
  
  LOG(INFO) << "Result is successfully generated, responding now";
  
  return RespondNow(WithArguments(std::move(result)));
}

ExtensionFunction::ResponseAction WootzReplaceAdFunction::Run() {
  LOG(INFO) << "WootzReplaceAdFunction::Run started with arguments: " << args().size();
  
  // Validate arguments structure
  if (args().size() < 3 || !args()[0].is_bool() || !args()[1].is_dict() || !args()[2].is_list()) {
    LOG(ERROR) << "WootzReplaceAdFunction: Invalid arguments provided";
    return RespondNow(Error("Invalid arguments format"));
  }

  bool is_enabled = args()[0].GetBool();
  
  Profile* profile = Profile::FromBrowserContext(browser_context());
  if (!profile) {
    LOG(ERROR) << "WootzReplaceAdFunction: No profile found";
    return RespondNow(Error("No profile found"));
  }
  profile->GetPrefs()->SetBoolean(subresource_filter::prefs::kAdBlockGlobalEnabled, is_enabled);
  
  if(!is_enabled) {
    LOG(INFO) << "WootzReplaceAdFunction: AdBlocking is disabled";
    return RespondNow(NoArguments());
  }

  const base::Value::Dict& ad_config = args()[1].GetDict();
  const base::Value::List& selectors_list = args()[2].GetList();
  
  // Extract values from adConfig
  const std::string* ad_unit_path = ad_config.FindString("adUnitPath");
  const std::string* id_prefix = ad_config.FindString("idPrefix");
  const std::string* script_url = ad_config.FindString("scriptUrl");
  const base::Value::List* sizes_list = ad_config.FindList("sizes");
  
  // Validate required fields
  if (!ad_unit_path || !id_prefix || !sizes_list) {
    LOG(ERROR) << "WootzReplaceAdFunction: Missing required adConfig fields";
    return RespondNow(Error("adConfig missing required fields"));
  }

  std::vector<std::string> selectors;
  for (const auto& val : selectors_list) {
    if (val.is_string())
      selectors.push_back(val.GetString());
  }

  base::Value::List converted_sizes;
  for (const auto& size_value : *sizes_list) {
    if (!size_value.is_dict()) continue;
    
    const base::Value::Dict& size_dict = size_value.GetDict();
    
    int width = size_dict.FindInt("width").value_or(-1);
    int height = size_dict.FindInt("height").value_or(-1);
    
    // Skip if either width or height is invalid
    if (width <= 0 || height <= 0) continue;
    
    // Create a pair array [width, height]
    base::Value::List size_pair;
    size_pair.Append(width);
    size_pair.Append(height);
    converted_sizes.Append(std::move(size_pair));
  }

  std::string sizes_json;
  base::JSONWriter::Write(converted_sizes, &sizes_json);

  PrefService* prefs = profile->GetPrefs();
  
  prefs->SetString(subresource_filter::prefs::kAdReplacementAdUnitPath, *ad_unit_path);
  prefs->SetString(subresource_filter::prefs::kAdReplacementIdPrefix, *id_prefix);
  prefs->SetString(subresource_filter::prefs::kAdReplacementScriptUrl, *script_url);
  prefs->SetString(subresource_filter::prefs::kAdReplacementSizes, sizes_json);
  
  // Set selectors
  subresource_filter::prefs::SetAdReplacementSelectors(prefs, selectors);

  LOG(INFO) << "WootzReplaceAdFunction: Successfully stored ad replacement configuration";
  LOG(INFO) << "  - Ad Unit Path: " << *ad_unit_path;
  LOG(INFO) << "  - ID Prefix: " << *id_prefix;
  LOG(INFO) << "  - Selectors count: " << selectors.size();

  return RespondNow(NoArguments());
}

ExtensionFunction::ResponseAction WootzGetPageStateFunction::Run() {
  
  content::WebContents* web_contents = nullptr;
  bool debug_mode = true;
  bool include_hidden = true;
  bool is_background_web_contents = false;
  absl::optional<int> background_web_contents_id;
  
  // Parse options from arguments
  if (args().size() >= 1 && args()[0].is_dict()) {
    const base::Value::Dict& options = args()[0].GetDict();
    debug_mode = options.FindBool("debugMode").value_or(true);
    include_hidden = options.FindBool("includeHidden").value_or(true);
    is_background_web_contents = options.FindBool("isBackgroundWebContents").value_or(false);
    
    if (is_background_web_contents) {
      if (auto id = options.FindInt("backgroundWebContentsId")) {
        background_web_contents_id = id;
        web_contents = WebContentsIdToJavaWebContents(background_web_contents_id.value());
        if (!web_contents) {
          return RespondNow(Error("Background web contents not found"));
        }
        LOG(INFO) << "Successfully got background web contents with ID: " << background_web_contents_id.value();
      } else {
        LOG(ERROR) << "Missing backgroundWebContentsId for background web contents";
        return RespondNow(Error("backgroundWebContentsId is required when isBackgroundWebContents is true"));
      }
    }
  }

  // If not background web contents, get active web contents
  if (!web_contents) {
    web_contents = TabModelList::GetCurrentTabModel()->GetActiveWebContents();
    if (!web_contents) {
      LOG(ERROR) << "No active web contents found for GetPageState";
      return RespondNow(Error("No active tab found"));
    }
  }

  auto* factory = automation::AutomationControllerFactory::FromWebContents(web_contents);
  if (!factory) {
    LOG(ERROR) << "Failed to get AutomationControllerFactory";
    return RespondNow(Error("AutomationControllerFactory not available"));
  }

  auto* controller = factory->GetDriverForFrame(web_contents->GetPrimaryMainFrame());
  if (!controller) {
    LOG(ERROR) << "Failed to get AutomationController";
    return RespondNow(Error("AutomationController not available"));
  }

  controller->GetPageState(
      debug_mode, include_hidden,
      base::BindOnce(&WootzGetPageStateFunction::OnGetPageStateComplete,
                     this));

  return RespondLater();
}

void WootzGetPageStateFunction::OnGetPageStateComplete(bool success, const std::string& state) {
  
  if (!success) {
    LOG(ERROR) << "GetPageState failed in WootzAPI";
    Respond(Error("Failed to get page state"));
    return;
  }

  absl::optional<base::Value> parsed = base::JSONReader::Read(state);
  if (!parsed || !parsed->is_dict()) {
    LOG(ERROR) << "Failed to parse page state JSON in WootzAPI";
    Respond(Error("Failed to parse page state"));
    return;
  };

  base::Value::Dict result;
  result.Set("success", true);
  result.Set("pageState", std::move(*parsed));

  base::Value::List args;
  args.Append(std::move(result));
  
  Respond(ArgumentList(std::move(args)));
}

ExtensionFunction::ResponseAction WootzPerformActionFunction::Run() {
  
  content::WebContents* web_contents = nullptr;

  if (args().size() < 2 || !args()[0].is_string() || !args()[1].is_dict()) {
    LOG(ERROR) << "Invalid arguments for PerformAction";
    return RespondNow(Error("Invalid arguments"));
  }

  const std::string& action = args()[0].GetString();
  const base::Value::Dict& action_params = args()[1].GetDict();
  
  // Handle background web contents
  bool is_background_web_contents = action_params.FindBool("isBackgroundWebContents").value_or(false);
  if (is_background_web_contents) {
    if (auto background_id = action_params.FindInt("backgroundWebContentsId")) {
      web_contents = WebContentsIdToJavaWebContents(background_id.value());
      if (!web_contents) {
        LOG(ERROR) << "Background web contents not found with ID: " << background_id.value();
        return RespondNow(Error("Background web contents not found"));
      }
    } else {
      LOG(ERROR) << "Missing backgroundWebContentsId for background web contents";
      return RespondNow(Error("backgroundWebContentsId is required when isBackgroundWebContents is true"));
    }
  }

  // If not background web contents, get active web contents
  if (!web_contents) {
    web_contents = TabModelList::GetCurrentTabModel()->GetActiveWebContents();
    if (!web_contents) {
      LOG(ERROR) << "No active web contents found for PerformAction";
      return RespondNow(Error("No active tab found"));
    }
  }
  auto* factory = automation::AutomationControllerFactory::FromWebContents(web_contents);
  if (!factory) {
    LOG(ERROR) << "ailed to get AutomationControllerFactory";
    return RespondNow(Error("AutomationControllerFactory not available"));
  }

  auto* controller = factory->GetDriverForFrame(web_contents->GetPrimaryMainFrame());
  if (!controller) {
    LOG(ERROR) << "Failed to get AutomationController";
    return RespondNow(Error("AutomationController not available"));
  }

  controller->PerformAction(
      action, action_params,
      base::BindOnce(&WootzPerformActionFunction::OnActionComplete,
                     this));

  return RespondLater();
}

void WootzPerformActionFunction::OnActionComplete(bool success) {
  
  base::Value::Dict result;
  result.Set("success", success);
  
  if (!success) {
    LOG(ERROR) << "PerformAction failed in WootzAPI";
    result.Set("error", "Action execution failed");
  }
  
  base::Value::List args;
  args.Append(std::move(result));
  Respond(ArgumentList(std::move(args)));
}


ExtensionFunction::ResponseAction WootzSubmitSamlResponseFunction::Run() {
  LOG(ERROR) << "SAML: WootzSubmitSamlResponseFunction::Run() called";
  
  if (args().empty() || !args()[0].is_string()) {
    LOG(ERROR) << "SAML ERROR: Invalid arguments";
    return RespondNow(Error("XML response is required"));
  }

  std::string xml_response = args()[0].GetString();
  
  if (xml_response.empty()) {
    LOG(ERROR) << "SAML ERROR: Empty XML response";
    return RespondNow(Error("XML response cannot be empty"));
  }
  
  // Store in preferences
  Profile* profile = Profile::FromBrowserContext(browser_context());
  if (profile) {
    // Use the proper SAML prefs constant from saml_prefs.h
          profile->GetPrefs()->SetString(saml::prefs::kSamlResponse, xml_response);
    LOG(ERROR) << "SAML: Response stored in preferences";
    
    // **NEW**: Process SAML response automatically
    saml_verifier::SamlVerifier::ProcessNewSamlResponse(profile->GetPrefs());
  }

  LOG(ERROR) << "SAML: Processing complete";

  base::Value::Dict result;
  result.Set("success", true);
  return RespondNow(WithArguments(std::move(result)));
}

ExtensionFunction::ResponseAction WootzCreateBackgroundWebContentsFunction::Run() {
  // Validate arguments
  if (args().size() < 2 || !args()[0].is_int() || !args()[1].is_string()) {
    base::Value::Dict result;
    result.Set("success", false);
    result.Set("error", "Missing or invalid URL argument");
    return RespondNow(WithArguments(std::move(result)));
  }
  int webContentsId = args()[0].GetInt();
  std::string url = args()[1].GetString();


  JNIEnv* env = base::android::AttachCurrentThread();

  Java_WootzAppBackgroundContentService_createBackgroundWebContents(
    env,
    webContentsId,
    base::android::ConvertUTF8ToJavaString(env,url)
  );

  // Get the newly created WebContents
  content::WebContents* web_contents = WebContentsIdToJavaWebContents(webContentsId);
  if (web_contents) {
    // Create and attach the AutomationControllerFactory
    automation::AutomationControllerFactory::CreateForWebContents(web_contents);
    LOG(INFO) << "Created AutomationControllerFactory for background WebContents ID: " << webContentsId;
  } else {
    LOG(ERROR) << "Failed to get WebContents after creation for ID: " << webContentsId;
  }

  base::Value::Dict result;
  result.Set("success", true);
  return RespondNow(WithArguments(std::move(result)));
}

ExtensionFunction::ResponseAction WootzDestroyBackgroundWebContentsFunction::Run() {
  if (args().empty() || !args()[0].is_int()) {
    base::Value::Dict result;
    result.Set("success", false);
    result.Set("error", "Missing or invalid webContentsId argument");
    return RespondNow(WithArguments(std::move(result)));
  }
  int webContentsId = args()[0].GetInt();

  JNIEnv* env = base::android::AttachCurrentThread();
  Java_WootzAppBackgroundContentService_destroyBackgroundWebContents(
      env,
      webContentsId);

  base::Value::Dict result;
  result.Set("success", true);
  result.Set("message", "Background WebContents [" + std::to_string(webContentsId) + "] destroyed via Java service");
  return RespondNow(WithArguments(std::move(result)));
}

// ===== WootzMaskSensitiveElementsFunction =====

WootzMaskSensitiveElementsFunction::WootzMaskSensitiveElementsFunction() = default;
WootzMaskSensitiveElementsFunction::~WootzMaskSensitiveElementsFunction() = default;

ExtensionFunction::ResponseAction WootzMaskSensitiveElementsFunction::Run() {
  LOG(INFO) << "[WootzAPI][Masking] maskSensitiveElements called";
  
  if (args().empty() || !args()[0].is_list()) {
    LOG(ERROR) << "[WootzAPI][Masking] Invalid arguments - expected array of selectors";
    return RespondNow(Error("Expected array of selectors"));
  }

  const base::Value::List& selectors_list = args()[0].GetList();
  std::vector<std::string> selectors;
  
  for (const auto& selector_value : selectors_list) {
    if (selector_value.is_string()) {
      selectors.push_back(selector_value.GetString());
    }
  }
  
  // Extract optional tabId parameter
  int tab_id = -1; // -1 means use active tab
  if (args().size() > 1 && args()[1].is_int()) {
    tab_id = args()[1].GetInt();
    LOG(INFO) << "[WootzAPI][Masking] Using specified tab ID: " << tab_id;
  }
  
  LOG(INFO) << "[WootzAPI][Masking] Got " << selectors.size() << " selectors to mask";
  for (const auto& selector : selectors) {
    LOG(INFO) << "[WootzAPI][Masking] Selector: " << selector;
  }
  
  SendSelectorsToRenderer(selectors, tab_id);
  
  return RespondLater();
}

void WootzMaskSensitiveElementsFunction::OnMaskingComplete(int masked_count) {
  LOG(INFO) << "[WootzAPI][Masking] Masking complete: " << masked_count << " elements masked";
  
  base::Value::Dict result;
  result.Set("success", true);
  result.Set("masked", masked_count);
  
  Respond(WithArguments(std::move(result)));
}


void WootzMaskSensitiveElementsFunction::SendSelectorsToRenderer(const std::vector<std::string>& selectors, int tab_id) {
  LOG(INFO) << "[WootzAPI][Masking] Sending " << selectors.size() << " selectors to renderer via Mojo:";
  for (const auto& selector : selectors) {
    LOG(INFO) << "[WootzAPI][Masking] - " << selector;
  }
  
  content::WebContents* web_contents = nullptr;
  
  if (tab_id != -1) {
    // Get WebContents by specific tab ID
    if (!ExtensionTabUtil::GetTabById(tab_id, browser_context(), 
                                      include_incognito_information(), 
                                      &web_contents)) {
      LOG(ERROR) << "[WootzAPI][Masking] Failed to get WebContents for tab ID: " << tab_id;
      OnMaskingComplete(0);
      return;
    }
    LOG(INFO) << "[WootzAPI][Masking] Targeting specific tab ID: " << tab_id;
  } else {
    // Use the same approach as WootzReplaceElementFunction - get active tab directly
    web_contents = TabModelList::GetCurrentTabModel()->GetActiveWebContents();
    if (!web_contents) {
      LOG(ERROR) << "[WootzAPI][Masking] Unable to get WebContents";
      OnMaskingComplete(0);
      return;
    }
    LOG(INFO) << "[WootzAPI][Masking] Targeting active tab (no tab ID provided)";
  }
  
  LOG(INFO) << "[WootzAPI][Masking] Targeting tab with URL: " << web_contents->GetVisibleURL().spec();
  
  // Use factory pattern like WootzReplaceElementFunction
  auto* factory = sensitive_masking::ContentSensitiveMaskingDriverFactory::FromWebContents(web_contents);
  if (!factory) {
    LOG(ERROR) << "[WootzAPI][Masking] ContentSensitiveMaskingDriverFactory not available";
    OnMaskingComplete(0);
    return;
  }
  
  // Get driver for the main frame
  auto* driver = factory->GetDriverForFrame(web_contents->GetPrimaryMainFrame());
  if (!driver) {
    LOG(ERROR) << "[WootzAPI][Masking] Failed to get masking driver for main frame";
    OnMaskingComplete(0);
    return;
  }
  
  // Send selectors using the factory's driver
  driver->UpdateMaskingSelectorsDirectly(selectors,
    base::BindOnce(&WootzMaskSensitiveElementsFunction::OnMaskingComplete,
                   weak_factory_.GetWeakPtr()));
}
  
ExtensionFunction::ResponseAction WootzChangeWootzAppSearchConfigurationFunction::Run(){
  if(args().size() != 3 || !args()[0].is_string() || !args()[1].is_string() || !args()[2].is_string()) {
    LOG(ERROR)<<"Invalid Arguments";
    return RespondNow(Error("Invalid arguments"));
  }

  std::u16string search_engine_keyword = base::UTF8ToUTF16(args()[0].GetString());
  std::string ai_model_name = args()[1].GetString();
  std::string api_key = args()[2].GetString();


  Profile* profile = Profile::FromBrowserContext(browser_context());
  if (!profile) {
    LOG(ERROR) << "WootzReplaceAdFunction: No profile found";
    return RespondNow(Error("No profile found"));
  }

  TemplateURLService* template_url_service= TemplateURLServiceFactory::GetForProfile(profile);

  if(!template_url_service) {
    return RespondNow(Error("No TemplateURLService found"));
  }

  TemplateURL* template_url = template_url_service->GetTemplateURLForKeyword(search_engine_keyword);
  template_url_service->SetUserSelectedDefaultSearchProvider(
      template_url, search_engines::ChoiceMadeLocation::kOther);

  auto prefs = android::shared_preferences::GetChromeSharedPreferences();

  prefs.WriteString(kWootzAppSearchAIModelName, ai_model_name);
  prefs.WriteString(kWootzAppSearchAIModelAPIKey, api_key);

  return RespondNow(NoArguments());
}

}  // namespace extensions

void JNI_WootzBridge_OnConsentResult(JNIEnv* env, jboolean consented){
  // Implement the consent result handling here
  LOG(INFO) << "DKT: Consent result: " << (consented ? "true" : "false");
}

void JNI_WootzBridge_OnDropdownButtonClicked(JNIEnv* env, const base::android::JavaParamRef<jstring>& selectedFeature, const base::android::JavaParamRef<jstring>& extensionId, const base::android::JavaParamRef<jstring>& extensionName) {
  std::string feature = base::android::ConvertJavaStringToUTF8(env, selectedFeature);
  std::string extId = base::android::ConvertJavaStringToUTF8(env, extensionId);
  std::string extName = base::android::ConvertJavaStringToUTF8(env, extensionName);

  LOG(ERROR) << "JNI: OnDropdownButtonClicked called with feature: " << feature;
  LOG(ERROR) << "JNI: Extension ID: " << extId;
  LOG(ERROR) << "JNI: Extension Name: " << extName;
  
  ProfileManager* profile_manager = g_browser_process->profile_manager();
  if (!profile_manager) {
    LOG(ERROR) << "JNI: ProfileManager not available";
    return;
  }
  Profile* profile = profile_manager->GetPrimaryUserProfile();
  if (!profile) {
    LOG(ERROR) << "JNI: Primary user profile not available";
    return;
  }
  extensions::WootzAPI* wootz_api = extensions::WootzAPI::GetFactoryInstance()->Get(profile);
  if (!wootz_api) {
    LOG(ERROR) << "JNI: WootzAPI instance not available";
    return;
  }
  wootz_api->OnDropdownButtonClicked(feature, extId, extName);
}

// extern "C" JNIEXPORT void JNICALL
// Java_org_chromium_chrome_browser_extensions_WootzBridge_nativeOnConsentDialogResult(
//     JNIEnv* env,
//     jclass clazz,
//     jlong native_ptr,
//     jboolean consented) {
//   auto* function = reinterpret_cast<extensions::WootzShowConsentDialogAndMaybeStartServiceFunction*>(native_ptr);
//   function->OnConsentDialogResult(env, consented);
// }
