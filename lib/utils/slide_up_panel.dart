import 'package:flutter/material.dart';

Future slideUpPanel(context, screen, {bool canDismiss = true}) async {
  return showModalBottomSheet(
      shape: const RoundedRectangleBorder(
        borderRadius: BorderRadius.vertical(
          top: Radius.circular(20),
        ),
      ),
      context: context,
      useRootNavigator: true,
      isScrollControlled: true,
      isDismissible: canDismiss,
      enableDrag: canDismiss,
      builder: (context) {
        return Padding(
          padding: MediaQuery.of(context).viewInsets,
          child: ScrollConfiguration(
            behavior: RemoveScrollGlow(),
            child: SingleChildScrollView(
              child: Container(
                child: screen,
                constraints: BoxConstraints(
                  maxHeight: MediaQuery.of(context).size.height * 0.70,
                ),
              ),
            ),
          ),
        );
      });
}

class RemoveScrollGlow extends ScrollBehavior {
  @override
  Widget buildViewportChrome(
      BuildContext context, Widget child, AxisDirection axisDirection) {
    return child;
  }
}
