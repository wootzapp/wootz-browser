// import 'dart:io';

// import 'package:cryptowallet/screens/dapp.dart';
// import 'package:cryptowallet/utils/app_config.dart';
// import 'package:cryptowallet/utils/rpc_urls.dart';
// import 'package:device_info_plus/device_info_plus.dart';
// import 'package:flutter/material.dart';
// import 'package:flutter_svg/svg.dart';
// import 'package:flutter_gen/gen_l10n/app_localization.dart';
// import 'package:hive_flutter/hive_flutter.dart';

// class DappUI extends StatefulWidget {
//   const DappUI({Key key}) : super(key: key);

//   @override
//   State<DappUI> createState() => _DappUIState();
// }

// class _DappUIState extends State<DappUI> with AutomaticKeepAliveClientMixin {
//   final _scaffoldKey = GlobalKey<ScaffoldState>();
//   final searchController = TextEditingController();
//   @override
//   bool get wantKeepAlive => true;
//   @override
//   void dispose() {
//     searchController.dispose();
//     super.dispose();
//   }

//   Future<void> navigateToDappCondition(
//     BuildContext context, [
//     String url,
//   ]) async {
//     final pref = Hive.box(secureStorageKey);
//     bool hasWallet = pref.get(currentMmenomicKey) != null;
//     double androidVersion = Platform.isAndroid
//         ? double.parse((await DeviceInfoPlugin().androidInfo).version.release)
//         : 0;

//     final bool useHybridComposition = androidVersion >= 10;
//     if (hasWallet) {
//       await navigateToDappBrowser(
//         context,
//         url,
//       );
//       return;
//     }

//     await Navigator.push(
//       context,
//       MaterialPageRoute(
//         builder: (ctx) => Dapp(
//           provider: '',
//           init: '',
//           data: url,
//           useHybridComposition: useHybridComposition,
//         ),
//       ),
//     );
//   }

//   @override
//   Widget build(BuildContext context) {
//     super.build(context);
//     return Scaffold(
//       key: _scaffoldKey,
//       body: SafeArea(
//         child: SingleChildScrollView(
//           child: Padding(
//             padding: const EdgeInsets.all(30),
//             child: Column(
//               children: [
//                 Row(
//                   mainAxisAlignment: MainAxisAlignment.spaceBetween,
//                   children: [
//                     const Text(
//                       'Browser',
//                       style:
//                           TextStyle(fontWeight: FontWeight.bold, fontSize: 24),
//                     ),
//                     IconButton(
//                         onPressed: () async {
//                           await navigateToDappCondition(context);
//                         },
//                         icon: const Icon(
//                           Icons.search,
//                           size: 30,
//                         ))
//                   ],
//                 ),
//                 const SizedBox(
//                   height: 20,
//                 ),
//                 Align(
//                   alignment: Alignment.centerLeft,
//                   child: Text(
//                     AppLocalizations.of(context).favourites,
//                     style: const TextStyle(
//                         color: Colors.grey, fontSize: 16, letterSpacing: 3),
//                   ),
//                 ),
//                 // search field with search icon
//                 const Divider(),
//                 const SizedBox(
//                   height: 20,
//                 ),
//                 Row(
//                   mainAxisAlignment: MainAxisAlignment.spaceBetween,
//                   children: [
//                     Expanded(
//                       child: GestureDetector(
//                         onTap: () async {
//                           await navigateToDappCondition(
//                             context,
//                             walletURL,
//                           );
//                         },
//                         child: Card(
//                           shape: RoundedRectangleBorder(
//                               borderRadius: BorderRadius.circular(10)),
//                           child: Padding(
//                             padding: const EdgeInsets.all(12.0),
//                             child: Row(children: [
//                               SvgPicture.asset('assets/swap_dapp.svg'),
//                               const SizedBox(
//                                 width: 10,
//                               ),
//                               Text(AppLocalizations.of(context).exchange)
//                             ]),
//                           ),
//                         ),
//                       ),
//                     ),
//                     Expanded(
//                       child: Card(
//                         shape: RoundedRectangleBorder(
//                             borderRadius: BorderRadius.circular(10)),
//                         child: Padding(
//                           padding: const EdgeInsets.all(12.0),
//                           child: Row(children: [
//                             SvgPicture.asset('assets/stake_dapp.svg'),
//                             const SizedBox(
//                               width: 10,
//                             ),
//                             Text(AppLocalizations.of(context).stake)
//                           ]),
//                         ),
//                       ),
//                     ),
//                   ],
//                 ),

//                 Row(
//                   mainAxisAlignment: MainAxisAlignment.spaceBetween,
//                   children: [
//                     Expanded(
//                       child: Card(
//                         shape: RoundedRectangleBorder(
//                             borderRadius: BorderRadius.circular(10)),
//                         child: Padding(
//                           padding: const EdgeInsets.all(12.0),
//                           child: Row(children: [
//                             SvgPicture.asset('assets/fiat_dapp.svg'),
//                             const SizedBox(
//                               width: 10,
//                             ),
//                             Text(AppLocalizations.of(context).fiat)
//                           ]),
//                         ),
//                       ),
//                     ),
//                     Expanded(
//                       child: GestureDetector(
//                         onTap: () async {
//                           await navigateToDappCondition(context);
//                         },
//                         child: Card(
//                           shape: RoundedRectangleBorder(
//                               borderRadius: BorderRadius.circular(10)),
//                           child: Padding(
//                             padding: const EdgeInsets.all(12.0),
//                             child: Row(children: [
//                               SvgPicture.asset('assets/browser_dapp.svg'),
//                               const SizedBox(
//                                 width: 10,
//                               ),
//                               Text(AppLocalizations.of(context).browser)
//                             ]),
//                           ),
//                         ),
//                       ),
//                     ),
//                   ],
//                 ),
//                 const SizedBox(
//                   height: 20,
//                 ),
//                 Align(
//                   alignment: Alignment.centerLeft,
//                   child: Text(
//                     AppLocalizations.of(context).all,
//                     style: const TextStyle(
//                       color: Colors.grey,
//                       fontSize: 16,
//                       letterSpacing: 3,
//                     ),
//                   ),
//                 ),
//                 // search field with search icon
//                 const Divider(),
//                 const SizedBox(
//                   height: 20,
//                 ),
//                 // card widget
//                 Column(
//                   crossAxisAlignment: CrossAxisAlignment.start,
//                   children: [
//                     GestureDetector(
//                       onTap: () async {},
//                       child: Container(
//                         decoration:
//                             const BoxDecoration(color: Colors.transparent),
//                         clipBehavior: Clip.hardEdge,
//                         width: double.infinity,
//                         child: Padding(
//                           padding: const EdgeInsets.all(8.0),
//                           child: Row(
//                             crossAxisAlignment: CrossAxisAlignment.center,
//                             children: [
//                               SvgPicture.asset('assets/vr-dapp.svg'),
//                               const SizedBox(
//                                 width: 20,
//                               ),
//                               Column(
//                                 mainAxisAlignment: MainAxisAlignment.start,
//                                 crossAxisAlignment: CrossAxisAlignment.start,
//                                 children: [
//                                   Row(
//                                     children: [
//                                       Text(
//                                         AppLocalizations.of(context)
//                                             .virtualReality,
//                                         style: const TextStyle(
//                                           fontSize: 16,
//                                           fontWeight: FontWeight.bold,
//                                         ),
//                                       ),
//                                       const SizedBox(
//                                         width: 10,
//                                       ),
//                                       SvgPicture.asset('assets/new_dapp.svg'),
//                                     ],
//                                   ),
//                                   Text(
//                                     AppLocalizations.of(context)
//                                         .virtualRealityDescription,
//                                     style: const TextStyle(
//                                       fontSize: 14,
//                                       fontWeight: FontWeight.bold,
//                                       color: Colors.grey,
//                                     ),
//                                   ),
//                                 ],
//                               ),
//                             ],
//                           ),
//                         ),
//                       ),
//                     ),
//                     const Divider(),
//                     GestureDetector(
//                       onTap: () async {
//                         await navigateToDappBrowser(
//                           context,
//                           marketPlaceUrl,
//                         );
//                       },
//                       child: Container(
//                         decoration:
//                             const BoxDecoration(color: Colors.transparent),
//                         clipBehavior: Clip.hardEdge,
//                         width: double.infinity,
//                         child: Padding(
//                           padding: const EdgeInsets.all(8.0),
//                           child: Row(
//                             crossAxisAlignment: CrossAxisAlignment.center,
//                             children: [
//                               SvgPicture.asset('assets/social_dapp.svg'),
//                               const SizedBox(
//                                 width: 20,
//                               ),
//                               Column(
//                                 mainAxisAlignment: MainAxisAlignment.start,
//                                 crossAxisAlignment: CrossAxisAlignment.start,
//                                 children: [
//                                   Row(
//                                     children: [
//                                       Text(
//                                         AppLocalizations.of(context).exchange,
//                                         style: const TextStyle(
//                                           fontSize: 16,
//                                           fontWeight: FontWeight.bold,
//                                         ),
//                                       ),
//                                       const SizedBox(
//                                         width: 10,
//                                       ),
//                                       SvgPicture.asset('assets/new_dapp.svg'),
//                                     ],
//                                   ),
//                                   Text(
//                                     AppLocalizations.of(context)
//                                         .exchangeDescript,
//                                     style: const TextStyle(
//                                       fontSize: 14,
//                                       fontWeight: FontWeight.bold,
//                                       color: Colors.grey,
//                                     ),
//                                     overflow: TextOverflow.fade,
//                                   ),
//                                 ],
//                               ),
//                             ],
//                           ),
//                         ),
//                       ),
//                     ),
//                     const Divider(),
//                     GestureDetector(
//                       onTap: () async {
//                         await navigateToDappCondition(context, ecommerceUrl);
//                       },
//                       child: Container(
//                         decoration:
//                             const BoxDecoration(color: Colors.transparent),
//                         clipBehavior: Clip.hardEdge,
//                         width: double.infinity,
//                         child: Padding(
//                           padding: const EdgeInsets.all(8.0),
//                           child: Row(
//                             crossAxisAlignment: CrossAxisAlignment.center,
//                             children: [
//                               SvgPicture.asset('assets/e-commerce-dapp.svg'),
//                               const SizedBox(
//                                 width: 20,
//                               ),
//                               Column(
//                                 mainAxisAlignment: MainAxisAlignment.start,
//                                 crossAxisAlignment: CrossAxisAlignment.start,
//                                 children: [
//                                   Row(
//                                     children: [
//                                       Text(
//                                         AppLocalizations.of(context).eCommerce,
//                                         style: const TextStyle(
//                                           fontSize: 16,
//                                           fontWeight: FontWeight.bold,
//                                         ),
//                                       ),
//                                       const SizedBox(
//                                         width: 10,
//                                       ),
//                                       SvgPicture.asset('assets/new_dapp.svg'),
//                                     ],
//                                   ),
//                                   Text(
//                                     AppLocalizations.of(context).buyAndSell,
//                                     style: const TextStyle(
//                                       fontSize: 14,
//                                       fontWeight: FontWeight.bold,
//                                       color: Colors.grey,
//                                     ),
//                                   ),
//                                 ],
//                               ),
//                             ],
//                           ),
//                         ),
//                       ),
//                     ),
//                     const Divider(),
//                     GestureDetector(
//                       onTap: () async {
//                         await navigateToDappCondition(
//                           context,
//                           blogUrl,
//                         );
//                       },
//                       child: Container(
//                         decoration:
//                             const BoxDecoration(color: Colors.transparent),
//                         clipBehavior: Clip.hardEdge,
//                         width: double.infinity,
//                         child: Padding(
//                           padding: const EdgeInsets.all(8.0),
//                           child: Row(
//                             crossAxisAlignment: CrossAxisAlignment.center,
//                             children: [
//                               SvgPicture.asset('assets/news.svg'),
//                               const SizedBox(
//                                 width: 20,
//                               ),
//                               Column(
//                                 mainAxisAlignment: MainAxisAlignment.start,
//                                 crossAxisAlignment: CrossAxisAlignment.start,
//                                 children: [
//                                   Text(
//                                     AppLocalizations.of(context).blog,
//                                     style: const TextStyle(
//                                       fontSize: 16,
//                                       fontWeight: FontWeight.bold,
//                                     ),
//                                   ),
//                                   Text(
//                                     AppLocalizations.of(context).readLastest,
//                                     style: const TextStyle(
//                                       fontSize: 14,
//                                       fontWeight: FontWeight.bold,
//                                       color: Colors.grey,
//                                     ),
//                                   ),
//                                 ],
//                               ),
//                             ],
//                           ),
//                         ),
//                       ),
//                     ),
//                     const Divider(),
//                   ],
//                 ),
//               ],
//             ),
//           ),
//         ),
//       ),
//     );
//   }
// }
