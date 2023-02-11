import 'dart:convert';

import 'package:cryptowallet/components/loader.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/material.dart';
import 'package:flutter/src/foundation/key.dart';
import 'package:flutter/src/widgets/framework.dart';
import 'package:flutter/src/widgets/placeholder.dart';
import 'package:hive/hive.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

import '../utils/app_config.dart';
import 'build_row.dart';

class ProfilesTabView extends StatefulWidget {
  Function onTap;
  ProfilesTabView({Key key, this.onTap}) : super(key: key);

  @override
  State<ProfilesTabView> createState() => _ProfilesTabViewState();
}

class _ProfilesTabViewState extends State<ProfilesTabView> {
  List<Tab> tabProfiles = [];
  List tabView = [];

  int tabIndex = 0;

  @override
  void initState() {
    super.initState();

    WidgetsBinding.instance.addPostFrameCallback((_) async {
      setTabProfiles();
    });
  }

  void setTabProfiles() {
    final pref = Hive.box(secureStorageKey);
    final mnemonics = jsonDecode(pref.get(mnemonicListKey)) as List;
    final currentMnemonic = pref.get(currentMmenomicKey);

    List<Tab> tempTabProfiles = [];
    List tempTabView = [];

    final ethEnabledBlockChain = <Widget>[];
    for (String i in getEVMBlockchains().keys) {
      Map blockChainDetails = getEVMBlockchains()[i];
      bool isSelected = false;
      // if (selectedChainId != null &&
      //     blockChainDetails['chainId'] == selectedChainId) {
      //   isSelected = true;
      // }

      ethEnabledBlockChain.add(
        InkWell(
          onTap: () async {
            print('index $tabIndex');
            await pref.put(currentMmenomicKey, mnemonics[tabIndex]['phrase']);
            blockChainDetails['name'] = i;
            widget.onTap(blockChainDetails);
          },
          child: buildRow(
            blockChainDetails['image'],
            i,
            isSelected: isSelected,
          ),
        ),
      );
    }
    for (int i = 0; i < mnemonics.length; i++) {
      // print(mnemonics[i]['phrase']);
      // if (currentMnemonic == mnemonics[i]['phrase']) {
      //   index = i;
      // }
      tempTabProfiles.add(
        Tab(
          icon: Icon(Icons.account_box),
          text: '${mnemonics[i]['name'] ?? 'main Profile ${i + 1}'}',
        ),
      );
      tempTabView.add(
        Container(
          color: Colors.transparent,
          child: ListView(
            shrinkWrap: true,
            children: <Widget>[
              const SizedBox(height: 20),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  const Align(
                    alignment: Alignment.centerRight,
                    child: IconButton(
                      onPressed: null,
                      icon: Icon(
                        Icons.close,
                        color: Colors.transparent,
                      ),
                    ),
                  ),
                  Text(
                    AppLocalizations.of(context).selectBlockchains,
                    style: const TextStyle(
                      fontWeight: FontWeight.bold,
                      fontSize: 20.0,
                    ),
                  ),
                  Align(
                    alignment: Alignment.centerRight,
                    child: IconButton(
                      onPressed: () {
                        if (Navigator.canPop(context)) {
                          Navigator.pop(context);
                        }
                      },
                      icon: const Icon(Icons.close),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 20),
              ...ethEnabledBlockChain,
              const SizedBox(height: 20),
            ],
          ),
        ),
      );
    }
    setState(() {
      tabProfiles = tempTabProfiles;
      tabView = tempTabView;
    });
  }

  @override
  Widget build(BuildContext context) {
    if (tabProfiles?.isEmpty == null) {
      return const Loader();
    } else {
      return DefaultTabController(
          length: tabProfiles.length,
          child: Builder(builder: (context) {
            // index = DefaultTabController.of(context).index;

            return Scaffold(
              appBar: AppBar(
                bottom: TabBar(
                    onTap: (index) {
                      tabIndex = index;
                      print('tabBar index $tabIndex');
                    },
                    tabs: [
                      ...tabProfiles,
                    ]),
              ),
              body: TabBarView(children: [
                ...tabView,
              ]),
            );
          }));
    }
  }
}
