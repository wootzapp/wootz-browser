import 'dart:async';
import 'dart:io';

import 'package:cryptowallet/screens/dapp.dart';
import 'package:cryptowallet/screens/open_app_pin_failed.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:device_info_plus/device_info_plus.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:get/get_state_manager/get_state_manager.dart';
import 'package:get/route_manager.dart';
import 'package:get/state_manager.dart';
import 'firebase_options.dart';
import 'package:firebase_crashlytics/firebase_crashlytics.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:hive_flutter/hive_flutter.dart';
import 'package:page_transition/page_transition.dart';

import 'package:flutter_gen/gen_l10n/app_localization.dart';

void main() async {
  Paint.enableDithering = true;
  WidgetsFlutterBinding.ensureInitialized();
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

  runApp(
    RestartWidget(
      child: MyApp(
        userDarkMode: pref.get(darkModekey, defaultValue: false),
        locale: Locale.fromSubtags(
          languageCode: pref.get(languageKey, defaultValue: 'en'),
        ),
      ),
    ),
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

  void restartApp() {
    setState(() {
      key = UniqueKey();
    });
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
  final Locale locale;

  const MyApp({Key key, this.userDarkMode, this.locale}) : super(key: key);
  static _MyAppState of(BuildContext context) =>
      context.findAncestorStateOfType<_MyAppState>();

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  Rx<Locale> _locale;

  @override
  initState() {
    super.initState();
    _locale = widget.locale.obs;
  }

  void setLocale(Locale value) {
    _locale.value = value;
  }

  @override
  Widget build(BuildContext context) {
    MyApp.themeNotifier.value =
        widget.userDarkMode ? ThemeMode.dark : ThemeMode.light;

    return ValueListenableBuilder(
        valueListenable: MyApp.themeNotifier,
        builder: (_, ThemeMode currentMode, __) {
          return Obx(
            () => GetMaterialApp(
              locale: _locale.value,
              debugShowCheckedModeBanner: false,
              theme: lightTheme,
              localizationsDelegates: AppLocalizations.localizationsDelegates,
              supportedLocales: AppLocalizations.supportedLocales,
              darkTheme: darkTheme,
              themeMode: currentMode,
              home: const MyHomePage(),
            ),
          );
        });
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
        Get.off(
          nextWidget,
          transition: Transition.leftToRight,
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
    } else if (hasWallet) {
      nextWidget = await dappWidget(
        context,
        walletURL,
      );
    } else {
      nextWidget = const Dapp(
        provider: '',
        init: '',
        data: walletURL,
      );
    }
    await Future.delayed(const Duration(milliseconds: 2500));

    Get.off(
      nextWidget,
      transition: Transition.leftToRight,
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
