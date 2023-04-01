import 'dart:typed_data';

import 'package:dartez/utils/sodium/platform_impl/sodium_utils_base.dart';

class SodiumUtilsImpl extends SodiumUtilsBase {
  @override
  Future<void> init() {
    throw Exception("Stub implementation");
  }

  @override
  Uint8List close(Uint8List message, Uint8List nonce, Uint8List keyBytes) {
    throw Exception("Stub implementation");
  }

  @override
  Uint8List nonce() {
    throw Exception("Stub implementation");
  }

  @override
  Uint8List open(Uint8List nonceAndCiphertext, Uint8List key) {
    throw Exception("Stub implementation");
  }

  @override
  KeyPair publicKey(Uint8List sk) {
    throw Exception("Stub implementation");
  }

  @override
  Uint8List pwhash(String passphrase, Uint8List salt) {
    throw Exception("Stub implementation");
  }

  @override
  Uint8List rand(int length) {
    throw Exception("Stub implementation");
  }

  @override
  Uint8List salt() {
    throw Exception("Stub implementation");
  }

  @override
  Uint8List sign(Uint8List simpleHash, Uint8List key) {
    throw Exception("Stub implementation");
  }

  @override
  getSodium() {
    throw Exception("Stub implementation");
  }

  @override
  KeyPair cryptoSignSeedKeypair(Uint8List seed) {
    throw Exception("Stub implementation");
  }

  @override
  Uint8List cryptoSignDetached(Uint8List message, Uint8List key) {
    throw Exception("Stub implementation");
  }
}
