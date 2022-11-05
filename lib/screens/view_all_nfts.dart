import 'dart:async';

import 'package:cryptowallet/screens/viewerc721_collection.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:hive/hive.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

class ViewAllNFTs extends StatefulWidget {
  const ViewAllNFTs({Key key}) : super(key: key);

  @override
  State<ViewAllNFTs> createState() => _ViewAllNFTsState();
}

class _ViewAllNFTsState extends State<ViewAllNFTs>
    with AutomaticKeepAliveClientMixin {
  @override
  bool get wantKeepAlive => true;
  bool isLoaded = false;
  ValueNotifier nftLoaded = ValueNotifier(false);
  @override
  Widget build(BuildContext context) {
    super.build(context);
    return Scaffold(
      body: SizedBox(
        height: double.infinity,
        child: SafeArea(
          child: RefreshIndicator(
            onRefresh: () async {
              await Future.delayed(const Duration(seconds: 2));
              if (mounted) setState(() {});
            },
            child: SingleChildScrollView(
              physics: const AlwaysScrollableScrollPhysics(),
              child: Container(
                color: Colors.transparent,
                child: Padding(
                  padding: const EdgeInsets.all(30),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      const SizedBox(
                        height: 10,
                      ),
                      const Text(
                        'NFTs',
                        style: TextStyle(
                          fontSize: 24,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      const SizedBox(
                        height: 20,
                      ),
                      ValueListenableBuilder(
                        valueListenable: nftLoaded,
                        builder: (context, value, _) {
                          return value == true
                              ? Container()
                              : Text(
                                  AppLocalizations.of(context)
                                      .yourAssetWillAppear,
                                  style: const TextStyle(fontSize: 18),
                                );
                        },
                      ),
                      for (int i = 0;
                          i < getEVMBlockchains().values.length;
                          i++)
                        BlockChainNFTs(
                          nftLoaded: nftLoaded,
                          chainId: getEVMBlockchains().values.toList()[i]
                              ['chainId'],
                          coinType: getEVMBlockchains().values.toList()[i]
                              ['coinType'],
                          networkName: getEVMBlockchains().keys.toList()[i],
                        )
                    ],
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}

class BlockChainNFTs extends StatefulWidget {
  final int coinType;
  final int chainId;

  final String networkName;
  final ValueNotifier nftLoaded;

  const BlockChainNFTs({
    Key key,
    this.coinType,
    this.networkName,
    this.chainId,
    this.nftLoaded,
  }) : super(key: key);

  @override
  State<BlockChainNFTs> createState() => _BlockChainNFTsState();
}

class _BlockChainNFTsState extends State<BlockChainNFTs> {
  Timer timer;

  Map nftData;

  bool skipNetworkRequest = true;

  @override
  void initState() {
    super.initState();
    getAllNFTs();
    timer = Timer.periodic(
      httpPollingDelay,
      (Timer t) async => await getAllNFTs(),
    );
  }

  @override
  void dispose() {
    timer?.cancel();
    super.dispose();
  }

  Future getAllNFTs() async {
    final mnemonic = Hive.box(secureStorageKey).get(currentMmenomicKey);
    final response = await getEthereumFromMemnomic(
      mnemonic,
      widget.coinType,
    );
    final data = await viewUserTokens(
      widget.chainId,
      response['eth_wallet_address'],
      skipNetworkRequest: skipNetworkRequest,
    );

    if (skipNetworkRequest) skipNetworkRequest = false;

    if (data['success'] != null && data['success']) {
      List nfts = data['msg']['erc721'];
      if (widget.nftLoaded.value == false && nfts.isNotEmpty) {
        widget.nftLoaded.value = true;
      }
      if (nfts.isNotEmpty) {
        nftData = data;
        if (mounted) setState(() {});
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    return nftData == null
        ? Container()
        : Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Text(
                widget.networkName,
                style: const TextStyle(
                  fontSize: 18,
                ),
              ),
              const SizedBox(
                height: 20,
              ),
              for (final nftItem in nftData['msg']['erc721'] as List) ...[
                GestureDetector(
                  onTap: () {
                    if (kDebugMode) {
                      print(nftItem['tokenAddress']);
                    }
                    Navigator.push(
                      context,
                      MaterialPageRoute(
                        builder: (context) => ViewERC721Collection(
                          collectionName: nftItem['tokenName'].split('(')[0],
                          tokenAddress: nftItem['tokenAddress'],
                          networkName: widget.networkName,
                          chainId: widget.chainId,
                          coinType: widget.coinType,
                        ),
                      ),
                    );
                  },
                  child: Card(
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(
                        10,
                      ),
                    ),
                    child: Padding(
                      padding: const EdgeInsets.all(10.0),
                      child: Row(
                        children: [
                          CircleAvatar(
                            child: Text(
                              ellipsify(
                                str: nftItem['tokenAmount'].split(' ')[1],
                                maxLength: 3,
                              ),
                              textAlign: TextAlign.center,
                              style: const TextStyle(
                                color: Colors.white,
                              ),
                            ),
                          ),
                          const SizedBox(
                            width: 10,
                          ),
                          Flexible(
                            child: Column(
                              crossAxisAlignment: CrossAxisAlignment.start,
                              children: [
                                Text(
                                  ellipsify(
                                    str: nftItem['tokenName'].split('(')[0],
                                  ),
                                  style: const TextStyle(
                                    fontSize: 18,
                                    fontWeight: FontWeight.bold,
                                    overflow: TextOverflow.fade,
                                  ),
                                ),
                                Text(
                                  '${ellipsify(
                                    str: nftItem['tokenAmount'].split(' ')[0],
                                  )} ${ellipsify(
                                    str: nftItem['tokenAmount'].split(' ')[1],
                                  )}',
                                  style: const TextStyle(
                                    fontSize: 16,
                                    fontWeight: FontWeight.bold,
                                    color: Colors.grey,
                                    overflow: TextOverflow.fade,
                                  ),
                                ),
                              ],
                            ),
                          )
                        ],
                      ),
                    ),
                  ),
                ),
                const Divider()
              ],
              const SizedBox(
                height: 20,
              ),
            ],
          );
  }
}
