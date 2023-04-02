import 'dart:typed_data';

import 'package:blake2b/blake2b_hash.dart';
import 'package:bs58check/bs58check.dart' as bs58check;
import 'package:convert/convert.dart';
import 'package:dartez/chain/tezos/tezos_language_util.dart';
import 'package:dartez/helper/generateKeys.dart';
import 'package:dartez/src/soft-signer/soft_signer.dart';
import 'package:dartez/types/tezos/tezos_chain_types.dart';

class TezosMessageUtils {
  static String writeBranch(String branch) {
    return hex.encode(bs58check.decode(branch).sublist(2).toList());
  }

  static String writeInt(int value) {
    if (value < 0) {
      throw new Exception('Use writeSignedInt to encode negative numbers');
    }
    var byteHexList = Uint8List.fromList(hex.decode(twoByteHex(value)));
    for (var i = 0; i < byteHexList.length; i++) {
      if (i != 0) byteHexList[i] ^= 0x80;
    }
    var result = hex.encode(byteHexList.reversed.toList());
    return result;
  }

  static String twoByteHex(int n) {
    if (n < 128) {
      var s = ('0' + n.toRadixString(16));
      return s.substring(s.length - 2);
    }
    String h = '';
    if (n > 2147483648) {
      var r = BigInt.from(n);
      while (r.compareTo(BigInt.zero) != 0) {
        var _h = ('0' + (r & BigInt.from(127)).toRadixString(16));
        h = _h.substring(_h.length - 2) + h;
        r = r >> 7;
      }
    } else {
      var r = n;
      while (r > 0) {
        var _h = ('0' + (r & 127).toRadixString(16));
        h = _h.substring(_h.length - 2) + h;
        r = r >> 7;
      }
    }
    return h;
  }

  static String writeAddress(String address) {
    var bs58checkdata = bs58check.decode(address).sublist(3);
    var _hex = hex.encode(bs58checkdata);
    if (address.startsWith("tz1")) {
      return "0000" + _hex;
    } else if (address.startsWith("tz2")) {
      return "0001" + _hex;
    } else if (address.startsWith("tz3")) {
      return "0002" + _hex;
    } else if (address.startsWith("KT1")) {
      return "01" + _hex + "00";
    } else {
      throw new Exception(
          'Unrecognized address prefix: ${address.substring(0, 3)}');
    }
  }

  static String writePublicKey(String publicKey) {
    if (publicKey.startsWith("edpk")) {
      return "00" + hex.encode(bs58check.decode(publicKey).sublist(4));
    } else if (publicKey.startsWith("sppk")) {
      return "01" + hex.encode(bs58check.decode(publicKey).sublist(4));
    } else if (publicKey.startsWith("p2pk")) {
      return "02" + hex.encode(bs58check.decode(publicKey).sublist(4));
    } else {
      throw new Exception('Unrecognized key type');
    }
  }

  static String readPublicKey(String hex, Uint8List b) {
    if (hex.length != 66 && hex.length != 68) {
      throw new Exception("Incorrect hex length, ${hex.length} to parse a key");
    }
    var hint = hex.substring(0, 2);
    if (hint == "00") {
      return GenerateKeys.readKeysWithHint(b, '0d0f25d9');
    } else if (hint == "01" && hex.length == 68) {
      return GenerateKeys.readKeysWithHint(b, '03fee256');
    } else if (hint == "02" && hex.length == 68) {
      return GenerateKeys.readKeysWithHint(b, '03b28b7f');
    } else {
      throw new Exception('Unrecognized key type');
    }
  }

  static dynamic readKeyWithHint(Uint8List b, String hint) {
    Uint8List key = !(b.runtimeType == Uint8List) ? Uint8List.fromList(b) : b;
    String keyHex = hex.encode(key);
    if (hint == 'edsk') {
      return GenerateKeys.readKeysWithHint(b, '2bf64e07');
    } else if (hint == 'edpk') {
      return readPublicKey("00$keyHex", b);
    } else if (hint == 'sppk') {
      return readPublicKey("01$keyHex", b);
    } else if (hint == 'p2pk') {
      return readPublicKey("02$keyHex", b);
    } else {
      throw new Exception("Unrecognized key hint, $hint");
    }
  }

  static Uint8List simpleHash(Uint8List message, int size) {
    return Uint8List.fromList(Blake2bHash.hashWithDigestSize(256, message));
  }

  static String readSignatureWithHint(Uint8List opSignature, SignerCurve hint) {
    opSignature = Uint8List.fromList(opSignature);
    if (hint == SignerCurve.ED25519) {
      return GenerateKeys.readKeysWithHint(opSignature, '09f5cd8612');
    } else if (hint == SignerCurve.SECP256K1) {
      return GenerateKeys.readKeysWithHint(opSignature, '0d7365133f');
    } else if (hint == SignerCurve.SECP256R1) {
      return GenerateKeys.readKeysWithHint(opSignature, '36f02c34');
    } else {
      throw Exception('Unrecognized signature hint, "$hint"');
    }
  }

  static String writeBoolean(bool b) {
    return b ? "ff" : "00";
  }

  static encodeBigMapKey(Uint8List key) {
    return readBufferWithHint(simpleHash(key, 32), 'expr');
  }

  static readBufferWithHint(address, hint) {
    var buffer = address;
    if (hint == 'op') {
      return bs58check
          .encode(Uint8List.fromList(hex.decode('0574' + hex.encode(buffer))));
    } else if (hint == 'p') {
      return bs58check
          .encode(Uint8List.fromList(hex.decode('02aa' + hex.encode(buffer))));
    } else if (hint == 'expr') {
      return bs58check.encode(
          Uint8List.fromList(hex.decode('0d2c401b' + hex.encode(buffer))));
    } else if (hint == '') {
      return bs58check.encode(buffer);
    } else {
      throw new Exception('Unsupported hint $hint');
    }
  }

  static String writeSignedInt(value) {
    if (value == 0 || value == '0') {
      return '00';
    }
    var n = BigInt.from(value).abs();

    var l = n.bitLength;
    var arr = [];
    for (var i = 0; i < l; i += 7) {
      var byte = BigInt.zero;
      if (i == 0) {
        byte = n & BigInt.parse('0x3f');
        n = n >> 6;
      } else {
        byte = n & BigInt.parse('0x7f');
        n = n >> 7;
      }
      if (value < 0 && i == 0) {
        byte = byte | BigInt.parse('0x40');
      }
      if (i + 7 < l) {
        byte = byte | BigInt.parse('0x80');
      }
      arr.add(byte);
    }
    if (l % 7 == 0) {
      arr[arr.length - 1] = arr[arr.length - 1] | BigInt.parse('0x80');
      arr.add(1);
    }
    var output = arr.map((v) {
      int newNum = int.parse(v.toString());
      var str = '0' + newNum.toRadixString(16).toString();
      str = str.substring(str.length - 2);
      return str;
    }).join('');
    return output;
    // return arr
    //     .map((w) => ('0' + w.toRadixString(16))
    //         .substring(0, w.toRadixString(16).length - 2))
    //     .toList()
    //     .join('');
  }

  static String? writeString(value) {
    var len = dataLength(value.length);
    var text = value
        .split('')
        .map((String c) => c.codeUnitAt(0).toRadixString(16))
        .toList()
        .join('');
    return len + text;
  }

  static dataLength(value) {
    var data = ('0000000' + (value).toString(16));
    return data.substring(0, data.length - 8);
  }

  static readAddress(String hexValue, Uint8List b) {
    if (hexValue.length != 44 && hexValue.length != 42) {
      throw new Exception("Incorrect hex length to parse an address");
    }
    var implicitHint = hexValue.length == 44
        ? hexValue.substring(0, 4)
        : "00" + hexValue.substring(0, 2);
    if (implicitHint == "0000") {
      return GenerateKeys.readKeysWithHint(b, '06a19f');
    } else if (implicitHint == "0001") {
      return GenerateKeys.readKeysWithHint(b, '06a1a1');
    } else if (implicitHint == "0002") {
      return GenerateKeys.readKeysWithHint(b, '06a1a4');
    } else if (hexValue.substring(0, 2) == "01" && hexValue.length == 44) {
      return GenerateKeys.readKeysWithHint(b, '025a79');
    } else {
      throw new Exception("Unrecognized address type");
    }
  }

  static readAddressWithHint(Uint8List b, String hint) {
    Uint8List address =
        !(b.runtimeType == Uint8List) ? Uint8List.fromList(b) : b;
    String hexValue = hex.encode(address);
    if (hint == 'tz1') {
      return readAddress("0000$hexValue", b);
    } else if (hint == 'tz2') {
      return readAddress("0001$hexValue", b);
    } else if (hint == 'tz3') {
      return readAddress("0002$hexValue", b);
    } else if (hint == 'kt1') {
      return readAddress("01$hexValue}00", b);
    } else {
      throw new Exception("Unrecognized address hint, '$hint'");
    }
  }

  static writePackedData(value, type, format) {
    switch (type) {
      case 'int':
        return '0500' + writeSignedInt(value);
      case 'nat':
        return '0500' + writeInt(int.parse(value));
      case 'string':
        return '0501' + writeString(value)!;
      case 'key_hash':
        var address = writeAddress(value).substring(2);
        return '050a${dataLength(address.length / 2)}$address';
      case 'address':
        var address = writeAddress(value);
        return '050a${dataLength(address.length / 2)}$address';
      case 'bytes':
        var buffer = hex.encode(value);
        return '050a${dataLength(buffer.length / 2)}$buffer';
      default:
        try {
          if (format == TezosParameterFormat.Micheline) {
            return '05${TezosLanguageUtil.translateMichelineToHex(value)}';
          } else if (format == TezosParameterFormat.Michelson) {
            var micheline =
                TezosLanguageUtil.translateMichelsonToMicheline(value)!;
            return '05${TezosLanguageUtil.translateMichelineToHex(micheline)}';
          } else {
            throw new Exception('Unsupported format $format provided');
          }
        } catch (e) {
          throw new Exception(
              'Unrecognized data type or format: $type, $format : $e');
        }
    }
  }
}
