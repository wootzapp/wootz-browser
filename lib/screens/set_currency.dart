import 'dart:convert';

import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:hive/hive.dart';
import 'package:http/http.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

class SetCurrency extends StatefulWidget {
  final String data;
  const SetCurrency({this.data, Key key}) : super(key: key);

  @override
  _SetCurrencyState createState() => _SetCurrencyState();
}

class _SetCurrencyState extends State<SetCurrency> {
  final _scaffoldKey = GlobalKey<ScaffoldState>();
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(AppLocalizations.of(context).selectCurrency),
      ),
      key: _scaffoldKey,
      body: SafeArea(
        child: SingleChildScrollView(
          child: Padding(
            padding: const EdgeInsets.all(25),
            child: Column(
              children: [
                FutureBuilder(
                  future: () async {
                    if (kDebugMode) {
                      print('debugging');
                    }
                    final pref = Hive.box(secureStorageKey);
                    final defaultCurrency =
                        pref.get('defaultCurrency') ?? 'USD';

                    final currencyList = await getCurrencyJson();
                    return {
                      'defaultCurrency': defaultCurrency,
                      'currencyList': currencyList
                    };
                  }(),
                  builder: (ctx, snapshot) {
                    final currencyWidget = <Widget>[];
                    if (snapshot.hasData) {
                      final data = snapshot.data as Map;
                      final currencyList =
                          jsonDecode(data['currencyList']) as Map;

                      for (final currency in currencyList.keys) {
                        currencyWidget.add(
                          InkWell(
                            onTap: () async {
                              try {
                                final pref = Hive.box(secureStorageKey);
                                final responseBody = jsonDecode((await get(
                                        Uri.parse(
                                            coinGeckoSupportedCurrencies)))
                                    .body) as List;

                                if (responseBody.contains(
                                    currency.toString().toLowerCase())) {
                                  await pref.put(
                                    'defaultCurrency',
                                    currency,
                                  );
                                  Navigator.pop(context);
                                } else {
                                  ScaffoldMessenger.of(context)
                                      .showSnackBar(SnackBar(
                                    backgroundColor: Colors.red,
                                    content: Text(
                                      '$currency is not supported yet',
                                      style:
                                          const TextStyle(color: Colors.white),
                                    ),
                                    duration: const Duration(seconds: 2),
                                  ));
                                }
                              } catch (e) {
                                ScaffoldMessenger.of(context).showSnackBar(
                                  const SnackBar(
                                    backgroundColor: Colors.red,
                                    content: Text(
                                      'Could not change currency, please try again later.',
                                      style: TextStyle(color: Colors.white),
                                    ),
                                    duration: Duration(seconds: 2),
                                  ),
                                );
                              }
                            },
                            child: Row(
                              mainAxisAlignment: MainAxisAlignment.spaceBetween,
                              children: [
                                Flexible(
                                  child: Row(
                                    children: [
                                      CircleAvatar(
                                        radius: 20,
                                        backgroundColor: Colors.white,
                                        backgroundImage: AssetImage(
                                            'assets/currency_flags/${currency.toLowerCase()}.png'),
                                        onBackgroundImageError:
                                            (object, stacktrace) {
                                          Container();
                                        },
                                      ),
                                      const SizedBox(
                                        width: 10,
                                      ),
                                      Flexible(
                                        child: Column(
                                          crossAxisAlignment:
                                              CrossAxisAlignment.start,
                                          children: [
                                            Text(
                                              currency,
                                              style: const TextStyle(
                                                  fontSize: 16,
                                                  fontWeight: FontWeight.w500),
                                              overflow: TextOverflow.fade,
                                            ),
                                            const SizedBox(
                                              height: 10,
                                            ),
                                            Row(
                                              children: [
                                                Flexible(
                                                  child: Text(
                                                    currencyList[currency],
                                                    style: const TextStyle(
                                                      fontSize: 15,
                                                      color: Colors.grey,
                                                    ),
                                                  ),
                                                ),
                                              ],
                                            ),
                                          ],
                                        ),
                                      )
                                    ],
                                  ),
                                ),
                                if (currency == data['defaultCurrency'])
                                  Container(
                                    decoration: const BoxDecoration(
                                        shape: BoxShape.circle,
                                        color: Colors.blue),
                                    child: const Padding(
                                      padding: EdgeInsets.all(2),
                                      child: Icon(
                                        Icons.check,
                                        size: 20,
                                        color: Colors.white,
                                      ),
                                    ),
                                  )
                              ],
                            ),
                          ),
                        );

                        currencyWidget.add(const Divider());
                      }
                    }
                    return Column(
                      children: currencyWidget,
                    );
                  },
                )
              ],
            ),
          ),
        ),
      ),
    );
  }
}
