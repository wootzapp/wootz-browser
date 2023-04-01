import 'dart:convert';
import 'dart:typed_data';
import 'utils/rest_api.dart';

import 'package:bs58check/bs58check.dart' as bs58check;
import 'utils/network.dart';
import 'package:fixnum/fixnum.dart';

/// Works with both legacy and cashAddr formats of the address
///
/// There is no reason to instanciate this class. All constants, functions, and methods are static.
/// It is assumed that all necessary data to work with addresses are kept in the instance of [ECPair] or [Transaction]
class Address {
  static const formatCashAddr = 0;
  static const formatLegacy = 1;

  static const _CHARSET = 'qpzry9x8gf2tvdw0s3jn54khce6mua7l';
  static const _CHARSET_INVERSE_INDEX = {
    'q': 0,
    'p': 1,
    'z': 2,
    'r': 3,
    'y': 4,
    '9': 5,
    'x': 6,
    '8': 7,
    'g': 8,
    'f': 9,
    '2': 10,
    't': 11,
    'v': 12,
    'd': 13,
    'w': 14,
    '0': 15,
    's': 16,
    '3': 17,
    'j': 18,
    'n': 19,
    '5': 20,
    '4': 21,
    'k': 22,
    'h': 23,
    'c': 24,
    'e': 25,
    '6': 26,
    'm': 27,
    'u': 28,
    'a': 29,
    '7': 30,
    'l': 31,
  };

  /// Returns information about the given Bitcoin Cash address.
  ///
  /// See https://developer.bitcoin.com/bitbox/docs/util for details about returned format
  static Future<Map<String, dynamic>> validateAddress(String address) async =>
      await RestApi.sendGetRequest("util/validateAddress", address);

  /// Returns details of the provided address or addresses
  ///
  /// * If [addresses] is [String], it will return [Map] with address details.
  /// * If [addresses] is [List] of Strings and [returnAsMap] is true, it will return [Map] with cashAddress used as keys
  /// and [Map] with address details in values.
  /// * If [addresses] is [List] of Strings and [returnAsMap] is false, it will return [List] with address details sorted
  /// in the same order as the addresses provided.
  /// * Otherwise it will throw [FormatException]
  ///
  /// See https://developer.bitcoin.com/bitbox/docs/address#details for details about returned format. However
  /// note, that processing from array to map is done on the library side
  static Future<dynamic> details(addresses, [returnAsMap = false]) async =>
      await _sendRequest("details", addresses, returnAsMap);

  /// Returns list of unconfirmed transactions
  ///
  /// * If [addresses] is [String], it will return [Map] with unconfirmed transaction details.
  /// * If [addresses] is [List] of Strings and [returnAsMap] is true, it will return [Map] with cashAddress used as keys
  /// and [Map] objects with address details in values.
  /// * If [addresses] is [List] of Strings and [returnAsMap] is false, it will return [List] with details sorted in the
  /// same order as the input list of addresses.
  /// * Otherwise it will throw [FormatException]
  ///
  /// See https://developer.bitcoin.com/bitbox/docs/address#unconfirmed for details about the returned format. However
  /// note, that processing from array to map is done on the library side
  static Future<dynamic> getUnconfirmed(addresses,
      [returnAsMap = false]) async {
    final result = await _sendRequest("unconfirmed", addresses);

    if (result is Map) {
      return Utxo.convertMapListToUtxos(result["utxos"]);
    } else if (result is List<Map>) {
      final returnList = <Map>[];
      final returnMap = <String, List>{};

      result.forEach((addressUtxoMap) {
        if (returnAsMap) {
          returnMap[addressUtxoMap["cashAddr"]] =
              Utxo.convertMapListToUtxos(addressUtxoMap["utxos"]);
        } else {
          addressUtxoMap["utxos"] =
              Utxo.convertMapListToUtxos(addressUtxoMap["utxos"]);
          returnList.add(addressUtxoMap);
        }
      });

      return returnAsMap ? returnMap : returnList;
    }
  }

  /// Returns [Utxo] or a [List] of Utxos for the address or addresses.
  ///
  /// * If [addresses] is [String], it will return a list of [Utxo]
  /// * If [addresses] is [List] of Strings and [returnAsMap] is true, it will return [Map] where key is the cashAddr
  /// and values are Lists of [Utxo]
  /// * If [addresses] is [List] of Strings and [returnAsMap] is false, it will return [List] similar to
  /// https://developer.bitcoin.com/bitbox/docs/address#utxo except, that utxo entries are converted to [Utxo] objects
  /// * Otherwise it will throw [FormatException]
  static Future<dynamic> utxo(addresses, [returnAsMap = false]) async {
    // don't reuse _sendRequests's returnAsMap parameter here because we need to iterate through the list anyway
    final result = await _sendRequest("utxo", addresses);

    if (result is Map) {
      return Utxo.convertMapListToUtxos(result["utxos"]);
    } else if (result is List) {
      final returnList = <Map>[];
      final returnMap = <String, List>{};

      result.forEach((addressUtxoMap) {
        if (returnAsMap) {
          returnMap[addressUtxoMap["cashAddress"]] =
              Utxo.convertMapListToUtxos(addressUtxoMap["utxos"]);
        } else {
          addressUtxoMap["utxos"] =
              Utxo.convertMapListToUtxos(addressUtxoMap["utxos"]);
          returnList.add(addressUtxoMap);
        }
      });

      return returnAsMap ? returnMap : returnList;
    } else {
      throw FormatException("Invalid format returned: $result");
    }
  }

  /// Returns details of all transactions of this address
  ///
  /// Returns
  /// * [Map] if [addresses] is [String] with a signle address
  /// * [List] if [addresses] is [List] of addresses and [returnAsMap] is false
  /// * [Map] with address details with cashAddress as key
  ///
  /// See https://developer.bitcoin.com/bitbox/docs/address#transactions for format details.
  /// Note, that conversion from List to Map when [returnAsMap] is true takes place in this library
  static Future<dynamic> transactions(addresses, [returnAsMap = false]) async =>
      await _sendRequest("transactions", addresses, returnAsMap);

  /// Converts legacy address to cash address
  static String toCashAddress(String legacyAddress,
      [bool includePrefix = true]) {
    final decoded = Address._decodeLegacyAddress(legacyAddress);
    String prefix = "";
    if (includePrefix) {
      switch (decoded["version"]) {
        case Network.bchPublic:
          prefix = "bitcoincash";
          break;
        case Network.bchTestnetPublic:
          prefix = "bchtest";
          break;
        default:
          throw FormatException("Unsupported address format: $legacyAddress");
      }
    }

    final cashAddress = Address._encode(prefix, "P2PKH", decoded["hash"]);
    return cashAddress;
  }

  /// Converts cashAddr format to legacy address
  static String toLegacyAddress(String cashAddress) {
    final decoded = _decodeCashAddress(cashAddress);
    final testnet = decoded['prefix'] == "bchtest";

    final version = !testnet ? Network.bchPublic : Network.bchTestnetPublic;
    return toBase58Check(decoded["hash"], version);
  }

  /// Detects type of the address and returns [formatCashAddr] or [formatLegacy]
  static int detectFormat(String address) {
    // decode the address to determine the format
    final decoded = _decode(address);
    // return the format
    return decoded["format"];
  }

  /// Generates legacy address format
  static String toBase58Check(Uint8List hash, int version) {
    Uint8List payload = Uint8List(21);
    payload[0] = version;
    payload.setRange(1, payload.length, hash);
    return bs58check.encode(payload);
  }

  /*
  static Uint8List _toOutputScript(address, network) {
    return bscript.compile([
      Opcodes.OP_DUP,
      Opcodes.OP_HASH160,
      address,
      Opcodes.OP_EQUALVERIFY,
      Opcodes.OP_CHECKSIG
    ]);
  }*/

  /// Encodes a hash from a given type into a Bitcoin Cash address with the given prefix.
  /// [prefix] - Network prefix. E.g.: 'bitcoincash'.
  /// [type] is currently unused - the library works only with _P2PKH_
  /// [hash] is the address hash, which can be decode either using [_decodeCashAddress()] or [_decodeLegacyAddress()]
  static _encode(String prefix, String type, Uint8List hash) {
    final prefixData = _prefixToUint5List(prefix) + Uint8List(1);
    final versionByte = _getHashSizeBits(hash);
    final payloadData =
        _convertBits(Uint8List.fromList([versionByte] + hash), 8, 5);
    final checksumData = prefixData + payloadData + Uint8List(8);
    final payload = payloadData + _checksumToUint5Array(_polymod(checksumData));
    return "$prefix:" + _base32Encode(payload);
  }

  /// Helper method for sending generic requests to Bitbox API. Accepts [String] or [List] of Strings and optionally
  /// converts the List returned by Bitbox into [Map], which uses cashAddress as a key
  static Future<dynamic> _sendRequest(String path, dynamic addresses,
      [bool returnAsMap = false]) async {
    assert(addresses is String || addresses is List<String>);

    if (addresses is String) {
      return await RestApi.sendGetRequest("address/$path", addresses) as Map;
    } else if (addresses is List<String>) {
      return await RestApi.sendPostRequest(
          "address/$path", "addresses", addresses,
          returnKey: returnAsMap ? "cashAddress" : null);
    } else {
      throw TypeError();
    }
  }

  /// Derives an array from the given prefix to be used in the computation of the address' checksum.
  static Uint8List _prefixToUint5List(String prefix) {
    Uint8List result = Uint8List(prefix.length);
    for (int i = 0; i < prefix.length; i++) {
      result[i] = prefix.codeUnitAt(i) & 31;
    }
    return result;
  }

  /// Returns the bit representation of the length in bits of the given hash within the version byte.
  static int _getHashSizeBits(hash) {
    switch (hash.length * 8) {
      case 160:
        return 0;
      case 192:
        return 1;
      case 224:
        return 2;
      case 256:
        return 3;
      case 320:
        return 4;
      case 384:
        return 5;
      case 448:
        return 6;
      case 512:
        return 7;
      default:
        throw Exception('Invalid hash size: ' + hash.length + '.');
    }
  }

  /// Retrieves the the length in bits of the encoded hash from its bit representation within the version byte.
  static int _getHashSize(versionByte) {
    switch (versionByte & 7) {
      case 0:
        return 160;
      case 1:
        return 192;
      case 2:
        return 224;
      case 3:
        return 256;
      case 4:
        return 320;
      case 5:
        return 384;
      case 6:
        return 448;
      case 7:
        return 512;
    }

    return -1;
  }

  /// Decodes the given address into:
  /// * (for cashAddr): constituting prefix (e.g. _bitcoincash_)
  /// * (for legacy): version
  /// * hash
  /// * format
  static Map<String, dynamic> _decode(String address) {
    try {
      return _decodeLegacyAddress(address);
    } catch (e) {}

    try {
      return _decodeCashAddress(address);
    } catch (e) {}

    throw FormatException("Invalid address format : $address");
  }

  /// Decodes legacy address into a [Map] with version, hash and format
  static Map<String, dynamic> _decodeLegacyAddress(String address) {
    Uint8List buffer = bs58check.decode(address);

    return <String, dynamic>{
      "version": buffer.first,
      "hash": buffer.sublist(1),
      "format": formatLegacy,
    };
  }

  /// Decodes the given address into its constituting prefix, type and hash
  ///
  /// if [address] doesn't contain prefix (e.g. bitcoincash:), it will try and validate different prefixes and return
  /// the correct one
  static Map<String, dynamic> _decodeCashAddress(String address) {
    if (!_hasSingleCase(address)) {
      throw FormatException("Address has both lower and upper case: $address");
    }

    // split the address with : separator to find out it if contains prefix
    final pieces = address.toLowerCase().split(":");

    // placeholder for different prefixes to be tested later
    List<String> prefixes;

    // check if the address contained : separator by looking at number of splitted pieces
    if (pieces.length == 2) {
      // if it contained the separator, use the first piece as a single prefix
      prefixes = <String>[pieces.first];
      address = pieces.last;
    } else if (pieces.length == 1) {
      // if it came without separator, try all three possible formats
      prefixes = <String>["bitcoincash", "bchtest", "bchreg"];
    } else {
      // if it came with more than one separator, throw a format exception
      throw FormatException("Invalid Address Format: $address");
    }

    String exception;
    // try to decode the address with either one or all three possible prefixes
    for (int i = 0; i < prefixes.length; i++) {
      final payload = _base32Decode(address);

      if (!_validChecksum(prefixes[i], payload)) {
        exception = "Invalid checksum: $address";
        continue;
      }

      final payloadData =
          _fromUint5Array(payload.sublist(0, payload.length - 8));
      final hash = payloadData.sublist(1);

      if (_getHashSize(payloadData[0]) != hash.length * 8) {
        exception = "Invalid hash size: $address";
        continue;
      }

      // If the loop got all the way here, it means validations went through and the address was decoded.
      // Return the decoded data
      return <String, dynamic>{
        "prefix": prefixes[i],
        "hash": hash,
        "format": formatCashAddr
      };
    }

    // if the loop went through all possible formats and didn't return data from the function, it means there were
    // validation issues. Throw a format exception
    throw FormatException(exception);
  }

  /// Converts a list of 5-bit integers back into an array of 8-bit integers, removing extra zeroes left from padding
  /// if necessary.
  static Uint8List _fromUint5Array(Uint8List data) {
    return _convertBits(data, 5, 8, true);
  }

  /// Returns a list representation of the given checksum to be encoded within the address' payload.
  static Uint8List _checksumToUint5Array(int checksum) {
    Uint8List result = Uint8List(8);
    for (int i = 0; i < 8; i++) {
      result[7 - i] = checksum & 31;
      checksum = checksum >> 5;
    }

    return result;
  }

  /// Converts a list of integers made up of 'from' bits into an  array of integers made up of 'to' bits.
  /// The output array is zero-padded if necessary, unless strict mode is true.
  static Uint8List _convertBits(List data, int from, int to,
      [bool strictMode = false]) {
    final length = strictMode
        ? (data.length * from / to).floor()
        : (data.length * from / to).ceil();
    int mask = (1 << to) - 1;
    var result = Uint8List(length);
    int index = 0;
    Int32 accumulator = Int32(0);
    int bits = 0;
    for (int i = 0; i < data.length; ++i) {
      var value = data[i];
      accumulator = (accumulator << from) | value;
      bits += from;
      while (bits >= to) {
        bits -= to;
        result[index] = ((accumulator >> bits) & mask).toInt();
        ++index;
      }
    }

    if (!strictMode) {
      if (bits > 0) {
        result[index] = ((accumulator << (to - bits)) & mask).toInt();
        ++index;
      }
    } else {
      if (bits < from && ((accumulator << (to - bits)) & mask).toInt() != 0) {
        throw FormatException(
            "Input cannot be converted to $to bits without padding, but strict mode was used.");
      }
    }
    return result;
  }

  /// Computes a checksum from the given input data as specified for the CashAddr format:
  // https://github.com/Bitcoin-UAHF/spec/blob/master/cashaddr.md.
  static int _polymod(List data) {
    const GENERATOR = [
      0x98f2bc8e61,
      0x79b76d99e2,
      0xf33e5fb3c4,
      0xae2eabe2a8,
      0x1e4f43e470
    ];

    int checksum = 1;

    for (int i = 0; i < data.length; ++i) {
      final value = data[i];
      final topBits = checksum >> 35;
      checksum = ((checksum & 0x07ffffffff) << 5) ^ value;

      for (int j = 0; j < GENERATOR.length; ++j) {
        if ((topBits >> j) & 1 == 1) {
          checksum = checksum ^ GENERATOR[j];
        }
      }
    }

    return checksum ^ 1;
  }

  static Uint8List _base32Decode(String string) {
    final data = Uint8List(string.length);
    for (int i = 0; i < string.length; i++) {
      final value = string[i];
      if (!_CHARSET_INVERSE_INDEX.containsKey(value))
        throw FormatException("Invalid character '$value'");
      data[i] = _CHARSET_INVERSE_INDEX[string[i]];
    }

    return data;
  }

  static _base32Encode(List data) {
    String base32 = '';
    for (int i = 0; i < data.length; ++i) {
      var value = data[i];
      //validate(0 <= value && value < 32, 'Invalid value: ' + value + '.');
      base32 += _CHARSET[value];
    }
    return base32;
  }

  static bool _hasSingleCase(String address) {
    return address == address.toLowerCase() || address == address.toUpperCase();
  }

  /// Verify that the payload has not been corrupted by checking that the checksum is valid.
  static _validChecksum(String prefix, Uint8List payload) {
    final prefixData = _prefixToUint5List(prefix) + Uint8List(1);
    final checksumData = prefixData + payload;
    return _polymod(checksumData) == 0;
  }
}

/// Container for to make it easier to work with Utxos
class Utxo {
  final String txid;
  final int vout;
  final double amount;
  final int satoshis;
  final int height;
  final int confirmations;

  Utxo(this.txid, this.vout, this.amount, this.satoshis, this.height,
      this.confirmations);

  /// Create [Utxo] instance from utxo [Map]
  Utxo.fromMap(Map<String, dynamic> utxoMap)
      : this.txid = utxoMap['txid'],
        this.vout = utxoMap['vout'],
        this.amount = utxoMap['amount'],
        this.satoshis = utxoMap['satoshis'],
        this.height = utxoMap.containsKey('height') ? utxoMap['height'] : null,
        this.confirmations = utxoMap['confirmations'];

  /// Converts List of utxo maps into a list of [Utxo] objects
  static List<Utxo> convertMapListToUtxos(List utxoMapList) {
    final utxos = <Utxo>[];
    utxoMapList.forEach((utxoMap) {
      utxos.add(Utxo.fromMap(utxoMap));
    });

    return utxos;
  }

  @override
  String toString() => jsonEncode({
        "txid": txid,
        "vout": vout,
        "amount": amount,
        "satoshis": satoshis,
        "height": height,
        "confirmations": confirmations
      });
}
