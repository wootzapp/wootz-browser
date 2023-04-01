import 'package:dartez/helper/http_helper.dart';

class ConseilDataClient {
  static executeEntityQuery(
      serverInfo, platform, network, entity, query) async {
    var url = '${serverInfo['url']}/v2/data/$platform/$network/$entity';

    var res = await HttpHelper.performPostRequest(url, '', query, headers: {
      'apiKey': serverInfo['apiKey'],
      'cache': 'no-store',
    });
    return res;
  }
}
