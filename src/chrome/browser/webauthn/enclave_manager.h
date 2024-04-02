// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WEBAUTHN_ENCLAVE_MANAGER_H_
#define CHROME_BROWSER_WEBAUTHN_ENCLAVE_MANAGER_H_

#include <deque>
#include <optional>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/functional/callback_forward.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/observer_list.h"
#include "base/sequence_checker.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/trusted_vault/trusted_vault_connection.h"
#include "device/fido/enclave/types.h"
#include "services/network/public/mojom/network_context.mojom-forward.h"

namespace crypto {
class RefCountedUserVerifyingSigningKey;
}  // namespace crypto

namespace network {
class SharedURLLoaderFactory;
}  // namespace network

namespace signin {
class IdentityManager;
class PrimaryAccountAccessTokenFetcher;
}  // namespace signin

namespace unexportable_keys {
class RefCountedUnexportableSigningKey;
}

namespace webauthn_pb {
class EnclaveLocalState;
class EnclaveLocalState_User;
class EnclaveLocalState_WrappedPIN;
}  // namespace webauthn_pb

// EnclaveManager stores and manages the passkey enclave state. One instance
// exists per-profile, owned by `EnclaveManagerFactory`.
//
// The state exposed from this class is per-primary-account. This class watches
// the `IdentityManager` and, when the primary account changes, the result of
// functions like `is_registered` will suddenly change too. If an account is
// removed from the cookie jar (and it's not primary) then state for that
// account will be erased. Any pending operations will be canceled when the
// primary account changes and their callback will be run with `false`.
//
// When `is_ready` is true then this class can produce wrapped security domain
// secrets and signing callbacks to use to perform passkey operations with the
// enclave, which is the ultimate point of this class.
class EnclaveManager : public KeyedService {
 public:
  // Many actions report results using a `Callback`. The boolean argument
  // is true if the operation is successful and false otherwise.
  // These callbacks never hairpin. (I.e. are never called before the function
  // that they were passed to returns.)
  using Callback = base::OnceCallback<void(bool)>;

  struct StoreKeysArgs;
  class Observer : public base::CheckedObserver {
   public:
    // OnKeyStores is called when MagicArch provides keys to the EnclaveManager
    // by calling `StoreKeys`.
    virtual void OnKeysStored() = 0;
  };

  EnclaveManager(
      const base::FilePath& base_dir,
      signin::IdentityManager* identity_manager,
      raw_ptr<network::mojom::NetworkContext> network_context,
      scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory);
  ~EnclaveManager() override;
  EnclaveManager(const EnclaveManager&) = delete;
  EnclaveManager(const EnclaveManager&&) = delete;

  // Returns true if there are no current operations pending.
  bool is_idle() const;
  // Returns true if the persistent state has been loaded from the disk. (Or
  // else the loading failed and an empty state is being used.)
  bool is_loaded() const;
  // Returns true if the current user has been registered with the enclave.
  bool is_registered() const;
  // Returns true if `StoreKeys` has been called and thus `AddDeviceToAccount`
  // or `AddDeviceAndPINToAccount` can be called.
  bool has_pending_keys() const;
  // Returns true if the current user has joined the security domain and has one
  // or more wrapped security domain secrets available. (This implies
  // `is_registered`.)
  bool is_ready() const;
  // Returns the number of times that `StoreKeys` has been called.
  unsigned store_keys_count() const;

  // Load the persisted state from disk. Harmless to call if `is_loaded`.
  void Load(base::OnceClosure closure);
  // Register with the enclave if not already registered.
  void RegisterIfNeeded(Callback callback);
  // Set up an account with a newly-created PIN.
  void SetupWithPIN(std::string pin, Callback callback);
  // Adds the current device to the security domain. Only valid to call after
  // `StoreKeys` has been called and thus `has_pending_keys` returns true. If
  // `serialized_wrapped_pin` has a value then it is taken to be the contents
  // of a WrappedPIN protobuf for the current GPM PIN. If you want to add a
  // new PIN to the account, see `AddDeviceAndPINToAccount`.
  //
  // Returns false if `serialized_wrapped_pin` fails to parse and true
  // otherwise.
  bool AddDeviceToAccount(std::optional<std::string> serialized_wrapped_pin,
                          Callback callback);
  // Adds the current device, and a GPM PIN, to the security domain. Only valid
  // to call after `StoreKeys` has been called and thus `has_pending_keys`
  // returns true.
  void AddDeviceAndPINToAccount(std::string pin, Callback callback);

  // Get a callback to sign with the registered "hw" key. Only valid to call if
  // `is_ready`.
  device::enclave::SigningCallback HardwareKeySigningCallback();
  // Get a callback to sign with the registered "uv" key. Only valid to call if
  // `is_ready`.
  device::enclave::SigningCallback UserVerifyingKeySigningCallback();
  // Fetch a wrapped security domain secret for the given epoch. Only valid to
  // call if `is_ready`.
  std::optional<std::vector<uint8_t>> GetWrappedSecret(int32_t version);
  // Fetch all wrapped security domain secrets, for when it's unknown which one
  // a WebauthnCredentialSpecifics will need. Only valid to call if `is_ready`.
  std::vector<std::vector<uint8_t>> GetWrappedSecrets();
  // Get the version and value of the current wrapped secret. Only valid to call
  // if `is_ready`.
  std::pair<int32_t, std::vector<uint8_t>> GetCurrentWrappedSecret();
  // Returns true if a wrapped PIN is available for the current user. Requires
  // `is_ready`.
  bool has_wrapped_pin() const;
  // Returns true if the wrapped PIN is arbitrary. I.e. is a general
  // alphanumeric string. If false then the wrapped PIN is a 6-digit numeric
  // string. Requires `has_wrapped_pin` to be true.
  bool wrapped_pin_is_arbitrary() const;
  // Returns a copy of the wrapped PIN for passing to `MakeClaimedPINSlowly`.
  // Requires `has_wrapped_pin`.
  std::unique_ptr<webauthn_pb::EnclaveLocalState_WrappedPIN> GetWrappedPIN();

  // Enumerates the types of user verifying signing keys that the EnclaveManager
  // might have for the currently signed-in user.
  enum class UvKeyState {
    // No UV key present; perform user verification using a PIN.
    kNone,
    // A UV key is present and `UserVerifyingKeySigningCallback` will return a
    // signing callback where the UI is handled by the system.
    kUsesSystemUI,
    // A UV key is present and `UserVerifyingKeySigningCallback` will return a
    // valid callback. However, Chrome UI needs to be shown in order to collect
    // biometrics.
    kUsesChromeUI,
  };
  UvKeyState uv_key_state() const;

  // Get an access token for contacting the enclave.
  std::unique_ptr<signin::PrimaryAccountAccessTokenFetcher> GetAccessToken(
      base::OnceCallback<void(std::optional<std::string>)> callback);

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  // This function is called by the MagicArch integration when the user
  // successfully completes recovery.
  void StoreKeys(const std::string& gaia_id,
                 std::vector<std::vector<uint8_t>> keys,
                 int last_key_version);

  // Slowly compute a PIN claim for the given PIN for submission to the enclave.
  static std::unique_ptr<device::enclave::ClaimedPIN> MakeClaimedPINSlowly(
      std::string pin,
      std::unique_ptr<webauthn_pb::EnclaveLocalState_WrappedPIN> wrapped_pin);

  // If background processes need to be stopped then return true and call
  // `on_stop` when stopped. Otherwise return false.
  bool RunWhenStoppedForTesting(base::OnceClosure on_stop);

  webauthn_pb::EnclaveLocalState& local_state_for_testing() const;

  // These methods get internal URLs so that tests can reply when they're
  // fetched.
  static std::string_view recovery_key_store_url_for_testing();
  static std::string_view recovery_key_store_cert_url_for_testing();
  static std::string_view recovery_key_store_sig_url_for_testing();

 private:
  class StateMachine;
  class IdentityObserver;
  struct PendingAction;
  friend class StateMachine;

  // Starts a `StateMachine` to process the current request.
  void Act();

  // Is called when reading the state file from disk has completed.
  // (Successfully or otherwise.)
  void LoadComplete(std::optional<std::string> contents);

  // Called when `identity_observer_` reports a change in the signed-in state of
  // the Profile. Also called once the local state has finished loading. In
  // that case `is_post_load` will be false and any "change" in primary
  // identity doesn't cause a reset.
  void HandleIdentityChange(bool is_post_load = false);

  // Called when a `StateMachine` has stopped (or needs to stop).
  void Stopped();

  // Called when the primary user changes and all pending actions are stopped.
  void CancelAllActions();

  // Can be called at any point to serialise the current value of `local_state_`
  // to disk. Only a single write happens at a time. If a write is already
  // happening, the request will be queued. If a request is already queued, this
  // call will replace that queued write.
  void WriteState(webauthn_pb::EnclaveLocalState* new_state);
  void DoWriteState(std::string serialized);
  void WriteStateComplete(bool success);

  // Accessors for the HW and UV keys, invoking the supplied callbacks with
  // the result. These can complete synchronously if the respective key is
  // cached, or will attempt to load them asynchronously otherwise.
  // If the key fails to load, the callback will be invoked with nullptr and
  // the device's enclave registration will be reset.
  void GetHardwareKeyForSignature(
      base::OnceCallback<void(
          scoped_refptr<unexportable_keys::RefCountedUnexportableSigningKey>)>
          callback);
  void GetUserVerifyingKeyForSignature(
      base::OnceCallback<void(
          scoped_refptr<crypto::RefCountedUserVerifyingSigningKey>)> callback);

  const base::FilePath file_path_;
  const raw_ptr<signin::IdentityManager> identity_manager_;
  const raw_ptr<network::mojom::NetworkContext> network_context_;
  const scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory_;
  const std::unique_ptr<trusted_vault::TrustedVaultConnection>
      trusted_vault_conn_;

  std::unique_ptr<webauthn_pb::EnclaveLocalState> local_state_;
  bool loading_ = false;
  raw_ptr<const webauthn_pb::EnclaveLocalState_User> user_ = nullptr;
  std::unique_ptr<CoreAccountInfo> primary_account_info_;
  std::unique_ptr<IdentityObserver> identity_observer_;

  std::optional<std::string> pending_write_;
  bool currently_writing_ = false;
  base::OnceClosure write_finished_callback_;

  std::unique_ptr<StoreKeysArgs> pending_keys_;
  std::unique_ptr<StateMachine> state_machine_;
  std::vector<base::OnceClosure> load_callbacks_;
  std::deque<std::unique_ptr<PendingAction>> pending_actions_;

  // Allow keys to persist across sequences because loading them is slow.
  scoped_refptr<crypto::RefCountedUserVerifyingSigningKey> user_verifying_key_;
  scoped_refptr<unexportable_keys::RefCountedUnexportableSigningKey>
      hardware_key_;

  unsigned store_keys_count_ = 0;

  base::ObserverList<Observer> observer_list_;

  SEQUENCE_CHECKER(sequence_checker_);
  base::WeakPtrFactory<EnclaveManager> weak_ptr_factory_{this};
};

#endif  // CHROME_BROWSER_WEBAUTHN_ENCLAVE_MANAGER_H_
