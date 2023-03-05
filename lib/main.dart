import 'package:cryptowallet/screens/open_app_pin_failed.dart';
import 'package:cryptowallet/utils/app_config.dart';
import 'package:cryptowallet/utils/navigator_service.dart';
import 'package:cryptowallet/utils/rpc_urls.dart';
import 'package:cryptowallet/utils/wc_connector.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:cryptowallet/models/browser_model.dart';
import 'package:cryptowallet/models/webview_model.dart';
import 'package:flutter_downloader/flutter_downloader.dart';
import 'package:flutter_phoenix/flutter_phoenix.dart';
import 'package:hive/hive.dart';
import 'package:multi_value_listenable_builder/multi_value_listenable_builder.dart';
import 'package:path_provider/path_provider.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:provider/provider.dart';

import 'package:flutter_gen/gen_l10n/app_localization.dart';

import 'browser.dart';
import 'models/provider.dart';

// ignore: non_constant_identifier_names
late final String WEB_ARCHIVE_DIR;
// ignore: non_constant_identifier_names
late final double TAB_VIEWER_BOTTOM_OFFSET_1;
// ignore: non_constant_identifier_names
late final double TAB_VIEWER_BOTTOM_OFFSET_2;
// ignore: non_constant_identifier_names
late final double TAB_VIEWER_BOTTOM_OFFSET_3;
// ignore: constant_identifier_names
const double TAB_VIEWER_TOP_OFFSET_1 = 0.0;
// ignore: constant_identifier_names
const double TAB_VIEWER_TOP_OFFSET_2 = 10.0;
// ignore: constant_identifier_names
const double TAB_VIEWER_TOP_OFFSET_3 = 20.0;
// ignore: constant_identifier_names
const double TAB_VIEWER_TOP_SCALE_TOP_OFFSET = 250.0;
// ignore: constant_identifier_names
const double TAB_VIEWER_TOP_SCALE_BOTTOM_OFFSET = 230.0;

void main() async {
  WidgetsFlutterBinding.ensureInitialized();

  WEB_ARCHIVE_DIR = (await getApplicationSupportDirectory()).path;

  TAB_VIEWER_BOTTOM_OFFSET_1 = 130.0;
  TAB_VIEWER_BOTTOM_OFFSET_2 = 140.0;
  TAB_VIEWER_BOTTOM_OFFSET_3 = 150.0;

  await FlutterDownloader.initialize(debug: kDebugMode);

  await Permission.camera.request();
  await Permission.microphone.request();
  await Permission.storage.request();

  runApp(
    MultiProvider(
      providers: [
        ChangeNotifierProvider(
          create: (context) => WebViewModel(),
        ),
        ChangeNotifierProxyProvider<WebViewModel, BrowserModel>(
          update: (context, webViewModel, browserModel) {
            browserModel!.setCurrentWebViewModel(webViewModel);
            return browserModel;
          },
          create: (BuildContext context) => BrowserModel(),
        ),
      ],
      child: const FlutterBrowserApp(),
    ),
  );
}

class MyApp extends StatefulWidget {
  static ValueNotifier<ThemeMode> themeNotifier = ValueNotifier(ThemeMode.dark);

  static bool getCoinGeckoData = true;
  static DateTime lastcoinGeckoData = DateTime.now();

  final bool? userDarkMode;
  // final ValueNotifier<Locale> locale;
  final Locale? locale;

  const MyApp({Key? key, this.userDarkMode, this.locale}) : super(key: key);
  static _MyAppState? of(BuildContext context) =>
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
    _locale.value = widget.locale!;
  }

  void setLocale(Locale locale) {
    _locale.value = locale;
  }

  @override
  Widget build(BuildContext context) {
    MyApp.themeNotifier.value =
        widget.userDarkMode! ? ThemeMode.dark : ThemeMode.light;
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

class RestartWidget extends StatefulWidget {
  const RestartWidget({Key? key, required this.child}) : super(key: key);

  final Widget child;

  static void restartApp(BuildContext context) {
    context.findAncestorStateOfType<_RestartWidgetState>()!.restartApp();
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

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key? key}) : super(key: key);

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

class FlutterBrowserApp extends StatelessWidget {
  const FlutterBrowserApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Browser',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        visualDensity: VisualDensity.adaptivePlatformDensity,
      ),
      initialRoute: '/',
      routes: {
        '/': (context) => const Browser(),
      },
    );
  }
}
