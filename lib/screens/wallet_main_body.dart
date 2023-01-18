import 'dart:async';
import 'package:cryptowallet/components/portfolio.dart';
import 'package:cryptowallet/components/user_balance.dart';
import 'package:cryptowallet/components/user_details_placeholder.dart';
import 'package:cryptowallet/screens/send_token.dart';
import 'package:cryptowallet/screens/user_added_tokens.dart';
import 'package:cryptowallet/screens/add_custom_token.dart';
import 'package:cryptowallet/screens/token.dart';
import 'package:cryptowallet/utils/bitcoin_util.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/scheduler.dart';
// import 'package:get/get.dart';
import 'package:hive/hive.dart';
import 'package:provider/provider.dart';
import 'package:receive_sharing_intent/receive_sharing_intent.dart';
import '../model/provider.dart';
import '../utils/app_config.dart';
import '../utils/get_blockchain_widget.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

class WalletMainBody extends StatefulWidget {
  const WalletMainBody({Key key}) : super(key: key);

  @override
  _WalletMainBodyState createState() => _WalletMainBodyState();
}

Future<void> handleAllIntent(String value, BuildContext context) async {
  if (value == null) return;
  bool isWalletConnect = value.trim().startsWith('wc:');

  Widget navigateWidget;

  if (isWalletConnect) {
  } else {
    Map scannedData = await processEIP681(value);
    navigateWidget = scannedData['success']
        ? SendToken(
            data: scannedData['msg'],
          )
        : null;
  }

  if (navigateWidget == null) return;

  SchedulerBinding.instance.addPostFrameCallback((_) {
    if (Navigator.canPop(context)) {
      Navigator.of(context).pushReplacement(
        MaterialPageRoute(builder: (_) => navigateWidget),
      );
      return;
    }

    Navigator.of(context).push(
      MaterialPageRoute(builder: (_) => navigateWidget),
    );
  });
}

class _WalletMainBodyState extends State<WalletMainBody>
    with AutomaticKeepAliveClientMixin {
  @override
  bool get wantKeepAlive => true;
  StreamSubscription _intentDataStreamSubscription;
  List<ValueNotifier<double>> cryptoBalanceListNotifiers =
      <ValueNotifier<double>>[];
  ValueNotifier<double> walletNotifier = ValueNotifier(null);

  // RxList<Widget> blockChainsArray = <Widget>[].obs;
  final blockChainsArray = ValueNotifier<List<Widget>>(<Widget>[]);

  List<Timer> cryptoBalancesTimer = <Timer>[];
  GlobalKey<UserAddedTokensState> globalKey = GlobalKey();

  @override
  void dispose() {
    _intentDataStreamSubscription.cancel();
    for (Timer cryptoTimer in cryptoBalancesTimer) {
      cryptoTimer?.cancel();
    }
    for (ValueNotifier cryptoNotifier in cryptoBalanceListNotifiers) {
      cryptoNotifier?.dispose();
    }
    walletNotifier?.dispose();
    super.dispose();
  }

  @override
  initState() {
    super.initState();
    initializeBlockchains();

    _intentDataStreamSubscription =
        ReceiveSharingIntent.getTextStream().listen((String value) async {
      await handleAllIntent(value, context);
    }, onError: (err) {
      if (kDebugMode) {
        print("getLinkStream error: $err");
      }
    });

    ReceiveSharingIntent.getInitialText().then((String value) async {
      await handleAllIntent(value, context);
    }).catchError((err) {
      if (kDebugMode) {
        print("getLinkStream error: $err");
      }
    });
  }

  void initializeBlockchains() {
    blockChainsArray.value = <Widget>[];

    final mnemonic = Hive.box(secureStorageKey).get(currentMmenomicKey);

    for (String name in getBitCoinPOSBlockchains().keys) {
      Map bitcoinBlockchain = Map.from(getBitCoinPOSBlockchains()[name])
        ..addAll({'name': name});

      final notifier = ValueNotifier<double>(null);

      cryptoBalanceListNotifiers.add(notifier);

      final currentList = blockChainsArray.value;

      currentList.addAll([
        InkWell(
            onTap: () {
              Navigator.of(context).push(
                MaterialPageRoute(
                  builder: (_) => Token(data: bitcoinBlockchain),
                ),
              );
            },
            child: GetBlockChainWidget(
              name_: name,
              symbol_: bitcoinBlockchain['symbol'],
              hasPrice_: true,
              image_: AssetImage(bitcoinBlockchain['image']),
              cryptoAmount_: ValueListenableBuilder(
                valueListenable: notifier,
                builder: ((_, double value, Widget __) {
                  if (value == null) {
                    () async {
                      final getBitcoinDetails = await getBitcoinFromMemnomic(
                        mnemonic,
                        bitcoinBlockchain['POSNetwork'],
                      );
                      try {
                        notifier.value = await getBitcoinAddressBalance(
                          getBitcoinDetails['address'],
                          bitcoinBlockchain['POSNetwork'],
                          skipNetworkRequest: notifier.value == null,
                        );
                      } catch (_) {}

                      cryptoBalancesTimer.add(
                        Timer.periodic(httpPollingDelay, (timer) async {
                          try {
                            notifier.value = await getBitcoinAddressBalance(
                              getBitcoinDetails['address'],
                              bitcoinBlockchain['POSNetwork'],
                              skipNetworkRequest: notifier.value == null,
                            );
                          } catch (_) {}
                        }),
                      );
                    }();
                    return Container();
                  }

                  return UserBalance(
                    symbol: bitcoinBlockchain['symbol'],
                    balance: value,
                  );
                }),
              ),
            )),
      ]);

      currentList.add(const Divider());

      blockChainsArray.value = currentList;
    }

    for (String i in getEVMBlockchains().keys) {
      final Map evmBlockchain = Map.from(getEVMBlockchains()[i])
        ..addAll({'name': i});

      final notifier = ValueNotifier<double>(null);
      cryptoBalanceListNotifiers.add(notifier);

      final currentList = blockChainsArray.value;

      currentList.add(
        InkWell(
          onTap: () {
            Navigator.of(context).push(
              MaterialPageRoute(
                builder: (_) => Token(
                  data: evmBlockchain,
                ),
              ),
            );
          },
          child: GetBlockChainWidget(
            name_: i,
            symbol_: evmBlockchain['symbol'],
            hasPrice_: true,
            image_: AssetImage(
              evmBlockchain['image'],
            ),
            cryptoAmount_: ValueListenableBuilder(
              valueListenable: notifier,
              builder: ((context, value, child) {
                if (value == null) {
                  () async {
                    final getEthereumDetails = await getEthereumFromMemnomic(
                      mnemonic,
                      evmBlockchain['coinType'],
                    );
                    try {
                      notifier.value = await getEthereumAddressBalance(
                        getEthereumDetails['eth_wallet_address'],
                        evmBlockchain['rpc'],
                        coinType: evmBlockchain['coinType'],
                        skipNetworkRequest: notifier.value == null,
                      );
                    } catch (_) {}

                    cryptoBalancesTimer.add(
                      Timer.periodic(httpPollingDelay, (timer) async {
                        try {
                          notifier.value = await getEthereumAddressBalance(
                            getEthereumDetails['eth_wallet_address'],
                            evmBlockchain['rpc'],
                            coinType: evmBlockchain['coinType'],
                            skipNetworkRequest: notifier.value == null,
                          );
                        } catch (_) {}
                      }),
                    );
                  }();
                  return Container();
                }
                return UserBalance(
                  symbol: evmBlockchain['symbol'],
                  balance: value,
                );
              }),
            ),
          ),
        ),
      );
      currentList.add(const Divider());
      blockChainsArray.value = currentList;
    }

    for (String i in getSolanaBlockChains().keys) {
      final Map solanaBlockchain = Map.from(getSolanaBlockChains()[i])
        ..addAll({'name': i});

      final notifier = ValueNotifier<double>(null);
      cryptoBalanceListNotifiers.add(notifier);

      final currentList = blockChainsArray.value;

      currentList.addAll([
        InkWell(
          onTap: () {
            Navigator.of(context).push(
              MaterialPageRoute(
                builder: (_) => Token(
                  data: solanaBlockchain,
                ),
              ),
            );
          },
          child: GetBlockChainWidget(
            name_: i,
            symbol_: solanaBlockchain['symbol'],
            hasPrice_: true,
            image_: AssetImage(solanaBlockchain['image']),
            cryptoAmount_: ValueListenableBuilder(
              valueListenable: notifier,
              builder: ((context, value, child) {
                if (value == null) {
                  () async {
                    final getSolanaDetails =
                        await getSolanaFromMemnomic(mnemonic);
                    try {
                      notifier.value = await getSolanaAddressBalance(
                        getSolanaDetails['address'],
                        solanaBlockchain['solanaCluster'],
                        skipNetworkRequest: notifier.value == null,
                      );
                    } catch (_) {}

                    cryptoBalancesTimer.add(
                      Timer.periodic(httpPollingDelay, (timer) async {
                        try {
                          notifier.value = await getSolanaAddressBalance(
                            getSolanaDetails['address'],
                            solanaBlockchain['solanaCluster'],
                            skipNetworkRequest: notifier.value == null,
                          );
                        } catch (_) {}
                      }),
                    );
                  }();
                  return Container();
                }
                return UserBalance(
                  symbol: solanaBlockchain['symbol'],
                  balance: value,
                );
              }),
            ),
          ),
        ),
      ]);

      currentList.add(const Divider());

      blockChainsArray.value = currentList;
    }
    for (String i in getStellarBlockChains().keys) {
      final Map stellarBlockChain = Map.from(getStellarBlockChains()[i])
        ..addAll({'name': i});
      final notifier = ValueNotifier<double>(null);
      cryptoBalanceListNotifiers.add(notifier);

      final currentList = blockChainsArray.value;

      currentList.addAll([
        InkWell(
            onTap: () {
              Navigator.of(context).push(
                MaterialPageRoute(
                  builder: (_) => Token(
                    data: stellarBlockChain,
                  ),
                ),
              );
            },
            child: GetBlockChainWidget(
              name_: i,
              symbol_: stellarBlockChain['symbol'],
              hasPrice_: true,
              image_: AssetImage(
                stellarBlockChain['image'],
              ),
              cryptoAmount_: ValueListenableBuilder(
                valueListenable: notifier,
                builder: ((context, value, child) {
                  if (value == null) {
                    () async {
                      final getStellarDetails =
                          await getStellarFromMemnomic(mnemonic);
                      try {
                        notifier.value = await getStellarAddressBalance(
                          getStellarDetails['address'],
                          stellarBlockChain['sdk'],
                          stellarBlockChain['cluster'],
                          skipNetworkRequest: notifier.value == null,
                        );
                      } catch (_) {}

                      cryptoBalancesTimer.add(
                        Timer.periodic(httpPollingDelay, (timer) async {
                          try {
                            notifier.value = await getStellarAddressBalance(
                              getStellarDetails['address'],
                              stellarBlockChain['sdk'],
                              stellarBlockChain['cluster'],
                              skipNetworkRequest: notifier.value == null,
                            );
                          } catch (_) {}
                        }),
                      );
                    }();
                    return Container();
                  }
                  return UserBalance(
                    symbol: stellarBlockChain['symbol'],
                    balance: value,
                  );
                }),
              ),
            )),
      ]);

      currentList.add(const Divider());

      blockChainsArray.value = currentList;
    }

    for (String i in getFilecoinBlockChains().keys) {
      final Map filecoinBlockchain = Map.from(getFilecoinBlockChains()[i])
        ..addAll({'name': i});

      final notifier = ValueNotifier<double>(null);
      cryptoBalanceListNotifiers.add(notifier);

      final currentList = blockChainsArray.value;

      currentList.addAll([
        InkWell(
            onTap: () {
              Navigator.push(
                  context,
                  MaterialPageRoute(
                    builder: (_) => Token(
                      data: filecoinBlockchain,
                    ),
                  ));
            },
            child: GetBlockChainWidget(
              name_: i,
              symbol_: filecoinBlockchain['symbol'],
              hasPrice_: true,
              image_: AssetImage(
                filecoinBlockchain['image'],
              ),
              cryptoAmount_: ValueListenableBuilder(
                valueListenable: notifier,
                builder: ((context, value, child) {
                  if (value == null) {
                    () async {
                      final getFilecoinDetails = await getFileCoinFromMemnomic(
                        mnemonic,
                        filecoinBlockchain['prefix'],
                      );
                      try {
                        notifier.value = await getFileCoinAddressBalance(
                          getFilecoinDetails['address'],
                          baseUrl: filecoinBlockchain['baseUrl'],
                          skipNetworkRequest: notifier.value == null,
                        );
                      } catch (_) {}

                      cryptoBalancesTimer.add(
                        Timer.periodic(httpPollingDelay, (timer) async {
                          try {
                            notifier.value = await getFileCoinAddressBalance(
                              getFilecoinDetails['address'],
                              baseUrl: filecoinBlockchain['baseUrl'],
                              skipNetworkRequest: notifier.value == null,
                            );
                          } catch (_) {}
                        }),
                      );
                    }();
                    return Container();
                  }
                  return UserBalance(
                    symbol: filecoinBlockchain['symbol'],
                    balance: value,
                  );
                }),
              ),
            )),
      ]);

      currentList.add(const Divider());

      blockChainsArray.value = currentList;
    }

    for (String i in getCardanoBlockChains().keys) {
      final Map cardanoBlockchain = Map.from(getCardanoBlockChains()[i])
        ..addAll({'name': i});

      final notifier = ValueNotifier<double>(null);
      cryptoBalanceListNotifiers.add(notifier);

      final currentList = blockChainsArray.value;

      currentList.addAll([
        InkWell(
            onTap: () {
              Navigator.push(
                  context,
                  MaterialPageRoute(
                    builder: (_) => Token(
                      data: cardanoBlockchain,
                    ),
                  ));
            },
            child: GetBlockChainWidget(
              name_: i,
              symbol_: cardanoBlockchain['symbol'],
              hasPrice_: true,
              image_: AssetImage(
                cardanoBlockchain['image'],
              ),
              cryptoAmount_: ValueListenableBuilder(
                valueListenable: notifier,
                builder: ((context, value, child) {
                  if (value == null) {
                    () async {
                      final getCardanoDetails = await getCardanoFromMemnomic(
                        mnemonic,
                        cardanoBlockchain['cardano_network'],
                      );
                      try {
                        notifier.value = await getCardanoAddressBalance(
                          getCardanoDetails['address'],
                          cardanoBlockchain['cardano_network'],
                          cardanoBlockchain['blockFrostKey'],
                          skipNetworkRequest: notifier.value == null,
                        );
                      } catch (_) {}

                      cryptoBalancesTimer.add(
                        Timer.periodic(httpPollingDelay, (timer) async {
                          try {
                            notifier.value = await getCardanoAddressBalance(
                              getCardanoDetails['address'],
                              cardanoBlockchain['cardano_network'],
                              cardanoBlockchain['blockFrostKey'],
                              skipNetworkRequest: notifier.value == null,
                            );
                          } catch (_) {}
                        }),
                      );
                    }();
                    return Container();
                  }
                  return UserBalance(
                    symbol: cardanoBlockchain['symbol'],
                    balance: value,
                  );
                }),
              ),
            )),
      ]);
      currentList.add(const Divider());

      blockChainsArray.value = currentList;
    }
  }

  @override
  Widget build(BuildContext context) {
    super.build(context);

    return ValueListenableBuilder(
        valueListenable: blockChainsArray,
        builder: (context, value, child) {
          return Scaffold(
            body: SafeArea(
              child: RefreshIndicator(
                onRefresh: () async {
                  await Future.delayed(const Duration(seconds: 2));
                },
                child: SingleChildScrollView(
                  physics: const AlwaysScrollableScrollPhysics(),
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.start,
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Container(
                        color: Theme.of(context)
                            .bottomNavigationBarTheme
                            .backgroundColor,
                        child: Padding(

                          padding: const EdgeInsets.all(20),
                          child: Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: [
                              Row(
                                children: const [
                                  UserDetailsPlaceHolder(
                                    size: .5,
                                    showHi: true,
                                  ),
                                  SizedBox(
                                    width: 20,
                                  ),
                                ],
                              ),
                              IconButton(
                                onPressed: () {
                                  print('cancel wallet ');
                                  if (Navigator.canPop(context)) {
                                    Navigator.of(context).pop();
                                  }
                                },
                                icon: const Icon(
                                  Icons.close,
                                ),
                              )
                            ],
                          ),
                        ),
                      ),
                      const SizedBox(
                        height: 30,
                      ),
                      const Portfolio(),
                      Padding(
                        padding: const EdgeInsets.all(20),
                        child: Row(
                          mainAxisAlignment: MainAxisAlignment.spaceBetween,
                          children: [
                            Text(
                              AppLocalizations.of(context).assets,
                              style: const TextStyle(
                                  fontSize: 16, fontWeight: FontWeight.bold),
                            ),
                            //
                            GestureDetector(
                              onTap: () async {
                                await Navigator.of(context).push(
                                  PageRouteBuilder(
                                    pageBuilder: (_, __, ___) =>
                                        const AddCustomToken(),
                                    transitionsBuilder:
                                        (_, animation, __, child) =>
                                            SlideTransition(
                                      position: Tween<Offset>(
                                        begin: const Offset(1, 0),
                                        end: Offset.zero,
                                      ).animate(animation),
                                      child: child,
                                    ),
                                  ),
                                );

                                globalKey?.currentState?.getUserAddedToken();
                              },
                              child: Container(
                                color: Colors.transparent,
                                child: Card(
                                  child: Padding(
                                    padding: const EdgeInsets.all(8.0),
                                    child: Row(
                                      children: [
                                        Text(
                                          AppLocalizations.of(context).addToken,
                                          style: const TextStyle(
                                            fontSize: 16,
                                          ),
                                        ),
                                        const Icon(
                                          Icons.add,
                                          size: 20,
                                        ),
                                      ],
                                    ),
                                  ),
                                ),
                              ),
                            ),
                          ],
                        ),
                      ),
                      Padding(
                        padding: const EdgeInsets.fromLTRB(20, 0, 20, 0),
                        child: Column(
                          mainAxisAlignment: MainAxisAlignment.start,
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            ...blockChainsArray.value,
                            UserAddedTokens(
                              key: globalKey,
                            ),
                            const SizedBox(
                              height: 20,
                            )
                          ],
                        ),
                      ),
                    ],
                  ),
                ),
              ),
            ),
          );
        });
  }
}
