import 'dart:convert';
import 'package:http/http.dart' as http;

class HttpHelper {
  static Future<dynamic> performPostRequest(server, command, payload,
      {Map<String, String?>? headers}) async {
    var response = await http.post(
      command.isEmpty ? Uri.parse(server) : Uri.parse('$server/$command'),
      headers: {
        'content-type': 'application/json',
      },
      encoding: Encoding.getByName('utf8'),
      body: utf8.encode(json.encode(payload)),
    );
    return response.body != '' || response.body.length > 0
        ? response.body
        : null;
  }

  static Future<dynamic> performGetRequest(server, command) async {
    var response = await http.get(Uri.parse('$server/$command'));
    if (response.body != '' || response.body.length > 0)
      return jsonDecode(response.body);
    return null;
  }
}
