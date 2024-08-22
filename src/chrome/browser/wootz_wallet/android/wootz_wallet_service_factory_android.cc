/* Copyright (c) 2021 The Wootz Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "chrome/browser/wootz_wallet/wootz_wallet_service_factory.h"

#include "base/android/jni_android.h"
#include "chrome/android/chrome_jni_headers/WootzWalletServiceFactory_jni.h"
#include "components/wootz_wallet/browser/wootz_wallet_service.h"
#include "components/wootz_wallet/common/wootz_wallet.mojom.h"
#include "chrome/browser/profiles/profile.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"

namespace chrome {
namespace android {

namespace {
template <class T>
jlong BindWalletService(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& profile_android) {
  auto* profile = Profile::FromJavaObject(profile_android);
  if (auto* wootz_wallet_service =
          wootz_wallet::WootzWalletServiceFactory::GetServiceForContext(
              profile)) {
    mojo::PendingRemote<T> pending;
    wootz_wallet_service->Bind(pending.InitWithNewPipeAndPassReceiver());
    return static_cast<jlong>(pending.PassPipe().release().value());
  }
  return static_cast<jlong>(mojo::kInvalidHandleValue);
}
}  // namespace

static jlong JNI_WootzWalletServiceFactory_GetInterfaceToWootzWalletService(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& profile_android) {
  return BindWalletService<wootz_wallet::mojom::WootzWalletService>(
      env, profile_android);
}

static jlong JNI_WootzWalletServiceFactory_GetInterfaceToWootzWalletP3A(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& profile_android) {
  return BindWalletService<wootz_wallet::mojom::WootzWalletP3A>(
      env, profile_android);
}

static jlong JNI_WootzWalletServiceFactory_GetInterfaceToJsonRpcService(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& profile_android) {
  return BindWalletService<wootz_wallet::mojom::JsonRpcService>(
      env, profile_android);
}

static jlong JNI_WootzWalletServiceFactory_GetInterfaceToKeyringService(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& profile_android) {
  return BindWalletService<wootz_wallet::mojom::KeyringService>(
      env, profile_android);
}

static jlong JNI_WootzWalletServiceFactory_GetInterfaceToTxService(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& profile_android) {
  return BindWalletService<wootz_wallet::mojom::TxService>(env,
                                                           profile_android);
}

static jlong JNI_WootzWalletServiceFactory_GetInterfaceToEthTxManagerProxy(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& profile_android) {
  return BindWalletService<wootz_wallet::mojom::EthTxManagerProxy>(
      env, profile_android);
}

static jlong JNI_WootzWalletServiceFactory_GetInterfaceToSolanaTxManagerProxy(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& profile_android) {
  return BindWalletService<wootz_wallet::mojom::SolanaTxManagerProxy>(
      env, profile_android);
}

}  // namespace android
}  // namespace chrome
