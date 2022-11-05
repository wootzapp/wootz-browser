import 'dart:io';

import 'package:cryptowallet/utils/language_locale.dart';
import 'package:flutter/material.dart';
import 'package:flutter/src/foundation/key.dart';
import 'package:flutter/src/widgets/container.dart';
import 'package:flutter/src/widgets/framework.dart';
import 'package:flutter_gen/gen_l10n/app_localization.dart';

class Language extends StatefulWidget {
  const Language({Key key}) : super(key: key);

  @override
  State<Language> createState() => _LanguageState();
}

class _LanguageState extends State<Language> {
  List languages;
  String languageCode;
  @override
  initState() {
    super.initState();
    languageCode = Platform.localeName.split('_')[0];
    languages = context
        .findAncestorWidgetOfExactType<MaterialApp>()
        ?.supportedLocales
        ?.toList();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(
          AppLocalizations.of(context).language,
        ),
      ),
      body: SafeArea(
        child: RefreshIndicator(
          onRefresh: () async {
            await Future.delayed(const Duration(seconds: 2));
            setState(() {});
          },
          child: SingleChildScrollView(
            physics: const AlwaysScrollableScrollPhysics(),
            child: Padding(
              padding: const EdgeInsets.all(25.0),
              child: Column(
                mainAxisAlignment: MainAxisAlignment.start,
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  if (languages != null)
                    for (Locale locale in languages) ...[
                      InkWell(
                        onTap: () async {
                          try {} catch (e) {
                            ScaffoldMessenger.of(context).showSnackBar(
                              const SnackBar(
                                backgroundColor: Colors.red,
                                content: Text(
                                  'Could not change currency, please try again later.',
                                  style: TextStyle(color: Colors.white),
                                ),
                                duration: Duration(seconds: 2),
                              ),
                            );
                          }
                        },
                        child: Row(
                          mainAxisAlignment: MainAxisAlignment.spaceBetween,
                          children: [
                            Flexible(
                              child: Row(
                                children: [
                                  Flexible(
                                    child: Column(
                                      crossAxisAlignment:
                                          CrossAxisAlignment.start,
                                      children: [
                                        Text(
                                          LanguageLocal.getDisplayLanguage(
                                            locale.languageCode,
                                          )['name'],
                                          style: const TextStyle(
                                            fontSize: 16,
                                            fontWeight: FontWeight.w500,
                                          ),
                                          overflow: TextOverflow.fade,
                                        ),
                                        const SizedBox(
                                          height: 10,
                                        ),
                                        Row(
                                          children: [
                                            Flexible(
                                              child: Text(
                                                locale.languageCode,
                                                style: const TextStyle(
                                                  fontSize: 15,
                                                  color: Colors.grey,
                                                ),
                                              ),
                                            ),
                                          ],
                                        ),
                                      ],
                                    ),
                                  )
                                ],
                              ),
                            ),
                            if (locale.languageCode == languageCode)
                              Container(
                                decoration: const BoxDecoration(
                                    shape: BoxShape.circle, color: Colors.blue),
                                child: const Padding(
                                  padding: EdgeInsets.all(2),
                                  child: Icon(
                                    Icons.check,
                                    size: 20,
                                    color: Colors.white,
                                  ),
                                ),
                              )
                          ],
                        ),
                      ),
                      const Divider()
                    ]
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}
