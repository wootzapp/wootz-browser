import 'package:flutter/material.dart';
import 'package:flutter/src/foundation/key.dart';
import 'package:flutter/src/widgets/container.dart';
import 'package:flutter/src/widgets/framework.dart';
import 'package:ntcdcrypto/ntcdcrypto.dart';

class ConvertToShemirSecret extends StatefulWidget {
  final String secret;
  const ConvertToShemirSecret({Key key, this.secret}) : super(key: key);

  @override
  State<ConvertToShemirSecret> createState() => _ConvertToShemirSecretState();
}

class _ConvertToShemirSecretState extends State<ConvertToShemirSecret> {
  SSS sss = SSS();
  @override
  Widget build(BuildContext context) {
    print(sss.create(4, 5, widget.secret, false));
    return Scaffold();
  }
}
