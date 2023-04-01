import 'dart:convert';
import 'dart:typed_data';

import 'package:convert/convert.dart';
import 'package:dartez/chain/tezos/tezos_language_util.dart';
import 'package:dartez/chain/tezos/tezos_message_codec.dart';
import 'package:dartez/chain/tezos/tezos_message_utils.dart';
import 'package:dartez/chain/tezos/tezos_node_reader.dart';
import 'package:dartez/helper/constants.dart';
import 'package:dartez/helper/http_helper.dart';
import 'package:dartez/models/key_store_model.dart';
import 'package:dartez/models/operation_model.dart';
import 'package:dartez/src/soft-signer/soft_signer.dart';
import 'package:dartez/types/tezos/tezos_chain_types.dart';
import 'package:dartez/utils/fee_estimater.dart';

class TezosNodeWriter {
  static Future<Map<String, Object?>> sendTransactionOperation(String server,
      SoftSigner signer, KeyStoreModel keyStore, String to, int amount, int fee,
      {int offset = 54, required bool isKeyRevealed}) async {
    var results = await Future.wait(!isKeyRevealed
        ? [
            TezosNodeReader.getCounterForAccount(
                server, keyStore.publicKeyHash),
            TezosNodeReader.getBlockAtOffset(server, offset),
            TezosNodeReader.isManagerKeyRevealedForAccount(
                server, keyStore.publicKeyHash)
          ]
        : [
            TezosNodeReader.getCounterForAccount(
                server, keyStore.publicKeyHash),
            TezosNodeReader.getBlockAtOffset(server, offset)
          ]);

    var counter = (results[0] as int) + 1;
    var blockHead = results[1];
    isKeyRevealed = !isKeyRevealed ? results[2] as bool : isKeyRevealed;

    OperationModel transaction = new OperationModel(
      destination: to,
      amount: amount.toString(),
      counter: counter,
      source: keyStore.publicKeyHash,
      gasLimit: TezosConstants.DefaultTransactionGasLimit,
      storageLimit: TezosConstants.DefaultTransactionStorageLimit,
    );

    var operations = await appendRevealOperation(
      server,
      keyStore.publicKey,
      keyStore.publicKeyHash,
      counter - 1,
      <OperationModel>[transaction],
      isKeyRevealed,
    );
    return sendOperation(server, operations, signer, offset, blockHead);
  }

  static sendDelegationOperation(String server, SoftSigner signer,
      KeyStoreModel keyStore, String delegate, int fee, offset) async {
    var counter = await TezosNodeReader.getCounterForAccount(
            server, keyStore.publicKeyHash) +
        1;

    OperationModel delegation = new OperationModel(
      counter: counter,
      kind: 'delegation',
      source: keyStore.publicKeyHash,
      delegate: delegate,
    );

    var operations = await appendRevealOperation(server, keyStore.publicKey,
        keyStore.publicKeyHash, counter - 1, [delegation]);
    return sendOperation(server, operations, signer, offset);
  }

  static sendContractOriginationOperation(
      String server,
      SoftSigner signer,
      KeyStoreModel keyStore,
      int amount,
      String delegate,
      int fee,
      int storageLimit,
      int gasLimit,
      String code,
      String storage,
      TezosParameterFormat codeFormat,
      int offset) async {
    var counter = await TezosNodeReader.getCounterForAccount(
            server, keyStore.publicKeyHash) +
        1;
    var operation = constructContractOriginationOperation(
        keyStore,
        amount,
        delegate,
        fee,
        storageLimit,
        gasLimit,
        code,
        storage,
        codeFormat,
        counter);
    var operations = await appendRevealOperation(server, keyStore.publicKey,
        keyStore.publicKeyHash, counter - 1, [operation]);
    return sendOperation(server, operations, signer, offset);
  }

  static Future<Map<String, dynamic>> sendContractInvocationOperation(
    String server,
    SoftSigner signer,
    KeyStoreModel keyStore,
    List<String> contract,
    List<int> amount,
    int fee,
    int storageLimit,
    int gasLimit,
    List<String> entrypoint,
    List<String> parameters, {
    var parameterFormat = TezosParameterFormat.Michelson,
    offset = 54,
    bool? preapply,
    bool? gasEstimation = false,
  }) async {
    var counter = await TezosNodeReader.getCounterForAccount(
            server, keyStore.publicKeyHash) +
        1;

    var transactions = <OperationModel>[];

    for (var i = 0; i < entrypoint.length; i++) {
      transactions.add(
        constructContractInvocationOperation(
          keyStore.publicKeyHash,
          counter + i,
          contract[i],
          amount[i],
          fee,
          storageLimit,
          gasLimit,
          entrypoint[i],
          parameters[i],
          parameterFormat is List ? parameterFormat[i] : parameterFormat,
        ),
      );
    }
    var operations = await appendRevealOperation(server, keyStore.publicKey,
        keyStore.publicKeyHash, counter - 1, [...transactions]);
    return sendOperation(
        server, operations, signer, offset, null, preapply, gasEstimation);
  }

  static sendIdentityActivationOperation(String server, SoftSigner signer,
      KeyStoreModel keyStore, String activationCode) async {
    var activation = OperationModel(
      kind: 'activate_account',
      pkh: keyStore.publicKeyHash,
      secret: activationCode,
    );
    return await sendOperation(server, [activation], signer, 54);
  }

  static sendKeyRevealOperation(
      String server, signer, KeyStoreModel keyStore, fee, offset) async {
    var counter = (await TezosNodeReader.getCounterForAccount(
            server, keyStore.publicKeyHash)) +
        1;

    var revealOp = OperationModel(
      kind: 'reveal',
      source: keyStore.publicKeyHash,
      counter: counter,
      publicKey: keyStore.publicKey,
    );

    var operations = [revealOp];
    return sendOperation(server, operations, signer, offset);
  }

  static constructContractInvocationOperation(
      String publicKeyHash,
      int counter,
      String contract,
      int amount,
      int fee,
      int storageLimit,
      int gasLimit,
      entrypoint,
      String parameters,
      TezosParameterFormat parameterFormat) {
    OperationModel transaction = new OperationModel(
      destination: contract,
      amount: amount.toString(),
      counter: counter,
      source: publicKeyHash,
    );

    if (parameters != '') {
      if (parameterFormat == TezosParameterFormat.Michelson) {
        var michelineParams =
            TezosLanguageUtil.translateMichelsonToMicheline(parameters)!;
        transaction.parameters = {
          'entrypoint': entrypoint.isEmpty ? 'default' : entrypoint,
          'value': jsonDecode(michelineParams)
        };
      } else if (parameterFormat == TezosParameterFormat.Micheline) {
        transaction.parameters = {
          'entrypoint': entrypoint.isEmpty ? 'default' : entrypoint,
          'value': jsonDecode(parameters)
        };
      } else if (parameterFormat == TezosParameterFormat.MichelsonLambda) {
        var michelineLambda = TezosLanguageUtil.translateMichelsonToMicheline(
            'code $parameters')!;
        transaction.parameters = {
          'entrypoint': entrypoint.isEmpty ? 'default' : entrypoint,
          'value': jsonDecode(michelineLambda)
        };
      }
    } else if (entrypoint != null) {
      transaction.parameters = {'entrypoint': entrypoint, 'value': []};
    }
    return transaction;
  }

  static Future<List<OperationModel>> appendRevealOperation(
      String server,
      String? publicKey,
      String publicKeyHash,
      int accountOperationIndex,
      List<OperationModel> operations,
      [bool isKeyRevealed = false]) async {
    isKeyRevealed = !isKeyRevealed
        ? await TezosNodeReader.isManagerKeyRevealedForAccount(
            server, publicKeyHash)
        : isKeyRevealed;
    var counter = accountOperationIndex + 1;
    for (var i = 0; i < operations.length; i++) {
      operations[i].fee = '1500'; // default gas fee
    }
    if (!isKeyRevealed) {
      var revealOp = OperationModel(
        counter: counter,
        fee: '0', // Reveal Fee will be covered by the appended operation
        source: publicKeyHash,
        kind: 'reveal',
        gasLimit: TezosConstants.DefaultKeyRevealGasLimit,
        storageLimit: TezosConstants.DefaultKeyRevealStorageLimit,
        publicKey: publicKey,
      );
      for (var index = 0; index < operations.length; index++) {
        var c = accountOperationIndex + 2 + index;
        operations[index].counter = c;
      }

      return <OperationModel>[revealOp, ...operations];
    }

    return prepareOperation(server, operations);
  }

  static Future<List<OperationModel>> prepareOperation(
      String server, List<OperationModel> operations) async {
    var feeEstimater = FeeEstimater(server, operations);
    var estimate = await feeEstimater.getEstimateOperationGroup();
    operations[0].fee = estimate['estimatedFee'].toString();
    for (var i = 0; i < operations.length; i++) {
      operations[i].gasLimit = estimate['operationResources'][i]['gas'];
      operations[i].storageLimit =
          estimate['operationResources'][i]['storageCost'];
    }
    return operations;
  }

  static Future<Map<String, Object?>> sendOperation(String server,
      List<OperationModel> operations, SoftSigner signer, int offset,
      [blockHead, preapply, gasEstimation]) async {
    var _blockHead =
        blockHead ?? await TezosNodeReader.getBlockAtOffset(server, offset);
    var blockHash = _blockHead['hash'].toString().substring(0, 51);
    var forgedOperationGroup = forgeOperations(blockHash, operations);
    var opSignature = signer.signOperation(Uint8List.fromList(hex.decode(
        TezosConstants.OperationGroupWatermark + forgedOperationGroup)));
    var signedOpGroup = Uint8List.fromList(
        hex.decode(forgedOperationGroup) + opSignature.toList());
    var base58signature = TezosMessageUtils.readSignatureWithHint(
        opSignature, signer.getSignerCurve());
    var opPair = {'bytes': signedOpGroup, 'signature': base58signature};
    var appliedOp = await preapplyOperation(
        server, blockHash, _blockHead['protocol'], operations, opPair);
    if (preapply != null &&
        preapply &&
        gasEstimation != null &&
        gasEstimation) {
      return {
        'opPair': opPair,
        'gasEstimation': operations[0].fee.toString(),
      };
    }
    if (preapply != null && preapply) return opPair;
    var injectedOperation = await injectOperation(server, opPair);

    return {'appliedOp': appliedOp[0], 'operationGroupID': injectedOperation};
  }

  static String forgeOperations(
      String branch, List<OperationModel> operations) {
    String encoded = TezosMessageUtils.writeBranch(branch);
    for (var i = 0; i < operations.length; i++) {
      encoded += TezosMessageCodec.encodeOperation(operations[i]);
    }
    return encoded;
  }

  static preapplyOperation(String server, String branch, protocol,
      List<OperationModel> operations, Map<String, Object> signedOpGroup,
      {String chainid = 'main'}) async {
    var payload = [
      {
        'protocol': protocol,
        'branch': branch,
        'contents': operations,
        'signature': signedOpGroup['signature']
      }
    ];
    var response = await HttpHelper.performPostRequest(server,
        'chains/$chainid/blocks/head/helpers/preapply/operations', payload);
    var json;
    try {
      json = jsonDecode(response);
    } catch (err) {
      throw new Exception(
          'Could not parse JSON from response of chains/$chainid/blocks/head/helpers/preapply/operation: $response for $payload');
    }
    parseRPCError(jsonDecode(response));
    return json;
  }

  static void parseRPCError(json) {
    var errors = '';
    try {
      var arr = json is List ? json : [json];
      if (json[0]['kind'] != null) {
        errors = arr.fold(
            '',
            (previousValue, element) =>
                '$previousValue${element['kind']} : ${element['id']}, ');
      } else if (arr.length == 1 &&
          arr[0]['contents'].length == 1 &&
          arr[0]['contents'][0]['kind'].toString() == "activate_account") {
      } else {}
    } catch (err) {
      if (json.toString().startsWith('Failed to parse the request body: ')) {
        errors = json.toString().toString().substring(34);
      } else {
        var hash = json
            .toString()
            .replaceFirst('/\"/g', "'")
            .replaceFirst('/\n/', "'");
        if (hash.length == 51 && hash[0] == 'o') {
        } else {
          print(
              "failed to parse errors: '$err' from '${json.toString()}'\n, PLEASE report this to the maintainers");
        }
      }
    }

    if (errors.length > 0) {
      throw Exception(errors);
    }
  }

  static String? parseRPCOperationResult(result) {
    if (result["contents"] == null) {
      return "${result["id"].toString()}";
    } else if (result["status"].toString() == 'applied') {
      return '';
    } else {
      return result["status"].toString();
    }
  }

  static injectOperation(String server, Map<String, dynamic> opPair,
      {chainid = 'main'}) async {
    var response = await HttpHelper.performPostRequest(
        server,
        'injection/operation?chain=$chainid',
        hex.encode(opPair['bytes'] as List<int>));
    response = response.toString().replaceAll('"', '');
    return response;
  }

  static OperationModel constructContractOriginationOperation(
      KeyStoreModel keyStore,
      int amount,
      String delegate,
      int fee,
      int storageLimit,
      int gasLimit,
      String code,
      String storage,
      TezosParameterFormat codeFormat,
      int counter) {
    var parsedCode;
    var parsedStorage;
    if (codeFormat == TezosParameterFormat.Michelson) {
      parsedCode =
          jsonDecode(TezosLanguageUtil.translateMichelsonToMicheline(code)!);
      parsedStorage =
          jsonDecode(TezosLanguageUtil.translateMichelsonToMicheline(storage)!);
    } else if (codeFormat == TezosParameterFormat.Micheline) {
      parsedCode = jsonDecode(code);
      parsedStorage = jsonDecode(storage);
    }

    return OperationModel(
      kind: 'origination',
      source: keyStore.publicKeyHash,
      counter: counter,
      amount: amount.toString(),
      delegate: delegate,
      script: {
        'code': parsedCode,
        'storage': parsedStorage,
      },
    );
  }
}
