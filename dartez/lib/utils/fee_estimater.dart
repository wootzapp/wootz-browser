import 'dart:convert';

import 'package:dartez/chain/tezos/tezos_node_writer.dart';
import 'package:dartez/dartez.dart';
import 'package:dartez/helper/constants.dart';
import 'package:dartez/helper/http_helper.dart';
import 'package:dartez/models/operation_model.dart';
import 'package:flutter/foundation.dart';

import 'dart:math' as math;

class FeeEstimater {
  String server;
  String chainId;
  List<OperationModel> operations;
  int GAS_BUFFER = 100;

  FeeEstimater(this.server, this.operations, {this.chainId = 'main'});

  Future<Map<String, dynamic>> getEstimateOperationGroup() async {
    var operationResources = [];

    for (var i = 0; i < operations.length; i++) {
      var priorConsumedResources = <String, dynamic>{
        'gas': 0,
        'storageCost': 0
      };

      if (i > 0) {
        var priorTransactions = operations.sublist(0, i);
        priorConsumedResources =
            await estimateOperation(server, chainId, priorTransactions);
      }
      var currentTransactions = operations.sublist(0, i + 1);
      var currentConsumedResources =
          await estimateOperation(server, chainId, currentTransactions);

      var gasLimitDelta =
          currentConsumedResources['gas'] - priorConsumedResources['gas'];
      var storageLimitDelta = currentConsumedResources['storageCost'] -
          priorConsumedResources['storageCost'];

      operationResources.add({
        'gas': gasLimitDelta == GAS_BUFFER
            ? gasLimitDelta + TezosConstants.GasLimitPadding
            : gasLimitDelta,
        'storageCost': storageLimitDelta + TezosConstants.StorageLimitPadding
      });
    }

    var staticFee =
        (operations.where((o) => o.kind == 'reveal').toList().length == 1)
            ? 1270
            : 0;

    var validBranch = 'BMLxA4tQjiu1PT2x3dMiijgvMTQo8AVxkPBPpdtM8hCfiyiC1jz';
    var gasLimitTotal = operationResources
        .map((r) => r['gas'])
        .fold<int>(0, (a, c) => (a + c).toInt());
    var storageLimitTotal = operationResources
        .map((r) => r['storageCost'])
        .fold<int>(0, (a, c) => (a + c).toInt());
    var forgedOperationGroup =
        TezosNodeWriter.forgeOperations(validBranch, operations);
    var groupSize = forgedOperationGroup.length / 2 + 64;
    var estimatedFee = staticFee +
        (gasLimitTotal ~/ 10) +
        TezosConstants.BaseOperationFee +
        groupSize +
        TezosConstants.DefaultBakerVig;
    var estimatedStorageBurn = double.parse(
            (storageLimitTotal * TezosConstants.StorageRate).toString())
        .ceil();

    if (num.parse(operations[0].fee!) < estimatedFee) {
      estimatedFee += 16;
    }

    // debugPrint('group estimate' +
    //     operationResources.toString() +
    //     '' +
    //     estimatedFee.toString() +
    //     '' +
    //     estimatedStorageBurn.toString());

    return {
      'operationResources': operationResources,
      'estimatedFee': estimatedFee.ceil(),
      'estimatedStorageBurn': estimatedStorageBurn
    };
  }

  estimateOperation(String server, String chainId,
      List<OperationModel> priorTransactions) async {
    var naiveOperationGasCap = math
        .min((TezosConstants.BlockGasCap / priorTransactions.length).floor(),
            TezosConstants.OperationGasCap)
        .toString();

    var localOperations = [...priorTransactions]
        .map((e) => e
          ..gasLimit = int.parse(naiveOperationGasCap)
          ..storageLimit = TezosConstants.OperationStorageCap)
        .toList();

    var responseJSON = await dryRunOperation(server, chainId, localOperations);

    var gas = 0;
    var storageCost = 0;
    var staticFee = 0;
    for (var ele in responseJSON['contents']) {
      try {
        gas += (int.parse(ele['metadata']['operation_result']
                        ['consumed_milligas']
                    .toString()) ~/
                1000) +
            GAS_BUFFER;
        storageCost += int.parse((ele['metadata']['operation_result']
                    ['paid_storage_size_diff'] ??
                '0')
            .toString());

        if (ele['kind'] == 'origination' ||
            ele['metadata']['operation_result']
                ['allocated_destination_contract']) {
          storageCost += TezosConstants.EmptyAccountStorageBurn;
        } else if (ele['kind'] == 'reveal') {
          staticFee += 1270;
        }
      } catch (e) {
        // debugPrint(e.toString());
      }

      var internalOperations = ele['metadata']['internal_operation_results'];
      if (internalOperations == null) {
        continue;
      }

      for (var internalOperation in internalOperations) {
        var result = internalOperation['result'];
        gas += (int.parse(result['consumed_milligas'] ?? '0') ~/ 1000) +
            GAS_BUFFER;
        storageCost += int.parse(result['paid_storage_size_diff'] ?? '0');
        if (internalOperation['kind'] == 'origination') {
          storageCost += TezosConstants.EmptyAccountStorageBurn;
        }
      }
    }

    var validBranch = 'BMLxA4tQjiu1PT2x3dMiijgvMTQo8AVxkPBPpdtM8hCfiyiC1jz';
    var forgedOperationGroup =
        TezosNodeWriter.forgeOperations(validBranch, priorTransactions);
    var operationSize = forgedOperationGroup.length / 2 + 64;
    var estimatedFee = staticFee +
        (gas / 10).ceil() +
        TezosConstants.BaseOperationFee +
        operationSize +
        TezosConstants.DefaultBakerVig;
    var estimatedStorageBurn =
        (storageCost * TezosConstants.StorageRate).ceil();
    // debugPrint(
    //     'TezosNodeWriter.estimateOperation; gas: $gas, storage: $storageCost, fee estimate: $estimatedFee, burn estimate: $estimatedStorageBurn');

    return {
      'gas': gas,
      'storageCost': storageCost,
      'estimatedFee': estimatedFee,
      'estimatedStorageBurn': estimatedStorageBurn
    };
  }

  dryRunOperation(String server, String chainId,
      List<OperationModel> localOperations) async {
    const fake_signature =
        'edsigtkpiSSschcaCt9pUVrpNPf7TTcgvgDEDD6NCEHMy8NNQJCGnMfLZzYoQj74yLjo9wx6MPVV29CvVzgi7qEcEUok3k7AuMg';
    var fakeBranch = await Dartez.getBlock(server);
    var payload = {
      'operation': {
        'branch': fakeBranch,
        'contents': localOperations,
        'signature': fake_signature
      }
    };
    var response = await HttpHelper.performPostRequest(
        server, 'chains/$chainId/blocks/head/helpers/scripts/run_operation', {
      'chain_id': 'NetXdQprcVkpaWU',
      ...payload,
    });

    return jsonDecode(response.toString());
  }
}
