import 'dart:math';

class MichelsonGrammarTokenizer {
  Map<String, dynamic> delimiters;
  List<GrammarResultModel> tokens = [];
  late var buffer;
  late int index;
  var line;
  var lastLineBreak;

  MichelsonGrammarTokenizer(this.delimiters) : super();

  feed(String text) {
    tokenize(text);
    return tokens;
  }

  var numindex = 0;

  next() {
    if (tokens.isEmpty) feed(this.buffer.toString());
    var result = tokens.length > numindex
        ? {
            "type": tokens[numindex].type,
            "value": tokens[numindex].value,
            "text": tokens[numindex].value,
            "col": tokens[numindex].columnNumber,
            "line": '1',
          }
        : null;
    numindex++;
    return result;
  }

  has(tokenType) {
    return true;
  }

  reset(data, {state}) {
    this.buffer = data;
    this.index = 0;
    this.line = state != null ? state.line : 1;
    this.lastLineBreak = state != null ? -state.col : 0;
  }

  save() {
    return {
      'line': this.line,
      'col': this.index - this.lastLineBreak,
    };
  }

  formatError(token, message) {
    var value = token['text'];
    int index = token['col'];
    var start = max(0, index - token['col']);
    var firstLine = this.buffer.substring(start, index + value.length);
    message += " at line " +
        token['line'] +
        " col " +
        token['col'].toString() +
        ":\n\n";
    message += "  " + firstLine + "\n";
    message +=
        "  " + List.generate(token['col'] + 1, (e) => '').join(" ") + "^";
    return message;
  }

  tokenize(String chunk) {
    for (int i = 0; i < chunk.length; i++) {
      final char = chunk[i];
      if (char == ' ') {
        var argSeq = getKeyFromValue(char);
        tokens.add(GrammarResultModel(argSeq.key, char)..columnNumber = i);
        continue;
      }
      if (isArrayOrStringContaines(char, isRegexCheck: true)) {
        var seq = char;
        for (int j = i + 1; j < i + 55; j++) {
          if (j >= chunk.length) break;
          if (isArrayOrStringContaines(seq) && chunk[j] != '_') {
            var __seq = chunk[j];
            for (var i = 1; i < 10; i++) {
              try {
                var ele = chunk[j + i];
                if (isArrayOrStringContaines(seq + __seq)) {
                  seq += __seq;
                  break;
                }
                __seq += ele;
              } catch (e) {}
            }
            var argSeq = getKeyFromValue(seq);
            tokens.add(GrammarResultModel(argSeq.key, seq)..columnNumber = i);
            i += (seq.length - 1);
            seq = '';
            break;
          }
          seq += chunk[j];
        }
        if (seq.isNotEmpty) {
          if (tokens.length > 0 &&
              tokens.last.type == 'word' &&
              getKeyFromValue(char, isRegex: false) == null) {
            tokens.last = tokens.last..value += char;
          } else if (tokens.length > 0 &&
              tokens.last.type == 'number' &&
              getKeyFromValue(char, isRegex: true).key == "number") {
            tokens.last = tokens.last..value += char;
          } else if (isArrayOrStringContaines(seq)) {
            var argSeq = getKeyFromValue(seq);
            tokens.add(GrammarResultModel(argSeq.key, seq)..columnNumber = i);
            i += (seq.length - 1);
          } else {
            if (seq != char &&
                isArrayOrStringContaines(
                    seq.substring(0,
                        seq.indexOf(' ') == -1 ? seq.length : seq.indexOf(' ')),
                    isRegexCheck: true) &&
                seq.length > 10) {
              var argSeq = getKeyFromValue(
                  seq.substring(0,
                      seq.indexOf(' ') == -1 ? seq.length : seq.indexOf(' ')),
                  isRegex: true);
              if (argSeq.key == 'number')
                while (seq != '' &&
                    !isNumeric(
                        seq.substring(seq.length - 2 < 0 ? 0 : seq.length - 2)))
                  seq = seq.substring(0, seq.length - 1);
              tokens.add(GrammarResultModel(
                  argSeq.key,
                  seq.substring(
                      0,
                      seq.indexOf(' ') == -1
                          ? seq.length
                          : seq.indexOf(')') != -1
                              ? seq.indexOf(')') < seq.indexOf(' ')
                                  ? seq.indexOf(')')
                                  : seq.indexOf(' ')
                              : seq.indexOf(' ')))
                ..columnNumber = i);
              i += (seq
                      .substring(
                          0,
                          seq.indexOf(' ') == -1
                              ? seq.length
                              : seq.indexOf(')') != -1
                                  ? seq.indexOf(')') < seq.indexOf(' ')
                                      ? seq.indexOf(')')
                                      : seq.indexOf(' ')
                                  : seq.indexOf(' '))
                      .length -
                  1);
            } else if (char != '%') {
              var argSeq = getKeyFromValue(char);
              tokens
                  .add(GrammarResultModel(argSeq.key, char)..columnNumber = i);
            }
          }
        }
      } else if (char == '%' || char == ':') {
        var nextIndex = i + 30;
        if (nextIndex >= chunk.length - 1) nextIndex = chunk.length - 1;
        var data = chunk.substring(
          i,
          nextIndex,
        );
        var endIndex = getEndIndexOfVar(data);
        tokens.add(GrammarResultModel('annot', data.substring(0, endIndex))
          ..columnNumber = i);
        i += endIndex - 1;
      } else if (char == '"') {
        var nextIndex = i + 200;
        if (nextIndex >= chunk.length - 1) nextIndex = chunk.length;
        var data = chunk.substring(
          i + 1,
          nextIndex,
        );
        var rs = chunk.substring(i, i + data.indexOf('"') + 2);
        tokens.add(GrammarResultModel('string', rs)..columnNumber = i);
        i += rs.length - 1;
      } else {
        print("No match found ==> $char");
      }
    }
  }

  bool isNumeric(String s) {
    if (s == '') {
      return false;
    }
    return double.tryParse(s) != null;
  }

  int getEndIndexOfVar(String text) {
    for (var i = 0; i < text.length; i++) {
      if (isArrayOrStringContaines(text[i]) || text[i] == ' ') {
        return i;
      }
    }
    return text.length;
  }

  bool isArrayOrStringContaines(char, {isRegexCheck = false}) {
    if (delimiters.values.contains(char)) return true;
    var result = false;
    var _keys = delimiters.keys.toList();
    for (var i = 0; i < _keys.length; i++) {
      var value = delimiters[_keys[i]];
      if (value is List) {
        result = value.contains(char);
        if (result) break;
      }
      if (value is RegExp && isRegexCheck) {
        result = value.hasMatch(char);
        if (result) break;
      }
    }

    return result;
  }

  getKeyFromValue(String char, {isRegex = true}) {
    var result;
    for (var data in delimiters.entries) {
      var isContain = data.value is List
          ? data.value.contains(char)
          : data.value is RegExp && isRegex
              ? (data.value as RegExp).hasMatch(char)
              : data.value == char;
      if (isContain) result = data;
      if (result != null) break;
    }
    return result;
  }
}

class GrammarResultModel {
  String? type;
  String value;
  int? columnNumber;

  GrammarResultModel(this.type, this.value);

  toJson() => {'type': type, 'value': value};
}
