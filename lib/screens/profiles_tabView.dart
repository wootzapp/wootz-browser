import 'dart:convert';

import 'package:cryptowallet/components/loader.dart';
import 'package:cryptowallet/config/colors.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/material.dart';
import 'package:flutter/src/foundation/key.dart';
import 'package:flutter/src/widgets/framework.dart';
import 'package:flutter/src/widgets/placeholder.dart';
import 'package:hive/hive.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';
import 'package:provider/provider.dart';

import '../components/user_details_placeholder.dart';
import '../model/provider.dart';
import '../utils/app_config.dart';
import 'build_row.dart';

class ProfilesTabView extends StatefulWidget {
  Function onTap;
  ProfilesTabView({
    Key key,
    this.onTap,
  }) : super(key: key);

  @override
  State<ProfilesTabView> createState() => _ProfilesTabViewState();
}

class _ProfilesTabViewState extends State<ProfilesTabView> {
  List<Tab> tabProfiles = [];
  List tabView = [];
  Map chainDetails = getEthereumDetailsFromChainId(1);

  int tabIndex = 0;
  int currentProfileIndex = 0;
  String currentTabProfile = pref.get(currentMmenomicKey);
  int currentTabChain = 1;

  int windowId;

  @override
  void initState() {
    super.initState();
    getChainDetails(currentTabChain);
    WidgetsBinding.instance.addPostFrameCallback((_) async {
      setTabProfiles();
    });
  }

  void setTabProfiles() {
    final tabUserData = Provider.of<ProviderClass>(context, listen: false);

    windowId = tabUserData.currentWindowId;
    print('windowId $windowId');

    final pref = Hive.box(secureStorageKey);
    final mnemonics = jsonDecode(pref.get(mnemonicListKey)) as List;
    currentTabProfile = tabUserData.tabUserCred[windowId]['profile'];
    currentTabChain = tabUserData.tabUserCred[windowId]['chain'];

    // print('currentTabProfile $currentTabProfile');
    // print('currentTabChain $currentTabChain');

    // int selectedChainId = currentTabChain;

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
            // print('index $tabIndex');
            // await pref.put(currentMmenomicKey, mnemonics[tabIndex]['phrase']);
            // print(currentTabProfile);
            currentTabProfile = mnemonics[tabIndex]['phrase'];
            final tabUserData =
                Provider.of<ProviderClass>(context, listen: false);
            tabUserData.changeTabUserCred(
                windowId, currentTabProfile, blockChainDetails['chainId']);
            // print(currentTabProfile);
            blockChainDetails['name'] = i;
            if (widget.onTap != null) {
              widget.onTap(currentTabProfile, blockChainDetails['chainId']);
            } else {
              Navigator.pop(context);
            }
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
      if (currentTabProfile == mnemonics[i]['phrase']) {
        currentProfileIndex = i;
      }
      tempTabProfiles.add(
        Tab(
          icon: const Icon(Icons.account_box),
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
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  // const Align(
                  //   // alignment: Alignment.centerRight,
                  //   child: IconButton(
                  //     onPressed: null,
                  //     icon: Icon(
                  //       Icons.close,
                  //       color: Colors.transparent,
                  //     ),
                  //   ),
                  // ),
                  Text(
                    AppLocalizations.of(context).selectBlockchains,
                    style: const TextStyle(
                      fontWeight: FontWeight.bold,
                      fontSize: 20.0,
                    ),
                  ),
                  // Align(
                  //   alignment: Alignment.centerRight,
                  //   child: IconButton(
                  //     onPressed: () {
                  //       if (Navigator.canPop(context)) {
                  //         Navigator.pop(context);
                  //       }
                  //     },
                  //     icon: const Icon(Icons.close),
                  //   ),
                  // ),
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
      chainDetails = getEthereumDetailsFromChainId(currentTabChain);
    });
  }

  void getChainDetails(int chainId) {
    // print('chainId $chainId');
    chainDetails = getEthereumDetailsFromChainId(chainId);
    // print(chainDetails);
    // print('getChainDetails-> ${chainDetails['image']}');
  }

  @override
  Widget build(BuildContext context) {
    if (tabProfiles?.isEmpty == null) {
      return const Loader();
    } else {
      getChainDetails(currentTabChain ?? 1);
      return DefaultTabController(
        length: tabProfiles.length,
        child: Builder(
          builder: (context) {
            // index = DefaultTabController.of(context).index;

            return Scaffold(
              appBar: AppBar(
                leading: Padding(
                  padding: const EdgeInsets.only(left: 6),
                  child: GestureDetector(
                    onTap: () {},
                    child: const UserDetailsPlaceHolder(
                      size: .5,
                      showHi: false,
                    ),
                  ),
                ),
                title: Text(
                  'Main Profile ${currentProfileIndex + 1}',
                  style: const TextStyle(color: black),
                ),
                actions: <Widget>[
                  Padding(
                    padding: const EdgeInsets.only(right: 20),
                    child: GestureDetector(
                      onTap: () {},
                      child: CircleAvatar(
                        backgroundImage: AssetImage(chainDetails['image']),
                      ),
                    ),
                  ),
                ],
                backgroundColor: white,
                bottom: TabBar(
                    onTap: (index) {
                      tabIndex = index;
                      // print('tabBar index $tabIndex');
                    },
                    tabs: [
                      ...tabProfiles,
                    ]),
              ),
              body: TabBarView(
                children: [
                  ...tabView,
                ],
              ),
            );
          },
        ),
      );
    }
  }
}
