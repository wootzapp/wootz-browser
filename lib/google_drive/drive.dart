import 'dart:io';

import 'package:cryptowallet/utils/app_config.dart';
import 'package:googleapis/drive/v3.dart' as ga;
import 'package:googleapis_auth/auth_io.dart';
import 'package:hive/hive.dart';
import 'package:http/http.dart' as http;
import 'package:url_launcher/url_launcher.dart';
import 'package:flutter_dotenv/flutter_dotenv.dart';

const _scopes = [ga.DriveApi.DriveFileScope];

class GoogleDrive {
  final box = Hive.box(secureStorageKey);
  Future saveCredentials(
    AccessToken token,
    String refreshToken,
  ) async {
    await box.put(
      googleDriveKey,
      {
        'type': token.type,
        'data': token.data,
        'expiry': token.expiry.toIso8601String(),
        'refreshToken': refreshToken,
      },
    );
  }

  void clearCredentials() {
    box.delete(googleDriveKey);
  }

  Map getCredentials() {
    return box.get(googleDriveKey);
  }

  // Get authenticated http client
  Future<http.Client> getHttpClient() async {
    var credentials = await box.get(googleDriveKey);
    await dotenv.load(fileName: "assets/.env", mergeWith: {});
    if (credentials == null) {
      var authClient = await clientViaUserConsent(
        ClientId(
          dotenv.env['CLIENT_ID'],
          dotenv.env['CLIENT_SECRET'],
        ),
        _scopes,
        (url) async {
          await launchUrl(Uri.parse(url));
        },
      );

      return authClient;
    } else {
      return authenticatedClient(
        http.Client(),
        AccessCredentials(
          AccessToken(
            credentials['type'],
            credentials['data'],
            DateTime.parse(
              credentials['expiry'],
            ),
          ),
          credentials['refreshToken'],
          _scopes,
        ),
      );
    }
  }

  Future upload(
    File file,
    String fileName,
  ) async {
    var client = await getHttpClient();
    var drive = ga.DriveApi(client);

    await drive.files.create(
      ga.File()..name = fileName,
      uploadMedia: ga.Media(
        file.openRead(),
        file.lengthSync(),
      ),
    );
  }
}
