import 'package:bitbox/bitbox.dart';

class Bip21 {
  static Map<String, dynamic> decode(String uri) {
    if (uri.indexOf('bitcoincash') != 0 || uri['bitcoincash'.length] != ":")
      throw ("Invalid BIP21 URI");

    int split = uri.indexOf("?");
    Map<String, String> uriOptions = Uri.parse(uri).queryParameters;

    Map<String, dynamic> options = Map.from({
      "message": uriOptions["message"],
      "label": uriOptions["label"],
    });

    String address = uri.substring(0, split == -1 ? null : split);

    if (uriOptions["amount"] != null) {
      if (uriOptions["amount"].indexOf(",") != -1)
        throw ("Invalid amount: commas are invalid");

      double amount = double.tryParse(uriOptions["amount"]);
      if (amount == null || amount.isNaN)
        throw ("Invalid amount: not a number");
      if (!amount.isFinite) throw ("Invalid amount: not finite");
      if (amount < 0) throw ("Invalid amount: not positive");
      options["amount"] = amount;
    }

    return {
      'address': address,
      'options': options,
    };
  }

  static String encode(String address, Map<String, dynamic> options) {
    var hasCashPrefix = address.startsWith('bitcoincash:');
    if (!hasCashPrefix) {
      address = 'bitcoincash:$address';
    }

    // Using toLegacyAddress method to test validity of the address format
    try {
      Address.toLegacyAddress(address);
    } catch (e) {
      throw FormatException("Invalid Address Format: $address");
    }

    String query = "";
    if (options != null && options.isNotEmpty) {
      if (options['amount'] != null) {
        if (!options['amount'].isFinite) throw ("Invalid amount: not finite");
        if (options['amount'] < 0) throw ("Invalid amount: not positive");
      }

      Map<String, dynamic> uriOptions = Map.from(options);
      uriOptions.removeWhere((key, value) => value == null);
      uriOptions.forEach((key, value) {
        uriOptions[key] = value.toString();
      });

      if (uriOptions.isEmpty) uriOptions = null;
      query = Uri(queryParameters: uriOptions).toString();
      // Dart isn't following RFC-3986...
      query = query.replaceAll(RegExp(r"\+"), "%20");
    }

    return "$address$query";
  }
}
