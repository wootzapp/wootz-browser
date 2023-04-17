import 'package:dartez/chain/tezos/tezos_message_utils.dart';
import 'package:dartez/michelson_parser/grammar/michelin_grammar_tokenizer.dart';

class MichelineGrammar {
  MichelinGrammarTokenizer? lexer;
  var lbrace, _, colon, quotedValue, rbrace, comma, lbracket, rbracket;
  final List defaultMichelsonKeywords = [
    '"parameter"',
    '"storage"',
    '"code"',
    '"False"',
    '"Elt"',
    '"Left"',
    '"None"',
    '"Pair"',
    '"Right"',
    '"Some"',
    '"True"',
    '"Unit"',
    '"PACK"',
    '"UNPACK"',
    '"BLAKE2B"',
    '"SHA256"',
    '"SHA512"',
    '"ABS"',
    '"ADD"',
    '"AMOUNT"',
    '"AND"',
    '"BALANCE"',
    '"CAR"',
    '"CDR"',
    '"CHECK_SIGNATURE"',
    '"COMPARE"',
    '"CONCAT"',
    '"CONS"',
    '"CREATE_ACCOUNT"',
    '"CREATE_CONTRACT"',
    '"IMPLICIT_ACCOUNT"',
    '"DIP"',
    '"DROP"',
    '"DUP"',
    '"EDIV"',
    '"EMPTY_MAP"',
    '"EMPTY_SET"',
    '"EQ"',
    '"EXEC"',
    '"FAILWITH"',
    '"GE"',
    '"GET"',
    '"GT"',
    '"HASH_KEY"',
    '"IF"',
    '"IF_CONS"',
    '"IF_LEFT"',
    '"IF_NONE"',
    '"INT"',
    '"LAMBDA"',
    '"LE"',
    '"LEFT"',
    '"LOOP"',
    '"LSL"',
    '"LSR"',
    '"LT"',
    '"MAP"',
    '"MEM"',
    '"MUL"',
    '"NEG"',
    '"NEQ"',
    '"NIL"',
    '"NONE"',
    '"NOT"',
    '"NOW"',
    '"OR"',
    '"PAIR"',
    '"PUSH"',
    '"RIGHT"',
    '"SIZE"',
    '"SOME"',
    '"SOURCE"',
    '"SENDER"',
    '"SELF"',
    '"STEPS_TO_QUOTA"',
    '"SUB"',
    '"SWAP"',
    '"TRANSFER_TOKENS"',
    '"SET_DELEGATE"',
    '"UNIT"',
    '"UPDATE"',
    '"XOR"',
    '"ITER"',
    '"LOOP_LEFT"',
    '"ADDRESS"',
    '"CONTRACT"',
    '"ISNAT"',
    '"CAST"',
    '"RENAME"',
    '"bool"',
    '"contract"',
    '"int"',
    '"key"',
    '"key_hash"',
    '"lambda"',
    '"list"',
    '"map"',
    '"big_map"',
    '"nat"',
    '"option"',
    '"or"',
    '"pair"',
    '"set"',
    '"signature"',
    '"string"',
    '"bytes"',
    '"mutez"',
    '"timestamp"',
    '"unit"',
    '"operation"',
    '"address"',
    '"SLICE"',
    '"DIG"',
    '"DUG"',
    '"EMPTY_BIG_MAP"',
    '"APPLY"',
    '"chain_id"',
    '"CHAIN_ID"'
  ];
  late List languageKeywords;

  MichelineGrammar() {
    languageKeywords = defaultMichelsonKeywords;
    lexer = MichelinGrammarTokenizer({
      'lbrace': '{',
      'rbrace': '}',
      'lbracket': '[',
      'rbracket': ']',
      'colon': ":",
      'comma': ",",
      '_': RegExp(r'[ \t]+'),
      'quotedValue': RegExp(r'"(?:\\["\\]|[^\n"\\])*"')
    });
  }

  id(d) {
    return d[0];
  }

  void setKeywordList(List list) {
    languageKeywords = list;
  }

  int getCodeForKeyword(String? word) {
    return languageKeywords.indexOf(word);
  }

  String? getKeywordForWord(int index) {
    return languageKeywords[index];
  }

  String staticIntToHex(d) {
    final prefix = '00';
    final String text = getMapValue(d[6]);
    final value = TezosMessageUtils.writeSignedInt(
        int.parse(text.substring(1, text.length - 1)));
    return prefix + value;
  }

  String staticStringToHex(d) {
    final String prefix = '01';
    var text = getMapValue(d[6]);
    text = text.substring(1, text.length - 1);
    text = text.replaceAll(RegExp(r'\\"'), '"');
    final len = encodeLength(text.length);
    text = text
        .split('')
        .map((c) => c.codeUnitAt(0).toRadixString(16).toString())
        .join('');
    return prefix + len + text;
  }

  String staticBytesToHex(d) {
    var prefix = '0a';
    var bytes = getMapValue(d[6]);
    bytes = bytes.substring(1, bytes.length - 1);
    var len = encodeLength(bytes.length ~/ 2);
    return prefix + len + bytes;
  }

  removeSameMatched(list) {
    var newArr = [
      [-1]
    ];
    for (var i = 0; i < list.length; i++) {
      if (list[i][0] != newArr[newArr.length - 1][0]) {
        newArr.add(list[i]);
      }
    }
    return newArr..removeAt(0);
  }

  removeIfNextIsSame(list) {
    for (var i = 0; i < list.length; i++) {
      if (i != 0 && list[i - 1][0] == list[i][0]) {
        list.removeAt(i);
      }
    }
    return list;
  }

  String staticArrayToHex(List d) {
    List matchedArray = d[2];
    matchedArray = removeIfNextIsSame(removeIfNextIsSame(matchedArray));
    final String prefix = '02';
    String content = matchedArray.map((e) => e[0]).join('');
    final len = encodeLength(int.parse((content.length ~/ 2).toString()));
    return prefix + len + content;
  }

  String primBareToHex(d) {
    final String prefix = '03';
    final String prim = encodePrimitive(getMapValue(d[6]));
    return prefix + prim;
  }

  String primAnnToHex(List d) {
    final String prefix = '04';
    final String prim = encodePrimitive(getMapValue(d[6]));
    String ann = d[15].map((e) {
      String t = getMapValue(e[0]);
      t = t.substring(1, t.length - 1);
      return t;
    }).join('');
    ann = ann
        .split('')
        .map((e) => e.codeUnitAt(0).toRadixString(16).toString())
        .join('');
    ann = encodeLength(int.parse((ann.length ~/ 2).toString())) + ann;
    return prefix + prim + ann;
  }

  String primArgToHex(List d) {
    String prefix = '05';
    if (d[15].length > 2) {
      for (var i = 0; i < d[15].length; i++) {
        if (i != 0 && d[15][i - 1][0] == d[15][i][0]) {
          d[15].removeAt(i);
        }
      }
    }
    if (d[15].length == 2) {
      prefix = '07';
    } else {
      if (d[15].length > 2) {
        prefix = '09';
      }
    }
    final String prim = encodePrimitive(getMapValue(d[6]));
    String? args = d[15].map((e) => e[0]).join('');
    String? newArgs = '';
    if (prefix == '09') {
      newArgs = '0000000' +
          int.parse((args!.length ~/ 2).toString())
              .toRadixString(16)
              .toString();
      newArgs = newArgs.substring(newArgs.length - 8);
      newArgs = newArgs + args + '00000000';
    }
    newArgs = newArgs == '' ? args! : newArgs;
    return prefix + prim + newArgs;
  }

  String primArgAnnToHex(List d) {
    String prefix = '06';
    if (d[15].length > 2) {
      for (var i = 0; i < d[15].length; i++) {
        if (i != 0 && d[15][i - 1][0] == d[15][i][0]) {
          d[15].removeAt(i);
        }
      }
    }
    if (d[15].length == 2) {
      prefix = '08';
    } else if (d[15].length > 2) {
      prefix = '09';
    }
    String prim = encodePrimitive(getMapValue(d[6]));
    String? args = d[15].map((v) => v[0]).join('');
    String ann = d[26].map((v) {
      String t = getMapValue(v[0]);
      t = t.substring(1, t.length - 1);
      return t;
    }).join(' ');
    ann = ann.split('').map((e) {
      String d = e.codeUnitAt(0).toRadixString(16).toString();
      return d;
    }).join('');
    ann = encodeLength(int.parse((ann.length ~/ 2).toString())) + ann;
    String? newArgs = '';
    if (prefix == '09') {
      newArgs = '0000000' +
          int.parse((args!.length ~/ 2).toString())
              .toRadixString(16)
              .toString();
      newArgs = newArgs.substring(newArgs.length - 8);
      newArgs = newArgs + args;
    }
    newArgs = newArgs == '' ? args! : newArgs;
    return prefix + prim + newArgs + ann;
  }

  String encodePrimitive(String? p) {
    String result = '00' + getCodeForKeyword(p).toRadixString(16).toString();
    result = result.substring(result.length - 2);
    return result;
  }

  String encodeLength(int l) {
    String output = '0000000' + l.toRadixString(16).toString();
    return output.substring(output.length - 8);
  }

  // String writeSignedInt(int value) {
  //   if (value == 0) {
  //     return '00';
  //   }
  //   final BigInt n = BigInt.from(value).abs();
  //   final l = n.bitLength.toInt();
  //   List arr = [];
  //   BigInt v = n;
  //   for (var i = 0; i < l; i += 7) {
  //     BigInt byte = BigInt.zero;
  //     if (i == 0) {
  //       byte = v & BigInt.from(0x3f);
  //       v = v >> 6;
  //     } else {
  //       byte = v & BigInt.from(0x7f);
  //       v = v >> 7;
  //     }

  //     if (value < 0 && i == 0) {
  //       byte = byte | BigInt.from(0x40);
  //     }

  //     if (i + 7 < l) {
  //       byte = byte | BigInt.from(0x80);
  //     }
  //     arr.add(byte.toInt());
  //   }

  //   if (l % 7 == 0) {
  //     arr[arr.length - 1] =
  //         arr[arr.length - 1] == null ? 0x80 : arr[arr.length - 1];
  //     arr.add(1);
  //   }

  //   var output = arr.map((v) {
  //     int newNum = int.parse(v.toString());
  //     var str = '0' + newNum.toRadixString(16).toString();
  //     str = str.substring(str.length - 2);
  //     return str;
  //   }).join('');
  //   return output;
  // }

  getMapValue(d) {
    return d is List ? d[0]['value'].toString() : d['value'];
  }

  Map<String, dynamic> get grammar => {
        'Lexer': lexer,
        'ParserRules': [
          {
            "name": "main",
            "symbols": ["staticObject"],
            "postprocess": id
          },
          {
            "name": "main",
            "symbols": ["primBare"],
            "postprocess": id
          },
          {
            "name": "main",
            "symbols": ["primArg"],
            "postprocess": id
          },
          {
            "name": "main",
            "symbols": ["primAnn"],
            "postprocess": id
          },
          {
            "name": "main",
            "symbols": ["primArgAnn"],
            "postprocess": id
          },
          {
            "name": "main",
            "symbols": ["anyArray"],
            "postprocess": id
          },
          {"name": r"staticInt$ebnf$1", "symbols": []},
          {
            "name": r"staticInt$ebnf$1",
            "symbols": [
              r"staticInt$ebnf$1",
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "staticInt",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              (lexer!.has("_") ? {'type': "_"} : _),
              {"literal": "\"int\""},
              r"staticInt$ebnf$1",
              (lexer!.has("colon") ? {'type': "colon"} : colon),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("quotedValue")
                  ? {'type': "quotedValue"}
                  : quotedValue),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": staticIntToHex
          },
          {"name": r"staticString$ebnf$1", "symbols": []},
          {
            "name": r"staticString$ebnf$1",
            "symbols": [
              r"staticString$ebnf$1",
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "staticString",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              (lexer!.has("_") ? {'type': "_"} : _),
              {"literal": "\"string\""},
              r"staticString$ebnf$1",
              (lexer!.has("colon") ? {'type': "colon"} : colon),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("quotedValue")
                  ? {'type': "quotedValue"}
                  : quotedValue),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": staticStringToHex
          },
          {"name": r"staticBytes$ebnf$1", "symbols": []},
          {
            "name": r"staticBytes$ebnf$1",
            "symbols": [
              r"staticBytes$ebnf$1",
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "staticBytes",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              (lexer!.has("_") ? {'type': "_"} : _),
              {"literal": "\"bytes\""},
              r"staticBytes$ebnf$1",
              (lexer!.has("colon") ? {'type': "colon"} : colon),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("quotedValue")
                  ? {'type': "quotedValue"}
                  : quotedValue),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": staticBytesToHex
          },
          {
            "name": "staticObject",
            "symbols": ["staticInt"],
            "postprocess": id
          },
          {
            "name": "staticObject",
            "symbols": ["staticString"],
            "postprocess": id
          },
          {
            "name": "staticObject",
            "symbols": ["staticBytes"],
            "postprocess": id
          },
          {"name": r"primBare$ebnf$1", "symbols": []},
          {
            "name": r"primBare$ebnf$1",
            "symbols": [
              r"primBare$ebnf$1",
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "primBare",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              (lexer!.has("_") ? {'type': "_"} : _),
              {"literal": "\"prim\""},
              r"primBare$ebnf$1",
              (lexer!.has("colon") ? {'type': "colon"} : colon),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("quotedValue")
                  ? {'type': "quotedValue"}
                  : quotedValue),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": primBareToHex
          },
          {
            "name": r"primArg$ebnf$1",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primArg$ebnf$1",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArg$ebnf$2",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primArg$ebnf$2",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArg$ebnf$3$subexpression$1$ebnf$1",
            "symbols": [
              (lexer!.has("comma") ? {'type': "comma"} : comma)
            ],
            "postprocess": id
          },
          {
            "name": r"primArg$ebnf$3$subexpression$1$ebnf$1",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArg$ebnf$3$subexpression$1$ebnf$2",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primArg$ebnf$3$subexpression$1$ebnf$2",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArg$ebnf$3$subexpression$1",
            "symbols": [
              "any",
              r"primArg$ebnf$3$subexpression$1$ebnf$1",
              r"primArg$ebnf$3$subexpression$1$ebnf$2"
            ]
          },
          {
            "name": r"primArg$ebnf$3",
            "symbols": [r"primArg$ebnf$3$subexpression$1"]
          },
          {
            "name": r"primArg$ebnf$3$subexpression$2$ebnf$1",
            "symbols": [
              (lexer!.has("comma") ? {'type': "comma"} : comma)
            ],
            "postprocess": id
          },
          {
            "name": r"primArg$ebnf$3$subexpression$2$ebnf$1",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArg$ebnf$3$subexpression$2$ebnf$2",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primArg$ebnf$3$subexpression$2$ebnf$2",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArg$ebnf$3$subexpression$2",
            "symbols": [
              "any",
              r"primArg$ebnf$3$subexpression$2$ebnf$1",
              r"primArg$ebnf$3$subexpression$2$ebnf$2"
            ]
          },
          {
            "name": r"primArg$ebnf$3",
            "symbols": [r"primArg$ebnf$3", r"primArg$ebnf$3$subexpression$2"],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "primArg",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              (lexer!.has("_") ? {'type': "_"} : _),
              {"literal": "\"prim\""},
              r"primArg$ebnf$1",
              (lexer!.has("colon") ? {'type': "colon"} : colon),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("quotedValue")
                  ? {'type': "quotedValue"}
                  : quotedValue),
              (lexer!.has("comma") ? {'type': "comma"} : comma),
              (lexer!.has("_") ? {'type': "_"} : _),
              {"literal": "\"args\""},
              r"primArg$ebnf$2",
              (lexer!.has("colon") ? {'type': "colon"} : colon),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("lbracket") ? {'type': "lbracket"} : lbracket),
              (lexer!.has("_") ? {'type': "_"} : _),
              r"primArg$ebnf$3",
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("rbracket") ? {'type': "rbracket"} : rbracket),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": primArgToHex
          },
          {
            "name": r"primAnn$ebnf$1",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primAnn$ebnf$1",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primAnn$ebnf$2",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primAnn$ebnf$2",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primAnn$ebnf$3$subexpression$1$ebnf$1",
            "symbols": [
              (lexer!.has("comma") ? {'type': "comma"} : comma)
            ],
            "postprocess": id
          },
          {
            "name": r"primAnn$ebnf$3$subexpression$1$ebnf$1",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primAnn$ebnf$3$subexpression$1$ebnf$2",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primAnn$ebnf$3$subexpression$1$ebnf$2",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primAnn$ebnf$3$subexpression$1",
            "symbols": [
              (lexer!.has("quotedValue")
                  ? {'type': "quotedValue"}
                  : quotedValue),
              r"primAnn$ebnf$3$subexpression$1$ebnf$1",
              r"primAnn$ebnf$3$subexpression$1$ebnf$2"
            ]
          },
          {
            "name": r"primAnn$ebnf$3",
            "symbols": [r"primAnn$ebnf$3$subexpression$1"]
          },
          {
            "name": r"primAnn$ebnf$3$subexpression$2$ebnf$1",
            "symbols": [
              (lexer!.has("comma") ? {'type': "comma"} : comma)
            ],
            "postprocess": id
          },
          {
            "name": r"primAnn$ebnf$3$subexpression$2$ebnf$1",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primAnn$ebnf$3$subexpression$2$ebnf$2",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primAnn$ebnf$3$subexpression$2$ebnf$2",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primAnn$ebnf$3$subexpression$2",
            "symbols": [
              (lexer!.has("quotedValue")
                  ? {'type': "quotedValue"}
                  : quotedValue),
              r"primAnn$ebnf$3$subexpression$2$ebnf$1",
              r"primAnn$ebnf$3$subexpression$2$ebnf$2"
            ]
          },
          {
            "name": r"primAnn$ebnf$3",
            "symbols": [r"primAnn$ebnf$3", r"primAnn$ebnf$3$subexpression$2"],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "primAnn",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              (lexer!.has("_") ? {'type': "_"} : _),
              {"literal": "\"prim\""},
              r"primAnn$ebnf$1",
              (lexer!.has("colon") ? {'type': "colon"} : colon),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("quotedValue")
                  ? {'type': "quotedValue"}
                  : quotedValue),
              (lexer!.has("comma") ? {'type': "comma"} : comma),
              (lexer!.has("_") ? {'type': "_"} : _),
              {"literal": "\"annots\""},
              r"primAnn$ebnf$2",
              (lexer!.has("colon") ? {'type': "colon"} : colon),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("lbracket") ? {'type': "lbracket"} : lbracket),
              (lexer!.has("_") ? {'type': "_"} : _),
              r"primAnn$ebnf$3",
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("rbracket") ? {'type': "rbracket"} : rbracket),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": primAnnToHex
          },
          {
            "name": r"primArgAnn$ebnf$1",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primArgAnn$ebnf$1",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArgAnn$ebnf$2",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primArgAnn$ebnf$2",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArgAnn$ebnf$3$subexpression$1$ebnf$1",
            "symbols": [
              (lexer!.has("comma") ? {'type': "comma"} : comma)
            ],
            "postprocess": id
          },
          {
            "name": r"primArgAnn$ebnf$3$subexpression$1$ebnf$1",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArgAnn$ebnf$3$subexpression$1$ebnf$2",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primArgAnn$ebnf$3$subexpression$1$ebnf$2",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArgAnn$ebnf$3$subexpression$1",
            "symbols": [
              "any",
              r"primArgAnn$ebnf$3$subexpression$1$ebnf$1",
              r"primArgAnn$ebnf$3$subexpression$1$ebnf$2"
            ]
          },
          {
            "name": r"primArgAnn$ebnf$3",
            "symbols": [r"primArgAnn$ebnf$3$subexpression$1"]
          },
          {
            "name": r"primArgAnn$ebnf$3$subexpression$2$ebnf$1",
            "symbols": [
              (lexer!.has("comma") ? {'type': "comma"} : comma)
            ],
            "postprocess": id
          },
          {
            "name": r"primArgAnn$ebnf$3$subexpression$2$ebnf$1",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArgAnn$ebnf$3$subexpression$2$ebnf$2",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primArgAnn$ebnf$3$subexpression$2$ebnf$2",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArgAnn$ebnf$3$subexpression$2",
            "symbols": [
              "any",
              r"primArgAnn$ebnf$3$subexpression$2$ebnf$1",
              r"primArgAnn$ebnf$3$subexpression$2$ebnf$2"
            ]
          },
          {
            "name": r"primArgAnn$ebnf$3",
            "symbols": [
              r"primArgAnn$ebnf$3",
              r"primArgAnn$ebnf$3$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": r"primArgAnn$ebnf$4",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primArgAnn$ebnf$4",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArgAnn$ebnf$5$subexpression$1$ebnf$1",
            "symbols": [
              (lexer!.has("comma") ? {'type': "comma"} : comma)
            ],
            "postprocess": id
          },
          {
            "name": r"primArgAnn$ebnf$5$subexpression$1$ebnf$1",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArgAnn$ebnf$5$subexpression$1$ebnf$2",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primArgAnn$ebnf$5$subexpression$1$ebnf$2",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArgAnn$ebnf$5$subexpression$1",
            "symbols": [
              (lexer!.has("quotedValue")
                  ? {'type': "quotedValue"}
                  : quotedValue),
              r"primArgAnn$ebnf$5$subexpression$1$ebnf$1",
              r"primArgAnn$ebnf$5$subexpression$1$ebnf$2"
            ]
          },
          {
            "name": r"primArgAnn$ebnf$5",
            "symbols": [r"primArgAnn$ebnf$5$subexpression$1"]
          },
          {
            "name": r"primArgAnn$ebnf$5$subexpression$2$ebnf$1",
            "symbols": [
              (lexer!.has("comma") ? {'type': "comma"} : comma)
            ],
            "postprocess": id
          },
          {
            "name": r"primArgAnn$ebnf$5$subexpression$2$ebnf$1",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArgAnn$ebnf$5$subexpression$2$ebnf$2",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"primArgAnn$ebnf$5$subexpression$2$ebnf$2",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"primArgAnn$ebnf$5$subexpression$2",
            "symbols": [
              (lexer!.has("quotedValue")
                  ? {'type': "quotedValue"}
                  : quotedValue),
              r"primArgAnn$ebnf$5$subexpression$2$ebnf$1",
              r"primArgAnn$ebnf$5$subexpression$2$ebnf$2"
            ]
          },
          {
            "name": r"primArgAnn$ebnf$5",
            "symbols": [
              r"primArgAnn$ebnf$5",
              r"primArgAnn$ebnf$5$subexpression$2"
            ],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "primArgAnn",
            "symbols": [
              (lexer!.has("lbrace") ? {'type': "lbrace"} : lbrace),
              (lexer!.has("_") ? {'type': "_"} : _),
              {"literal": "\"prim\""},
              r"primArgAnn$ebnf$1",
              (lexer!.has("colon") ? {'type': "colon"} : colon),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("quotedValue")
                  ? {'type': "quotedValue"}
                  : quotedValue),
              (lexer!.has("comma") ? {'type': "comma"} : comma),
              (lexer!.has("_") ? {'type': "_"} : _),
              {"literal": "\"args\""},
              r"primArgAnn$ebnf$2",
              (lexer!.has("colon") ? {'type': "colon"} : colon),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("lbracket") ? {'type': "lbracket"} : lbracket),
              (lexer!.has("_") ? {'type': "_"} : _),
              r"primArgAnn$ebnf$3",
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("rbracket") ? {'type': "rbracket"} : rbracket),
              (lexer!.has("comma") ? {'type': "comma"} : comma),
              (lexer!.has("_") ? {'type': "_"} : _),
              {"literal": "\"annots\""},
              r"primArgAnn$ebnf$4",
              (lexer!.has("colon") ? {'type': "colon"} : colon),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("lbracket") ? {'type': "lbracket"} : lbracket),
              (lexer!.has("_") ? {'type': "_"} : _),
              r"primArgAnn$ebnf$5",
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("rbracket") ? {'type': "rbracket"} : rbracket),
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("rbrace") ? {'type': "rbrace"} : rbrace)
            ],
            "postprocess": primArgAnnToHex
          },
          {
            "name": "primAny",
            "symbols": ["primBare"],
            "postprocess": id
          },
          {
            "name": "primAny",
            "symbols": ["primArg"],
            "postprocess": id
          },
          {
            "name": "primAny",
            "symbols": ["primAnn"],
            "postprocess": id
          },
          {
            "name": "primAny",
            "symbols": ["primArgAnn"],
            "postprocess": id
          },
          {
            "name": "any",
            "symbols": ["primAny"],
            "postprocess": id
          },
          {
            "name": "any",
            "symbols": ["staticObject"],
            "postprocess": id
          },
          {
            "name": "any",
            "symbols": ["anyArray"],
            "postprocess": id
          },
          {
            "name": "anyArray",
            "symbols": [
              (lexer!.has("lbracket") ? {'type': "lbracket"} : lbracket),
              (lexer!.has("rbracket") ? {'type': "rbracket"} : rbracket)
            ],
            "postprocess": (d) => '0200000000'
          },
          {
            "name": r"anyArray$ebnf$1$subexpression$1$ebnf$1",
            "symbols": [
              (lexer!.has("comma") ? {'type': "comma"} : comma)
            ],
            "postprocess": id
          },
          {
            "name": r"anyArray$ebnf$1$subexpression$1$ebnf$1",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"anyArray$ebnf$1$subexpression$1$ebnf$2",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"anyArray$ebnf$1$subexpression$1$ebnf$2",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"anyArray$ebnf$1$subexpression$1",
            "symbols": [
              "any",
              r"anyArray$ebnf$1$subexpression$1$ebnf$1",
              r"anyArray$ebnf$1$subexpression$1$ebnf$2"
            ]
          },
          {
            "name": r"anyArray$ebnf$1",
            "symbols": [r"anyArray$ebnf$1$subexpression$1"]
          },
          {
            "name": r"anyArray$ebnf$1$subexpression$2$ebnf$1",
            "symbols": [
              (lexer!.has("comma") ? {'type': "comma"} : comma)
            ],
            "postprocess": id
          },
          {
            "name": r"anyArray$ebnf$1$subexpression$2$ebnf$1",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"anyArray$ebnf$1$subexpression$2$ebnf$2",
            "symbols": [
              (lexer!.has("_") ? {'type': "_"} : _)
            ],
            "postprocess": id
          },
          {
            "name": r"anyArray$ebnf$1$subexpression$2$ebnf$2",
            "symbols": [],
            "postprocess": (d) => null
          },
          {
            "name": r"anyArray$ebnf$1$subexpression$2",
            "symbols": [
              "any",
              r"anyArray$ebnf$1$subexpression$2$ebnf$1",
              r"anyArray$ebnf$1$subexpression$2$ebnf$2"
            ]
          },
          {
            "name": r"anyArray$ebnf$1",
            "symbols": [r"anyArray$ebnf$1", r"anyArray$ebnf$1$subexpression$2"],
            "postprocess": (d) => d[0]..addAll([d[1]])
          },
          {
            "name": "anyArray",
            "symbols": [
              (lexer!.has("lbracket") ? {'type': "lbracket"} : lbracket),
              (lexer!.has("_") ? {'type': "_"} : _),
              r"anyArray$ebnf$1",
              (lexer!.has("_") ? {'type': "_"} : _),
              (lexer!.has("rbracket") ? {'type': "rbracket"} : rbracket)
            ],
            "postprocess": staticArrayToHex
          }
        ],
        'ParserStart': "main",
      };
}
