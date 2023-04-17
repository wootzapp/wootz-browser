import 'dart:typed_data';

import 'package:bip39/src/bip39_base.dart' as bip39;

class Mnemonic {
  /// Generate bip39 mnemonic
  static String generate({int strength = 128}) =>
      bip39.generateMnemonic(strength: strength);

  /// Create root seed from mnemonic
  static Uint8List toSeed(String mnemonic) => bip39.mnemonicToSeed(mnemonic);
}
