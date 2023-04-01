import 'dart:async';

import 'package:dartez/helper/http_helper.dart';

class TezosNodeReader {
  static Future<int> getCounterForAccount(String server, String publicKeyHash,
      {String chainid = 'main'}) async {
    var response = await HttpHelper.performGetRequest(server,
        'chains/$chainid/blocks/head/context/contracts/$publicKeyHash/counter');
    return int.parse(response.toString().replaceAll('"', ''), radix: 10);
  }

  static Future<bool> isManagerKeyRevealedForAccount(
      String server, String publicKeyHash) async {
    var managerKey =
        await getAccountManagerForBlock(server, 'head', publicKeyHash);
    return managerKey.length > 0;
  }

  static Future<String> getAccountManagerForBlock(
      String server, String block, String publicKeyHash,
      {String chainid = 'main'}) async {
    var response = await HttpHelper.performGetRequest(server,
        'chains/$chainid/blocks/$block/context/contracts/$publicKeyHash/manager_key');
    return response != null ? response.toString() : '';
  }

  static Future<Map<dynamic, dynamic>?> getBlockAtOffset(
      String server, int offset,
      {String chainid = 'main'}) async {
    if (offset <= 0) {
      return await getBlock(server);
    }
    var head = await (getBlock(server));
    var response = await HttpHelper.performGetRequest(
        server, 'chains/$chainid/blocks/${head!['header']['level'] - offset}');
    return response;
  }

  static Future<Map<dynamic, dynamic>?> getBlock(String server,
      {String hash = 'head', String chainid = 'main'}) async {
    var response = await HttpHelper.performGetRequest(
        server, 'chains/$chainid/blocks/$hash');
    return response;
  }

  static Future<Map<dynamic, dynamic>?> getContractStorage(server, accountHash,
      {block = 'head', chainid = 'main'}) async {
    return await HttpHelper.performGetRequest(server,
        'chains/$chainid/blocks/$block/context/contracts/$accountHash/storage');
  }

  static getValueForBigMapKey(String server, String index, String key,
      {String? block, String? chainid}) async {
    return await HttpHelper.performGetRequest(
        server, 'chains/$chainid/blocks/$block/context/big_maps/$index/$key');
  }
}
