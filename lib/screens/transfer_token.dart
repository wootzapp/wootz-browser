import 'dart:async';
import 'dart:math';
import 'package:cryptowallet/api/notification_api.dart';
import 'package:cryptowallet/components/loader.dart';
import 'package:cryptowallet/config/colors.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/bitcoin_util.dart';
import 'package:cryptowallet/utils/format_money.dart';
// import 'package:cardano_wallet_sdk/cardano_wallet_sdk.dart' as cardano;
import 'package:cryptowallet/utils/stellar_utils.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'dart:convert';
import 'package:cryptowallet/utils/rpc_urls.dart';
// import 'package:get/get.dart';
import 'package:hive/hive.dart';
import 'package:http/http.dart';
import 'package:intl/intl.dart';
import 'package:multi_value_listenable_builder/multi_value_listenable_builder.dart';
import 'package:web3dart/web3dart.dart' as web3;
import 'package:web3dart/web3dart.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import '../utils/filecoin_util.dart';

class TransferToken extends StatefulWidget {
  final Map data;
  final String? cryptoDomain;
  const TransferToken({Key? key, required this.data, this.cryptoDomain})
      : super(key: key);

  @override
  _TransferTokenState createState() => _TransferTokenState();
}

class _TransferTokenState extends State<TransferToken> {
  final _scaffoldKey = GlobalKey<ScaffoldState>();
  // RxBool isSending = false.obs;
  final isSending = ValueNotifier<bool>(false);
  // RxBool allowSend = true.obs;
  final allowSend = ValueNotifier<bool>(true);

  bool? get kDebugMode => null;
  late Timer timer;
  // RxMap transactionFeeMap = {}.obs;
  final transactionFeeMap = ValueNotifier<Map<dynamic, dynamic>>({});
  late bool isContract;
  late bool isBitcoinType;
  late bool isSolana;
  late bool isCardano;
  late bool isFilecoin;
  late bool isStellar;
  late bool isNFTTransfer;
  // RxString userAddress = ''.obs;
  final userAddress = ValueNotifier<String>('');

  @override
  void initState() {
    super.initState();
    isContract = widget.data['contractAddress'] != null;
    isBitcoinType = widget.data['POSNetwork'] != null;
    isSolana = widget.data['default'] == 'SOL';
    isCardano = widget.data['default'] == 'ADA';
    isFilecoin = widget.data['default'] == 'FIL';
    isStellar = widget.data['default'] == 'XLM';
    isNFTTransfer = widget.data['isNFT'] != null;
    getTransactionFee();
    getUserAddress();
    timer = Timer.periodic(
      httpPollingDelay,
      (Timer t) async => await getTransactionFee(),
    );
  }

  @override
  void dispose() {
    timer?.cancel();
    super.dispose();
  }

  late Map transactionFee;

  Future getTransactionFee() async {
    try {
      String mnemonic = Hive.box(secureStorageKey).get(currentMmenomicKey);

      if (isContract) {
        final client = web3.Web3Client(
          widget.data['rpc'],
          Client(),
        );

        Map response = await getEthereumFromMemnomic(
          mnemonic,
          widget.data['coinType'],
        );

        final sendingAddress = web3.EthereumAddress.fromHex(
          response['eth_wallet_address'],
        );

        final contract = web3.DeployedContract(
            web3.ContractAbi.fromJson(
                isNFTTransfer ? erc721Abi : erc20Abi, widget.data['name']),
            web3.EthereumAddress.fromHex(widget.data['contractAddress']));

        web3.ContractFunction decimalsFunction;

        BigInt decimals = BigInt.zero;

        if (!isNFTTransfer) {
          decimalsFunction = contract.function('decimals');
          decimals = (await client.call(
                  contract: contract, function: decimalsFunction, params: []))
              .first;
        }

        final transfer = isNFTTransfer
            ? contract.findFunctionsByName('safeTransferFrom').toList()[0]
            : contract.function('transfer');

        final contractData = isNFTTransfer
            ? transfer.encodeCall([
                sendingAddress,
                web3.EthereumAddress.fromHex(widget.data['recipient']),
                BigInt.from(widget.data['tokenId'])
              ])
            : transfer.encodeCall(
                [
                  web3.EthereumAddress.fromHex(widget.data['recipient']),
                  BigInt.from(
                    double.parse(widget.data['amount']) *
                        pow(
                          10,
                          double.parse(decimals.toString()),
                        ),
                  )
                ],
              );

        final transactionFee = await getEtherTransactionFee(
          widget.data['rpc'],
          contractData,
          sendingAddress,
          web3.EthereumAddress.fromHex(
            widget.data['contractAddress'],
          ),
        );
        final userBalance =
            (await client.getBalance(sendingAddress)).getInWei.toDouble() /
                pow(10, etherDecimals);

        final blockChainCost = transactionFee / pow(10, etherDecimals);

        transactionFeeMap.value = {
          'transactionFee': blockChainCost,
          'userBalance': userBalance,
        };
      } else if (isBitcoinType) {
        final getBitcoinDetails = await getBitcoinFromMemnomic(
          mnemonic,
          widget.data['POSNetwork'],
        );
        final bitCoinBalance = await getBitcoinAddressBalance(
          getBitcoinDetails['address'],
          widget.data['POSNetwork'],
        );

        final getUnspentOutput =
            await getUnspentTransactionBitcoinType(widget.data['POSNetwork']);

        final fee = await getBitcoinTypeNetworkFee(
            widget.data['POSNetwork'],
            (double.parse(widget.data['amount']) * pow(10, 8)).toInt(),
            getUnspentOutput);

        double feeInBitcoin = fee / pow(10, bitCoinDecimals);

        transactionFeeMap.value = {
          'transactionFee': feeInBitcoin,
          'userBalance': bitCoinBalance,
        };
      } else if (isSolana) {
        final getSolanaDetails = await getSolanaFromMemnomic(mnemonic);
        final solanaCoinBalance = await getSolanaAddressBalance(
          getSolanaDetails['address'],
          widget.data['solanaCluster'],
        );

        final fees = await getSolanaClient(widget.data['solanaCluster'])
            .rpcClient
            .getFees();

        transactionFeeMap.value = {
          'transactionFee':
              fees.feeCalculator.lamportsPerSignature / pow(10, solanaDecimals),
          'userBalance': solanaCoinBalance,
        };
      } else if (isCardano) {
        final getCardanoDetails = await getCardanoFromMemnomic(
          mnemonic,
          widget.data['cardano_network'],
        );
        final cardanoCoinBalance = await getCardanoAddressBalance(
          getCardanoDetails['address'],
          widget.data['cardano_network'],
          widget.data['blockFrostKey'],
        );

        final fees = maxFeeGuessForCardano / pow(10, cardanoDecimals);

        transactionFeeMap.value = {
          'transactionFee': fees,
          'userBalance': cardanoCoinBalance,
        };
      } else if (isFilecoin) {
        final getFileCoinDetails = await getFileCoinFromMemnomic(
          mnemonic,
          widget.data['prefix'],
        );
        final fileCoinBalance = await getFileCoinAddressBalance(
          getFileCoinDetails['address'],
          baseUrl: widget.data['baseUrl'],
        );

        final fees = await getFileCoinTransactionFee(
          widget.data['prefix'],
          widget.data['baseUrl'],
        );

        transactionFeeMap.value = {
          'transactionFee': fees,
          'userBalance': fileCoinBalance,
        };
      } else if (isStellar) {
        final getStellarDetails = await getStellarFromMemnomic(mnemonic);

        final stellarBalance = await getStellarAddressBalance(
          getStellarDetails['address'],
          widget.data['sdk'],
          widget.data['cluster'],
        );

        final fees = await getStellarGas(
          widget.data['recipient'],
          widget.data['amount'],
          widget.data['sdk'],
        );

        transactionFeeMap.value = {
          'transactionFee': fees,
          'userBalance': stellarBalance,
        };
      } else {
        final response = await getEthereumFromMemnomic(
          mnemonic,
          widget.data['coinType'],
        );
        final client = web3.Web3Client(
          widget.data['rpc'],
          Client(),
        );

        final transactionFee = await getEtherTransactionFee(
          widget.data['rpc'],
          null,
          web3.EthereumAddress.fromHex(
            response['eth_wallet_address'],
          ),
          web3.EthereumAddress.fromHex(
            widget.data['recipient'],
          ),
        );

        final userBalance = (await client.getBalance(
                    EthereumAddress.fromHex(response['eth_wallet_address'])))
                .getInWei
                .toDouble() /
            pow(10, etherDecimals);

        final blockChainCost = transactionFee / pow(10, etherDecimals);

        transactionFeeMap.value = {
          'transactionFee': blockChainCost,
          'userBalance': userBalance,
        };
      }
    } catch (_) {}
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      key: _scaffoldKey,
      appBar: AppBar(title: Text(AppLocalizations.of(context)!.transfer)),
      body: SafeArea(
        child: RefreshIndicator(
          onRefresh: () async {
            await Future.delayed(const Duration(seconds: 2));
          },
          child: SingleChildScrollView(
            physics: const AlwaysScrollableScrollPhysics(),
            child: Padding(
                padding: const EdgeInsets.all(25),
                child: MultiValueListenableBuilder(
                  valueListenables: [
                    isSending,
                    allowSend,
                    transactionFeeMap,
                    userAddress
                  ],
                  builder: (context, value, child) => Column(
                    mainAxisAlignment: MainAxisAlignment.start,
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        '${isNFTTransfer ? '' : '-'}${isNFTTransfer ? '' : formatMoney(widget.data['amount'])} ${isContract ? ellipsify(str: widget.data['symbol']) : widget.data['symbol']}',
                        style: const TextStyle(
                            fontSize: 25, fontWeight: FontWeight.bold),
                      ),
                      const SizedBox(
                        height: 20,
                      ),
                      Text(
                        AppLocalizations.of(context)!.asset,
                        style: const TextStyle(
                            fontSize: 18, fontWeight: FontWeight.bold),
                      ),
                      const SizedBox(
                        height: 10,
                      ),
                      Text(
                        '${isContract ? ellipsify(str: widget.data['name']) : widget.data['name']} (${isContract ? ellipsify(str: widget.data['symbol']) : widget.data['symbol']})',
                        style: const TextStyle(fontSize: 16),
                      ),
                      const SizedBox(
                        height: 20,
                      ),
                      Text(
                        AppLocalizations.of(context)!.from,
                        style: const TextStyle(
                            fontSize: 18, fontWeight: FontWeight.bold),
                      ),
                      const SizedBox(
                        height: 10,
                      ),
                      Text(
                        userAddress.value ?? '',
                        style: const TextStyle(fontSize: 16),
                      ),
                      const SizedBox(
                        height: 20,
                      ),
                      Text(
                        AppLocalizations.of(context)!.to,
                        style: const TextStyle(
                            fontSize: 18, fontWeight: FontWeight.bold),
                      ),
                      const SizedBox(
                        height: 10,
                      ),
                      Text(
                        widget.cryptoDomain != null
                            ? '${widget.cryptoDomain} (${widget.data['recipient']})'
                            : widget.data['recipient'],
                        style: const TextStyle(fontSize: 16),
                      ),
                      const SizedBox(
                        height: 20,
                      ),
                      if (transactionFeeMap.value.isNotEmpty) ...[
                        Text(
                          AppLocalizations.of(context)!.transactionFee,
                          style: const TextStyle(
                              fontSize: 18, fontWeight: FontWeight.bold),
                        ),
                        const SizedBox(
                          height: 10,
                        ),
                        widget.data['default'] != null
                            ? Text(
                                '${transactionFeeMap != null ? transactionFeeMap.value['transactionFee'] : '0'}  ${widget.data['default']}',
                                style: const TextStyle(fontSize: 16),
                              )
                            : Container(),
                        widget.data['network'] != null
                            ? Text(
                                '${transactionFeeMap != null ? transactionFeeMap.value['transactionFee'] : '0'}  ${getEVMBlockchains()[widget.data['network']]['symbol']}',
                                style: const TextStyle(fontSize: 16),
                              )
                            : Container(),
                        const SizedBox(
                          height: 20,
                        ),
                      ],
                      if (userAddress == widget.data['recipient'])
                        Column(
                          children: [
                            Container(
                              decoration: BoxDecoration(
                                borderRadius:
                                    const BorderRadius.all(Radius.circular(10)),
                                color: Colors.red[100],
                              ),
                              child: Padding(
                                padding: const EdgeInsets.all(10),
                                child: Row(
                                  children: [
                                    SizedBox(
                                      width: 20,
                                      height: 20,
                                      child: Checkbox(
                                        value: allowSend.value,
                                        onChanged: (onChanged) {
                                          allowSend.value = onChanged!;
                                        },
                                        fillColor:
                                            MaterialStateProperty.resolveWith(
                                          (states) => appBackgroundblue,
                                        ),
                                      ),
                                    ),
                                    const SizedBox(
                                      width: 10,
                                    ),
                                    Text(
                                      AppLocalizations.of(context)!
                                          .sendingToYourself,
                                      textAlign: TextAlign.center,
                                      style: const TextStyle(
                                        color: Colors.red,
                                      ),
                                    ),
                                  ],
                                ),
                              ),
                            ),
                            const SizedBox(
                              height: 20,
                            ),
                          ],
                        ),
                      if (allowSend.value)
                        transactionFeeMap != null
                            ? Container(
                                color: Colors.transparent,
                                width: double.infinity,
                                child: ElevatedButton(
                                  style: ButtonStyle(
                                    backgroundColor:
                                        MaterialStateProperty.resolveWith(
                                            (states) => appBackgroundblue),
                                    shape: MaterialStateProperty.resolveWith(
                                      (states) => RoundedRectangleBorder(
                                        borderRadius: BorderRadius.circular(10),
                                      ),
                                    ),
                                  ),
                                  onPressed: transactionFeeMap
                                                  .value['userBalance'] ==
                                              null ||
                                          transactionFeeMap
                                                  .value['userBalance'] <=
                                              0
                                      ? () {
                                          // Get.snackbar(
                                          //   '',
                                          //   AppLocalizations.of(context)!
                                          //       .insufficientBalance,
                                          //   colorText: Colors.white,
                                          //   backgroundColor: Colors.red,
                                          // );
                                          ScaffoldMessenger.of(context)
                                              .showSnackBar(SnackBar(
                                            content: Text(
                                                AppLocalizations.of(context)!
                                                    .insufficientBalance),
                                            backgroundColor: Colors.red,
                                            action: SnackBarAction(
                                              label: 'OK',
                                              onPressed: () {},
                                              textColor: Colors.white,
                                            ),
                                          ));
                                        }
                                      : () async {
                                          if (isSending.value) return;
                                          if (await authenticate(context)) {
                                            // Get.closeAllSnackbars();
                                            ScaffoldMessenger.of(context)
                                                .removeCurrentSnackBar();

                                            isSending.value = true;

                                            try {
                                              final pref =
                                                  Hive.box(secureStorageKey);
                                              String mnemonic =
                                                  pref.get(currentMmenomicKey);

                                              String userAddress = '';
                                              String transactionHash = '';
                                              int coinDecimals = 0;
                                              String userTransactionsKey = '';
                                              if (isContract) {
                                                final client = web3.Web3Client(
                                                  widget.data['rpc'],
                                                  Client(),
                                                );

                                                Map response =
                                                    await getEthereumFromMemnomic(
                                                  mnemonic,
                                                  widget.data['coinType'],
                                                );
                                                final credentials =
                                                    EthPrivateKey.fromHex(
                                                  response[
                                                      'eth_wallet_privateKey'],
                                                );

                                                final sendingAddress = web3
                                                    .EthereumAddress.fromHex(
                                                  response[
                                                      'eth_wallet_address'],
                                                );

                                                final contract =
                                                    web3.DeployedContract(
                                                  web3.ContractAbi.fromJson(
                                                    isNFTTransfer
                                                        ? erc721Abi
                                                        : erc20Abi,
                                                    widget.data['name'],
                                                  ),
                                                  web3.EthereumAddress.fromHex(
                                                    widget.data[
                                                        'contractAddress'],
                                                  ),
                                                );

                                                web3.ContractFunction
                                                    decimalsFunction;

                                                BigInt decimals;

                                                ContractFunction transfer;

                                                List _parameters;

                                                if (isNFTTransfer) {
                                                  _parameters = [
                                                    sendingAddress,
                                                    web3.EthereumAddress
                                                        .fromHex(widget
                                                            .data['recipient']),
                                                    BigInt.from(
                                                      widget.data['tokenId'],
                                                    )
                                                  ];

                                                  transfer = contract
                                                      .findFunctionsByName(
                                                          'safeTransferFrom')
                                                      .toList()[0];
                                                } else {
                                                  transfer = contract
                                                      .function('transfer');

                                                  decimalsFunction = contract
                                                      .function('decimals');
                                                  decimals = (await client.call(
                                                          contract: contract,
                                                          function:
                                                              decimalsFunction,
                                                          params: []))
                                                      .first;

                                                  _parameters = [
                                                    web3.EthereumAddress
                                                        .fromHex(
                                                      widget.data['recipient'],
                                                    ),
                                                    BigInt.from(
                                                      double.parse(widget
                                                              .data['amount']) *
                                                          pow(10,
                                                              decimals.toInt()),
                                                    )
                                                  ];

                                                  coinDecimals =
                                                      decimals.toInt();
                                                }

                                                final trans = await client
                                                    .signTransaction(
                                                  credentials,
                                                  Transaction.callContract(
                                                    contract: contract,
                                                    function: transfer,
                                                    parameters: _parameters,
                                                  ),
                                                  chainId:
                                                      widget.data['chainId'],
                                                );

                                                transactionHash = await client
                                                    .sendRawTransaction(trans);

                                                userAddress = response[
                                                    'eth_wallet_address'];

                                                userTransactionsKey =
                                                    '${widget.data['contractAddress']}${widget.data['rpc']} Details';

                                                await client.dispose();
                                              } else if (isBitcoinType) {
                                                final getBitCoinDetails =
                                                    await getBitcoinFromMemnomic(
                                                  mnemonic,
                                                  widget.data['POSNetwork'],
                                                );

                                                double amount = double.parse(
                                                  widget.data['amount'],
                                                );
                                                int amountToSend = (amount *
                                                        pow(10,
                                                            bitCoinDecimals))
                                                    .toInt();

                                                final transaction =
                                                    await sendBTCType(
                                                  widget.data['recipient'],
                                                  amountToSend,
                                                  widget.data['POSNetwork'],
                                                );
                                                transactionHash =
                                                    transaction['txid'];

                                                coinDecimals = bitCoinDecimals;
                                                userAddress = getBitCoinDetails[
                                                    'address'];

                                                userTransactionsKey =
                                                    '${widget.data['default']} Details';
                                              } else if (isSolana) {
                                                final mnemonic = pref
                                                    .get(currentMmenomicKey);
                                                final getSolanaDetails =
                                                    await getSolanaFromMemnomic(
                                                        mnemonic);

                                                final transaction =
                                                    await sendSolana(
                                                  widget.data['recipient'],
                                                  (double.parse(widget
                                                              .data['amount']) *
                                                          pow(10,
                                                              solanaDecimals))
                                                      .toInt(),
                                                  widget.data['solanaCluster'],
                                                );
                                                transactionHash =
                                                    transaction['txid'];

                                                coinDecimals = solanaDecimals;
                                                userAddress =
                                                    getSolanaDetails['address'];
                                                userTransactionsKey =
                                                    '${widget.data['default']} Details';
                                              } else if (isCardano) {
                                                final getCardanoDetails =
                                                    await getCardanoFromMemnomic(
                                                  mnemonic,
                                                  widget
                                                      .data['cardano_network'],
                                                );

                                                double amount = double.parse(
                                                  widget.data['amount'],
                                                );

                                                int amountToSend = (amount *
                                                        pow(10,
                                                            cardanoDecimals))
                                                    .toInt();
                                                //FIXME: cardano
                                                // final transaction =
                                                //     await compute(
                                                //   sendCardano,
                                                //   {
                                                //     'cardanoNetwork':
                                                //         widget.data[
                                                //             'cardano_network'],
                                                //     'blockfrostForCardanoApiKey':
                                                //         widget.data[
                                                //             'blockFrostKey'],
                                                //     'mnemonic': mnemonic,
                                                //     'lovelaceToSend':
                                                //         amountToSend,
                                                //     'senderAddress':
                                                //         cardano.ShelleyAddress
                                                //             .fromBech32(
                                                //       getCardanoDetails[
                                                //           'address'],
                                                //     ),
                                                //     'recipientAddress':
                                                //         cardano.ShelleyAddress
                                                //             .fromBech32(
                                                //       widget.data['recipient'],
                                                //     )
                                                //   },
                                                // );
                                                // transactionHash =
                                                //     transaction['txid'];

                                                // coinDecimals = cardanoDecimals;
                                                // userAddress = getCardanoDetails[
                                                //     'address'];
                                                // userTransactionsKey =
                                                //     '${widget.data['default']} Details';
                                              } else if (isFilecoin) {
                                                final getFileCoinDetails =
                                                    await getFileCoinFromMemnomic(
                                                  mnemonic,
                                                  widget.data['prefix'],
                                                );

                                                double amount = double.parse(
                                                  widget.data['amount'],
                                                );

                                                int amounToSend = (amount *
                                                        pow(10,
                                                            fileCoinDecimals))
                                                    .toInt();

                                                final transaction =
                                                    await sendFilecoin(
                                                  widget.data['recipient'],
                                                  amounToSend,
                                                  baseUrl:
                                                      widget.data['baseUrl'],
                                                  addressPrefix:
                                                      widget.data['prefix'],
                                                );
                                                transactionHash =
                                                    transaction['txid'];

                                                coinDecimals = fileCoinDecimals;
                                                userAddress =
                                                    getFileCoinDetails[
                                                        'address'];
                                                userTransactionsKey =
                                                    '${widget.data['default']} Details';
                                              } else if (isStellar) {
                                                Map getStellarDetails =
                                                    await getStellarFromMemnomic(
                                                  mnemonic,
                                                );

                                                final transaction =
                                                    await sendStellar(
                                                  widget.data['recipient'],
                                                  widget.data['amount'],
                                                  widget.data['sdk'],
                                                  widget.data['cluster'],
                                                );
                                                transactionHash =
                                                    transaction['txid'];

                                                coinDecimals = stellarDecimals;
                                                userAddress = getStellarDetails[
                                                    'address'];
                                                userTransactionsKey =
                                                    '${widget.data['default']} Details';
                                              } else {
                                                final client = web3.Web3Client(
                                                  widget.data['rpc'],
                                                  Client(),
                                                );

                                                final mnemonic = pref
                                                    .get(currentMmenomicKey);
                                                final response =
                                                    await getEthereumFromMemnomic(
                                                  mnemonic,
                                                  widget.data['coinType'],
                                                );

                                                final credentials =
                                                    EthPrivateKey.fromHex(
                                                  response[
                                                      'eth_wallet_privateKey'],
                                                );
                                                final gasPrice =
                                                    await client.getGasPrice();

                                                final trans = await client
                                                    .signTransaction(
                                                  credentials,
                                                  web3.Transaction(
                                                    from: web3.EthereumAddress
                                                        .fromHex(response[
                                                            'eth_wallet_address']),
                                                    to: web3.EthereumAddress
                                                        .fromHex(widget
                                                            .data['recipient']),
                                                    value: web3.EtherAmount
                                                        .inWei(BigInt.from(double
                                                                .parse(widget
                                                                        .data[
                                                                    'amount']) *
                                                            pow(10,
                                                                etherDecimals))),
                                                    gasPrice: gasPrice,
                                                  ),
                                                  chainId:
                                                      widget.data['chainId'],
                                                );

                                                transactionHash = await client
                                                    .sendRawTransaction(trans);

                                                coinDecimals = etherDecimals;
                                                userAddress = response[
                                                    'eth_wallet_address'];
                                                userTransactionsKey =
                                                    '${widget.data['default']}${widget.data['rpc']} Details';

                                                await client.dispose();
                                              }

                                              if (transactionHash == null) {
                                                throw Exception(
                                                    'Sending failed');
                                              }
                                              // Get.snackbar(
                                              //   '',
                                              //   AppLocalizations.of(context)!
                                              //       .trxSent,
                                              // );
                                              ScaffoldMessenger.of(context)
                                                  .showSnackBar(SnackBar(
                                                content: Text(
                                                  AppLocalizations.of(context)!
                                                      .trxSent,
                                                ),
                                                backgroundColor: Colors.red,
                                                action: SnackBarAction(
                                                  label: 'OK',
                                                  onPressed: () {},
                                                  textColor: Colors.white,
                                                ),
                                              ));

                                              String tokenSent = isNFTTransfer
                                                  ? widget.data['tokenId']
                                                      .toString()
                                                  : widget.data['amount'];

                                              NotificationApi.showNotification(
                                                title:
                                                    '${widget.data['symbol']} Sent',
                                                body:
                                                    '$tokenSent ${widget.data['symbol']} sent to ${widget.data['recipient']}',
                                              );

                                              if (isNFTTransfer) {
                                                isSending.value = false;
                                                return;
                                              }

                                              String formattedDate = DateFormat(
                                                      "yyyy-MM-dd HH:mm:ss")
                                                  .format(
                                                DateTime.now(),
                                              );

                                              final mapData = {
                                                'time': formattedDate,
                                                'from': userAddress,
                                                'to': widget.data['recipient'],
                                                'value': double.parse(
                                                      widget.data['amount'],
                                                    ) *
                                                    pow(10, coinDecimals),
                                                'decimal': coinDecimals,
                                                'transactionHash':
                                                    transactionHash
                                              };

                                              List userTransactions = [];
                                              String jsonEncodedUsrTrx =
                                                  pref.get(userTransactionsKey);

                                              if (jsonEncodedUsrTrx != null) {
                                                userTransactions = json.decode(
                                                  jsonEncodedUsrTrx,
                                                );
                                              }

                                              userTransactions.insert(
                                                  0, mapData);
                                              userTransactions.length =
                                                  maximumTransactionToSave;
                                              await pref.put(
                                                userTransactionsKey,
                                                jsonEncode(userTransactions),
                                              );
                                              isSending.value = false;
                                              if (Navigator.canPop(context)) {
                                                int count = 0;
                                                // Get.until(
                                                //     (route) => count++ == 3);
                                                Navigator.of(context).popUntil(
                                                    (route) => count++ == 3);
                                                //   Future.delayed(Duration(seconds: 3), () {
                                                //   Navigator.pushAndRemoveUntil(
                                                //     context,
                                                //     MaterialPageRoute(builder: (context) => NextScreen()),
                                                //     (route) => count++ == 3,
                                                //   );
                                                // });
                                              }
                                            } catch (e) {
                                              isSending.value = false;
                                              // Get.snackbar(
                                              //   '',
                                              //   e.toString(),
                                              //   colorText: Colors.white,
                                              //   backgroundColor: Colors.red,
                                              // );
                                              ScaffoldMessenger.of(context)
                                                  .showSnackBar(SnackBar(
                                                content: Text(
                                                  e.toString(),
                                                ),
                                                backgroundColor: Colors.red,
                                                action: SnackBarAction(
                                                  label: 'OK',
                                                  onPressed: () {},
                                                  textColor: Colors.white,
                                                ),
                                              ));
                                            }
                                          } else {
                                            // Get.snackbar(
                                            //   '',
                                            //   AppLocalizations.of(context)!
                                            //       .authFailed,
                                            //   colorText: Colors.white,
                                            //   backgroundColor: Colors.red,
                                            // );
                                            ScaffoldMessenger.of(context)
                                                .showSnackBar(SnackBar(
                                              content: Text(
                                                  AppLocalizations.of(context)!
                                                      .authFailed),
                                              backgroundColor: Colors.red,
                                              action: SnackBarAction(
                                                label: 'OK',
                                                onPressed: () {},
                                                textColor: Colors.white,
                                              ),
                                            ));
                                          }
                                        },
                                  child: Padding(
                                    padding: const EdgeInsets.all(15),
                                    child: isSending.value
                                        ? Container(
                                            color: Colors.transparent,
                                            width: 20,
                                            height: 20,
                                            child: const Loader(color: white),
                                          )
                                        : Text(
                                            AppLocalizations.of(context)!.send,
                                            style: const TextStyle(
                                              color: Colors.white,
                                              fontWeight: FontWeight.bold,
                                              fontSize: 16,
                                            ),
                                          ),
                                  ),
                                ),
                              )
                            : SizedBox(
                                width: double.infinity,
                                child: ElevatedButton(
                                  style: ButtonStyle(
                                    backgroundColor:
                                        MaterialStateProperty.resolveWith(
                                            (states) => appBackgroundblue),
                                    shape: MaterialStateProperty.resolveWith(
                                      (states) => RoundedRectangleBorder(
                                        borderRadius: BorderRadius.circular(10),
                                      ),
                                    ),
                                  ),
                                  onPressed: null,
                                  child: Padding(
                                    padding: const EdgeInsets.all(15),
                                    child: Text(
                                      AppLocalizations.of(context)!.loading,
                                      style: const TextStyle(
                                          color: Colors.white,
                                          fontWeight: FontWeight.bold,
                                          fontSize: 16),
                                    ),
                                  ),
                                ),
                              ),
                    ],
                  ),
                )),
          ),
        ),
      ),
    );
  }

  getUserAddress() async {
    String mnemonic = Hive.box(secureStorageKey).get(currentMmenomicKey);
    if (isBitcoinType) {
      final getPOSblockchainDetails = await getBitcoinFromMemnomic(
        mnemonic,
        widget.data['POSNetwork'],
      );
      userAddress.value = getPOSblockchainDetails['address'];
    } else if (isSolana) {
      final getSolanaDetails = await getSolanaFromMemnomic(mnemonic);
      userAddress.value = getSolanaDetails['address'];
    } else if (isCardano) {
      final getCardanoDetails = await getCardanoFromMemnomic(
        mnemonic,
        widget.data['cardano_network'],
      );
      userAddress.value = getCardanoDetails['address'];
    } else if (isFilecoin) {
      final getFileCoinDetails = await getFileCoinFromMemnomic(
        mnemonic,
        widget.data['prefix'],
      );
      userAddress.value = getFileCoinDetails['address'];
    } else if (isStellar) {
      final getStellarDetails = await getStellarFromMemnomic(mnemonic);
      userAddress.value = getStellarDetails['address'];
    } else {
      userAddress.value = (await getEthereumFromMemnomic(
        mnemonic,
        widget.data['coinType'],
      ))['eth_wallet_address'];
    }

    if (userAddress.value == widget.data['recipient']) allowSend.value = false;
  }
}
