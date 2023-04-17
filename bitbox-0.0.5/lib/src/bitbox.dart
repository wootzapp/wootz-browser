import 'utils/rest_api.dart';

import 'utils/network.dart';
import 'transactionbuilder.dart';

class Bitbox {
  static const restUrl = "https://rest.bitcoin.com/v2/";
  static const trestUrl = "https://trest.bitcoin.com/v2/";

  /// Set non-default rest Url
  static void setRestUrl([String restUrl = restUrl]) {
    RestApi.restUrl = restUrl;
  }

  /// Create a Transaction builder
  ///
  /// It is possible to call [TransactionBuilder] directly and pass [Network] parameter, this just makes it easier
  static TransactionBuilder transactionBuilder({testnet: false}) =>
      TransactionBuilder(
          network: testnet ? Network.bitcoinCashTest() : Network.bitcoinCash());
}
