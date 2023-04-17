import 'dart:typed_data';

abstract class SodiumUtilsBase {

  Future<void> init();

  getSodium();

  Uint8List rand(int length);

  Uint8List salt();

  Uint8List pwhash(String passphrase, Uint8List salt);

  Uint8List nonce();

  Uint8List close(Uint8List message, Uint8List nonce, Uint8List keyBytes);

  Uint8List open(Uint8List nonceAndCiphertext, Uint8List key);

  Uint8List sign(Uint8List simpleHash, Uint8List key);

  KeyPair cryptoSignSeedKeypair(Uint8List seed);

  Uint8List cryptoSignDetached(Uint8List message, Uint8List key);

  KeyPair publicKey(Uint8List sk);
}

class KeyPair {
  final Uint8List pk;
  final Uint8List sk;

  KeyPair(this.pk, this.sk);
}
