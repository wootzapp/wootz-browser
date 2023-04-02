import 'dart:typed_data';
import 'package:blake2b/blake2b_hash.dart';
import 'package:convert/convert.dart';
import 'package:bs58check/bs58check.dart' as bs58check;

enum PrefixEnum {
  tz1,
  tz2,

  edpk,
  edsk,

  spsk,
  sppk,

  edsig,
  spsig,
}

class GenerateKeys {
  static Map<PrefixEnum, String> keyPrefixes = {
    PrefixEnum.tz1: "06a19f",
    PrefixEnum.edsk: "2bf64e07",
    PrefixEnum.edpk: "0d0f25d9",
    PrefixEnum.edsig: "09f5cd8612",
    //
    PrefixEnum.tz2: "06a1a1",
    PrefixEnum.spsk: "11a2e0c9",
    PrefixEnum.sppk: "03fee256",
    PrefixEnum.spsig: "0d7365133f",
  };

  static String computeKeyHash(Uint8List publicKey, String hint) {
    Uint8List blake2bHash = Blake2bHash.hashWithDigestSize(160, publicKey);
    String uintToString = String.fromCharCodes(blake2bHash);
    String stringToHexString = hex.encode(uintToString.codeUnits);
    String finalStringToDecode = hint + stringToHexString;
    List<int> listOfHexDecodedInt = hex.decode(finalStringToDecode);
    String publicKeyHash = bs58check.encode(listOfHexDecodedInt as Uint8List);
    return publicKeyHash;
  }

  static String readKeysWithHint(
    Uint8List key,
    String hint,
  ) {
    String uint8ListToString = String.fromCharCodes(key);
    String stringToHexString = hex.encode(uint8ListToString.codeUnits);
    String concatinatingHexStringWithHint = hint + stringToHexString;
    List<int> convertingHexStringToListOfInt =
        hex.decode(concatinatingHexStringWithHint);
    String base58String =
        bs58check.encode(convertingHexStringToListOfInt as Uint8List);
    return base58String;
  }

  static Uint8List writeKeyWithHint(
    String key,
    String hint,
  ) {
    if (hint == 'edsk' ||
        hint == 'edpk' ||
        hint == 'sppk' ||
        hint == 'spsk' ||
        hint == 'p2pk' ||
        hint == '2bf64e07' ||
        hint == '0d0f25d9') {
      return bs58check.decode(key).sublist(4);
    } else
      throw Exception("Unrecognized key hint, '$hint'");
  }

  Uint8List serializeBigInt(BigInt bi) {
    
    return Uint8List.fromList(hex.decode(bi.toRadixString(16)));

    // Uint8List array = Uint8List((bi.bitLength / 8).ceil());
    // for (int i = 0; i < array.length; i++) {
    //   array[i] = (bi >> (i * 8)).toUnsigned(8).toInt();
    // }
    // return array;
  }

  BigInt deserializeBigInt(Uint8List array) {
    return BigInt.parse(hex.encode(array));
    // var bi = BigInt.zero;
    // for (var byte in array.reversed) {
    //   bi <<= 8;
    //   bi |= BigInt.from(byte);
    // }
    // return bi;
  }
}
