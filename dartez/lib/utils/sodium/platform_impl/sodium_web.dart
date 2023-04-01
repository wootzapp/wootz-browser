@JS('sodium')
library sodium;

import 'dart:typed_data';

import 'package:js/js.dart';

external num get crypto_pwhash_SALTBYTES;
external num get crypto_box_SEEDBYTES;
external num get crypto_pwhash_ALG_ARGON2I13;
external num get crypto_box_NONCEBYTES;
external num get crypto_secretbox_NONCEBYTES;

external Uint8List crypto_pwhash(num outlen, Uint8List passwd, Uint8List salt,
    num opslimit, num memlimit, int alg);
external Uint8List crypto_secretbox_easy(Uint8List m, Uint8List n, Uint8List k);
external Uint8List crypto_secretbox_open_easy(
    Uint8List c, Uint8List n, Uint8List k);
external Uint8List crypto_sign_detached(Uint8List m, Uint8List sk);
external Uint8List crypto_sign_ed25519_sk_to_seed(Uint8List sk);

external Uint8List randombytes_buf(num length);

external _KeyPair crypto_sign_seed_keypair(Uint8List seed);

@JS()
@anonymous
class _KeyPair {
  external factory _KeyPair(
      {required Uint8List publicKey,
      required Uint8List privateKey,
      required String keyType});
  external Uint8List get publicKey;
  external Uint8List get privateKey;
  external String? get keyType;
}
