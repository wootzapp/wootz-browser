import 'dart:async';
import 'dart:io';

import 'package:cryptowallet/screens/dapp.dart';
import 'package:cryptowallet/screens/open_app_pin_failed.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/navigator_service.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:cryptowallet/utils/wallet_black.dart';
import 'package:cryptowallet/utils/web_notifications.dart';
import 'package:device_info_plus/device_info_plus.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:flutter_downloader/flutter_downloader.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
import 'package:flutter_phoenix/flutter_phoenix.dart';
import 'package:multi_value_listenable_builder/multi_value_listenable_builder.dart';
// import 'package:get/get.dart';
import 'package:provider/provider.dart';
import 'firebase_options.dart';
import 'package:bip32/bip32.dart';
import 'package:firebase_crashlytics/firebase_crashlytics.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:hive_flutter/hive_flutter.dart';
import 'package:page_transition/page_transition.dart';

import 'package:flutter_gen/gen_l10n/app_localization.dart';

import 'model/provider.dart';
import 'utils/wc_connector.dart';

void main() async {
  Paint.enableDithering = true;
  WidgetsFlutterBinding.ensureInitialized();
  if (!kIsWeb && defaultTargetPlatform == TargetPlatform.android) {
    await InAppWebViewController.setWebContentsDebuggingEnabled(true);
  }
  await FlutterDownloader.initialize();
  await Hive.initFlutter();

  if (Platform.isIOS) {
    await Firebase.initializeApp(
      options: const FirebaseOptions(
        apiKey: 'AIzaSyCgZEhbaF-KLBXW4J00GXg4Xmav8fS_EfU',
        appId: '1:753261675970:ios:00cd66a3716e9be4c4825b',
        messagingSenderId: '753261675970',
        projectId: 'browser-252c6',
      ),
    );
  } else {
    await Firebase.initializeApp();
  }

  FocusManager.instance.primaryFocus?.unfocus();
  // make app always in portrait mode
  SystemChrome.setPreferredOrientations([
    DeviceOrientation.portraitUp,
    DeviceOrientation.portraitDown,
  ]);
  // change error widget
  ErrorWidget.builder = (FlutterErrorDetails details) {
    FirebaseCrashlytics.instance.recordFlutterFatalError(details);
    if (kReleaseMode) {
      return Container();
    }
    return Container(
      color: Colors.red,
      child: Center(
        child: Text(
          details.exceptionAsString(),
          style: const TextStyle(color: Colors.white),
        ),
      ),
    );
  };

  FlutterError.onError = (errorDetails) async {
    await FirebaseCrashlytics.instance.recordFlutterFatalError(errorDetails);
  };
  PlatformDispatcher.instance.onError = (error, stack) {
    FirebaseCrashlytics.instance.recordError(error, stack, fatal: true);
    return true;
  };
  final pref = await Hive.openBox(secureStorageKey);
  await WebNotificationPermissionDb.loadSavedPermissions();
  runApp(
    Phoenix(
        child: RestartWidget(
      child: MyApp(
        userDarkMode: pref.get(darkModekey, defaultValue: false),
        locale: Locale.fromSubtags(
          languageCode: pref.get(languageKey, defaultValue: 'en'),
        ),
      ),
    )),
  );
}

class RestartWidget extends StatefulWidget {
  const RestartWidget({Key key, this.child}) : super(key: key);

  final Widget child;

  static void restartApp(BuildContext context) {
    context.findAncestorStateOfType<_RestartWidgetState>().restartApp();
  }

  @override
  _RestartWidgetState createState() => _RestartWidgetState();
}

class _RestartWidgetState extends State<RestartWidget> {
  Key key = UniqueKey();

  void restartApp() async {
    // await Get.deleteAll(force: true); This will remove all the Initialized controllers instances from your memory.
    Phoenix.rebirth(context);
    // Get.reset();
  }

  @override
  Widget build(BuildContext context) {
    return KeyedSubtree(
      key: key,
      child: widget.child,
    );
  }
}

class MyApp extends StatefulWidget {
  static ValueNotifier<ThemeMode> themeNotifier = ValueNotifier(ThemeMode.dark);

  static bool getCoinGeckoData = true;
  static DateTime lastcoinGeckoData = DateTime.now();

  final bool userDarkMode;
  // final ValueNotifier<Locale> locale;
  final Locale locale;

  const MyApp({Key key, this.userDarkMode, this.locale}) : super(key: key);
  static _MyAppState of(BuildContext context) =>
      context.findAncestorStateOfType<_MyAppState>();

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final _locale = ValueNotifier<Locale>(const Locale("en", "US"));
  // Locale _locale;

  @override
  initState() {
    super.initState();
    _locale.value = widget.locale;
  }

  void setLocale(Locale locale) {
    _locale.value = locale;
  }

  @override
  Widget build(BuildContext context) {
    MyApp.themeNotifier.value =
        widget.userDarkMode ? ThemeMode.dark : ThemeMode.light;
    return ChangeNotifierProvider.value(
      value: _locale,
      child: MultiValueListenableBuilder(
          valueListenables: [MyApp.themeNotifier, _locale],
          builder: (context, value, child) {
            return ChangeNotifierProvider(
              create: (context) => ProviderClass(),
              child: MaterialApp(
                navigatorKey: NavigationService.navigatorKey, // set property
                locale: _locale.value,
                debugShowCheckedModeBanner: false,
                theme: lightTheme,
                localizationsDelegates: AppLocalizations.localizationsDelegates,
                supportedLocales: AppLocalizations.supportedLocales,
                darkTheme: darkTheme,
                themeMode: MyApp.themeNotifier.value,
                home: const MyHomePage(),
              ),
            );
          }),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key key}) : super(key: key);

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  @override
  void initState() {
    super.initState();
    checkNavigation();
    WcConnector();
  }

  Future<void> checkNavigation() async {
    final pref = Hive.box(secureStorageKey);
    bool hasWallet = pref.get(currentMmenomicKey) != null;
    int hasUnlockTime = pref.get(appUnlockTime, defaultValue: 1);

    bool defaultSecurity = true;

    Widget nextWidget;

    if (hasUnlockTime > 1) {
      nextWidget = OpenAppPinFailed(remainSec: hasUnlockTime);
      WidgetsBinding.instance.addPostFrameCallback((_) {
        Navigator.of(context).pushReplacement(
          MaterialPageRoute(builder: (_) => nextWidget),
        );
      });
      return;
    }

    if (hasWallet) {
      defaultSecurity = await authenticate(
        context,
        disableGoBack_: true,
      );
    }

    if (hasWallet && !defaultSecurity) {
      nextWidget = const OpenAppPinFailed();
    } else {
      nextWidget = await dappWidget(
        context,
        walletURL,
      );
    }
    await Future.delayed(const Duration(milliseconds: 2500));

    Navigator.of(context).pushReplacement(
      MaterialPageRoute(builder: (_) => nextWidget),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: SizedBox(
        width: double.infinity,
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            Image.asset(
              'assets/logo.png',
              width: 100,
            ),
            const Text(
              walletName,
              style: TextStyle(
                fontSize: 30,
                fontWeight: FontWeight.bold,
                letterSpacing: .3,
              ),
            ),
          ],
        ),
      ),
    );
  }
}
