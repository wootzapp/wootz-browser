import 'package:dartez/michelson_parser/michelson_parser.dart';

class TezosLanguageUtil {
  static var primitiveRecordOrder = ["prim", "args", "annots"];

  static String? translateMichelsonToMicheline(String code) {
    var result = MichelsonParser.parseMichelson(code);
    return result;
  }

  static String? translateMichelineToHex(p) {
    return MichelsonParser.translateMichelineToHex(p);
  }

  static dynamic normalizePrimitiveRecordOrder(data) {
    if (data is List) return data.map(normalizePrimitiveRecordOrder).toList();

    if (data is Map) {
      dynamic keys = data.keys.toList();
      keys.sort((k1, k2) =>
          TezosLanguageUtil.primitiveRecordOrder.indexOf(k1) -
          TezosLanguageUtil.primitiveRecordOrder.indexOf(k2));

      data = keys.fold({}, (obj, value) {
        return {
          ...obj,
          value: normalizePrimitiveRecordOrder(
              data[value] is int ? data[value].toString() : data[value])
        };
      });
    }
    return data;
  }
}
