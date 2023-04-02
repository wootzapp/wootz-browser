import 'dart:async';
import 'dart:convert';

import 'package:awesome_dialog/awesome_dialog.dart';
import 'package:cryptowallet/components/user_balance.dart';
import 'package:cryptowallet/screens/token.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
// import 'package:get/get.dart';
import 'package:hive/hive.dart';
import 'package:multi_value_listenable_builder/multi_value_listenable_builder.dart';

import '../utils/get_blockchain_widget.dart';
import '../utils/rpc_urls.dart';

class UserAddedTokens extends StatefulWidget {
  const UserAddedTokens({Key key}) : super(key: key);

  @override
  State<UserAddedTokens> createState() => UserAddedTokensState();
}

class UserAddedTokensState extends State<UserAddedTokens> {
  // RxList userAddedToken = [].obs;
  final userAddedToken = ValueNotifier<List>([]);
  List<ValueNotifier<double>> addedTokenListNotifiers =
      <ValueNotifier<double>>[];
  List<Timer> addedTokenListTimer = <Timer>[];
  double tokenBalance;
  int addedTokenListNotifiersCounter = 0;
  // RxBool toggler = false.obs;
  final toggler = ValueNotifier<bool>(false);
  @override
  void initState() {
    super.initState();
    getUserAddedToken();
  }

  @override
  void dispose() {
    for (Timer cryptoTimer in addedTokenListTimer) {
      cryptoTimer?.cancel();
    }
    for (ValueNotifier cryptoNotifier in addedTokenListNotifiers) {
      cryptoNotifier?.dispose();
    }
    super.dispose();
  }

  Future getUserAddedToken() async {
    final pref = Hive.box(secureStorageKey);
    final userTokenListKey = await getAddTokenKey();
    final prefToken = pref.get(userTokenListKey);

    List userTokenList = [];

    if (prefToken != null) {
      userTokenList = jsonDecode(prefToken);
    }

    final tokenList = [];

    for (final token in userTokenList) {
      tokenList.add({
        'name': token['name'],
        'symbol': token['symbol'],
        'decimals': token['decimals'],
        'contractAddress': token['contractAddress'],
        'network': token['network'],
        'chainId': token['chainId'],
        'blockExplorer': token['blockExplorer'],
        'rpc': token['rpc'],
        'coinType': token['coinType'],
        'noPrice': true,
      });
    }
    setState(() {
      userAddedToken.value = tokenList;
    });
  }

  @override
  Widget build(BuildContext context) {
    return MultiValueListenableBuilder(
        valueListenables: [userAddedToken, toggler],
        builder: (context, value, child) {
          List<Widget> addedTokens = <Widget>[];
          if (userAddedToken != null) {
            for (int i = 0; i < userAddedToken.value.length; i++) {
              final notifier = ValueNotifier<double>(0.0);
              addedTokenListNotifiers.add(notifier);
              addedTokens.addAll([
                Dismissible(
                  background: Container(),
                  key: UniqueKey(),
                  direction: DismissDirection.endToStart,
                  secondaryBackground: Container(
                    color: Colors.red,
                    margin: const EdgeInsets.symmetric(horizontal: 15),
                    alignment: Alignment.centerRight,
                    child: const Padding(
                      padding: EdgeInsets.only(right: 10),
                      child: Icon(
                        Icons.delete,
                        color: Colors.white,
                      ),
                    ),
                  ),
                  onDismissed: (DismissDirection direction) {
                    toggler.value = !toggler.value;
                  },
                  confirmDismiss: (DismissDirection direction) async {
                    final pref = Hive.box(secureStorageKey);

                    final userTokenListKey = await getAddTokenKey();

                    if (userAddedToken.value.isEmpty) return false;
                    String customTokenDetailsKey = await contractDetailsKey(
                      userAddedToken.value[i]['rpc'],
                      userAddedToken.value[i]['contractAddress'],
                    );

                    if (pref.get(customTokenDetailsKey) != null) {
                      await pref.delete(customTokenDetailsKey);
                    }
                    userAddedToken.value.removeAt(i);

                    await pref.put(
                      userTokenListKey,
                      jsonEncode(userAddedToken),
                    );
                    return true;
                  },
                  child: InkWell(
                    child: Column(
                      children: [
                        GetBlockChainWidget(
                          name_:
                              ellipsify(str: userAddedToken.value[i]['name']),
                          image_: userAddedToken.value[i]['image'] ??
                              AssetImage(userAddedToken.value[i]['image']),
                          priceWithCurrency_: '0',
                          hasPrice_: false,
                          cryptoChange_: 0,
                          symbol_: userAddedToken.value[i]['symbol'],
                          cryptoAmount_: ValueListenableBuilder(
                            valueListenable: notifier,
                            builder: ((_, double value, Widget __) {
                              if (value == null) {
                                () async {
                                  try {
                                    notifier.value = await getERC20TokenBalance(
                                      userAddedToken.value[i],
                                      skipNetworkRequest:
                                          notifier.value == null,
                                    );
                                  } catch (_) {}
                                  addedTokenListTimer.add(
                                    Timer.periodic(httpPollingDelay,
                                        (timer) async {
                                      try {
                                        notifier.value =
                                            await getERC20TokenBalance(
                                          userAddedToken.value[i],
                                          skipNetworkRequest:
                                              notifier.value == null,
                                        );
                                      } catch (_) {}
                                    }),
                                  );
                                }();

                                return Container();
                              }
                              return UserBalance(
                                symbol: ellipsify(
                                  str: userAddedToken.value[i]['symbol'],
                                ),
                                balance: value,
                              );
                            }),
                          ),
                        ),
                        const Divider()
                      ],
                    ),
                    onTap: () {
                      // Get.to(
                      //   Token(
                      //     data: userAddedToken[i],
                      //   ),
                      // );
                      Navigator.of(context).push(MaterialPageRoute(
                        builder: (_) => Token(
                          data: userAddedToken.value[i],
                        ),
                      ));
                    },
                  ),
                ),
              ]);
            }
          }
          return Column(
            children: addedTokens,
          );
        });
  }
}
