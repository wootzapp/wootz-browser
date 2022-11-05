import 'package:cryptowallet/screens/exchange_token.dart';
import 'package:cryptowallet/screens/view_all_nfts.dart';
import 'package:cryptowallet/screens/wallet_main_body.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:flutter_svg/svg.dart';
import 'package:flutter/material.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

class Wallet extends StatefulWidget {
  const Wallet({Key key}) : super(key: key);

  @override
  _WalletState createState() => _WalletState();
}

bool isDarkMode = true;

class _WalletState extends State<Wallet> {
  PageController pageController;
  int currentIndex_ = 0;

  @override
  void initState() {
    super.initState();
    enableScreenShot();
    FocusManager.instance.primaryFocus?.unfocus();
    pageController = PageController(initialPage: 0);
  }

  _onTapped(int index) {
    setState(() {
      currentIndex_ = index;
    });
    // remove keyboard focus
    FocusManager.instance.primaryFocus?.unfocus();
    pageController.animateToPage(index,
        duration: const Duration(milliseconds: 500), curve: Curves.easeInOut);
  }

  void onPageChanged(int index) {
    setState(() {
      currentIndex_ = index;
      // remove focus
      FocusManager.instance.primaryFocus?.unfocus();
    });
  }

  final pages = [
    const WalletMainBody(),
    Container(),
    const ExchangeToken(),
    const ViewAllNFTs(),
  ];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      bottomNavigationBar: BottomNavigationBar(
        type: BottomNavigationBarType.fixed,
        currentIndex: currentIndex_,
        elevation: 0,
        onTap: _onTapped,
        items: <BottomNavigationBarItem>[
          BottomNavigationBarItem(
            icon: SvgPicture.asset(
              'assets/home_bottom.svg',
              color: currentIndex_ == 0
                  ? Theme.of(context).bottomNavigationBarTheme.selectedItemColor
                  : Theme.of(context)
                      .bottomNavigationBarTheme
                      .unselectedItemColor,
            ),
            label: AppLocalizations.of(context).wallet,
          ),
          BottomNavigationBarItem(
            icon: SvgPicture.asset(
              'assets/dapp_bottom.svg',
              color: currentIndex_ == 1
                  ? Theme.of(context).bottomNavigationBarTheme.selectedItemColor
                  : Theme.of(context)
                      .bottomNavigationBarTheme
                      .unselectedItemColor,
            ),
            label: 'Dapp',
          ),
          BottomNavigationBarItem(
            icon: SvgPicture.asset(
              'assets/exchange_bottom.svg',
              color: currentIndex_ == 2
                  ? Theme.of(context).bottomNavigationBarTheme.selectedItemColor
                  : Theme.of(context)
                      .bottomNavigationBarTheme
                      .unselectedItemColor,
            ),
            label: AppLocalizations.of(context).swap,
          ),
          BottomNavigationBarItem(
            icon: SvgPicture.asset(
              'assets/nft_bottom.svg',
              color: currentIndex_ == 3
                  ? Theme.of(context).bottomNavigationBarTheme.selectedItemColor
                  : Theme.of(context)
                      .bottomNavigationBarTheme
                      .unselectedItemColor,
            ),
            label: 'NFTs',
          ),
        ],
      ),
      body: PageView(
        physics: const NeverScrollableScrollPhysics(),
        controller: pageController,
        onPageChanged: onPageChanged,
        children: pages,
      ),
    );
  }
}
