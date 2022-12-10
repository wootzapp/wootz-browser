import 'package:flutter/material.dart';
import 'package:font_awesome_flutter/font_awesome_flutter.dart';

class WalletBlack extends StatefulWidget {
  const WalletBlack({Key key}) : super(key: key);

  @override
  State<WalletBlack> createState() => _WalletBlackState();
}

class _WalletBlackState extends State<WalletBlack> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: SafeArea(
        child: SingleChildScrollView(
          child: Column(
            children: [
              Padding(
                padding: const EdgeInsets.all(15),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Card(
                      child: Padding(
                        padding: const EdgeInsets.all(10.0),
                        child: Column(children: [
                          Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: [
                              const Text('Wallets'),
                              Row(
                                children: const [
                                  Text('See all'),
                                  Icon(Icons.arrow_forward),
                                ],
                              ),
                            ],
                          ),
                          // const SizedBox(
                          //   height: 10,
                          // ),
                          // Row(
                          //   mainAxisAlignment: MainAxisAlignment.spaceBetween,
                          //   children: [
                          //     const Icon(FontAwesomeIcons.bitcoin),
                          //     Column(
                          //       crossAxisAlignment: CrossAxisAlignment.start,
                          //       children: const [
                          //         Text('BTC'),
                          //         Text('Bitcoin'),
                          //       ],
                          //     ),
                          //     Container(
                          //       padding: const EdgeInsets.all(5),
                          //       decoration: BoxDecoration(
                          //         borderRadius: BorderRadius.circular(20),
                          //         color: Colors.grey[300],
                          //       ),
                          //       child: Row(
                          //         children: [
                          //           const Icon(
                          //             FontAwesomeIcons.wallet,
                          //             size: 15,
                          //           ),
                          //           const SizedBox(
                          //             width: 10,
                          //           ),
                          //           Column(
                          //             crossAxisAlignment:
                          //                 CrossAxisAlignment.start,
                          //             children: const [
                          //               Text(
                          //                 '1.1784 BTC',
                          //                 style: TextStyle(
                          //                   fontWeight: FontWeight.bold,
                          //                 ),
                          //               ),
                          //               Text(
                          //                 '6,300 USD',
                          //                 style: TextStyle(
                          //                   fontSize: 10,
                          //                   color: Colors.grey,
                          //                 ),
                          //               ),
                          //             ],
                          //           )
                          //         ],
                          //       ),
                          //     ),
                          //     Column(
                          //       children: [
                          //         Row(
                          //           children: const [
                          //             Icon(
                          //               FontAwesomeIcons.link,
                          //               size: 15,
                          //             ),
                          //             SizedBox(
                          //               width: 5,
                          //             ),
                          //             Text(
                          //               '2.056 BTC',
                          //               style: TextStyle(
                          //                 fontSize: 10,
                          //                 color: Colors.grey,
                          //               ),
                          //             ),
                          //           ],
                          //         ),
                          //         const SizedBox(
                          //           height: 2,
                          //         ),
                          //         Row(
                          //           children: const [
                          //             Icon(
                          //               FontAwesomeIcons.bolt,
                          //               size: 13,
                          //             ),
                          //             SizedBox(
                          //               width: 5,
                          //             ),
                          //             Text(
                          //               '0.34 BTC',
                          //               style: TextStyle(
                          //                 fontSize: 10,
                          //                 color: Colors.grey,
                          //               ),
                          //             ),
                          //           ],
                          //         )
                          //       ],
                          //     ),
                          //     const Icon(
                          //       FontAwesomeIcons.qrcode,
                          //       size: 20,
                          //     ),
                          //     const Icon(
                          //       Icons.remove_red_eye_outlined,
                          //       size: 20,
                          //     ),
                          //   ],
                          // ),

                          const Divider(
                            color: Colors.grey,
                          ),
                          Row(
                            mainAxisAlignment: MainAxisAlignment.center,
                            children: const [
                              Icon(FontAwesomeIcons.wallet),
                              SizedBox(
                                width: 5,
                              ),
                              Text('Create new wallet'),
                            ],
                          )
                        ]),
                      ),
                    ),
                    const SizedBox(
                      height: 20,
                    ),
                    Card(
                      child: Padding(
                        padding: const EdgeInsets.all(10.0),
                        child: Column(children: [
                          Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: const [
                              Text('Send'),
                              Icon(
                                Icons.edit_outlined,
                              ),
                            ],
                          ),
                          const Divider(),
                          Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: [
                              const Icon(Icons.currency_bitcoin_sharp),
                              SizedBox(
                                width: 200,
                                height: 40,
                                child: TextFormField(
                                  autocorrect: false,
                                  keyboardType: TextInputType.visiblePassword,

                                  // controller: ,
                                  decoration: const InputDecoration(
                                    hintText: '',

                                    focusedBorder: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none),
                                    border: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none),
                                    enabledBorder: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none), // you
                                    filled: true,
                                  ),
                                ),
                              ),
                              const Icon(
                                FontAwesomeIcons.link,
                                size: 20,
                              ),
                              const Icon(
                                FontAwesomeIcons.qrcode,
                                size: 20,
                              ),
                            ],
                          ),
                          const SizedBox(
                            height: 15,
                          ),
                          Row(
                            mainAxisAlignment: MainAxisAlignment.spaceBetween,
                            children: [
                              const Icon(
                                FontAwesomeIcons.bitcoin,
                                size: 30,
                              ),
                              SizedBox(
                                width: 100,
                                height: 40,
                                child: TextFormField(
                                  autocorrect: false,
                                  keyboardType: TextInputType.visiblePassword,
                                  decoration: const InputDecoration(
                                    hintText: '',

                                    focusedBorder: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none),
                                    border: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none),
                                    enabledBorder: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none), // you
                                    filled: true,
                                  ),
                                ),
                              ),
                              const Icon(
                                FontAwesomeIcons.dollarSign,
                                size: 30,
                              ),
                              SizedBox(
                                width: 100,
                                height: 40,
                                child: TextFormField(
                                  autocorrect: false,
                                  keyboardType: TextInputType.visiblePassword,

                                  // controller: ,
                                  decoration: const InputDecoration(
                                    hintText: '',

                                    focusedBorder: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none),
                                    border: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none),
                                    enabledBorder: OutlineInputBorder(
                                        borderRadius: BorderRadius.all(
                                            Radius.circular(10.0)),
                                        borderSide: BorderSide.none), // you
                                    filled: true,
                                  ),
                                ),
                              ),
                            ],
                          ),
                          const SizedBox(
                            height: 15,
                          ),
                          SizedBox(
                            width: double.infinity,
                            child: ElevatedButton(
                              style: ButtonStyle(
                                backgroundColor:
                                    MaterialStateProperty.resolveWith(
                                        (states) => Colors.grey),
                                shape: MaterialStateProperty.resolveWith(
                                  (states) => RoundedRectangleBorder(
                                    borderRadius: BorderRadius.circular(10),
                                  ),
                                ),
                                textStyle: MaterialStateProperty.resolveWith(
                                  (states) =>
                                      const TextStyle(color: Colors.white),
                                ),
                              ),
                              onPressed: () {},
                              child: const Text('Send'),
                            ),
                          )
                        ]),
                      ),
                    ),
                    const SizedBox(
                      height: 15,
                    ),
                    const Text('Explore sites'),
                    const SizedBox(
                      height: 15,
                    ),
                    Row(
                      children: const [
                        Expanded(
                          child: Card(
                            child: Padding(
                              padding: EdgeInsets.all(20.0),
                              child: Text('Currence'),
                            ),
                          ),
                        ),
                        Expanded(
                          child: Card(
                            child: Padding(
                              padding: EdgeInsets.all(20.0),
                              child: Text('Exchange rate'),
                            ),
                          ),
                        ),
                      ],
                    ),
                    const SizedBox(
                      height: 10,
                    ),
                  ],
                ),
              ),
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(10),
                  child: Column(children: [
                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        const Text('Transactions history'),
                        Row(
                          children: const [
                            Text('See all'),
                            Icon(Icons.arrow_forward),
                          ],
                        )
                      ],
                    )
                  ]),
                ),
              ),
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(10),
                  child: Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        SizedBox(
                          width: 100,
                          child: Column(
                            children: const [
                              Icon(Icons.qr_code),
                              Text(
                                'Scan',
                                style: TextStyle(fontSize: 12),
                              ),
                            ],
                          ),
                        ),
                        SizedBox(
                          width: 100,
                          child: Column(
                            children: const [
                              Icon(Icons.search),
                              Text(
                                'Search Engine',
                                style: TextStyle(fontSize: 12),
                              ),
                            ],
                          ),
                        ),
                        SizedBox(
                          width: 100,
                          child: Column(
                            children: const [
                              Icon(FontAwesomeIcons.wallet),
                              Text(
                                'Create new wallet',
                                style: TextStyle(fontSize: 12),
                                textAlign: TextAlign.center,
                              ),
                            ],
                          ),
                        ),
                      ]),
                ),
              ),
              // const Spacer(),
              // SizedBox(
              //   height: 70,
              //   child: Stack(
              //     children: [
              //       Card(
              //         child: Padding(
              //           padding: const EdgeInsets.all(10),
              //           child: Column(children: [
              //             Row(
              //               mainAxisAlignment: MainAxisAlignment.spaceBetween,
              //               children: [
              //                 const Text('Transactions history'),
              //                 Row(
              //                   children: const [
              //                     Text('See all'),
              //                     Icon(Icons.arrow_forward),
              //                   ],
              //                 )
              //               ],
              //             )
              //           ]),
              //         ),
              //       ),
              //       // Positioned(
              //       //   bottom: 5,
              //       //   child: Row(
              //       //     children: [
              //       //       Card(
              //       //         child: Padding(
              //       //           padding: const EdgeInsets.all(8.0),
              //       //           child: Column(
              //       //               mainAxisSize: MainAxisSize.min,
              //       //               crossAxisAlignment: CrossAxisAlignment.start,
              //       //               children: const [
              //       //                 Text('+0.01 ETH'),
              //       //                 Text('\$0.86 USD'),
              //       //               ]),
              //       //         ),
              //       //       ),
              //       //       Card(
              //       //         child: Padding(
              //       //           padding: const EdgeInsets.all(8.0),
              //       //           child: Row(
              //       //             children: [
              //       //               const Icon(FontAwesomeIcons.arrowDown),
              //       //               const SizedBox(
              //       //                 width: 5,
              //       //               ),
              //       //               Column(
              //       //                 mainAxisSize: MainAxisSize.min,
              //       //                 crossAxisAlignment:
              //       //                     CrossAxisAlignment.start,
              //       //                 children: const [
              //       //                   Text('Receive Ether'),
              //       //                   Text(
              //       //                     'November 18 2022 - 15:48',
              //       //                     style: TextStyle(
              //       //                       fontSize: 13,
              //       //                       color: Colors.grey,
              //       //                     ),
              //       //                   ),
              //       //                 ],
              //       //               ),
              //       //               Container(
              //       //                 padding: const EdgeInsets.all(5),
              //       //                 decoration: BoxDecoration(
              //       //                   borderRadius: BorderRadius.circular(20),
              //       //                   color: Colors.grey[300],
              //       //                 ),
              //       //                 child: const Text(
              //       //                   'Confirmed',
              //       //                   style: TextStyle(fontSize: 8),
              //       //                 ),
              //       //               )
              //       //             ],
              //       //           ),
              //       //         ),
              //       //       ),
              //       //     ],
              //       //   ),
              //       // ),
              //     ],
              //   ),
              // ),
            ],
          ),
        ),
      ),
    );
  }
}
