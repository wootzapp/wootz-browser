class CoinPay {
  final String recipient;
  final double amount;
  final String coinScheme;
  List<String> references = [];
  final String label;
  final String message;
  final String memo;
  final String splToken;
  CoinPay({
    this.recipient,
    this.references = const [],
    this.amount,
    this.label,
    this.message,
    this.memo,
    this.coinScheme,
    this.splToken,
  });

  static CoinPay parseUri(String urlPay) {
    Uri uri = Uri.parse(urlPay);
    String recipient = uri.path;
    Map<String, dynamic> meta = uri.queryParametersAll;

    return CoinPay(
      recipient: recipient,
      references: meta['reference'] ?? [],
      amount: meta["amount"] != null ? double.parse(meta['amount'][0]) : null,
      label: meta["label"] != null ? meta["label"][0] : null,
      message: meta["message"] != null ? meta["message"][0] : null,
      memo: meta["memo"] != null ? meta["memo"][0] : null,
      splToken: meta["spl-token"] != null ? meta["spl-token"][0] : null,
      coinScheme: uri.scheme,
    );
  }

  String toUri() {
    String uri = '$coinScheme:$recipient';
    bool addQueryDelimeter = true;

    if (amount != null) {
      uri += "?amount=${amount.toString()}";
      addQueryDelimeter = false;
    }
    for (final ref in references) {
      uri += "${addQueryDelimeter ? "?" : "&"}reference=$ref";
      addQueryDelimeter = false;
    }
    if (label != null) {
      uri += "${addQueryDelimeter ? "?" : "&"}label=$label";
      addQueryDelimeter = false;
    }
    if (message != null) {
      uri += "${addQueryDelimeter ? "?" : "&"}message=$message";
      addQueryDelimeter = false;
    }
    if (memo != null) {
      uri += "${addQueryDelimeter ? "?" : "&"}memo=$memo";
      addQueryDelimeter = false;
    }
    if (splToken != null) {
      uri += "${addQueryDelimeter ? "?" : "&"}spl-token=$splToken";
      addQueryDelimeter = false;
    }

    return uri;
  }
}
