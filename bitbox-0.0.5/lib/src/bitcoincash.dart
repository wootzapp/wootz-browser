import 'utils/bip21.dart';

/// Bitcoin Cash specific utilities
class BitcoinCash {
  /// Converts Bitcoin Cash units to satoshi units
  static int toSatoshi(double bchAmount) {
    return (bchAmount * 100000000).round();
  }

  /// Converts satoshi units to Bitcoin Cash units
  static double fromSatoshi(int satoshi) {
    return satoshi / 100000000;
  }

  /// Calculates and returns byte count of a transaction
  static int getByteCount(int inputs, int outputs) {
    return ((inputs * 148 * 4 + 34 * 4 * outputs + 10 * 4) / 4).ceil();
  }

  /// Converts a bch [address] and its [options] into bip-21 uri
  ///
  /// The following example shows the format of [options]
  /// ```
  /// {
  ///    amount: 12.5,
  ///    label: 'coinbase donation',
  ///    message: "and ya don't stop",
  ///  }
  ///  ```
  static String encodeBIP21(String address, Map<String, dynamic> options) {
    return Bip21.encode(address, options);
  }

  /// Converts bip-21 [uri] into a bch address and its options
  ///
  /// The return Map will have the following format:
  /// ```
  /// {
  ///   address: 'qpum6dwnqmmysdggrprse8ccjq7ldcrfqgmmtgcmny',
  ///   options: {
  ///     amount: 12.5,
  ///     label: 'coinbase donation',
  ///     message: 'and ya don\'t stop'
  ///   }
  /// }
  /// ```
  static Map<String, dynamic> decodeBIP21(String uri) {
    return Bip21.decode(uri);
  }
}
