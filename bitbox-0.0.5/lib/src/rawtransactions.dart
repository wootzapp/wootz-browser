import 'utils/rest_api.dart';

/// Utilities for working raw transactions
class RawTransactions {
  /// Send raw transactions to the network.
  /// If broadcasting only one transaction, provide it as a [List] with one member.
  /// Returns [List] with the resulting txids
  static Future<List<String>> sendRawTransaction(List<String> hexes) async {
    final returnedTxIds = await RestApi.sendPostRequest(
        "rawtransactions/sendRawTransaction", "hexes", hexes);

    if (returnedTxIds != null && returnedTxIds is List) {
      return List<String>.generate(
          returnedTxIds.length, (index) => returnedTxIds[index]);
    } else {
      return null;
    }
  }
}
