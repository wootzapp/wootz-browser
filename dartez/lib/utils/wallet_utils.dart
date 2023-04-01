import 'package:convert/convert.dart';
import 'package:dartez/chain/tezos/tezos_message_utils.dart';
import 'package:dartez/dartez.dart';
import 'package:dartez/helper/generateKeys.dart';
import 'package:dartez/src/soft-signer/soft_signer.dart';
import 'package:dartez/utils/sodium/platform_impl/sodium_utils_base.dart';
import 'package:flutter/foundation.dart';
import 'package:secp256k1/secp256k1.dart';

class WalletUtils {
  getKeysFromMnemonic(Uint8List seedLength32, SignerCurve signerCurve) {
    if (signerCurve == SignerCurve.ED25519) {
      KeyPair keyPair = Dartez.sodiumUtils.cryptoSignSeedKeypair(seedLength32);
      String skKey = GenerateKeys.readKeysWithHint(
          keyPair.sk, GenerateKeys.keyPrefixes[PrefixEnum.edsk]!);
      return getKeysFromPrivateKey(skKey, signerCurve);
      // String pkKey = GenerateKeys.readKeysWithHint(
      //     keyPair.pk, GenerateKeys.keyPrefixes[PrefixEnum.edpk]!);
      // String pkKeyHash = GenerateKeys.computeKeyHash(
      //     keyPair.pk, GenerateKeys.keyPrefixes[PrefixEnum.tz1]!);
      // return [skKey, pkKey, pkKeyHash];
    } else if (signerCurve == SignerCurve.SECP256K1) {
      PrivateKey secpPk = PrivateKey.fromHex(hex.encode(seedLength32));
      var pk = GenerateKeys.readKeysWithHint(
          Uint8List.fromList(hex.decode(secpPk.toHex())),
          GenerateKeys.keyPrefixes[PrefixEnum.spsk]!);
      return getKeysFromPrivateKey(pk, signerCurve);
      // Uint8List yArray = GenerateKeys().serializeBigInt(secpPk.publicKey.Y);
      // var prefixVal =
      //     yArray[yArray.length - 1] % 2 == 1 ? 3 : 2; // Y odd / even
      // var pad = new List.filled(32, 0); // Zero-padding
      // var _publicKey = Uint8List.fromList([prefixVal] +
      //     (pad + GenerateKeys().serializeBigInt(secpPk.publicKey.X).toList())
      //         .reversed
      //         .take(32)
      //         .toList());
      // var publicKey = GenerateKeys.readKeysWithHint(
      //     _publicKey, GenerateKeys.keyPrefixes[PrefixEnum.sppk]!);
      // var pkH = GenerateKeys.computeKeyHash(
      //     GenerateKeys.writeKeyWithHint(publicKey, "sppk"),
      //     GenerateKeys.keyPrefixes[PrefixEnum.tz2]!);
      // return [pk, publicKey, pkH];
    }
  }

  List<String> getKeysFromPrivateKey(
      String privateKey, SignerCurve signerCurve) {
    if (signerCurve == SignerCurve.ED25519) {
      Uint8List secretKeyBytes =
          GenerateKeys.writeKeyWithHint(privateKey, 'edsk');
      KeyPair keys = Dartez.sodiumUtils.publicKey(secretKeyBytes);
      String pkKey = TezosMessageUtils.readKeyWithHint(keys.pk, 'edpk');
      String pk = TezosMessageUtils.readKeyWithHint(keys.sk, 'edsk');
      String pkKeyHash = GenerateKeys.computeKeyHash(
          keys.pk, GenerateKeys.keyPrefixes[PrefixEnum.tz1]!);
      return [pk, pkKey, pkKeyHash];
    } else if (signerCurve == SignerCurve.SECP256K1) {
      PrivateKey secpPk = PrivateKey.fromHex(
          hex.encode(GenerateKeys.writeKeyWithHint(privateKey, "spsk")));
      Uint8List yArray = GenerateKeys().serializeBigInt(secpPk.publicKey.Y);
      var prefixVal =
          yArray[yArray.length - 1] % 2 == 1 ? 3 : 2; // Y odd / even
      var pad = new List.filled(32, 0); // Zero-padding
      var _publicKey = Uint8List.fromList([prefixVal] +
          (pad + GenerateKeys().serializeBigInt(secpPk.publicKey.X).toList())
              .reversed
              .take(32)
              .toList()
              .reversed
              .toList());
      String publicKey = GenerateKeys.readKeysWithHint(
          _publicKey, GenerateKeys.keyPrefixes[PrefixEnum.sppk]!);
      String pkH = GenerateKeys.computeKeyHash(
          GenerateKeys.writeKeyWithHint(publicKey, "sppk"),
          GenerateKeys.keyPrefixes[PrefixEnum.tz2]!);
      return [privateKey, publicKey, pkH];
    }
    throw Exception("Unknow signer curve");
  }
}
