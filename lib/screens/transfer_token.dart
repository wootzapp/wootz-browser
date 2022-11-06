import 'dart:async';
import 'dart:math';
import 'package:cryptowallet/api/notification_api.dart';
import 'package:cryptowallet/components/loader.dart';
import 'package:cryptowallet/config/colors.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/bitcoin_util.dart';
import 'package:cryptowallet/utils/format_money.dart';
import 'package:cardano_wallet_sdk/cardano_wallet_sdk.dart' as cardano;
import 'package:cryptowallet/utils/stellar_utils.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'dart:convert';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:hive/hive.dart';
import 'package:http/http.dart';
import 'package:intl/intl.dart';
import 'package:web3dart/web3dart.dart' as web3;
import 'package:web3dart/web3dart.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import '../utils/filecoin_util.dart';

class TransferToken extends StatefulWidget {
  final Map data;
  final String cryptoDomain;
  const TransferToken({Key key, this.data, this.cryptoDomain})
      : super(key: key);

  @override
  _TransferTokenState createState() => _TransferTokenState();
}

class _TransferTokenState extends State<TransferToken> {
  final _scaffoldKey = GlobalKey<ScaffoldState>();
  bool isSending = false;

  bool get kDebugMode => null;
  Timer timer;
  Map transactionFeeMap;
  bool isContract;
  bool isBitcoinType;
  bool isSolana;
  bool isCardano;
  bool isFilecoin;
  bool isStellar;
  bool isNFTTransfer;

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

  Map transactionFee;

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

        BigInt decimals;

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

        transactionFeeMap = {
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

        transactionFeeMap = {
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

        transactionFeeMap = {
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

        transactionFeeMap = {
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

        transactionFeeMap = {
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

        transactionFeeMap = {
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

        transactionFeeMap = {
          'transactionFee': blockChainCost,
          'userBalance': userBalance,
        };
      }
      setState(() {});
    } catch (_) {}
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      key: _scaffoldKey,
      appBar: AppBar(title: Text(AppLocalizations.of(context).transfer)),
      body: SafeArea(
        child: RefreshIndicator(
          onRefresh: () async {
            await Future.delayed(const Duration(seconds: 2));
            setState(() {});
          },
          child: SingleChildScrollView(
            physics: const AlwaysScrollableScrollPhysics(),
            child: Padding(
              padding: const EdgeInsets.all(25),
              child: Column(
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
                    AppLocalizations.of(context).asset,
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
                    AppLocalizations.of(context).from,
                    style: const TextStyle(
                        fontSize: 18, fontWeight: FontWeight.bold),
                  ),
                  const SizedBox(
                    height: 10,
                  ),
                  FutureBuilder(
                      future: getUserAddress(),
                      builder: (context, snapshot) {
                        return Text(
                          snapshot.hasData
                              ? (snapshot.data as Map)['address']
                              : 'Loading...',
                          style: const TextStyle(fontSize: 16),
                        );
                      }),
                  const SizedBox(
                    height: 20,
                  ),
                  Text(
                    AppLocalizations.of(context).to,
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
                  Text(
                    AppLocalizations.of(context).transactionFee,
                    style: const TextStyle(
                        fontSize: 18, fontWeight: FontWeight.bold),
                  ),
                  const SizedBox(
                    height: 10,
                  ),
                  widget.data['default'] != null
                      ? Text(
                          '${transactionFeeMap != null ? transactionFeeMap['transactionFee'] : '0'}  ${widget.data['default']}',
                          style: const TextStyle(fontSize: 16),
                        )
                      : Container(),
                  widget.data['network'] != null
                      ? Text(
                          '${transactionFeeMap != null ? transactionFeeMap['transactionFee'] : '0'}  ${getEVMBlockchains()[widget.data['network']]['symbol']}',
                          style: const TextStyle(fontSize: 16),
                        )
                      : Container(),
                  const SizedBox(
                    height: 20,
                  ),
                  FutureBuilder(
                      future: getUserAddress(),
                      builder: (context, snapshot) {
                        if (snapshot.data['address'] ==
                            widget.data['recipient']) {
                          return Column(
                            children: [
                              Container(
                                decoration: BoxDecoration(
                                    borderRadius: const BorderRadius.all(
                                        Radius.circular(10)),
                                    color: Colors.red[100]),
                                child: Padding(
                                  padding: const EdgeInsets.all(10),
                                  child: Column(
                                    crossAxisAlignment:
                                        CrossAxisAlignment.center,
                                    mainAxisAlignment: MainAxisAlignment.center,
                                    children: [
                                      Text(
                                          AppLocalizations.of(context)
                                              .sendingToYourself,
                                          textAlign: TextAlign.center,
                                          style: const TextStyle(
                                            color: Colors.red,
                                          )),
                                    ],
                                  ),
                                ),
                              ),
                              const SizedBox(
                                height: 20,
                              ),
                            ],
                          );
                        }
                        return Container();
                      }),
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
                            onPressed: transactionFeeMap['userBalance'] ==
                                        null ||
                                    transactionFeeMap['userBalance'] <= 0
                                ? () {
                                    ScaffoldMessenger.of(context).showSnackBar(
                                      SnackBar(
                                        content: Text(
                                          AppLocalizations.of(context)
                                              .insufficientBalance,
                                          style: const TextStyle(
                                              color: Colors.white),
                                        ),
                                        backgroundColor: Colors.red,
                                      ),
                                    );
                                  }
                                : () async {
                                    if (isSending) return;
                                    if (await authenticate(context)) {
                                      ScaffoldMessenger.of(context)
                                          .hideCurrentSnackBar();
                                      setState(() {
                                        isSending = true;
                                      });
                                      try {
                                        final pref = Hive.box(secureStorageKey);
                                        String mnemonic =
                                            pref.get(currentMmenomicKey);

                                        String userAddress;
                                        String transactionHash;
                                        int coinDecimals;
                                        String userTransactionsKey;
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
                                            response['eth_wallet_privateKey'],
                                          );

                                          final sendingAddress =
                                              web3.EthereumAddress.fromHex(
                                            response['eth_wallet_address'],
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
                                              widget.data['contractAddress'],
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
                                              web3.EthereumAddress.fromHex(
                                                  widget.data['recipient']),
                                              BigInt.from(
                                                widget.data['tokenId'],
                                              )
                                            ];

                                            transfer = contract
                                                .findFunctionsByName(
                                                    'safeTransferFrom')
                                                .toList()[0];
                                          } else {
                                            transfer =
                                                contract.function('transfer');

                                            decimalsFunction =
                                                contract.function('decimals');
                                            decimals = (await client.call(
                                                    contract: contract,
                                                    function: decimalsFunction,
                                                    params: []))
                                                .first;

                                            _parameters = [
                                              web3.EthereumAddress.fromHex(
                                                widget.data['recipient'],
                                              ),
                                              BigInt.from(
                                                double.parse(
                                                        widget.data['amount']) *
                                                    pow(10, decimals.toInt()),
                                              )
                                            ];

                                            coinDecimals = decimals.toInt();
                                          }

                                          final trans =
                                              await client.signTransaction(
                                            credentials,
                                            Transaction.callContract(
                                              contract: contract,
                                              function: transfer,
                                              parameters: _parameters,
                                            ),
                                            chainId: widget.data['chainId'],
                                          );

                                          transactionHash = await client
                                              .sendRawTransaction(trans);

                                          userAddress =
                                              response['eth_wallet_address'];

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
                                                  pow(10, bitCoinDecimals))
                                              .toInt();

                                          final transaction = await sendBTCType(
                                            widget.data['recipient'],
                                            amountToSend,
                                            widget.data['POSNetwork'],
                                          );
                                          transactionHash = transaction['txid'];

                                          coinDecimals = bitCoinDecimals;
                                          userAddress =
                                              getBitCoinDetails['address'];

                                          userTransactionsKey =
                                              '${widget.data['default']} Details';
                                        } else if (isSolana) {
                                          final mnemonic =
                                              pref.get(currentMmenomicKey);
                                          final getSolanaDetails =
                                              await getSolanaFromMemnomic(
                                                  mnemonic);

                                          final transaction = await sendSolana(
                                            widget.data['recipient'],
                                            (double.parse(
                                                        widget.data['amount']) *
                                                    pow(10, solanaDecimals))
                                                .toInt(),
                                            widget.data['solanaCluster'],
                                          );
                                          transactionHash = transaction['txid'];

                                          coinDecimals = solanaDecimals;
                                          userAddress =
                                              getSolanaDetails['address'];
                                          userTransactionsKey =
                                              '${widget.data['default']} Details';
                                        } else if (isCardano) {
                                          final getCardanoDetails =
                                              await getCardanoFromMemnomic(
                                            mnemonic,
                                            widget.data['cardano_network'],
                                          );

                                          double amount = double.parse(
                                            widget.data['amount'],
                                          );

                                          int amountToSend = (amount *
                                                  pow(10, cardanoDecimals))
                                              .toInt();
                                          final transaction = await compute(
                                            sendCardano,
                                            {
                                              'cardanoNetwork': widget
                                                  .data['cardano_network'],
                                              'blockfrostForCardanoApiKey':
                                                  widget.data['blockFrostKey'],
                                              'mnemonic': mnemonic,
                                              'lovelaceToSend': amountToSend,
                                              'senderAddress': cardano
                                                  .ShelleyAddress.fromBech32(
                                                getCardanoDetails['address'],
                                              ),
                                              'recipientAddress': cardano
                                                  .ShelleyAddress.fromBech32(
                                                widget.data['recipient'],
                                              )
                                            },
                                          );
                                          transactionHash = transaction['txid'];

                                          coinDecimals = cardanoDecimals;
                                          userAddress =
                                              getCardanoDetails['address'];
                                          userTransactionsKey =
                                              '${widget.data['default']} Details';
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
                                                  pow(10, fileCoinDecimals))
                                              .toInt();

                                          final transaction =
                                              await sendFilecoin(
                                            widget.data['recipient'],
                                            amounToSend,
                                            baseUrl: widget.data['baseUrl'],
                                            addressPrefix:
                                                widget.data['prefix'],
                                          );
                                          transactionHash = transaction['txid'];

                                          coinDecimals = fileCoinDecimals;
                                          userAddress =
                                              getFileCoinDetails['address'];
                                          userTransactionsKey =
                                              '${widget.data['default']} Details';
                                        } else if (isStellar) {
                                          Map getStellarDetails =
                                              await getStellarFromMemnomic(
                                            mnemonic,
                                          );

                                          final transaction = await sendStellar(
                                            widget.data['recipient'],
                                            widget.data['amount'],
                                            widget.data['sdk'],
                                            widget.data['cluster'],
                                          );
                                          transactionHash = transaction['txid'];

                                          coinDecimals = stellarDecimals;
                                          userAddress =
                                              getStellarDetails['address'];
                                          userTransactionsKey =
                                              '${widget.data['default']} Details';
                                        } else {
                                          final client = web3.Web3Client(
                                            widget.data['rpc'],
                                            Client(),
                                          );

                                          final mnemonic =
                                              pref.get(currentMmenomicKey);
                                          final response =
                                              await getEthereumFromMemnomic(
                                            mnemonic,
                                            widget.data['coinType'],
                                          );

                                          final credentials =
                                              EthPrivateKey.fromHex(
                                            response['eth_wallet_privateKey'],
                                          );
                                          final gasPrice =
                                              await client.getGasPrice();

                                          final trans =
                                              await client.signTransaction(
                                            credentials,
                                            web3.Transaction(
                                              from: web3.EthereumAddress
                                                  .fromHex(response[
                                                      'eth_wallet_address']),
                                              to: web3.EthereumAddress.fromHex(
                                                  widget.data['recipient']),
                                              value: web3.EtherAmount.inWei(
                                                  BigInt.from(double.parse(
                                                          widget
                                                              .data['amount']) *
                                                      pow(10, etherDecimals))),
                                              gasPrice: gasPrice,
                                            ),
                                            chainId: widget.data['chainId'],
                                          );

                                          transactionHash = await client
                                              .sendRawTransaction(trans);

                                          coinDecimals = etherDecimals;
                                          userAddress =
                                              response['eth_wallet_address'];
                                          userTransactionsKey =
                                              '${widget.data['default']}${widget.data['rpc']} Details';

                                          await client.dispose();
                                        }

                                        if (transactionHash == null) {
                                          throw Exception('Sending failed');
                                        }
                                        ScaffoldMessenger.of(context)
                                            .showSnackBar(
                                          SnackBar(
                                            content: Text(
                                              AppLocalizations.of(context)
                                                  .trxSent,
                                            ),
                                          ),
                                        );

                                        String tokenSent = isNFTTransfer
                                            ? widget.data['tokenId'].toString()
                                            : widget.data['amount'];

                                        NotificationApi.showNotification(
                                          title:
                                              '${widget.data['symbol']} Sent',
                                          body:
                                              '$tokenSent ${widget.data['symbol']} sent to ${widget.data['recipient']}',
                                        );

                                        if (isNFTTransfer) {
                                          setState(() {
                                            isSending = false;
                                          });
                                          return;
                                        }

                                        String formattedDate =
                                            DateFormat("yyyy-MM-dd HH:mm:ss")
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
                                          'transactionHash': transactionHash
                                        };

                                        List userTransactions = [];
                                        String jsonEncodedUsrTrx =
                                            pref.get(userTransactionsKey);

                                        if (jsonEncodedUsrTrx != null) {
                                          userTransactions = json.decode(
                                            jsonEncodedUsrTrx,
                                          );
                                        }

                                        userTransactions.insert(0, mapData);
                                        userTransactions.length =
                                            maximumTransactionToSave;
                                        await pref.put(
                                          userTransactionsKey,
                                          jsonEncode(userTransactions),
                                        );
                                        setState(() {
                                          isSending = false;
                                        });
                                        if (Navigator.canPop(context)) {
                                          int count = 0;
                                          Navigator.popUntil(context, (route) {
                                            return count++ == 3;
                                          });
                                        }
                                      } catch (e) {
                                        setState(() {
                                          isSending = false;
                                          ScaffoldMessenger.of(context)
                                              .showSnackBar(
                                            SnackBar(
                                              backgroundColor: Colors.red,
                                              content: Text(
                                                e.toString(),
                                                style: const TextStyle(
                                                    color: Colors.white),
                                              ),
                                            ),
                                          );
                                        });
                                      }
                                    } else {
                                      ScaffoldMessenger.of(context)
                                          .showSnackBar(
                                        SnackBar(
                                          backgroundColor: Colors.red,
                                          content: Text(
                                            AppLocalizations.of(context)
                                                .authFailed,
                                            style: const TextStyle(
                                                color: Colors.white),
                                          ),
                                        ),
                                      );
                                    }
                                  },
                            child: Padding(
                              padding: const EdgeInsets.all(15),
                              child: isSending
                                  ? Container(
                                      color: Colors.transparent,
                                      width: 20,
                                      height: 20,
                                      child: const Loader(color: white),
                                    )
                                  : Text(
                                      AppLocalizations.of(context).send,
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
                                AppLocalizations.of(context).loading,
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
            ),
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
      return {'address': getPOSblockchainDetails['address']};
    } else if (isSolana) {
      final getSolanaDetails = await getSolanaFromMemnomic(mnemonic);
      return {'address': getSolanaDetails['address']};
    } else if (isCardano) {
      final getCardanoDetails = await getCardanoFromMemnomic(
        mnemonic,
        widget.data['cardano_network'],
      );
      return {'address': getCardanoDetails['address']};
    } else if (isFilecoin) {
      final getFileCoinDetails = await getFileCoinFromMemnomic(
        mnemonic,
        widget.data['prefix'],
      );
      return {'address': getFileCoinDetails['address']};
    } else if (isStellar) {
      final getStellarDetails = await getStellarFromMemnomic(mnemonic);
      return {'address': getStellarDetails['address']};
    } else {
      return {
        'address': (await getEthereumFromMemnomic(
          mnemonic,
          widget.data['coinType'],
        ))['eth_wallet_address']
      };
    }
  }
}
