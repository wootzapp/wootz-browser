import 'dart:io';
import 'package:flutter/foundation.dart';
import 'package:path_provider/path_provider.dart';

class FileReader {
  static Future<String> get _localPath async {
    final directory = await getApplicationDocumentsDirectory();

    return directory.path;
  }

  static Future<File> localFile(String fileName) async {
    final path = await _localPath;
    return File('$path/$fileName');
  }

  static Future<File> downloadFile(String fileName) async {
    final downloadDir = await getDownloadPath();
    return File('$downloadDir/$fileName');
  }

  static Future<String> readFile(String fileName) async {
    try {
      final file = await localFile(fileName);
      final contents = await file.readAsString();

      return contents;
    } catch (e) {
      return "";
    }
  }

  static Future<File> writeFile(String fileName, String content) async {
    final file = await localFile(fileName);

    return file.writeAsString(content);
  }

  static Future<File> writeDownloadFile(String fileName, String content) async {
    final file = await downloadFile(fileName);
    if (!await file.exists()) {
      await file.create(recursive: true);
    }
    return await file.writeAsString(content);
  }

  static Future<String> getDownloadPath() async {
    Directory directory;
    try {
      if (Platform.isIOS) {
        directory = await getApplicationDocumentsDirectory();
      } else {
        directory = Directory('/storage/emulated/0/Download');

        if (!await directory.exists()) {
          directory = await getExternalStorageDirectory();
        }
      }
    } catch (err, _) {
      if (kDebugMode) {
        print("Cannot get download folder path");
      }
    }
    return directory?.path;
  }
}
