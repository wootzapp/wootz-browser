import 'dart:typed_data';

import 'package:dartez/utils/sodium/platform_impl/sodium_utils_base.dart';
import 'sodium_web.dart' as sodium;

class SodiumUtilsImpl extends SodiumUtilsBase {
  @override
  Future<void> init() async {
    return;
  }

  @override
  Uint8List close(Uint8List message, Uint8List nonce, Uint8List keyBytes) {
    return sodium.crypto_secretbox_easy(message, nonce, keyBytes);
  }

  @override
  Uint8List nonce() {
    return rand(sodium.crypto_box_NONCEBYTES as int);
  }

  @override
  Uint8List open(Uint8List nonceAndCiphertext, Uint8List key) {
    var nonce = nonceAndCiphertext.sublist(
        0, sodium.crypto_secretbox_NONCEBYTES as int);
    var ciphertext =
        nonceAndCiphertext.sublist(sodium.crypto_secretbox_NONCEBYTES as int);

    return sodium.crypto_secretbox_open_easy(ciphertext, nonce, key);
  }

  @override
  KeyPair publicKey(Uint8List sk) {
    var seed = sodium.crypto_sign_ed25519_sk_to_seed(sk);
    var temp = sodium.crypto_sign_seed_keypair(seed);
    return KeyPair(temp.publicKey, temp.privateKey);
  }

  @override
  Uint8List pwhash(String passphrase, Uint8List salt) {
    return sodium.crypto_pwhash(
        sodium.crypto_box_SEEDBYTES as int,
        Uint8List.fromList(passphrase.codeUnits),
        salt,
        4,
        33554432,
        sodium.crypto_pwhash_ALG_ARGON2I13 as int);
  }

  @override
  Uint8List rand(int length) {
    return sodium.randombytes_buf(length);
  }

  @override
  Uint8List salt() {
    return Uint8List.fromList(
        rand(sodium.crypto_pwhash_SALTBYTES as int).toList());
  }

  @override
  Uint8List sign(Uint8List simpleHash, Uint8List key) {
    return sodium.crypto_sign_detached(simpleHash, key);
  }

  @override
  getSodium() {
    return null;
  }

  @override
  KeyPair cryptoSignSeedKeypair(Uint8List seed) {
    var temp = sodium.crypto_sign_seed_keypair(seed);
    return KeyPair(temp.publicKey, temp.privateKey);
  }

  @override
  Uint8List cryptoSignDetached(Uint8List message, Uint8List key) {
    return sodium.crypto_sign_detached(message, key);
  }
}
