import 'dart:typed_data';

import 'package:dartez/utils/sodium/platform_impl/sodium_utils_base.dart';
import 'package:flutter_sodium/flutter_sodium.dart' as sodium;

class SodiumUtilsImpl extends SodiumUtilsBase {
  @override
  Future<void> init() async {
    return sodium.Sodium.init();
  }

  @override
  Uint8List close(Uint8List message, Uint8List nonce, Uint8List keyBytes) {
    return sodium.Sodium.cryptoSecretboxEasy(message, nonce, keyBytes);
  }

  @override
  Uint8List nonce() {
    return rand(sodium.Sodium.cryptoBoxNoncebytes);
  }

  @override
  Uint8List open(Uint8List nonceAndCiphertext, Uint8List key) {
    var nonce =
        nonceAndCiphertext.sublist(0, sodium.Sodium.cryptoSecretboxNoncebytes);
    var ciphertext =
        nonceAndCiphertext.sublist(sodium.Sodium.cryptoSecretboxNoncebytes);

    return sodium.Sodium.cryptoSecretboxOpenEasy(ciphertext, nonce, key);
  }

  @override
  KeyPair publicKey(Uint8List sk) {
    var seed = sodium.Sodium.cryptoSignEd25519SkToSeed(sk);
    var temp = sodium.Sodium.cryptoSignSeedKeypair(seed);
    return KeyPair(temp.pk, temp.sk);
  }

  @override
  Uint8List pwhash(String passphrase, Uint8List salt) {
    return sodium.Sodium.cryptoPwhash(
        sodium.Sodium.cryptoBoxSeedbytes,
        Uint8List.fromList(passphrase.codeUnits),
        salt,
        4,
        33554432,
        sodium.Sodium.cryptoPwhashAlgArgon2i13);
  }

  @override
  Uint8List rand(int length) {
    return sodium.Sodium.randombytesBuf(length);
  }

  @override
  Uint8List salt() {
    return Uint8List.fromList(
        rand(sodium.Sodium.cryptoPwhashSaltbytes).toList());
  }

  @override
  Uint8List sign(Uint8List simpleHash, Uint8List key) {
    return sodium.Sodium.cryptoSignDetached(simpleHash, key);
  }

  @override
  getSodium() {
    return sodium.Sodium;
  }

  @override
  KeyPair cryptoSignSeedKeypair(Uint8List seed) {
    var temp = sodium.Sodium.cryptoSignSeedKeypair(seed);
    return KeyPair(temp.pk, temp.sk);
  }

  @override
  Uint8List cryptoSignDetached(Uint8List message, Uint8List key) {
    return sodium.Sodium.cryptoSignDetached(message, key);
  }
}
