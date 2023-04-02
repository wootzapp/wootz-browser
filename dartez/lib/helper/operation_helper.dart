import 'dart:async';

import 'package:dartez/helper/http_helper.dart';

class OperationHelper {
  /// Get operation status
  Future<String?> getOperationStatus(String server, String oprationHash) async {
    while (true) {
      var status = await _getStatus(server, oprationHash);
      if (status != "Pending") {
        return status;
      }
      await Future.delayed(Duration(seconds: 5));
    }
  }

  /// Get operation status
  Future<String?> _getStatus(String server, String opHash) async {
    var block = await getBlock(server);
    var operations = await (getOperations(server, block));
    var operation =
        operations!.where((element) => element['hash'] == opHash).toList();
    if (operation.isEmpty) {
      return "Pending";
    } else {
      var status =
          operation[0]['contents'][0]['metadata']['operation_result']['status'];
      return status;
    }
  }

  /// Get block
  Future<String> getBlock(String server) async {
    var response =
        await HttpHelper.performGetRequest(server, "chains/main/blocks");
    return response[0][0].toString();
  }

  /// Get operations from block
  Future<List<dynamic>?> getOperations(String server, String block) async {
    var response = await HttpHelper.performGetRequest(
        server, "chains/main/blocks/$block/operations/3");
    return response;
  }
}
