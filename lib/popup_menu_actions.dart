import 'package:cryptowallet/utils/app_config.dart';
import 'package:hive/hive.dart';

class PopupMenuActions {
  // ignore: constant_identifier_names
  static const String NEW_TAB = "New tab";
  // ignore: constant_identifier_names
  static const String NEW_INCOGNITO_TAB = "New incognito tab";
  // ignore: constant_identifier_names
  static const String FAVORITES = "Favorites";
  // ignore: constant_identifier_names
  static const String HISTORY = "History";
  // ignore: constant_identifier_names
  static const String WEB_ARCHIVES = "Web Archives";
  // ignore: constant_identifier_names
  static const String SHARE = "Share";
  // ignore: constant_identifier_names
  static const String FIND_ON_PAGE = "Find on page";
  // ignore: constant_identifier_names
  static const String DESKTOP_MODE = "Desktop mode";
  // ignore: constant_identifier_names
  static const String SETTINGS = "Settings";
  // ignore: constant_identifier_names
  static const String DEVELOPERS = "Developers";
  // ignore: constant_identifier_names
  static const String INAPPWEBVIEW_PROJECT = "InAppWebView Project";
  // ignore: constant_identifier_names
  static const String WALLET = "Wallet";
  // ignore: constant_identifier_names
  static const String QR_SCAN = "Wallet Connect";
  // ignore: constant_identifier_names
  static const String ADD_WALLET = "Add Wallet";
  // ignore: constant_identifier_names
  static const String VIEW_WALLETS = "View Wallets";
  static Box pref = Hive.box(secureStorageKey);
  static List<String> choices = <String>[
    NEW_TAB,
    NEW_INCOGNITO_TAB,
    FAVORITES,
    HISTORY,
    WEB_ARCHIVES,
    SHARE,
    FIND_ON_PAGE,
    DESKTOP_MODE,
    SETTINGS,
    DEVELOPERS,
    INAPPWEBVIEW_PROJECT,
    WALLET,
    QR_SCAN,
    if (pref.get(currentMmenomicKey) != null) ...[
      ADD_WALLET,
      VIEW_WALLETS,
    ],
  ];
}
