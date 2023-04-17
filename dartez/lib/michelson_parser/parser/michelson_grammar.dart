import 'package:dartez/michelson_parser/grammar/michelson_grammar_tokenizer.dart';

class MichelsonGrammar {
  MichelsonGrammarTokenizer? lexer;

  var macroCADRconst = 'C[AD]+R';
  var macroSETCADRconst = 'SET_C[AD]+R';
  var dIPmatcher = new RegExp('DII+P');
  var dUPmatcher = new RegExp('DUU+P');
  var macroASSERTlistConst = [
    'ASSERT',
    'ASSERT_EQ',
    'ASSERT_NEQ',
    'ASSERT_GT',
    'ASSERT_LT',
    'ASSERT_GE',
    'ASSERT_LE',
    'ASSERT_NONE',
    'ASSERT_SOME',
    'ASSERT_LEFT',
    'ASSERT_RIGHT',
    'ASSERT_CMPEQ',
    'ASSERT_CMPNEQ',
    'ASSERT_CMPGT',
    'ASSERT_CMPLT',
    'ASSERT_CMPGE',
    'ASSERT_CMPLE'
  ];
  var macroIFCMPlist = [
    'IFCMPEQ',
    'IFCMPNEQ',
    'IFCMPLT',
    'IFCMPGT',
    'IFCMPLE',
    'IFCMPGE'
  ];
  var macroCMPlist = ['CMPEQ', 'CMPNEQ', 'CMPLT', 'CMPGT', 'CMPLE', 'CMPGE'];
  var macroIFlist = ['IFEQ', 'IFNEQ', 'IFLT', 'IFGT', 'IFLE', 'IFGE'];

  var parameter;
  var storage;
  var code;
  var comparableType;
  var constantType;
  var singleArgType;
  var lparen;
  var rparen;
  var doubleArgType;
  var annot;
  var number;
  var string;
  var lbrace;
  var rbrace;
  var constantData;
  var singleArgData;
  var doubleArgData;
  var bytes;
  var elt;
  var semicolon;
  var baseInstruction;
  var macroCADR;
  var macroDIP;
  var macroDUP;
  var macroSETCADR;
  var macroASSERTlist;

  MichelsonGrammar() {
    lexer = new MichelsonGrammarTokenizer({
      'annot': RegExp(r'[\@\%\:][a-z_A-Z0-9]+'),
      'lparen': '(',
      'rparen': ')',
      'lbrace': '{',
      'rbrace': '}',
      'ws': RegExp(r'[ \t]+'),
      'semicolon': ";",
      'bytes': RegExp(r'0x[0-9a-fA-F]+'),
      'number': RegExp('-?[0-9]+(?!x)'),
      'parameter': ['parameter', 'Parameter'],
      'storage': ['Storage', 'storage'],
      'code': ['Code', 'code'],
      'comparableType': [
        'int',
        'nat',
        'string',
        'bytes',
        'mutez',
        'bool',
        'key_hash',
        'timestamp',
        'chain_id'
      ],
      'constantType': ['key', 'unit', 'signature', 'operation', 'address'],
      'singleArgType': ['option', 'list', 'set', 'contract'],
      'doubleArgType': ['pair', 'or', 'lambda', 'map', 'big_map'],
      'baseInstruction': [
        'ABS', 'ADD', 'ADDRESS', 'AMOUNT', 'AND', 'BALANCE', 'BLAKE2B', 'CAR',
        'CAST', 'CDR', 'CHECK_SIGNATURE',
        'COMPARE', 'CONCAT', 'CONS', 'CONTRACT', 'CREATE_CONTRACT', 'DIP',
        /*'DROP',*/ /*'DUP',*/ 'EDIV',
        /*'EMPTY_MAP',*/
        'EMPTY_SET', 'EQ', 'EXEC', 'FAIL', 'FAILWITH', 'GE', 'GET', 'GT',
        'HASH_KEY', 'IF', 'IF_CONS', 'IF_LEFT', 'IF_NONE',
        'IF_RIGHT', 'IMPLICIT_ACCOUNT', 'INT', 'ISNAT', 'ITER', 'LAMBDA', 'LE',
        'LEFT', 'LOOP', 'LOOP_LEFT', 'LSL', 'LSR', 'LT',
        'MAP', 'MEM', 'MUL', 'NEG', 'NEQ', 'NIL', 'NONE', 'NOT', 'NOW', 'OR',
        'PACK', 'PAIR', /*'PUSH',*/ 'REDUCE', 'RENAME', 'RIGHT', 'SELF',
        'SENDER', 'SET_DELEGATE', 'SHA256', 'SHA512', 'SIZE', 'SLICE', 'SOME',
        'SOURCE', 'STEPS_TO_QUOTA', 'SUB', 'SWAP',
        'TRANSFER_TOKENS', 'UNIT', 'UNPACK', 'UPDATE', 'XOR',
        'UNPAIR', 'UNPAPAIR', // TODO: macro
        'IF_SOME', // TODO: macro
        'IFCMPEQ', 'IFCMPNEQ', 'IFCMPLT', 'IFCMPGT', 'IFCMPLE', 'IFCMPGE',
        'CMPEQ', 'CMPNEQ', 'CMPLT', 'CMPGT', 'CMPLE',
        'CMPGE', 'IFEQ', 'NEQ', 'IFLT', 'IFGT', 'IFLE',
        'IFGE', // TODO: should be separate
        /*'DIG',*/ /*'DUG',*/ 'EMPTY_BIG_MAP', 'APPLY', 'CHAIN_ID'
      ],
      'macroCADR': macroCADRconst,
      'macroDIP': dIPmatcher,
      'macroDUP': dUPmatcher,
      'macroSETCADR': macroSETCADRconst,
      'macroASSERTlist': macroASSERTlistConst,
      'constantData': ['Unit', 'True', 'False', 'None', 'instruction'],
      'singleArgData': ['Left', 'Right', 'Some'],
      'doubleArgData': ['Pair'],
      'elt': "Elt",
      'word': RegExp(r'[a-zA-Z_0-9]+'),
      'string': RegExp(r'\"(?:\\\\[\"\\\\]|[^\\n\"\\\\])*\"'),
    });
  }

  scriptToJson(d) =>
      '[ ${d[0] is List ? d[0][0]['value'].toString() : d[0] is String ? d[0] : d[0]['value']}, ${d[2]}, { "prim": "code", "args": [ [ ${d[4].reduce((a, e) => a + ',' + e)} ] ] } ]';

  singleArgKeywordToJson(d) =>
      '{ "prim": "${d[0] is List ? d[0][0]['value'].toString() : d[0]['value']}", "args": [ ${d[2]} ] }';

  keywordToJson(d) {
    var word = d[0] is List ? d[0][0]['value'].toString() : d[0]['value'];

    if (d.length == 1) {
      if (checkKeyword(word)) {
        return expandKeyword(word, null);
      } else {
        return '{ "prim": "$word" }';
      }
    } else {
      var annot = d[1].map((x) => '"${getMapValue(x[1])}"');
      if (checkKeyword(word)) {
        return [expandKeyword(word, annot)];
      } else {
        return '{ "prim": "$word", "annots": [${listToString(annot)}] }';
      }
    }
  }

  checkKeyword(word) {
    if (checkAssert(word)) {
      return true;
    }
    if (checkCompare(word)) {
      return true;
    }
    if (checkDip(word)) {
      return true;
    }
    if (checkDup(word)) {
      return true;
    }
    if (checkFail(word)) {
      return true;
    }
    if (checkIf(word)) {
      return true;
    }
    if (checkCR(word)) {
      return true;
    }
    if (checkOther(word)) {
      return true;
    }
    if (checkSetCadr(word)) {
      return true;
    }
    return false;
  }

  expandKeyword(word, annot) {
    if (checkCR(word)) {
      return expandCR(word, annot);
    }
    if (checkAssert(word)) {
      return expandAssert(word, annot);
    }
    if (checkCompare(word)) {
      return expandCmp(word, annot);
    }
    if (checkDip(word)) {
      return expandDIP(word, annot);
    }
    if (checkDup(word)) {
      return expandDup(word, annot);
    }
    if (checkFail(word)) {
      return expandFail(word, annot);
    }
    if (checkIf(word)) {
      return expandIF(word, annot);
    }
    if (checkOther(word)) {
      return expandOther(word, annot);
    }
    if (checkSetCadr(word)) {
      return expandSetCadr(word, annot);
    }
    return false;
  }

  checkAssert(asser) => macroASSERTlistConst.contains(asser);
  expandAssert(asset, annot) {
    var annotation = annot == null ? ', "annots": [$annot]' : '';
    switch (asset) {
      case 'ASSERT':
        return '[{"prim":"IF","args":[[],[[{"prim":"UNIT"},{"prim":"FAILWITH"$annotation}]]]}]';
      case 'ASSERT_CMPEQ':
        return '[[{"prim":"COMPARE"},{"prim":"EQ"}],{"prim":"IF","args":[[],[[{"prim":"UNIT"},{"prim":"FAILWITH"$annotation}]]]}]';
      case 'ASSERT_CMPGE':
        return '[[{"prim":"COMPARE"},{"prim":"GE"}],{"prim":"IF","args":[[],[[{"prim":"UNIT"},{"prim":"FAILWITH"$annotation}]]]}]';
      case 'ASSERT_CMPGT':
        return '[[{"prim":"COMPARE"},{"prim":"GT"}],{"prim":"IF","args":[[],[[{"prim":"UNIT"},{"prim":"FAILWITH"$annotation}]]]}]';
      case 'ASSERT_CMPLE':
        return '[[{"prim":"COMPARE"},{"prim":"LE"}],{"prim":"IF","args":[[],[[{"prim":"UNIT"},{"prim":"FAILWITH"$annotation}]]]}]';
      case 'ASSERT_CMPLT':
        return '[[{"prim":"COMPARE"},{"prim":"LT"}],{"prim":"IF","args":[[],[[{"prim":"UNIT"},{"prim":"FAILWITH"$annotation}]]]}]';
      case 'ASSERT_CMPNEQ':
        return '[[{"prim":"COMPARE"},{"prim":"NEQ"}],{"prim":"IF","args":[[],[[{"prim":"UNIT"},{"prim":"FAILWITH"$annotation}]]]}]';
      case 'ASSERT_EQ':
        return '[{"prim":"EQ"},{"prim":"IF","args":[[],[[{"prim":"UNIT"},{"prim":"FAILWITH"$annotation}]]]]';
      case 'ASSERT_GE':
        return '[{"prim":"GE"},{"prim":"IF","args":[[],[[{"prim":"UNIT"},{"prim":"FAILWITH"$annotation}]]]}]';
      case 'ASSERT_GT':
        return '[{"prim":"GT"},{"prim":"IF","args":[[],[[{"prim":"UNIT"},{"prim":"FAILWITH"$annotation}]]]}]';
      case 'ASSERT_LE':
        return '[{"prim":"LE"},{"prim":"IF","args":[[],[[{"prim":"UNIT"},{"prim":"FAILWITH"$annotation}]]]}]';
      case 'ASSERT_LT':
        return '[{"prim":"LT"},{"prim":"IF","args":[[],[[{"prim":"UNIT"},{"prim":"FAILWITH"$annotation}]]]}]';
      case 'ASSERT_NEQ':
        return '[{"prim":"NEQ"},{"prim":"IF","args":[[],[[{"prim":"UNIT"},{"prim":"FAILWITH"$annotation}]]]}]';
      case 'ASSERT_NONE': // IF_NONE {} {FAIL}
        return '[{"prim":"IF_NONE","args":[[],[[{"prim":"UNIT"},{"prim":"FAILWITH"}]]]}]';
      case 'ASSERT_SOME': // IF_NONE {FAIL} {RENAME @x}
        return '[{"prim":"IF_NONE","args":[[[{"prim":"UNIT"},{"prim":"FAILWITH"}]],[]]}]';
      case 'ASSERT_LEFT': // IF_LEFT {RENAME @x} {FAIL}
        return '';
      case 'ASSERT_RIGHT': // IF_LEFT {FAIL} {RENAME @x}
        return '';
      default:
        throw new Exception('Could not process $asset');
    }
  }

  checkCompare(cmp) => macroCMPlist.contains(cmp);
  expandCmp(cmp, annot) {
    var op = cmp.substring(3);
    var binaryOp = keywordToJson(['$op']);
    var compare = keywordToJson(['COMPARE']);
    if (annot != null) {
      binaryOp = '{ "prim": "$op", "annots": [$annot] }';
    }

    return '[$compare, $binaryOp]';
  }

  checkDip(dip) => dIPmatcher.hasMatch(dip);
  expandDIP(dip, instruction, {annot}) {
    var t = '';
    if (dIPmatcher.hasMatch(dip)) {
      var c = dip.length - 2;
      for (var i = 0; i < c; i++) {
        t += '[{ "prim": "DIP", "args": [ ';
      }
      t = '$t [ $instruction ] ]';
      if (annot == null) {
        t = '$t, "annots": [$annot]';
      }
      t += ' }]';
      for (var i = 0; i < c - 1; i++) {
        t += ' ] }]';
      }
      return t;
    }

    throw new Exception('Unexpected parameter for DIP processing: $dip');
  }

  checkDup(dup) => dUPmatcher.hasMatch(dup);
  expandDup(dup, annot) {
    var t = '';
    if (dUPmatcher.hasMatch(dup)) {
      var c = dup.length - 3;
      for (var i = 0; i < c; i++) {
        t += '[{ "prim": "DIP", "args": [ ';
      }

      if (annot == null) {
        t += '[{ "prim": "DUP" }]';
      } else {
        t += '[{ "prim": "DUP", "annots": [$annot] }]';
      }

      for (var i = 0; i < c; i++) {
        t += ' ] },{"prim":"SWAP"}]';
      }
      return t;
    }

    throw new Exception('');
  }

  checkFail(fail) => fail == "FAIL";
  expandFail(fail, annot) {
    if (annot == null) {
      return '[ { "prim": "UNIT" }, { "prim": "FAILWITH" } ]';
    } else {
      return '[ { "prim": "UNIT" }, { "prim": "FAILWITH", "annots": [$annot] } ]';
    }
  }

  checkIf(ifStatement) => (macroIFCMPlist.contains(ifStatement) ||
      macroIFlist.contains(ifStatement) ||
      ifStatement == 'IF_SOME'); // TODO: IF_SOME
  expandIF(ifInstr, ifTrue, {ifFalse, annot}) {
    var annotation = annot != null ? ', "annots": [$annot]' : '';

    switch (ifInstr) {
      case 'IFCMPEQ':
        return '[{"prim":"COMPARE"},{"prim":"EQ"},{"prim":"IF","args":[ [$ifTrue] , [$ifFalse]]$annotation}]';
      case 'IFCMPGE':
        return '[{"prim":"COMPARE"},{"prim":"GE"},{"prim":"IF","args":[ [$ifTrue] , [$ifFalse]]$annotation}]';
      case 'IFCMPGT':
        return '[{"prim":"COMPARE"},{"prim":"GT"},{"prim":"IF","args":[ [$ifTrue] , [$ifFalse]]$annotation}]';
      case 'IFCMPLE':
        return '[{"prim":"COMPARE"},{"prim":"LE"},{"prim":"IF","args":[ [$ifTrue] , [$ifFalse]]$annotation}]';
      case 'IFCMPLT':
        return '[{"prim":"COMPARE"},{"prim":"LT"},{"prim":"IF","args":[ [$ifTrue] , [$ifFalse]]$annotation}]';
      case 'IFCMPNEQ':
        return '[{"prim":"COMPARE"},{"prim":"NEQ"},{"prim":"IF","args":[ [$ifTrue] , [$ifFalse]]$annotation}]';
      case 'IFEQ':
        return '[{"prim":"EQ"},{"prim":"IF","args":[ [$ifTrue] , [$ifFalse]]$annotation}]';
      case 'IFGE':
        return '[{"prim":"GE"},{"prim":"IF","args":[ [$ifTrue] , [$ifFalse]]$annotation}]';
      case 'IFGT':
        return '[{"prim":"GT"},{"prim":"IF","args":[ [$ifTrue] , [$ifFalse]]$annotation}]';
      case 'IFLE':
        return '[{"prim":"LE"},{"prim":"IF","args":[ [$ifTrue] , [$ifFalse]]$annotation}]';
      case 'IFLT':
        return '[{"prim":"LT"},{"prim":"IF","args":[ [$ifTrue] , [$ifFalse]]$annotation}]';
      case 'IFNEQ':
        return '[{"prim":"NEQ"},{"prim":"IF","args":[ [$ifTrue] , [$ifFalse]]$annotation}]';
      case 'IF_SOME':
        return '[{"prim":"IF_NONE","args":[ [$ifFalse], [$ifTrue]]$annotation}]';
      default:
        throw new Exception('Could not process $ifInstr');
    }
  }

  checkCR(cr) => RegExp(r'^C(A|D)(A|D)+R$').hasMatch(cr);
  expandCR(word, annot) {
    var expandedCR = word
        .substring(1, word.length - 1)
        .split('')
        .map((c) => (c == 'A' ? '{ "prim": "CAR" }' : '{ "prim": "CDR" }'));
    if (annot != null) {
      var lastChar = word.substring(word.length - 2, word.length - 1);
      if (lastChar == 'A') {
        expandedCR[expandedCR.length - 1] =
            '{ "prim": "CAR", "annots": [$annot] }';
      } else if (lastChar == 'D') {
        expandedCR[expandedCR.length - 1] =
            '{ "prim": "CDR", "annots": [$annot] }';
      }
    }

    return '[${expandedCR.join(', ')}]';
  }

  checkOther(word) =>
      (word == "UNPAIR" || word == "UNPAPAIR"); // TODO: dynamic matching
  expandOther(word, annot) {
    if (word == 'UNPAIR') {
      if (annot == null) {
        return '[ [ { "prim": "DUP" }, { "prim": "CAR" }, { "prim": "DIP", "args": [ [ { "prim": "CDR" } ] ] } ] ]';
      } else if (annot.length == 1) {
        return '[ [ { "prim": "DUP" }, { "prim": "CAR", "annots": [$annot] }, { "prim": "DIP", "args": [ [ { "prim": "CDR" } ] ]  } ] ]';
      } else if (annot.length == 2) {
        return r'[ [ { "prim": "DUP" }, { "prim": "CAR", "annots": [${annot[0]}] }, { "prim": "DIP", "args": [ [ { "prim": "CDR", "annots": [${annot[1]}] } ] ]  } ] ]';
      } else {
        return '';
      }
    }

    if (word == 'UNPAPAIR') {
      if (annot == null) {
        return """[ [ { "prim": "DUP" },
                            { "prim": "CAR" },
                            { "prim": "DIP", "args": [ [ { "prim": "CDR" } ] ] } ],
                            {"prim":"DIP","args":[[[{"prim":"DUP"},{"prim":"CAR"},{"prim":"DIP","args":[[{"prim":"CDR"}]]}]]]}]""";
      } else {
        return """[ [ { "prim": "DUP" },
                            { "prim": "CAR" },
                            { "prim": "DIP", "args": [ [ { "prim": "CDR" } ] ] } ],
                            {"prim":"DIP","args":[[[{"prim":"DUP"},{"prim":"CAR"},{"prim":"DIP","args":[[{"prim":"CDR"}]],"annots": [$annot]}]]]}]""";
      }
    }
  }

  checkSetCadr(s) => new RegExp(macroSETCADRconst).hasMatch(s);
  expandSetCadr(word, annot) => nestSetCadr(word.substring(5, word.length - 1));
  nestSetCadr(r) {
    if (r.length == 0) {
      return '';
    }

    var c = r[0];
    if (r.length == 1) {
      if (c == 'A') {
        return '[{"prim": "CDR","annots":["@%%"]}, {"prim": "SWAP"}, {"prim": "PAIR","annots":["%","%@"]}]';
      } else if (c == 'D') {
        return '[{"prim": "CAR","annots":["@%%"]}, {"prim": "PAIR","annots":["%@","%"]}]';
      }
    }

    if (c == 'A') {
      return '[{"prim": "DUP"}, {"prim": "DIP", "args": [[{"prim": "CAR","annots":["@%%"]}, ${nestSetCadr(r.substring(1))}]]}, {"prim": "CDR","annots":["@%%"]}, {"prim": "SWAP"}, {"prim": "PAIR","annots":["%@","%@"]}]';
    } else if (c == 'D') {
      return '[{"prim": "DUP"}, {"prim": "DIP", "args": [[{"prim": "CDR","annots":["@%%"]}, ${nestSetCadr(r.substring(1))}]]}, {"prim": "CAR","annots":["@%%"]}, {"prim": "PAIR","annots":["%@","%@"]}]';
    }
  }

  singleArgKeywordWithParenToJson(d) =>
      '{ "prim": "${d[2]}", "args": [ ${d[(4 + ((d.length == 7) ? 0 : 2))]} ] }';

  doubleArgKeywordToJson(d) =>
      '{ "prim": "${d[0]['value']}", "args": [ ${d[2]}, ${d[4]} ] }';

  doubleArgKeywordWithParenToJson(d) =>
      '{ "prim": "${d[2]['value']}", "args": [ ${d[4]}, ${d[6]} ] }';

  comparableTypeToJson(d) {
    var annot =
        d[3].map((x) => '"${x[1]['value'].toString()}"').toList()[0].toString();
    return '{ "prim": "${d[2]['value']}", "annots": [$annot]  }';
  }

  singleArgTypeKeywordWithParenToJson(d) {
    var annot = d[3].map((x) => '"${getMapValue(x[1])}"');
    return '{ "prim": "${getMapValue(d[2])}", "args": [ ${d[5]} ], "annots": [${listToString(annot)}] }';
  }

  doubleArgTypeKeywordWithParenToJson(d) {
    var annot =
        d[3].map((x) => '"${x[1]['value'].toString()}"').toList()[0].toString();
    // d[3].map((x) => '"${x[1]}"');
    return '{ "prim": "${d[2]['value']}", "args": [ ${d[5]}, ${d[7]} ], "annots": [$annot]  }';
  }

  intToJson(d) => '{ "int": "${int.parse(getMapValue(d[0]))}" }';

  bytesToJson(d) {
    return '{ "bytes": "${getMapValue(d[0]).substring(2)}" }';
  }

  stringToJson(d) => '{ "string": ${getMapValue(d[0])} }';

  // ignore: slash_for_doc_comments
  /**
     * Given a list of michelson instructions, convert it into JSON.
     * Example: "{CAR; NIL operation; PAIR;}" ->
     * [ '{ prim: CAR }',
     * '{ prim: NIL, args: [{ prim: operation }] }',
     * '{ prim: PAIR }' ]
     */
  instructionSetToJsonNoSemi(d) {
    var _d = d[2].map((x) => x[0]).toList();
    if (d[3] is String) {
      _d.add(d[3]);
    } else {
      _d.addAll(d[3]);
    }
    return removeIfNextIsSame(_d.map((x) => nestedArrayChecker(x)).toList())
        .toList();
  }

  instructionSetToJsonSemi(d) {
    return '${listToString(d[2].map((x) => x[0]).map((x) => nestedArrayChecker(x)))}';
  }

  dataListToJsonSemi(d) {
    return '[ ${listToString(d[2].map((x) => x[0]).map((x) => nestedArrayChecker(x)))} ]';
  }

  nestedArrayChecker(x) {
    if (x is List && x[0] is List) {
      // handles double array nesting
      return x[0];
    } else {
      return x;
    }
  }

  singleArgInstrKeywordToJson(d) {
    var word = '${d[0] is List ? d[0][0]['value'].toString() : d[0]['value']}';
    if (checkDip(word)) {
      return expandDIP(word, d[2]);
    } else {
      return '{ "prim": "$word", "args": [ [ ${listToString(d[2])} ] ] }';
    }
  }

  singleArgTypeKeywordToJson(d) {
    var word = '${d[0].toString()}';
    var annot = d[1].map((x) => '"${x[1]}"');
    if (checkDip(word)) {
      return expandDIP(word, d[2], annot: annot);
    } else {
      return '{ "prim": "${d[0]}", "args": [ ${d[3]} ], "annots": [$annot] }';
    }
  }

  tripleArgKeyWordToJson(d) =>
      '{ "prim": "${d[0]}", "args": [ ${d[2]}, ${d[4]}, [${d[6]}] ] }';

  tripleArgTypeKeyWordToJson(d) {
    var annot = d[1].map((x) => '"${x[1]}"');
    return '{ "prim": "${d[0]}", "args": [ ${d[3]}, ${d[5]}, ${d[7]} ], "annots": [$annot]  }';
  }

  doubleArgInstrKeywordToJson(d) {
    var word = '${d[0] is List ? d[0][0]['value'].toString() : d[0]['value']}';
    if (checkIf(word)) {
      return expandIF(word, d[2], ifFalse: d[4] != null);
    } else {
      return '{ "prim": "$word", "args": [ [${d[2]}], [${d[4]}] ] }';
    }
  }

  doubleArgTypeKeywordToJson(d) {
    var annot = d[1].map((x) => '"${x[1]}"');
    return '{ "prim": "${d[0]}", "args": [ ${d[4]}, ${d[6]} ], "annots": [$annot]  }';
  }

  pushToJson(d) => '{ "prim": "${d[0]}", "args": [${d[2]}, []] }';

  pushWithAnnotsToJson(d) {
    var annot = d[1].map((x) => '"${x[1]}"');
    return '{ "prim": "PUSH", "args": [ ${d[3]}, ${d[5]} ], "annots": [$annot]  }';
  }

  dipnToJson(d) => (d.length > 4)
      ? '{ "prim": "${d[0]}", "args": [ { "int": "${d[2]}" }, [ ${d[4]} ] ] }'
      : '{ "prim": "${d[0]}", "args": [ ${d[2]} ] }';

  dupnToJson(d) {
    var n = int.parse(d[2].toString());

    if (n == 1) {
      return '{ "prim": "DUP" }';
    } else if (n == 2) {
      return '[{ "prim": "DIP", "args": [[ {"prim": "DUP"} ]] }, { "prim": "SWAP" }]';
    } else {
      return '[{ "prim": "DIP", "args": [ {"int": "${n - 1}"}, [{ "prim": "DUP" }] ] }, { "prim": "DIG", "args": [ {"int": "$n"} ] }]';
    }
  }

  dignToJson(d) =>
      '{ "prim": "${getMapValue(d[0])}", "args": [ { "int": "${getMapValue(d[2])}" } ] }';

  dropnToJson(d) =>
      '{ "prim": "${getMapValue(d[0])}", "args": [ { "int": "${getMapValue(d[2])}" } ] }';

  subContractToJson(d) =>
      '{ "prim":"CREATE_CONTRACT", "args": [ [ ${d[4]}, ${d[6]}, {"prim": "code" , "args":[ [ ${d[8]} ] ] } ] ] }';

  instructionListToJson(d) {
    var instructionOne = [d[2]];
    var instructionList = d[3].map((x) => x[3]).toList();
    return instructionOne
      ..addAll(instructionList)
      ..map((x) => nestedArrayChecker(x));
  }

  doubleArgParenKeywordToJson(d) =>
      '{ "prim": "${d[0]}", "args": [ ${d[4]}, ${d[8]} ] }';

  listToString(list) =>
      removeIfNextIsSame(list.toList()).reduce((a, e) => a + ',' + e);

  removeIfNextIsSame(list) {
    for (var i = 0; i < list.length; i++) {
      if (i != 0 && list[i - 1] == list[i]) {
        list.removeAt(i);
      }
    }
    return list;
  }

  getMapValue(d) => d is List ? d[0]['value'].toString() : d['value'];

  id(x) => x[0];

  Map<String, dynamic> get grammar => {
        'Lexer': lexer,
        'ParserRules': [
          {
            "name": "main",
            "symbols": ["instruction"],
            "postprocess": id
          },
          {
            "name": "main",
            "symbols": ["data"],
            "postprocess": id
          },
          {
            "name": "main",
            "symbols": ["type"],
            "postprocess": id
          },
          {
            "name": "main",
            "symbols": ["parameter"],
            "postprocess": id
          },
          {
            "name": "main",
            "symbols": ["storage"],
            "postprocess": id
          },
          {
            "name": "main",
            "symbols": ["code"],
            "postprocess": id
          },
          {
            "name": "main",
            "symbols": ["script"],
            "postprocess": id
          },
          {
            "name": "main",
            "symbols": ["parameterValue"],
            "postprocess": id
          },
          {
            "name": "main",
            "symbols": ["storageValue"],
            "postprocess": id
          },
          {
            "name": "main",
            "symbols": ["typeData"],
            "postprocess": id
          },
          {
            "name": "script",
            "symbols": ["parameter", "_", "storage", "_", "code"],
            "postprocess": scriptToJson
          },
          {
            "name": "parameterValue",
            "symbols": [
              (lexer!.has("parameter") ? {'type': "parameter"} : parameter),
              "_",
              "typeData",
              "_",
              "semicolons"
            ],
            "postprocess": singleArgKeywordToJson
          },
          {
            "name": "storageValue",
            "symbols": [
              (lexer!.has("storage") ? {'type': "storage"} : storage),
              "_",
              "typeData",
              "_",
              "semicolons"
            ],
            "postprocess": singleArgKeywordToJson
          },
          {
            "name": "parameter",
            "symbols": [
              (lexer!.has("parameter") ? {'type': "parameter"} : parameter),
              "_",
              "type",
              "_",
              "semicolons"
            ],
            "postprocess": singleArgKeywordToJson
          },
          {
            "name": "storage",
            "symbols": [
              (lexer!.has("storage") ? {'type': "storage"} : storage),
              "_",
              "type",
              "_",
              "semicolons"
            ],
            "postprocess": singleArgKeywordToJson
          },
          {
            "name": "code",
            "symbols": [
              (lexer!.has("code") ? {'type': "code"} : code),
              "_",
              "subInstruction",
              "_",
              "semicolons",
              "_"
            ],
            "postprocess": (d) => d[2]
          },
          {
            "name": "code",
            "symbols": [
              (lexer!.has("code") ? {'type': "code"} : code),
              "_",
              {"literal": "{};"}
            ],
            "postprocess": (d) => "code {}"
          },
          {
            "name": "type",
            "symbols": [
              (lexer!.has("comparableType")
                  ? {'type': "comparableType"}
                  : comparableType)
            ],
            "postprocess": keywordToJson
          },
          {
            "name": "type",
            "symbols": [
              (lexer!.has("constantType")
                  ? {'type': "constantType"}
                  : constantType)
            ],
            "postprocess": keywordToJson
          },
          {
            "name": "type",
            "symbols": [
              (lexer!.has("singleArgType")
                  ? {'type': "singleArgType"}
                  : singleArgType),
              "_",
              "type"
            ],
            "postprocess": singleArgKeywordToJson
          },
          {
            "name": "type",
            "symbols": [
              (lexer!.has("lparen") ? {'type': "lparen"} : lparen),
              "_",
              (lexer!.has("singleArgType")
                  ? {'type': "singleArgType"}
                  : singleArgType),
              "_",
              "type",
              "_",
              (lexer!.has("rparen") ? {'type': "rparen"} : rparen)
            ],
            "postprocess": singleArgKeywordWithParenToJson
          },
          {
            "name": "type",
            "symbols": [
              (lexer!.has("lparen") ? {'type': "lparen"} : lparen),
              "_",
              (lexer!.has("singleArgType")
                  ? {'type': "singleArgType"}
                  : singleArgType),
              "_",
              (lexer!.has("lparen") ? {'type': "lparen"} : lparen),
              "_",
              "type",
              "_",
              (lexer!.has("rparen") ? {'type': "rparen"} : rparen),
              "_",
              (lexer!.has("rparen") ? {'type': "rparen"} : rparen)
            ],
            "postprocess": singleArgKeywordWithParenToJson
          },
          {
            "name": "type",
            "symbols": [
              (lexer!.has("doubleArgType")
                  ? {'type': "doubleArgType"}
                  : doubleArgType),
              "_",
              "type",
              "_",
              "type"
            ],
            "postprocess": doubleArgKeywordToJson
          },
          {
            "name": "type",
            "symbols": [
              (lexer!.has("lparen") ? {'type': "lparen"} : lparen),
              "_",
              (lexer!.has("doubleArgType")
                  ? {'type': "doubleArgType"}
                  : doubleArgType),
              "_",
              "type",
              "_",
              "type",
              "_",
              (lexer!.has("rparen") ? {'type': "rparen"} : rparen)
            ],
            "postprocess": doubleArgKeywordWithParenToJson
          },
          {
            "name": r"type$ebnf$1$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"type$ebnf$1",
            "symbols": [r"type$ebnf$1$subexpression$1"]
          },
          {
            "name": r"type$ebnf$1$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"type$ebnf$1",
            "symbols": [r"type$ebnf$1", r"type$ebnf$1$subexpression$2"],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "type",
            "symbols": [
              (lexer!.has("comparableType")
                  ? {'type': "comparableType"}
                  : comparableType),
              r"type$ebnf$1"
            ],
            "postprocess": keywordToJson
          },
          {
            "name": r"type$ebnf$2$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"type$ebnf$2",
            "symbols": [r"type$ebnf$2$subexpression$1"]
          },
          {
            "name": r"type$ebnf$2$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"type$ebnf$2",
            "symbols": [r"type$ebnf$2", r"type$ebnf$2$subexpression$2"],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "type",
            "symbols": [
              (lexer!.has("constantType")
                  ? {'type': "constantType"}
                  : constantType),
              r"type$ebnf$2"
            ],
            "postprocess": keywordToJson
          },
          {
            "name": r"type$ebnf$3$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"type$ebnf$3",
            "symbols": [r"type$ebnf$3$subexpression$1"]
          },
          {
            "name": r"type$ebnf$3$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"type$ebnf$3",
            "symbols": [r"type$ebnf$3", r"type$ebnf$3$subexpression$2"],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "type",
            "symbols": [
              (lexer!.has("lparen") ? {'type': "lparen"} : lparen),
              "_",
              (lexer!.has("comparableType")
                  ? {'type': "comparableType"}
                  : comparableType),
              r"type$ebnf$3",
              "_",
              (lexer!.has("rparen") ? {'type': "rparen"} : rparen)
            ],
            "postprocess": comparableTypeToJson
          },
          {
            "name": r"type$ebnf$4$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"type$ebnf$4",
            "symbols": [r"type$ebnf$4$subexpression$1"]
          },
          {
            "name": r"type$ebnf$4$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"type$ebnf$4",
            "symbols": [r"type$ebnf$4", r"type$ebnf$4$subexpression$2"],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "type",
            "symbols": [
              (lexer!.has("lparen") ? {'type': "lparen"} : lparen),
              "_",
              (lexer!.has("constantType")
                  ? {'type': "constantType"}
                  : constantType),
              r"type$ebnf$4",
              "_",
              (lexer!.has("rparen") ? {'type': "rparen"} : rparen)
            ],
            "postprocess": comparableTypeToJson
          },
          {
            "name": r"type$ebnf$5$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"type$ebnf$5",
            "symbols": [r"type$ebnf$5$subexpression$1"]
          },
          {
            "name": r"type$ebnf$5$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"type$ebnf$5",
            "symbols": [r"type$ebnf$5", r"type$ebnf$5$subexpression$2"],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "type",
            "symbols": [
              (lexer!.has("lparen") ? {'type': "lparen"} : lparen),
              "_",
              (lexer!.has("singleArgType")
                  ? {'type': "singleArgType"}
                  : singleArgType),
              r"type$ebnf$5",
              "_",
              "type",
              (lexer!.has("rparen") ? {'type': "rparen"} : rparen)
            ],
            "postprocess": singleArgTypeKeywordWithParenToJson
          },
          {
            "name": r"type$ebnf$6$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"type$ebnf$6",
            "symbols": [r"type$ebnf$6$subexpression$1"]
          },
          {
            "name": r"type$ebnf$6$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"type$ebnf$6",
            "symbols": [r"type$ebnf$6", r"type$ebnf$6$subexpression$2"],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "type",
            "symbols": [
              (lexer!.has("lparen") ? {'type': "lparen"} : lparen),
              "_",
              (lexer!.has("doubleArgType")
                  ? {'type': "doubleArgType"}
                  : doubleArgType),
              r"type$ebnf$6",
              "_",
              "type",
              "_",
              "type",
              (lexer!.has("rparen") ? {'type': "rparen"} : rparen)
            ],
            "postprocess": doubleArgTypeKeywordWithParenToJson
          },
          {
            "name": "typeData",
            "symbols": [
              (lexer!.has("singleArgType")
                  ? {'type': "singleArgType"}
                  : singleArgType),
              "_",
              "typeData"
            ],
            "postprocess": singleArgKeywordToJson
          },
          {
            "name": "typeData",
            "symbols": [
              (lexer!.has("lparen") ? {'type': "lparen"} : lparen),
              "_",
              (lexer!.has("singleArgType")
                  ? {'type': "singleArgType"}
                  : singleArgType),
              "_",
              "typeData",
              "_",
              (lexer!.has("rparen") ? {'type': "rparen"} : rparen)
            ],
            "postprocess": singleArgKeywordWithParenToJson
          },
          {
            "name": "typeData",
            "symbols": [
              (lexer!.has("doubleArgType")
                  ? {'type': "doubleArgType"}
                  : doubleArgType),
              "_",
              "typeData",
              "_",
              "typeData"
            ],
            "postprocess": doubleArgKeywordToJson
          },
          {
            "name": "typeData",
            "symbols": [
              (lexer!.has("lparen") ? {'type': "lparen"} : lparen),
              "_",
              (lexer!.has("doubleArgType")
                  ? {'type': "doubleArgType"}
                  : doubleArgType),
              "_",
              "typeData",
              "_",
              "typeData",
              "_",
              (lexer!.has("rparen") ? {'type': "rparen"} : rparen)
            ],
            "postprocess": doubleArgKeywordWithParenToJson
          },
          {
            "name": "typeData",
            "symbols": ["subTypeData"],
            "postprocess": id
          },
          {
            "name": "typeData",
            "symbols": ["subTypeElt"],
            "postprocess": id
          },
          {
            "name": "typeData",
            "symbols": [
              (lexer!.has("number") ? {'type': "number"} : number)
            ],
            "postprocess": intToJson
          },
          {
            "name": "typeData",
            "symbols": [
              (lexer!.has("string") ? {'type': "string"} : string)
            ],
            "postprocess": stringToJson
          },
          {
            "name": "typeData",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              "_",
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": (d) => []
          },
          {
            "name": "data",
            "symbols": [
              (lexer!.has("constantData")
                  ? {'type': "constantData"}
                  : constantData)
            ],
            "postprocess": keywordToJson
          },
          {
            "name": "data",
            "symbols": [
              (lexer!.has("singleArgData")
                  ? {'type': "singleArgData"}
                  : singleArgData),
              "_",
              "data"
            ],
            "postprocess": singleArgKeywordToJson
          },
          {
            "name": "data",
            "symbols": [
              (lexer!.has("doubleArgData")
                  ? {'type': "doubleArgData"}
                  : doubleArgData),
              "_",
              "data",
              "_",
              "data"
            ],
            "postprocess": doubleArgKeywordToJson
          },
          {
            "name": "data",
            "symbols": ["subData"],
            "postprocess": id
          },
          {
            "name": "data",
            "symbols": ["subElt"],
            "postprocess": id
          },
          {
            "name": "data",
            "symbols": [
              (lexer!.has("string") ? {'type': "string"} : string)
            ],
            "postprocess": stringToJson
          },
          {
            "name": "data",
            "symbols": [
              (lexer!.has("bytes") ? {'type': "bytes"} : bytes)
            ],
            "postprocess": bytesToJson
          },
          {
            "name": "data",
            "symbols": [
              (lexer!.has("number") ? {'type': "number"} : number)
            ],
            "postprocess": intToJson
          },
          {
            "name": "subData",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              "_",
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": (d) => "[]"
          },
          {
            "name": r"subData$ebnf$1$subexpression$1",
            "symbols": ["data", "_"]
          },
          {
            "name": r"subData$ebnf$1",
            "symbols": [r"subData$ebnf$1$subexpression$1"]
          },
          {
            "name": r"subData$ebnf$1$subexpression$2",
            "symbols": ["data", "_"]
          },
          {
            "name": r"subData$ebnf$1",
            "symbols": [r"subData$ebnf$1", r"subData$ebnf$1$subexpression$2"],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "subData",
            "symbols": [
              {"literal": "("},
              "_",
              r"subData$ebnf$1",
              {"literal": ")"}
            ],
            "postprocess": instructionSetToJsonSemi
          },
          {
            "name": r"subData$ebnf$2$subexpression$1$ebnf$1",
            "symbols": [
              {"literal": ";"}
            ],
            "postprocess": id
          },
          {
            "name": r"subData$ebnf$2$subexpression$1$ebnf$1",
            "symbols": [],
            // "postprocess": () => null
          },
          {
            "name": r"subData$ebnf$2$subexpression$1",
            "symbols": [
              "data",
              "_",
              r"subData$ebnf$2$subexpression$1$ebnf$1",
              "_"
            ]
          },
          {
            "name": r"subData$ebnf$2",
            "symbols": [r"subData$ebnf$2$subexpression$1"]
          },
          {
            "name": r"subData$ebnf$2$subexpression$2$ebnf$1",
            "symbols": [
              {"literal": ";"}
            ],
            "postprocess": id
          },
          {
            "name": r"subData$ebnf$2$subexpression$2$ebnf$1",
            "symbols": [],
            // "postprocess": () => null
          },
          {
            "name": r"subData$ebnf$2$subexpression$2",
            "symbols": [
              "data",
              "_",
              r"subData$ebnf$2$subexpression$2$ebnf$1",
              "_"
            ]
          },
          {
            "name": r"subData$ebnf$2",
            "symbols": [r"subData$ebnf$2", r"subData$ebnf$2$subexpression$2"],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "subData",
            "symbols": [
              {"literal": "{"},
              "_",
              r"subData$ebnf$2",
              {"literal": "}"}
            ],
            "postprocess": dataListToJsonSemi
          },
          {
            "name": "subElt",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              "_",
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": (d) => "[]"
          },
          {
            "name": r"subElt$ebnf$1$subexpression$1$ebnf$1",
            "symbols": [
              {"literal": ";"}
            ],
            "postprocess": id
          },
          {
            "name": r"subElt$ebnf$1$subexpression$1$ebnf$1",
            "symbols": [],
            // "postprocess": () => null
          },
          {
            "name": r"subElt$ebnf$1$subexpression$1",
            "symbols": ["elt", r"subElt$ebnf$1$subexpression$1$ebnf$1", "_"]
          },
          {
            "name": r"subElt$ebnf$1",
            "symbols": [r"subElt$ebnf$1$subexpression$1"]
          },
          {
            "name": r"subElt$ebnf$1$subexpression$2$ebnf$1",
            "symbols": [
              {"literal": ";"}
            ],
            "postprocess": id
          },
          {
            "name": r"subElt$ebnf$1$subexpression$2$ebnf$1",
            "symbols": [],
            // "postprocess": () => null
          },
          {
            "name": r"subElt$ebnf$1$subexpression$2",
            "symbols": ["elt", r"subElt$ebnf$1$subexpression$2$ebnf$1", "_"]
          },
          {
            "name": r"subElt$ebnf$1",
            "symbols": [r"subElt$ebnf$1", r"subElt$ebnf$1$subexpression$2"],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "subElt",
            "symbols": [
              {"literal": "{"},
              "_",
              r"subElt$ebnf$1",
              {"literal": "}"}
            ],
            "postprocess": dataListToJsonSemi
          },
          {
            "name": "elt",
            "symbols": [
              (lexer!.has("elt") ? {'type': "elt"} : elt),
              "_",
              "data",
              "_",
              "data"
            ],
            "postprocess": doubleArgKeywordToJson
          },
          {
            "name": "subTypeData",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              "_",
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": (d) => "[]"
          },
          {
            "name": r"subTypeData$ebnf$1$subexpression$1$ebnf$1",
            "symbols": [
              {"literal": ";"}
            ],
            "postprocess": id
          },
          {
            "name": r"subTypeData$ebnf$1$subexpression$1$ebnf$1",
            "symbols": [],
            // "postprocess": () => null
          },
          {
            "name": r"subTypeData$ebnf$1$subexpression$1",
            "symbols": [
              "data",
              r"subTypeData$ebnf$1$subexpression$1$ebnf$1",
              "_"
            ]
          },
          {
            "name": r"subTypeData$ebnf$1",
            "symbols": [r"subTypeData$ebnf$1$subexpression$1"]
          },
          {
            "name": r"subTypeData$ebnf$1$subexpression$2$ebnf$1",
            "symbols": [
              {"literal": ";"}
            ],
            "postprocess": id
          },
          {
            "name": r"subTypeData$ebnf$1$subexpression$2$ebnf$1",
            "symbols": [],
            // "postprocess": () => null
          },
          {
            "name": r"subTypeData$ebnf$1$subexpression$2",
            "symbols": [
              "data",
              r"subTypeData$ebnf$1$subexpression$2$ebnf$1",
              "_"
            ]
          },
          {
            "name": r"subTypeData$ebnf$1",
            "symbols": [
              r"subTypeData$ebnf$1",
              r"subTypeData$ebnf$1$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "subTypeData",
            "symbols": [
              {"literal": "{"},
              "_",
              r"subTypeData$ebnf$1",
              {"literal": "}"}
            ],
            "postprocess": instructionSetToJsonSemi
          },
          {
            "name": r"subTypeData$ebnf$2$subexpression$1$ebnf$1",
            "symbols": [
              {"literal": ";"}
            ],
            "postprocess": id
          },
          {
            "name": r"subTypeData$ebnf$2$subexpression$1$ebnf$1",
            "symbols": [],
            // "postprocess": () => null
          },
          {
            "name": r"subTypeData$ebnf$2$subexpression$1",
            "symbols": [
              "data",
              r"subTypeData$ebnf$2$subexpression$1$ebnf$1",
              "_"
            ]
          },
          {
            "name": r"subTypeData$ebnf$2",
            "symbols": [r"subTypeData$ebnf$2$subexpression$1"]
          },
          {
            "name": r"subTypeData$ebnf$2$subexpression$2$ebnf$1",
            "symbols": [
              {"literal": ";"}
            ],
            "postprocess": id
          },
          {
            "name": r"subTypeData$ebnf$2$subexpression$2$ebnf$1",
            "symbols": [],
            // "postprocess": () => null
          },
          {
            "name": r"subTypeData$ebnf$2$subexpression$2",
            "symbols": [
              "data",
              r"subTypeData$ebnf$2$subexpression$2$ebnf$1",
              "_"
            ]
          },
          {
            "name": r"subTypeData$ebnf$2",
            "symbols": [
              r"subTypeData$ebnf$2",
              r"subTypeData$ebnf$2$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "subTypeData",
            "symbols": [
              {"literal": "("},
              "_",
              r"subTypeData$ebnf$2",
              {"literal": ")"}
            ],
            "postprocess": instructionSetToJsonSemi
          },
          {
            "name": "subTypeElt",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              "_",
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": (d) => "[]"
          },
          {
            "name": r"subTypeElt$ebnf$1$subexpression$1$ebnf$1",
            "symbols": [
              {"literal": ";"}
            ],
            "postprocess": id
          },
          {
            "name": r"subTypeElt$ebnf$1$subexpression$1$ebnf$1",
            "symbols": [],
            // "postprocess": () => null
          },
          {
            "name": r"subTypeElt$ebnf$1$subexpression$1",
            "symbols": [
              "typeElt",
              r"subTypeElt$ebnf$1$subexpression$1$ebnf$1",
              "_"
            ]
          },
          {
            "name": r"subTypeElt$ebnf$1",
            "symbols": [r"subTypeElt$ebnf$1$subexpression$1"]
          },
          {
            "name": r"subTypeElt$ebnf$1$subexpression$2$ebnf$1",
            "symbols": [
              {"literal": ";"}
            ],
            "postprocess": id
          },
          {
            "name": r"subTypeElt$ebnf$1$subexpression$2$ebnf$1",
            "symbols": [],
            // "postprocess": () => null
          },
          {
            "name": r"subTypeElt$ebnf$1$subexpression$2",
            "symbols": [
              "typeElt",
              r"subTypeElt$ebnf$1$subexpression$2$ebnf$1",
              "_"
            ]
          },
          {
            "name": r"subTypeElt$ebnf$1",
            "symbols": [
              r"subTypeElt$ebnf$1",
              r"subTypeElt$ebnf$1$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "subTypeElt",
            "symbols": [
              {"literal": "[{"},
              "_",
              r"subTypeElt$ebnf$1",
              {"literal": "}]"}
            ],
            "postprocess": instructionSetToJsonSemi
          },
          {
            "name": r"subTypeElt$ebnf$2$subexpression$1$ebnf$1",
            "symbols": [
              {"literal": ";"}
            ],
            "postprocess": id
          },
          {
            "name": r"subTypeElt$ebnf$2$subexpression$1$ebnf$1",
            "symbols": [],
            // "postprocess": () => null
          },
          {
            "name": r"subTypeElt$ebnf$2$subexpression$1",
            "symbols": [
              "typeElt",
              "_",
              r"subTypeElt$ebnf$2$subexpression$1$ebnf$1",
              "_"
            ]
          },
          {
            "name": r"subTypeElt$ebnf$2",
            "symbols": [r"subTypeElt$ebnf$2$subexpression$1"]
          },
          {
            "name": r"subTypeElt$ebnf$2$subexpression$2$ebnf$1",
            "symbols": [
              {"literal": ";"}
            ],
            "postprocess": id
          },
          {
            "name": r"subTypeElt$ebnf$2$subexpression$2$ebnf$1",
            "symbols": [],
            // "postprocess": () => null
          },
          {
            "name": r"subTypeElt$ebnf$2$subexpression$2",
            "symbols": [
              "typeElt",
              "_",
              r"subTypeElt$ebnf$2$subexpression$2$ebnf$1",
              "_"
            ]
          },
          {
            "name": r"subTypeElt$ebnf$2",
            "symbols": [
              r"subTypeElt$ebnf$2",
              r"subTypeElt$ebnf$2$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "subTypeElt",
            "symbols": [
              {"literal": "[{"},
              "_",
              r"subTypeElt$ebnf$2",
              {"literal": "}]"}
            ],
            "postprocess": instructionSetToJsonSemi
          },
          {
            "name": "typeElt",
            "symbols": [
              (lexer!.has("elt") ? {'type': "elt"} : elt),
              "_",
              "typeData",
              "_",
              "typeData"
            ],
            "postprocess": doubleArgKeywordToJson
          },
          {
            "name": "subInstruction",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              "_",
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": (d) => ""
          },
          {
            "name": "subInstruction",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              "_",
              "instruction",
              "_",
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": (d) => d[2]
          },
          {
            "name": r"subInstruction$ebnf$1$subexpression$1",
            "symbols": [
              "instruction",
              "_",
              (lexer!.has("semicolon") ? {'type': "semicolon"} : semicolon),
              "_"
            ]
          },
          {
            "name": r"subInstruction$ebnf$1",
            "symbols": [r"subInstruction$ebnf$1$subexpression$1"]
          },
          {
            "name": r"subInstruction$ebnf$1$subexpression$2",
            "symbols": [
              "instruction",
              "_",
              (lexer!.has("semicolon") ? {'type': "semicolon"} : semicolon),
              "_"
            ]
          },
          {
            "name": r"subInstruction$ebnf$1",
            "symbols": [
              r"subInstruction$ebnf$1",
              r"subInstruction$ebnf$1$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "subInstruction",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              "_",
              r"subInstruction$ebnf$1",
              "instruction",
              "_",
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": instructionSetToJsonNoSemi
          },
          {
            "name": r"subInstruction$ebnf$2$subexpression$1",
            "symbols": [
              "instruction",
              "_",
              (lexer!.has("semicolon") ? {'type': "semicolon"} : semicolon),
              "_"
            ]
          },
          {
            "name": r"subInstruction$ebnf$2",
            "symbols": [r"subInstruction$ebnf$2$subexpression$1"]
          },
          {
            "name": r"subInstruction$ebnf$2$subexpression$2",
            "symbols": [
              "instruction",
              "_",
              (lexer!.has("semicolon") ? {'type': "semicolon"} : semicolon),
              "_"
            ]
          },
          {
            "name": r"subInstruction$ebnf$2",
            "symbols": [
              r"subInstruction$ebnf$2",
              r"subInstruction$ebnf$2$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "subInstruction",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              "_",
              r"subInstruction$ebnf$2",
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": instructionSetToJsonSemi
          },
          {
            "name": "instructions",
            "symbols": [
              (lexer!.has("baseInstruction")
                  ? {'type': "baseInstruction"}
                  : baseInstruction)
            ]
          },
          {
            "name": "instructions",
            "symbols": [
              (lexer!.has("macroCADR") ? {'type': "macroCADR"} : macroCADR)
            ]
          },
          {
            "name": "instructions",
            "symbols": [
              (lexer!.has("macroDIP") ? {'type': "macroDIP"} : macroDIP)
            ]
          },
          {
            "name": "instructions",
            "symbols": [
              (lexer!.has("macroDUP") ? {'type': "macroDUP"} : macroDUP)
            ]
          },
          {
            "name": "instructions",
            "symbols": [
              (lexer!.has("macroSETCADR")
                  ? {'type': "macroSETCADR"}
                  : macroSETCADR)
            ]
          },
          {
            "name": "instructions",
            "symbols": [
              (lexer!.has("macroASSERTlist")
                  ? {'type': "macroASSERTlist"}
                  : macroASSERTlist)
            ]
          },
          {
            "name": "instruction",
            "symbols": ["instructions"],
            "postprocess": keywordToJson
          },
          {
            "name": "instruction",
            "symbols": ["subInstruction"],
            "postprocess": id
          },
          {
            "name": r"instruction$ebnf$1$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$1",
            "symbols": [r"instruction$ebnf$1$subexpression$1"]
          },
          {
            "name": r"instruction$ebnf$1$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$1",
            "symbols": [
              r"instruction$ebnf$1",
              r"instruction$ebnf$1$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "instruction",
            "symbols": ["instructions", r"instruction$ebnf$1", "_"],
            "postprocess": keywordToJson
          },
          {
            "name": "instruction",
            "symbols": ["instructions", "_", "subInstruction"],
            "postprocess": singleArgInstrKeywordToJson
          },
          {
            "name": r"instruction$ebnf$2$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$2",
            "symbols": [r"instruction$ebnf$2$subexpression$1"]
          },
          {
            "name": r"instruction$ebnf$2$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$2",
            "symbols": [
              r"instruction$ebnf$2",
              r"instruction$ebnf$2$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "instruction",
            "symbols": [
              "instructions",
              r"instruction$ebnf$2",
              "_",
              "subInstruction"
            ],
            "postprocess": singleArgTypeKeywordToJson
          },
          {
            "name": "instruction",
            "symbols": ["instructions", "_", "type"],
            "postprocess": singleArgKeywordToJson
          },
          {
            "name": r"instruction$ebnf$3$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$3",
            "symbols": [r"instruction$ebnf$3$subexpression$1"]
          },
          {
            "name": r"instruction$ebnf$3$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$3",
            "symbols": [
              r"instruction$ebnf$3",
              r"instruction$ebnf$3$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "instruction",
            "symbols": ["instructions", r"instruction$ebnf$3", "_", "type"],
            "postprocess": singleArgTypeKeywordToJson
          },
          {
            "name": "instruction",
            "symbols": ["instructions", "_", "data"],
            "postprocess": singleArgKeywordToJson
          },
          {
            "name": r"instruction$ebnf$4$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$4",
            "symbols": [r"instruction$ebnf$4$subexpression$1"]
          },
          {
            "name": r"instruction$ebnf$4$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$4",
            "symbols": [
              r"instruction$ebnf$4",
              r"instruction$ebnf$4$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "instruction",
            "symbols": ["instructions", r"instruction$ebnf$4", "_", "data"],
            "postprocess": singleArgTypeKeywordToJson
          },
          {
            "name": "instruction",
            "symbols": [
              "instructions",
              "_",
              "type",
              "_",
              "type",
              "_",
              "subInstruction"
            ],
            "postprocess": tripleArgKeyWordToJson
          },
          {
            "name": r"instruction$ebnf$5$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$5",
            "symbols": [r"instruction$ebnf$5$subexpression$1"]
          },
          {
            "name": r"instruction$ebnf$5$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$5",
            "symbols": [
              r"instruction$ebnf$5",
              r"instruction$ebnf$5$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "instruction",
            "symbols": [
              "instructions",
              r"instruction$ebnf$5",
              "_",
              "type",
              "_",
              "type",
              "_",
              "subInstruction"
            ],
            "postprocess": tripleArgTypeKeyWordToJson
          },
          {
            "name": "instruction",
            "symbols": [
              "instructions",
              "_",
              "subInstruction",
              "_",
              "subInstruction"
            ],
            "postprocess": doubleArgInstrKeywordToJson
          },
          {
            "name": r"instruction$ebnf$6$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$6",
            "symbols": [r"instruction$ebnf$6$subexpression$1"]
          },
          {
            "name": r"instruction$ebnf$6$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$6",
            "symbols": [
              r"instruction$ebnf$6",
              r"instruction$ebnf$6$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "instruction",
            "symbols": [
              "instructions",
              r"instruction$ebnf$6",
              "_",
              "subInstruction",
              "_",
              "subInstruction"
            ],
            "postprocess": doubleArgTypeKeywordToJson
          },
          {
            "name": "instruction",
            "symbols": ["instructions", "_", "type", "_", "type"],
            "postprocess": doubleArgKeywordToJson
          },
          {
            "name": r"instruction$ebnf$7$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$7",
            "symbols": [r"instruction$ebnf$7$subexpression$1"]
          },
          {
            "name": r"instruction$ebnf$7$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$7",
            "symbols": [
              r"instruction$ebnf$7",
              r"instruction$ebnf$7$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "instruction",
            "symbols": [
              "instructions",
              r"instruction$ebnf$7",
              "_",
              "type",
              "_",
              "type"
            ],
            "postprocess": doubleArgTypeKeywordToJson
          },
          {
            "name": "instruction",
            "symbols": [
              {"literal": "PUSH"},
              "_",
              "type",
              "_",
              "data"
            ],
            "postprocess": doubleArgKeywordToJson
          },
          {
            "name": "instruction",
            "symbols": [
              {"literal": "PUSH"},
              "_",
              "type",
              "_",
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": pushToJson
          },
          {
            "name": r"instruction$ebnf$8$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$8",
            "symbols": [r"instruction$ebnf$8$subexpression$1"]
          },
          {
            "name": r"instruction$ebnf$8$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$8",
            "symbols": [
              r"instruction$ebnf$8",
              r"instruction$ebnf$8$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "instruction",
            "symbols": [
              {"literal": "PUSH"},
              r"instruction$ebnf$8",
              "_",
              "type",
              "_",
              "data"
            ],
            "postprocess": pushWithAnnotsToJson
          },
          {
            "name": r"instruction$ebnf$9",
            "symbols": [RegExp(r'[0-9]')]
          },
          {
            "name": r"instruction$ebnf$9",
            "symbols": [r"instruction$ebnf$9", RegExp(r'[0-9]')],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "instruction",
            "symbols": [
              {"literal": "DIP"},
              "_",
              r"instruction$ebnf$9",
              "_",
              "subInstruction"
            ],
            "postprocess": dipnToJson
          },
          {
            "name": r"instruction$ebnf$10",
            "symbols": [RegExp(r'[0-9]')]
          },
          {
            "name": r"instruction$ebnf$10",
            "symbols": [r"instruction$ebnf$10", RegExp(r'[0-9]')],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "instruction",
            "symbols": [
              {"literal": "DUP"},
              "_",
              r"instruction$ebnf$10"
            ],
            "postprocess": dupnToJson
          },
          {
            "name": "instruction",
            "symbols": [
              {"literal": "DUP"}
            ],
            "postprocess": keywordToJson
          },
          {
            "name": r"instruction$ebnf$11$subexpression$1",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$11",
            "symbols": [r"instruction$ebnf$11$subexpression$1"]
          },
          {
            "name": r"instruction$ebnf$11$subexpression$2",
            "symbols": [
              "_",
              (lexer!.has("annot") ? {'type': "annot"} : annot)
            ]
          },
          {
            "name": r"instruction$ebnf$11",
            "symbols": [
              r"instruction$ebnf$11",
              r"instruction$ebnf$11$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "instruction",
            "symbols": [
              {"literal": "DUP"},
              r"instruction$ebnf$11",
              "_"
            ],
            "postprocess": keywordToJson
          },
          {
            "name": r"instruction$ebnf$12",
            "symbols": [RegExp(r'[0-9]')]
          },
          {
            "name": r"instruction$ebnf$12",
            "symbols": [r"instruction$ebnf$12", RegExp(r'[0-9]')],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "instruction",
            "symbols": [
              {"literal": "DIG"},
              "_",
              r"instruction$ebnf$12"
            ],
            "postprocess": dignToJson
          },
          {
            "name": r"instruction$ebnf$13",
            "symbols": [RegExp(r'[0-9]')]
          },
          {
            "name": r"instruction$ebnf$13",
            "symbols": [r"instruction$ebnf$13", RegExp(r'[0-9]')],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "instruction",
            "symbols": [
              {"literal": "DUG"},
              "_",
              r"instruction$ebnf$13"
            ],
            "postprocess": dignToJson
          },
          {
            "name": r"instruction$ebnf$14",
            "symbols": [RegExp(r'[0-9]')]
          },
          {
            "name": r"instruction$ebnf$14",
            "symbols": [r"instruction$ebnf$14", RegExp(r'[0-9]')],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "instruction",
            "symbols": [
              {"literal": "DROP"},
              "_",
              r"instruction$ebnf$14"
            ],
            "postprocess": dropnToJson
          },
          {
            "name": "instruction",
            "symbols": [
              {"literal": "DROP"}
            ],
            "postprocess": keywordToJson
          },
          {
            "name": "instruction",
            "symbols": [
              {"literal": "CREATE_CONTRACT"},
              "_",
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              "_",
              "parameter",
              "_",
              "storage",
              "_",
              "code",
              "_",
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": subContractToJson
          },
          {
            "name": "instruction",
            "symbols": [
              {"literal": "EMPTY_MAP"},
              "_",
              "type",
              "_",
              "type"
            ],
            "postprocess": doubleArgKeywordToJson
          },
          {
            "name": "instruction",
            "symbols": [
              {"literal": "EMPTY_MAP"},
              "_",
              (lexer!.has("lparen") ? {'type': "lparen"} : lparen),
              "_",
              "type",
              "_",
              (lexer!.has("rparen") ? {'type': "rparen"} : rparen),
              "_",
              "type"
            ],
            "postprocess": doubleArgParenKeywordToJson
          },
          {"name": r"_$ebnf$1", "symbols": []},
          {
            "name": r"_$ebnf$1",
            "symbols": [r"_$ebnf$1", RegExp(r'[\s]')],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "_",
            "symbols": [r"_$ebnf$1"]
          },
          {
            "name": r"semicolons$ebnf$1",
            "symbols": [new RegExp('[;]')],
            "postprocess": id
          },
          {
            "name": r"semicolons$ebnf$1",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": "semicolons",
            "symbols": [r"semicolons$ebnf$1"]
          }
        ],
        'ParserStart': "main"
      };

  arrpush(d) => d[0].addAll([d[1]]).toList();

  joiner(d) => d.join('');
}
