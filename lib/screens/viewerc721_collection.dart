import 'dart:async';
import 'dart:convert';

import 'package:cached_network_image/cached_network_image.dart';
import 'package:cryptowallet/screens/send_token.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:hive/hive.dart';
import 'package:http/http.dart';
import 'package:web3dart/web3dart.dart' as web3;
import 'package:web3dart/web3dart.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

import '../components/loader.dart';

class ViewERC721Collection extends StatefulWidget {
  final String collectionName;
  final String networkName;
  final String tokenAddress;
  final int chainId;
  final int coinType;
  // constructor
  const ViewERC721Collection(
      {Key key,
      this.collectionName,
      this.networkName,
      this.tokenAddress,
      this.coinType,
      this.chainId})
      : super(key: key);
  @override
  _ViewERC721CollectionState createState() => _ViewERC721CollectionState();
}

class _ViewERC721CollectionState extends State<ViewERC721Collection> {
  // scaffold key
  final GlobalKey<ScaffoldState> _scaffoldKey = GlobalKey<ScaffoldState>();
  Timer timer;
  List nftItems_;

  @override
  void initState() {
    super.initState();
    timer = Timer.periodic(
      httpPollingDelay,
      (Timer t) async => await getErc721Collection(),
    );
  }

  @override
  void dispose() {
    timer?.cancel();
    super.dispose();
  }

  Future getErc721Collection() async {
    final pref = Hive.box(secureStorageKey);
    final saveNFTKey =
        '${widget.chainId}${widget.collectionName}${widget.tokenAddress}NFT';
    try {
      final client = web3.Web3Client(
        getEVMBlockchains()[widget.networkName]['rpc'],
        Client(),
      );
      final contract = web3.DeployedContract(
        web3.ContractAbi.fromJson(erc721Abi, ''),
        web3.EthereumAddress.fromHex(widget.tokenAddress),
      );

      final mnemonic = Hive.box(secureStorageKey).get(currentMmenomicKey);
      final response = await getEthereumFromMemnomic(
        mnemonic,
        widget.coinType,
      );

      final balanceFunction = contract.function('balanceOf');
      final tokenOfOwnerByIndex = contract.function('tokenOfOwnerByIndex');
      final tokenMetadataURI = contract.function('tokenURI');

      final balance = int.parse((await client.call(
              contract: contract,
              function: balanceFunction,
              params: [
            EthereumAddress.fromHex(response['eth_wallet_address'])
          ]))
          .first
          .toString());

      final nftItems = [];

      for (int i = 0; i < balance; i++) {
        final tokenId = int.parse((await client.call(
                contract: contract,
                function: tokenOfOwnerByIndex,
                params: [
              EthereumAddress.fromHex(response['eth_wallet_address']),
              BigInt.from(i)
            ]))
            .first
            .toString());
        final tokenMetadataURLRes = localHostToIpAddress((await client.call(
                contract: contract,
                function: tokenMetadataURI,
                params: [BigInt.from(tokenId)]))
            .first
            .toString());

        if (kDebugMode) {
          print(tokenMetadataURLRes);
        }
        Map tokenMetaData = {};

        try {
          tokenMetaData = jsonDecode(
              (await get(Uri.parse(ipfsTohttp(tokenMetadataURLRes)))
                      .timeout(networkTimeOutDuration))
                  .body) as Map;
        } catch (e) {
          //
        }

        tokenMetaData['tokenId'] = tokenId;

        if (kDebugMode) {
          print(tokenMetaData);
        }

        nftItems.add(tokenMetaData);
      }

      await pref.put(
        saveNFTKey,
        nftItems,
      );

      nftItems_ = nftItems;
    } catch (e) {
      if (kDebugMode) {
        print(e);
      }
      nftItems_ = pref.get(saveNFTKey) ?? [];
    }
    if (mounted) setState(() {});
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      key: _scaffoldKey,
      appBar: AppBar(
        title: Text(
          ellipsify(
            str: widget.collectionName,
          ),
        ),
      ),
      body: SafeArea(
        child: RefreshIndicator(
          onRefresh: () async {
            if (mounted) setState(() {});
          },
          child: SingleChildScrollView(
            physics: const AlwaysScrollableScrollPhysics(),
            child: Container(
              color: Colors.transparent,
              child: Padding(
                padding: const EdgeInsets.all(25),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    GestureDetector(
                      onTap: () async {
                        await Clipboard.setData(
                          ClipboardData(
                            text: widget.tokenAddress,
                          ),
                        );

                        ScaffoldMessenger.of(context).showSnackBar(
                          SnackBar(
                            content: Text(
                                AppLocalizations.of(context).copiedToClipboard),
                            duration: const Duration(seconds: 2),
                          ),
                        );
                      },
                      child: Container(
                        color: Colors.transparent,
                        child: Padding(
                          padding: const EdgeInsets.all(10),
                          child: Column(
                            mainAxisAlignment: MainAxisAlignment.start,
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              Text(
                                widget.tokenAddress,
                                style: const TextStyle(
                                    fontWeight: FontWeight.bold, fontSize: 18),
                              ),
                              const Icon(
                                Icons.copy,
                              )
                            ],
                          ),
                        ),
                      ),
                    ),
                    const SizedBox(height: 10),
                    Padding(
                      padding: const EdgeInsets.fromLTRB(10, 0, 0, 0),
                      child: Text(
                        widget.networkName,
                        style: const TextStyle(
                          color: Colors.grey,
                          fontSize: 16,
                        ),
                      ),
                    ),
                    const SizedBox(height: 10),
                    if (nftItems_ != null && nftItems_.isNotEmpty)
                      for (int i = 0; i < nftItems_.length; i++)
                        Column(
                          children: [
                            Card(
                              shape: RoundedRectangleBorder(
                                  borderRadius: BorderRadius.circular(10)),
                              child: Padding(
                                padding: const EdgeInsets.all(15.0),
                                child: Column(children: [
                                  if (nftItems_[i]['image'] != null)
                                    CachedNetworkImage(
                                      imageUrl:
                                          ipfsTohttp(nftItems_[i]['image']),
                                      width: double.infinity,
                                      height: 300,
                                      placeholder: (context, url) => Column(
                                        mainAxisAlignment:
                                            MainAxisAlignment.center,
                                        mainAxisSize: MainAxisSize.min,
                                        children: const [
                                          SizedBox(
                                            width: 20,
                                            height: 20,
                                            child: Loader(
                                              color: appPrimaryColor,
                                            ),
                                          )
                                        ],
                                      ),
                                      errorWidget: (context, url, error) =>
                                          const Icon(
                                        Icons.error,
                                        color: Colors.red,
                                      ),
                                      fit: BoxFit.cover,
                                    )
                                  else
                                    SizedBox(
                                      width: 300,
                                      height: 300,
                                      child: Center(
                                        child: Text(
                                          AppLocalizations.of(context)
                                              .couldNotFetchData,
                                          style: const TextStyle(
                                            fontSize: 18,
                                          ),
                                        ),
                                      ),
                                    ),
                                  const SizedBox(height: 10),
                                  Row(
                                    mainAxisAlignment:
                                        MainAxisAlignment.spaceBetween,
                                    children: [
                                      Text(
                                        '${AppLocalizations.of(context).tokenId}: ',
                                        style: const TextStyle(
                                            fontWeight: FontWeight.bold),
                                      ),
                                      Text(
                                        ellipsify(
                                          str: nftItems_[i]['tokenId']
                                              .toString(),
                                        ),
                                      )
                                    ],
                                  ),
                                  const SizedBox(
                                    height: 10,
                                  ),
                                  Row(
                                    mainAxisAlignment:
                                        MainAxisAlignment.spaceBetween,
                                    children: [
                                      Flexible(
                                        fit: FlexFit.loose,
                                        child: Text(
                                          AppLocalizations.of(context).name,
                                          softWrap: false,
                                          overflow: TextOverflow.fade,
                                          style: const TextStyle(
                                              fontWeight: FontWeight.bold),
                                        ),
                                      ),
                                      Flexible(
                                        fit: FlexFit.loose,
                                        child: Text(
                                          ellipsify(
                                            str: nftItems_[i]['name'] ?? '',
                                          ),
                                          softWrap: false,
                                          overflow: TextOverflow.fade,
                                        ),
                                      ),
                                    ],
                                  ),
                                  const SizedBox(
                                    height: 10,
                                  ),
                                  Row(
                                    mainAxisAlignment:
                                        MainAxisAlignment.spaceBetween,
                                    children: [
                                      Flexible(
                                        fit: FlexFit.loose,
                                        child: Text(
                                          AppLocalizations.of(context)
                                              .description,
                                          softWrap: false,
                                          overflow: TextOverflow.fade,
                                          style: const TextStyle(
                                              fontWeight: FontWeight.bold),
                                        ),
                                      ),
                                      Flexible(
                                        fit: FlexFit.loose,
                                        child: Text(
                                          ellipsify(
                                            str: nftItems_[i]['description'] ??
                                                '',
                                          ),
                                          softWrap: false,
                                          overflow: TextOverflow.fade,
                                        ),
                                      ),
                                    ],
                                  ),
                                  const SizedBox(
                                    height: 10,
                                  ),
                                  Row(
                                    mainAxisAlignment:
                                        MainAxisAlignment.spaceBetween,
                                    children: [
                                      Text(
                                        AppLocalizations.of(context)
                                            .tokenStandard,
                                        style: const TextStyle(
                                            fontWeight: FontWeight.bold),
                                      ),
                                      const Text('ERC-721')
                                    ],
                                  ),
                                  const SizedBox(
                                    height: 10,
                                  ),
                                  Container(
                                    color: Colors.transparent,
                                    width: double.infinity,
                                    height: 50,
                                    child: ElevatedButton(
                                      style: ButtonStyle(
                                        textStyle:
                                            MaterialStateProperty.resolveWith(
                                          (states) => const TextStyle(
                                              color: Colors.white),
                                        ),
                                        backgroundColor:
                                            MaterialStateProperty.resolveWith(
                                                (states) => appBackgroundblue),
                                        shape:
                                            MaterialStateProperty.resolveWith(
                                          (states) => RoundedRectangleBorder(
                                            borderRadius:
                                                BorderRadius.circular(10),
                                          ),
                                        ),
                                      ),
                                      onPressed: () async {
                                        try {
                                          await Navigator.push(
                                            context,
                                            MaterialPageRoute(
                                              builder: (ctx) => SendToken(
                                                data: {
                                                  'name': nftItems_[i]
                                                          ['name'] ??
                                                      'NFT',
                                                  'symbol':
                                                      '${nftItems_[i]['name'] ?? 'NFT'}  #${nftItems_[i]['tokenId']}',
                                                  'isNFT': true,
                                                  'tokenId': nftItems_[i]
                                                      ['tokenId'],
                                                  'contractAddress':
                                                      widget.tokenAddress,
                                                  'network': widget.networkName,
                                                  'rpc': getEVMBlockchains()[
                                                          widget.networkName]
                                                      ['rpc'],
                                                  'chainId':
                                                      getEVMBlockchains()[widget
                                                              .networkName]
                                                          ['chainId'],
                                                  'coinType':
                                                      getEVMBlockchains()[widget
                                                              .networkName]
                                                          ['coinType'],
                                                  'image': 'assets/logo.png'
                                                },
                                              ),
                                            ),
                                          );
                                        } catch (e) {
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
                                        }
                                      },
                                      child: Text(
                                        AppLocalizations.of(context).send,
                                        style: const TextStyle(
                                          fontWeight: FontWeight.bold,
                                          color: Colors.white,
                                        ),
                                      ),
                                    ),
                                  ),
                                ]),
                              ),
                            ),
                            const SizedBox(
                              height: 10,
                            )
                          ],
                        )
                    else
                      SizedBox(
                        height: MediaQuery.of(context).size.height * .5,
                        child: const Center(
                          child: Loader(),
                        ),
                      )
                  ],
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}
