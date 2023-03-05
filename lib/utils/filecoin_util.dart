import 'dart:convert';
import 'dart:math';
import 'package:cryptowallet/utils/rpc_urls.dart';

import 'package:hive/hive.dart';
import 'package:http/http.dart' as http;
import 'package:hive_flutter/hive_flutter.dart';

import 'app_config.dart';

Future<int> _getFileCoinNonce(
  String addressPrefix,
  String baseUrl,
) async {
  try {
    final pref = Hive.box(secureStorageKey);
    String mnemonic = pref.get(currentMmenomicKey);
    final fileCoinDetails = await getFileCoinFromMemnomic(
      mnemonic,
      addressPrefix,
    );
    final response = await http.get(Uri.parse(
        '$baseUrl/actor/balance?actor=${Uri.encodeQueryComponent(fileCoinDetails['address'])}'));
    final responseBody = response.body;
    if (response.statusCode ~/ 100 == 4 || response.statusCode ~/ 100 == 5) {
      throw Exception(responseBody);
    }

    return jsonDecode(responseBody)['data']['nonce'];
  } catch (e) {
    return 0;
  }
}

Future getFileCoinTransactionFee(
  String addressPrefix,
  String baseUrl,
) async {
  Map<String, dynamic> fileCoinFees = await _getFileCoinGas(
    addressPrefix,
    baseUrl,
  );

  return ((fileCoinFees['GasPremium'] + fileCoinFees['GasFeeCap']) *
          fileCoinFees['GasLimit']) /
      pow(10, fileCoinDecimals);
}

Future<Map<String, dynamic>> _getFileCoinGas(
  String addressPrefix,
  String baseUrl,
) async {
  try {
    final pref = Hive.box(secureStorageKey);
    String mnemonic = pref.get(currentMmenomicKey);
    final fileCoinDetails =
        await getFileCoinFromMemnomic(mnemonic, addressPrefix);
    final response = await http.get(Uri.parse(
        '$baseUrl/recommend/fee?method=Send&actor=${Uri.encodeQueryComponent(fileCoinDetails['address'])}'));
    final responseBody = response.body;
    if (response.statusCode ~/ 100 == 4 || response.statusCode ~/ 100 == 5) {
      throw Exception(responseBody);
    }

    Map jsonDecodedBody = jsonDecode(responseBody);

    return {
      'GasLimit': jsonDecodedBody['data']['gas_limit'],
      'GasPremium': int.tryParse(jsonDecodedBody['data']['gas_premium']) ?? 0,
      'GasFeeCap': int.tryParse(jsonDecodedBody['data']['gas_cap']) ?? 0,
    };
  } catch (e) {
    return {};
  }
}

Future<Map> sendFilecoin(
  String destinationAddress,
  int filecoinToSend, {
  required String baseUrl,
  required String addressPrefix,
  List<String> references = const [],
}) async {
  final pref = Hive.box(secureStorageKey);
  String mnemonic = pref.get(currentMmenomicKey);
  final fileCoinDetails =
      await getFileCoinFromMemnomic(mnemonic, addressPrefix);
  final nonce = await _getFileCoinNonce(
    addressPrefix,
    baseUrl,
  );

  final msg = {
    "Version": 0,
    "To": destinationAddress,
    "From": fileCoinDetails['address'],
    "Nonce": nonce,
    "Value": '$filecoinToSend',
    "GasLimit": 0,
    "GasFeeCap": "0",
    "GasPremium": "100000",
    "Method": 0,
    "Params": ""
  };

  msg.addAll(await _getFileCoinGas(addressPrefix, baseUrl));
//FIXME:
  return {};
  // final cid = await Flotus.messageCid(msg: json.encode(msg));

  // String sign = await Flotus.secpSign(ck: fileCoinDetails['ck'], msg: cid);
  // const signTypeSecp = 1;

  // final response = await http.post(
  //   Uri.parse('$baseUrl/message'),
  //   headers: {'Content-Type': 'application/json'},
  //   body: json.encode({
  //     'cid': cid,
  //     'raw': json.encode({
  //       "Message": msg,
  //       "Signature": {
  //         "Type": signTypeSecp,
  //         "Data": sign,
  //       },
  //     })
  //   }),
  // );
  // final responseBody = response.body;
  // if (response.statusCode ~/ 100 == 4 || response.statusCode ~/ 100 == 5) {
  //   throw Exception(responseBody);
  // }

  // Map jsonDecodedBody = json.decode(responseBody) as Map;
  // if (jsonDecodedBody['code'] ~/ 100 != 2) {
  //   throw Exception(jsonDecodedBody['detail']);
  // }

  // return {'txid': jsonDecodedBody['data'].toString()};
}
