import 'dart:typed_data';

import 'platform_impl/sodium_utils_base.dart';
import 'platform_impl/sodium_utils_stub.dart'
    if (dart.library.io) 'platform_impl/sodium_utils_io.dart'
    if (dart.library.html) 'platform_impl/sodium_utils_web.dart';

class SodiumUtils {
  final SodiumUtilsImpl _impl;

  SodiumUtils() : _impl = SodiumUtilsImpl();

  Future<void> init() => _impl.init();

  Uint8List close(Uint8List message, Uint8List nonce, Uint8List keyBytes) =>
      _impl.close(message, nonce, keyBytes);

  Uint8List nonce() => _impl.nonce();

  Uint8List open(Uint8List nonceAndCiphertext, Uint8List key) =>
      _impl.open(nonceAndCiphertext, key);

  KeyPair publicKey(Uint8List sk) => _impl.publicKey(sk);

  Uint8List pwhash(String passphrase, Uint8List salt) =>
      _impl.pwhash(passphrase, salt);

  Uint8List rand(int length) => _impl.rand(length);

  Uint8List salt() => _impl.salt();

  Uint8List sign(Uint8List simpleHash, Uint8List key) =>
      _impl.sign(simpleHash, key);

  KeyPair cryptoSignSeedKeypair(Uint8List seed) =>
      _impl.cryptoSignSeedKeypair(seed);

  Uint8List cryptoSignDetached(Uint8List message, Uint8List key) =>
      _impl.cryptoSignDetached(message, key);

  getSodium() => _impl.getSodium();
}
