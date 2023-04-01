import 'dart:typed_data';
import 'package:dartez/dartez.dart';

class CryptoUtils {
  static Uint8List generateSaltForPwHash() {
    return Dartez.sodiumUtils.salt();
  }

  static Uint8List encryptMessage(
      Uint8List message, String passphrase, Uint8List salt) {
    var keyBytes = Dartez.sodiumUtils.pwhash(passphrase, salt);
    var nonce = Dartez.sodiumUtils.nonce();
    var s = Dartez.sodiumUtils.close(message, nonce, keyBytes);

    return new Uint8List.fromList(nonce.toList() + s.toList());
  }

  static Uint8List decryptMessage(message, passphrase, salt) {
    var keyBytes = Dartez.sodiumUtils.pwhash(passphrase, salt);
    return Dartez.sodiumUtils.open(message, keyBytes);
  }

  static Uint8List signDetached(Uint8List simpleHash, Uint8List key) {
    return Dartez.sodiumUtils.sign(simpleHash, key);
  }
}
