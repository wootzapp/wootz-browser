import 'dart:convert';
import 'dart:typed_data';

import 'crypto/ecurve.dart';
import 'address.dart';
import 'package:hex/hex.dart';
import 'package:bs58check/bs58check.dart' as bs58check;

import 'crypto/crypto.dart';
import 'ecpair.dart';
import 'utils/network.dart';

// TODO: delete when done debugging
printHex(String prefix, Uint8List data) {
  String pr = "$prefix: ";
  data.forEach((val) {
    var value = val.toRadixString(16);
    if (value.length == 1) {
      value = "0" + value;
    }
    pr += value + " ";
  });
  print(pr);
}

/// Hierarchically Deterministic node, which can be used to create a
/// [HD wallet](https://developer.bitcoin.com/mastering-bitcoin-cash/3-keys-addresses-wallets/#hierarchical-deterministic-wallets-bip0032bip0044)
class HDNode {
  /// Private-public key pair
  final ECPair _keyPair;

  Uint8List _chainCode;

  int depth = 0;
  int index = 0;

  int parentFingerprint = 0x00000000;
  Uint8List get identifier => hash160(publicKeyList);
  Uint8List get fingerprint => identifier.sublist(0, 4);

  Uint8List get publicKeyList => _keyPair.publicKey;

  String get privateKey => HEX.encode(_keyPair.privateKey);
  String get publicKey => HEX.encode(publicKeyList);

  get rawPrivateKey => _keyPair.privateKey;

  static const HIGHEST_BIT = 0x80000000;
  static const UINT31_MAX = 2147483647; // 2^31 - 1
  static const UINT32_MAX = 4294967295; // 2^32 - 1

  HDNode(this._keyPair, this._chainCode);

  /// Creates an HDNode from [seed]. If [testnet] is not defined, mainnet is used
  factory HDNode.fromSeed(Uint8List seed, [bool testnet = false]) {
    final network = testnet ? Network.bitcoinCashTest() : Network.bitcoinCash();

    final key = utf8.encode('Bitcoin seed');

    final I = hmacSHA512(key, seed);

    final keyPair = ECPair(I.sublist(0, 32), null, network: network);

    final _chainCode = I.sublist(32);

    return HDNode(keyPair, _chainCode);
  }

  /// Creates [HDNode] from extended public key
  factory HDNode.fromXPub(String xPub) {
    final network =
        xPub[0] == "x" ? Network.bitcoinCash() : Network.bitcoinCashTest();
    HDNode hdNode = HDNode._fromBase58(xPub, network);

    return hdNode;
  }

  /// Creates [HDNode] from extended private key
  factory HDNode.fromXPriv(String xPriv) {
    final network =
        xPriv[0] == "x" ? Network.bitcoinCash() : Network.bitcoinCashTest();
    HDNode hdNode = HDNode._fromBase58(xPriv, network);

    return hdNode;
  }

  factory HDNode._fromBase58(String string, Network network) {
    Uint8List buffer = bs58check.decode(string);
    if (buffer.length != 78) throw new ArgumentError("Invalid buffer length");
    ByteData bytes = buffer.buffer.asByteData();
    // 4 bytes: version bytes
    var version = bytes.getUint32(0);
    if (version != network.bip32Private && version != network.bip32Public) {
      throw new ArgumentError("Invalid network version");
    }

//    // 1 byte: depth: 0x00 for master nodes, 0x01 for level-1 descendants, ...
    var depth = buffer[4];

    // 4 bytes: the fingerprint of the parent's key (0x00000000 if master key)
    var parentFingerprint = bytes.getUint32(5);
    if (depth == 0) {
      if (parentFingerprint != 0x00000000)
        throw new ArgumentError("Invalid parent fingerprint");
    }

    // 4 bytes: child number. This is the number i in xi = xpar/i, with xi the key being serialized.
    // This is encoded in MSB order. (0x00000000 if master key)
    var index = bytes.getUint32(9);
    if (depth == 0 && index != 0) throw new ArgumentError("Invalid index");

    // 32 bytes: the chain code
    Uint8List chainCode = buffer.sublist(13, 45);

    ECPair keyPair;
    // 33 bytes: private key data (0x00 + k)
    if (version == network.bip32Private) {
      if (bytes.getUint8(45) != 0x00)
        throw new ArgumentError("Invalid private key");
      Uint8List d = buffer.sublist(46, 78);
      keyPair = ECPair(d, null, network: network);
//      hdNode = HDNode.fromPrivateKey(d, chainCode, network);
    } else {
      // 33 bytes: public key data (0x02 + X or 0x03 + X)
      Uint8List Q = buffer.sublist(45, 78);
      keyPair = ECPair(null, Q, network: network);
//      hdNode = HDNode.fromPublicKey(Q, chainCode, network);
    }

    final hdNode = HDNode(keyPair, chainCode);
    hdNode.depth = depth;
    hdNode.index = index;
    hdNode.parentFingerprint = parentFingerprint;
    return hdNode;
  }

  /// Returns the node's public-private key pair as an instance of [ECPair]
  get keyPair => _keyPair;

  /// Derives a new [HDNode] from this instance based on the [path]
  HDNode derivePath(String path) {
    List<String> splitPath = path.split('/');

    if (splitPath[0] == 'm') {
      splitPath = splitPath.sublist(1);
    }

    return splitPath.fold(this, (HDNode prevHd, String indexStr) {
      int index;
      if (indexStr.endsWith("'")) {
        index = int.parse(indexStr.substring(0, indexStr.length - 1));
        return prevHd._deriveHardened(index);
      } else {
        index = int.parse(indexStr);
        return prevHd.derive(index);
      }
    });
  }

  /// Derives a new [HDNode] from this node based on [index]
  HDNode derive(int index) {
    if (index > UINT32_MAX || index < 0) throw ArgumentError("Expected UInt32");
    final isHardened = index >= HIGHEST_BIT;
    Uint8List data = new Uint8List(37);
    if (isHardened) {
      if (_isNeutered()) {
        throw ArgumentError("Missing private key for hardened child key");
      }
      data[0] = 0x00;
      data.setRange(1, 33, _keyPair.privateKey);
      data.buffer.asByteData().setUint32(33, index);
    } else {
      data.setRange(0, 33, publicKeyList);
      data.buffer.asByteData().setUint32(33, index);
    }

    final I = hmacSHA512(_chainCode, data);
    final IL = I.sublist(0, 32);
    final IR = I.sublist(32);
//    if (!ecc.isPrivate(IL)) {
//      return derive(index + 1);
//    }
    ECPair derivedKeyPair;
    if (!_isNeutered()) {
      final ki = ECurve.privateAdd(_keyPair.privateKey, IL);
      if (ki == null) return derive(index + 1);

      derivedKeyPair = ECPair(ki, null, network: this._keyPair.network);
    } else {
      final ki = ECurve.pointAddScalar(publicKeyList, IL, true);
      if (ki == null) return derive(index + 1);

      derivedKeyPair = ECPair(null, ki, network: this._keyPair.network);
    }
    final hd = HDNode(derivedKeyPair, IR);
    hd.depth = depth + 1;
    hd.index = index;
    hd.parentFingerprint = fingerprint.buffer.asByteData().getUint32(0);
    return hd;
  }

  /// Returns HDNode's extended private key
  String toXPriv() => _toBase58();

  /// Returns HDNode's extended public key
  String toXPub() => _neutered()._toBase58();

  /// Returns HDNode's address in legacy format
  String toLegacyAddress() => _keyPair.address;

  /// Returns HDNode's address in cashAddr format
  String toCashAddress() => Address.toCashAddress(toLegacyAddress());

  HDNode _deriveHardened(int index) {
    return derive(index + HIGHEST_BIT);
  }

  String _toBase58() {
    final version = (!_isNeutered())
        ? this._keyPair.network.bip32Private
        : this._keyPair.network.bip32Public;
    Uint8List buffer = new Uint8List(78);
    ByteData bytes = buffer.buffer.asByteData();
    bytes.setUint32(0, version);
    bytes.setUint8(4, this.depth);
    bytes.setUint32(5, parentFingerprint);
    bytes.setUint32(9, this.index);
    buffer.setRange(13, 45, _chainCode);
    if (!_isNeutered()) {
      bytes.setUint8(45, 0);
      buffer.setRange(46, 78, _keyPair.privateKey);
    } else {
      buffer.setRange(45, 78, publicKeyList);
    }
    return bs58check.encode(buffer);
  }

  HDNode _neutered() {
    final neutered = HDNode(
        ECPair(null, this.publicKeyList, network: this._keyPair.network),
        _chainCode);
    neutered.depth = this.depth;
    neutered.index = this.index;
    neutered.parentFingerprint = this.parentFingerprint;
    return neutered;
  }

  bool _isNeutered() {
    return this._keyPair.privateKey == null;
  }
}
