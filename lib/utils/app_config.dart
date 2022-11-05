import 'package:flutter/material.dart';

const walletAbbr = 'WTZ';
const walletName = 'WootzApp';
const walletURL = "https://google.com";
const walletIconURL = "$walletURL/game/logo.png";
// addresses
const tokenContractAddress = '0x677676c090c7CD76976E40285666e2D0A0108852';
const tokenSaleContractAddress = "0x677676c090c7CD76976E40285666e2D0A0108852";
const tokenStakingContractAddress =
    '0xa85037b56Dc212eEa0DFBd76aFDfF47EB33650F9';

const mnemonicKey = 'mnemonic';

// token sale
const minimumNetworkTokenForSwap = 1;

// networks -> check rpcurl.dart for more info
const tokenContractNetwork = 'Smart Chain';
const tokenSaleContractNetwork = "Smart Chain";
const tokenStakingContractNetwork = 'Polygon (Mumbai)';

// dapp links
const blogUrl = 'https://opensea.io/blog/';
const vrUrl = 'https://opensea.io/category/virtual-worlds';
const marketPlaceUrl = 'https://opensea.com';
const ecommerceUrl = 'https://opensea.com';

// social media links
const telegramLink = 'https://t.me/hodlverseofficial';
const twitterLink = 'https://twitter.com/hodlverseapp';
const mediumLink = 'https://medium.com/@hodlverse.app';
// const linkedInLink = 'https://www.linkedin.com/in/';
// const redditLink = 'https://reddit.com/r/';
// const discordLink = 'https://discord.gg/';
// const facebookLink = 'https://facebook.com/';
// const instagramLink = 'https://instagram.com/';

// color
const settingIconColor = Color(0xff3E7BEC);
const dividerColor = Color(0xffE6E6E3);
const appPrimaryColor = Color(0xff2469E9);
const red = Color(0xffeb6a61);
const green = Color(0xff01aa78);
const grey = Colors.grey;
const colorForAddress = Color(0xffEBF3FF);
const appBackgroundblue = Color(0xff0C66F1);
const primaryMaterialColor = MaterialColor(
  0xff2469E9,
  <int, Color>{
    50: appPrimaryColor,
    100: appPrimaryColor,
    200: appPrimaryColor,
    300: appPrimaryColor,
    400: appPrimaryColor,
    500: appPrimaryColor,
    600: appPrimaryColor,
    700: appPrimaryColor,
    800: appPrimaryColor,
    900: appPrimaryColor,
  },
);

// security
const secureStorageKey = 'box28aldk3qka';

const infuraApiKey = '53163c736f1d4ba78f0a39ffda8d87b4';

// settings key...not to be edited
const userUnlockPasscodeKey = 'userUnlockPasscode';
const darkModekey = 'userTheme';
const hideBalanceKey = 'hideUserBalance';
const dappChainIdKey = 'dappBrowserChainIdKey';
const eIP681ProcessingErrorMsg =
    'Ethereum request format not supported or Network Time Out';
const personalSignKey = 'Personal';
const normalSignKey = 'Normal Sign';
const typedMessageSignKey = "Typed Message";
const userSignInDataKey = 'user-sign-in-data';
const mnemonicListKey = 'mnemonics_List';
const currentMmenomicKey = 'mmemomic_mnemonic';
const currentUserWalletNameKey = 'current__walletNameKey';
const coinGeckoCryptoPriceKey = 'cryptoPricesKey';
const bookMarkKey = 'bookMarks';
const historyKey = 'broswer_kehsi_history';
const coinMarketCapApiKey = 'a4e88d80-acc3-4bb5-a8a6-151a2ddb5f32';
const appUnlockTime = 'applockksksietimeal382';

// template tags
const transactionhashTemplateKey = '{{TransactionHash}}';

// enable
const enableTestNet = true;
const usesCentralizedAuth = false;

// app theme
final darkTheme = ThemeData(
  dialogBackgroundColor: const Color(0xff171840),
  fontFamily: 'Roboto',
  primaryColor: const Color(0xff1F2051),
  bottomNavigationBarTheme: const BottomNavigationBarThemeData(
    unselectedItemColor: Colors.grey,
    backgroundColor: Color(0xff1F2051),
    selectedItemColor: Colors.white,
  ),
  backgroundColor: const Color(0xff171840),
  scaffoldBackgroundColor: const Color(0xff171840),
  cardColor: const Color(0xff1F2051),
  dividerColor: Colors.black54,
  bottomSheetTheme: const BottomSheetThemeData(
    backgroundColor: Color(0xff171840),
  ),
  colorScheme: ColorScheme.fromSwatch(primarySwatch: Colors.grey).copyWith(
    secondary: Colors.white,
    brightness: Brightness.dark,
    surface: const Color(0xff1F2051),
    onSurface: Colors.white,
  ),
);

final lightTheme = ThemeData(
  fontFamily: 'Roboto',
  primaryColor: Colors.white,
  backgroundColor: const Color(0xFFE5E5E5),
  bottomNavigationBarTheme: const BottomNavigationBarThemeData(
    type: BottomNavigationBarType.fixed,
    backgroundColor: Color(0xffEBF3FF),
    selectedItemColor: Colors.black,
    unselectedItemColor: Colors.grey,
    selectedIconTheme:
        IconThemeData(color: Colors.blue, opacity: 1.0, size: 30.0),
  ),
  checkboxTheme: CheckboxThemeData(
    fillColor: MaterialStateProperty.all(appPrimaryColor),
    checkColor: MaterialStateProperty.all(appPrimaryColor),
    overlayColor: MaterialStateProperty.all(appPrimaryColor),
  ),
  dividerColor: dividerColor,
  colorScheme:
      ColorScheme.fromSwatch(primarySwatch: primaryMaterialColor).copyWith(
    secondary: Colors.black,
    brightness: Brightness.light,
  ),
);

// preferences keys and app data
const userPinTrials = 3;
const pinLength = 6;
const maximumTransactionToSave = 30;
const maximumBrowserHistoryToSave = 20;
