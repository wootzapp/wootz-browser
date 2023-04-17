import 'dart:convert';

import 'package:dartez/reporting/conseil_data_client.dart';
import 'package:dartez/reporting/conseil_query_builder.dart';
import 'package:dartez/types/tezos/query_types.dart';

var bLOCKS = 'blocks';
var oPERATIONS = 'operations';

class TezosConseilClient {
  static awaitOperationConfirmation(serverInfo, network, hash, duration,
      {blocktime = 60}) async {
    if (blocktime == null) blocktime = 60;
    if (duration <= 0) {
      throw new Exception('Invalid duration');
    }
    var initialLevel = await getBlockHead(serverInfo, network);
    initialLevel = initialLevel['level'];
    var timeOffset = 180000;
    var startTime = new DateTime.now().millisecondsSinceEpoch - timeOffset;
    var estimatedEndTime = startTime + timeOffset + duration * blocktime * 1000;
    var currentLevel = initialLevel;
    var operationQuery = ConseilQueryBuilder.blankQuery();
    operationQuery = ConseilQueryBuilder.addPredicate(operationQuery,
        'operation_group_hash', QueryTypes.conseilOperator['EQ'], [hash],
        invert: false);
    operationQuery = ConseilQueryBuilder.addPredicate(operationQuery,
        'timestamp', QueryTypes.conseilOperator['AFTER'], [startTime],
        invert: false);
    operationQuery = ConseilQueryBuilder.setLimit(operationQuery, 1);
    while (initialLevel + duration > currentLevel) {
      var group =
          jsonDecode(await getOperations(serverInfo, network, operationQuery));
      if (group.length > 0) {
        return group[0];
      }
      currentLevel = (await getBlockHead(serverInfo, network))['level'];
      if (initialLevel + duration < currentLevel) {
        break;
      }
      if (new DateTime.now().millisecondsSinceEpoch > estimatedEndTime) {
        break;
      }
      await Future.delayed(Duration(milliseconds: blocktime * 1000));
    }
    throw new Exception(
        'Did not observe $hash on $network in $duration block${duration > 1 ? 's' : ''} since $initialLevel');
  }

  static getBlockHead(serverInfo, network) async {
    var query = ConseilQueryBuilder.setLimit(
        ConseilQueryBuilder.addOrdering(
            ConseilQueryBuilder.blankQuery(), 'level',
            direction: QueryTypes.conseilSortDirection['DESC']),
        1);
    var r = jsonDecode(
        await getTezosEntityData(serverInfo, network, bLOCKS, query));
    return r[0];
  }

  static getTezosEntityData(serverInfo, network, entity, query) async {
    return await ConseilDataClient.executeEntityQuery(
        serverInfo, 'tezos', network, entity, query);
  }

  static getOperations(serverInfo, network, query) {
    return getTezosEntityData(serverInfo, network, oPERATIONS, query);
  }
}
