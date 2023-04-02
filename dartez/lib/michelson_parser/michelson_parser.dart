import 'package:dartez/michelson_parser/parser/micheline_grammar.dart';
import 'package:dartez/michelson_parser/parser/michelson_grammar.dart';
import 'package:dartez/michelson_parser/parser/nearley.dart';

class MichelsonParser {
  static String? parseMichelson(String code) {
    var parser = Nearley();
    parser.parser(Nearley.fromCompiled(MichelsonGrammar().grammar));
    var cleanCode = preProcessMichelsonScript(code);
    cleanCode.map((_code) => parser.feed(_code)).toList();
    return parser.results[0];
  }

  static translateMichelineToHex(code) {
    var parser = Nearley();
    parser.parser(Nearley.fromCompiled(MichelineGrammar().grammar));
    parser.feed(normalizeMichelineWhiteSpace(code));
    return parser.results.join('');
  }

  static String normalizeMichelineWhiteSpace(String fragment) {
    return fragment
        .replaceAll(new RegExp(r'\n/g'), ' ')
        .replaceAll(new RegExp(r' +'), ' ')
        .replaceAll(new RegExp(r'\[{'), '[ {')
        .replaceAll(new RegExp(r'}\]'), '} ]')
        .replaceAll(new RegExp(r'},{'), '}, {')
        .replaceAll(new RegExp(r'\]}'), '] }')
        .replaceAll(new RegExp(r'":"'), '": "')
        .replaceAll(new RegExp(r'":\['), '": [')
        .replaceAll(new RegExp(r'{"'), '{ "')
        .replaceAll(new RegExp(r'"}'), '" }')
        .replaceAll(new RegExp(r',"'), ', "')
        .replaceAll(new RegExp(r'","'), '", "')
        .replaceAll(new RegExp(r'\[\['), '[ [')
        .replaceAll(new RegExp(r'\]\]'), '] ]')
        .replaceAll(new RegExp(r'\["'), '[ "')
        .replaceAll(new RegExp(r'"\]'), '" ]')
        .replaceAll(new RegExp(r'\[ +\]'), '\[\]')
        .trim();
  }

  static List preProcessMichelsonScript(String code) {
    var sections = Map();
    sections['parameter'] = code.indexOf(RegExp(r'(^|\s+)parameter'));
    sections['storage'] = code.indexOf(RegExp(r'(^|\s+)storage'));
    sections['code'] = code.indexOf(RegExp(r'(^|\s+)code'));
    var boundaries = sections.values.toList();
    boundaries.sort((a, b) => a - b);
    sections[
            sections.keys.firstWhere((key) => sections[key] == boundaries[0])] =
        code.substring(boundaries[0] < 0 ? 0 : boundaries[0],
            boundaries[1] < 0 ? 0 : boundaries[1]);
    sections[
            sections.keys.firstWhere((key) => sections[key] == boundaries[1])] =
        code.substring(boundaries[1] < 0 ? 0 : boundaries[1],
            boundaries[2] < 0 ? 0 : boundaries[2]);
    sections[
            sections.keys.firstWhere((key) => sections[key] == boundaries[2])] =
        code.substring(boundaries[2] < 0 ? 0 : boundaries[2]);
    var parts = [sections['parameter'], sections['storage'], sections['code']];
    return parts
        .map((p) => p
            .trim()
            .split('\n')
            .map(
              (l) => l.replaceAll(RegExp(r'\#[\s\S]+$'), '').trim(),
            )
            .toList()
            .where((e) => e.toString().isNotEmpty)
            .toList()
            .join(' '))
        .toList();
  }
}
