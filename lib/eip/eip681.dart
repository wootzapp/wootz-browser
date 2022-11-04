import 'package:decimal/decimal.dart';

class EIP681 {
  // Parse an Ethereum URI according to ERC-831 and ERC-681
// inspired from brunobar79 github

  static Map parse(String uri) {
    // to lowercase
    uri = uri.trim();

    if (uri == null || uri.runtimeType != String) {
      throw Exception('uri must be a string');
    }

    if (uri.substring(0, 9).toLowerCase() != 'ethereum:') {
      throw Exception('Not an Ethereum URI');
    }

    String prefix;
    String addressRegex = '(0x[\\w]{40})';

    if (uri.substring(9, 11).toLowerCase() == '0x') {
      prefix = null;
    } else {
      int cutOff = uri.indexOf('-', 9);
      String rest = uri.substring(9);

      if (cutOff != -1) {
        prefix = uri.substring(9, cutOff);
        rest = uri.substring(cutOff + 1);
      }

      // Adapting the regex if ENS name detected
      if (rest.substring(0, 2).toLowerCase() != '0x') {
        addressRegex =
            '([a-zA-Z0-9][a-zA-Z0-9-]{1,61}[a-zA-Z0-9].[a-zA-Z]{2,})';
      }
    }

    String fullRegex = ('^ethereum:(' +
        (prefix ?? '') +
        '-)?' +
        (addressRegex ?? '') +
        '\\@?([\\w]*)*\\/?([\\w]*)*');

    RegExp regex = RegExp(fullRegex);

    final match = regex.firstMatch(uri);

    if (match == null) {
      throw Exception('Invalid Ethereum URI');
    }

    dynamic parameters = uri.split('?');

    parameters = parameters.length > 1 ? parameters[1] : '';

    // convert params to map
    Map<String, String> params = {};

    parameters.split('&').forEach((param) {
      final keyValue = param.split('=');
      if (keyValue.length == 2) {
        // url decode key and value
        params[Uri.decodeComponent(keyValue[0])] =
            Uri.decodeComponent(keyValue[1]);
      }
    });
    Map finalMapping;

    Map<dynamic, dynamic> obj = {
      'scheme': 'ethereum',
      'target_address': match.group(2),
    };

    if (prefix != null) {
      obj['prefix'] = prefix;
    }

    if (match.group(3) != null) {
      obj['chainId'] = match.group(3);
    }

    if (match.group(4) != null) {
      obj['functionName'] = match.group(4);
    }
    if (params.isNotEmpty) {
      obj['parameters'] = params;
      String amountKey = 'amount';

      if (obj['functionName'] == 'transfer') {
        amountKey = 'uint256';
      } else if (obj['parameters']['value'] != null) {
        amountKey = 'value';
      }

      if (obj['parameters'][amountKey] != null) {
        final amount = obj['parameters'][amountKey];
        (obj['parameters']).remove(amountKey);
        finalMapping = {...obj};
        finalMapping['parameters'] = {
          ...finalMapping['parameters'],
          amountKey: Decimal.parse(amount).toString(),
        };

        if (Decimal.parse(finalMapping['parameters'][amountKey]).toBigInt() <
            BigInt.from(0)) {
          throw Exception('Invalid amount');
        }
      }
    }

    return finalMapping ?? obj;
  }

// Builds a valid Ethereum URI based on the initial parameters

  static String build({
    String prefix,
    String targetAddress,
    String chainId,
    String functionName,
    Map parameters,
  }) {
    String query;
    if (parameters != null && parameters.isNotEmpty) {
      String amountKey = 'amount';
      if (functionName == 'transfer') {
        amountKey = 'uint256';
      } else if (parameters['value'] != null) {
        amountKey = 'value';
      }

      Map parametersValues;

      if (parameters[amountKey] != null) {
        final amount = parameters[amountKey];
        parameters.remove(amountKey);

        parametersValues = {...parameters};
        parametersValues[amountKey] = Decimal.parse(amount).toBigInt();

        if ((parametersValues[amountKey] as BigInt).toInt() < 0) {
          throw Exception('Invalid amount');
        }
      }

      parameters = parametersValues ?? parameters;

      // convert map to query string
      query = parameters.keys.map((key) {
        return '$key=${parameters[key]}';
      }).join('&');
    }

    return "ethereum:${prefix != null ? prefix + '-' : ''}$targetAddress${chainId != null ? '@$chainId' : ''}${functionName != null ? '/$functionName' : ''}${query != null ? '?' + query : ''}";
  }
}
