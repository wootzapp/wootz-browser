import 'dart:math';

import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/foundation.dart';
import 'package:hive_flutter/hive_flutter.dart';
import 'package:stellar_flutter_sdk/stellar_flutter_sdk.dart' as stellar
    hide Row;

Future<bool> isActiveStellarAccount(
  String address,
  stellar.StellarSDK sdk,
) async {
  try {
    stellar.KeyPair senderKeyPair = stellar.KeyPair.fromAccountId(address);
    await sdk.accounts.account(senderKeyPair.accountId);
    return true;
  } catch (e, stackTrace) {
    if (kDebugMode) {
      print(stackTrace);
    }
    return false;
  }
}

Future<double> getStellarGas(
  String destinationAddress,
  String stellarToSend,
  stellar.StellarSDK sdk,
) async {
  try {
    final pref = Hive.box(secureStorageKey);
    String mnemonic = pref.get(currentMmenomicKey);
    Map getStellarDetails = await getStellarFromMemnomic(mnemonic);

    stellar.KeyPair senderKeyPair =
        stellar.KeyPair.fromSecretSeed(getStellarDetails['private_key']);
    stellar.AccountResponse sender =
        await sdk.accounts.account(senderKeyPair.accountId);

    stellar.Transaction transaction = stellar.TransactionBuilder(sender)
        .addOperation(stellar.PaymentOperationBuilder(
          destinationAddress,
          stellar.Asset.NATIVE,
          stellarToSend.toString(),
        ).build())
        .build();

    return transaction.fee / pow(10, stellarDecimals);
  } catch (e, stackTrace) {
    if (kDebugMode) {
      print(stackTrace);
    }
    return 0;
  }
}

Future<Map> sendStellar(
  String destinationAddress,
  String stellarToSend,
  stellar.StellarSDK sdk,
  stellar.Network cluster,
) async {
  final pref = Hive.box(secureStorageKey);
  String mnemonic = pref.get(currentMmenomicKey);
  Map getStellarDetails = await getStellarFromMemnomic(mnemonic);
  stellar.KeyPair senderKeyPair =
      stellar.KeyPair.fromSecretSeed(getStellarDetails['private_key']);

  stellar.AccountResponse sender =
      await sdk.accounts.account(senderKeyPair.accountId);
  stellar.Operation operation;
  if (await isActiveStellarAccount(destinationAddress, sdk)) {
    operation = stellar.PaymentOperationBuilder(
      destinationAddress,
      stellar.Asset.NATIVE,
      stellarToSend,
    ).build();
  } else {
    operation = stellar.CreateAccountOperationBuilder(
      destinationAddress,
      stellarToSend,
    ).build();
  }

  stellar.Transaction transaction =
      stellar.TransactionBuilder(sender).addOperation(operation).build();

  transaction.sign(
    senderKeyPair,
    cluster,
  );

  stellar.SubmitTransactionResponse response =
      await sdk.submitTransaction(transaction);

  if (response.success) {
    return {'txid': response.hash};
  }
  throw Exception('could not send coin');
}
