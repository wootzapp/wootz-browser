import 'dart:typed_data';
import 'dart:math';
import 'package:bip32/src/utils/ecurve.dart' as ecc;
import 'package:bip32/src/utils/wif.dart' as wif;
import 'package:bitbox/bitbox.dart';
import 'package:pointycastle/ecc/api.dart';

import 'address.dart';
import 'crypto/crypto.dart';
import 'utils/network.dart';

/// Stores a keypair and provides various methods and factories for creating it and working with it
class ECPair {
  final Uint8List _d;
  final Uint8List _q;
  final Network network;
  final bool compressed;

  /// Default constructor. If [network] is not provided, it will assume Bitcoin Cash mainnet
  ECPair(this._d, this._q, {network, this.compressed = true})
      : this.network = network ?? Network.bitcoinCash();

  /// Creates a keypair from the private key provided in WIF format
  factory ECPair.fromWIF(String wifPrivateKey, {Network network}) {
    wif.WIF decoded = wif.decode(wifPrivateKey);
    final version = decoded.version;
    // TODO support multi networks
    Network nw;
    if (network != null) {
      nw = network;
      if (nw.private != version) throw ArgumentError("Invalid network version");
    } else {
      if (version == Network.bchPrivate) {
        nw = Network.bitcoinCash();
      } else if (version == Network.bchTestnetPrivate) {
        nw = Network.bitcoinCashTest();
      } else {
        throw ArgumentError("Unknown network version");
      }
    }
    return ECPair.fromPrivateKey(decoded.privateKey,
        compressed: decoded.compressed, network: nw);
  }

  /// Creates a keypair from [publicKey. The returned keypair will contain [null] private key
  factory ECPair.fromPublicKey(Uint8List publicKey,
      {Network network, bool compressed}) {
    if (!ecc.isPoint(publicKey)) {
      throw ArgumentError("Point is not on the curve");
    }
    return ECPair(null, publicKey, network: network, compressed: compressed);
  }

  /// Creates a keypair from [privateKey]
  factory ECPair.fromPrivateKey(Uint8List privateKey,
      {Network network, bool compressed}) {
    if (privateKey.length != 32)
      throw ArgumentError(
          "Expected property privateKey of type Buffer(Length: 32)");
    if (!ecc.isPrivate(privateKey))
      throw ArgumentError("Private key not in range [1, n)");
    return ECPair(privateKey, null, network: network, compressed: compressed);
  }

  /// Creates a random keypair
  factory ECPair.makeRandom({Network network, bool compressed, Function rng}) {
    final rfunc = rng ?? _randomBytes;
    Uint8List d;
//    int beginTime = DateTime.now().millisecondsSinceEpoch;
    do {
      d = rfunc(32);
      if (d.length != 32) throw ArgumentError("Expected Buffer(Length: 32)");
//      if (DateTime.now().millisecondsSinceEpoch - beginTime > 5000) throw ArgumentError("Timeout");
    } while (!ecc.isPrivate(d));
    return ECPair.fromPrivateKey(d, network: network, compressed: compressed);
  }

  Uint8List get publicKey => _q ?? ecc.pointFromScalar(_d, compressed);

  Uint8List get privateKey => _d;

  String get address =>
      Address.toBase58Check(hash160(publicKey), network.pubKeyHash);

  /// Returns the private key in WIF format
  String toWIF() {
    if (privateKey == null) {
      throw ArgumentError("Missing private key");
    }
    return wif.encode(wif.WIF(
        version: network.private,
        privateKey: privateKey,
        compressed: compressed));
  }

  /// Returns signature object instead of serialized signature so that it is better handled in bip66 encoding
  ECSignature sign(Uint8List hash) {
    return eccSign(hash, privateKey);
  }

  /// Verifies whether the provided [signature] matches the [hash] using the keypair's [publicKey]
  bool verify(Uint8List hash, Uint8List signature) {
    return ecc.verify(hash, publicKey, signature);
  }

  /// custom EC signature function that returns a signature object instead of serialized result
  ECSignature eccSign(Uint8List hash, Uint8List x) {
    if (!ecc.isScalar(hash)) throw new ArgumentError(ecc.THROW_BAD_HASH);
    if (!ecc.isPrivate(x)) throw new ArgumentError(ecc.THROW_BAD_PRIVATE);

    ECSignature sig = ecc.deterministicGenerateK(hash, x);

    return sig;
  }
}

const int _SIZE_BYTE = 255;
Uint8List _randomBytes(int size) {
  final rng = Random.secure();
  final bytes = Uint8List(size);
  for (var i = 0; i < size; i++) {
    bytes[i] = rng.nextInt(_SIZE_BYTE);
  }
  return bytes;
}
