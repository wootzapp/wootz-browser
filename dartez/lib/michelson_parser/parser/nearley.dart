import 'dart:convert';
import 'dart:math';

import 'package:dartez/michelson_parser/grammar/michelson_grammar_tokenizer.dart';

var fail = {};

class Nearley {
  NearleyGrammar? grammar;
  var options;

  var lexer;
  var lexerState;
  late List<Column?> table;

  int? current;
  late var results;

  static NearleyGrammar fromCompiled(Map<String, dynamic> rules, {start}) {
    var lexer = rules['Lexer'];
    start = rules['ParserStart'];
    var redefinedRules = rules['ParserRules'].toList();
    var _rules = redefinedRules.map<Rule>((r) {
      return (new Rule(r['name'], r['symbols'], r['postprocess']));
    }).toList();
    var g = NearleyGrammar(_rules, start);
    g.lexer = lexer;
    return g;
  }

  void parser(NearleyGrammar rules, {start, options}) {
    grammar = rules;

    options = {
      'keepHistory': false,
      'lexer': grammar!.lexer ?? new StreamLexer(),
    };

    lexer = options['lexer'];
    lexerState = null;

    var column = new Column(grammar, 0);
    column.wants![grammar!.start] = [];
    column.predict(grammar!.start);
    column.process();
    table = [column];
    current = 0;
  }

  feed(chunk) {
    var lexer = this.lexer;
    lexer.tokens = <GrammarResultModel>[];
    lexer.numindex = 0;
    lexer.reset(chunk);
    var token = lexer.next();
    var column;
    while (token != null) {
      column = table[current!];

      if (current != 0) table[current! - 1] = null;

      var n = current! + 1;
      var nextColumn = new Column(grammar, n);

      var literal = token['text'] != null ? token['text'] : token['value'];
      var value = lexer is StreamLexer ? token['value'] : token;
      var scannable = column.scannable;
      for (var w = scannable.length - 1; 0 <= w; w--) {
        var state = scannable[w];
        var expect = state.rule.symbols[state.dot];

        if (expect is RegExp
            ? expect.hasMatch(value['value'])
            : expect['type'] != null
                ? expect['type'] == token['type']
                : expect['literal'] == literal) {
          var next = state.nextState({
            'data': value,
            'token': token,
            'isToken': true,
            'reference': n - 1
          });
          nextColumn.states.add(next);
        }
      }

      nextColumn.process();
      table.add(nextColumn);

      if (nextColumn.states.length == 0) {
        var err = new NearleyError(reportError(token));
        err.offset = this.current;
        err.token = token;
        throw Exception(err.error);
      }

      token = lexer.next();
      current = current! + 1;
    }

    if (column != null) {
      this.lexerState = lexer.save();
    }

    results = this.finish();

    return this;
  }

  finish() {
    var considerations = [];
    var start = this.grammar!.start;
    var column = this.table[this.table.length - 1];
    column!.states.forEach((t) {
      if (t!.rule.name == start &&
          t.dot == t.rule.symbols.length &&
          t.reference == 0 &&
          t.data != fail) {
        considerations.add(t);
      }
    });
    return considerations.map((c) {
      return c.data;
    }).toList();
  }

  reportError(token) {
    var lines = [];
    var tokenDisplay =
        (token['type'] != null ? token['type'] + " token: " : "") +
            jsonEncode(token['value'] != null ? token['value'] : token);
    lines.add(this.lexer.formatError(token, "Syntax error"));
    lines.add('Unexpected ' +
        tokenDisplay +
        '. Instead, I was expecting to see one of the following:\n');
    var lastColumnIndex = this.table.length - 2;
    var lastColumn = this.table[lastColumnIndex];
    var expectantStates = lastColumn!.states.where((state) {
      if (state!.rule.symbols.isNotEmpty) {
        var nextSymbol =
            state.rule.symbols[state.dot - 1 < 0 ? 0 : state.dot - 1];
        return nextSymbol != null && !(nextSymbol is String);
      } else {
        return false;
      }
    }).toList();

    var stateStacks = expectantStates.map((state) {
      return this.buildFirstStateStack(state, []);
    }).toList();

    stateStacks.forEach((stateStack) {
      var state = stateStack[0];
      var nextSymbol =
          state.rule.symbols[state.dot - 1 < 0 ? 0 : state.dot - 1];
      var symbolDisplay = this.getSymbolDisplay(nextSymbol);
      lines.add('A ' + symbolDisplay + ' based on:');
      this.displayStateStack(stateStack, lines);
    });

    lines.add("");
    return lines.join("\n");
  }

  String reportLexerError(lexerError) {
    var tokenDisplay, lexerMessage;

    var token = lexerError.token;
    if (token) {
      tokenDisplay = "input " + jsonEncode(token.text[0]) + " (lexer error)";
      lexerMessage = this.lexer.formatError(token, "Syntax error");
    } else {
      tokenDisplay = "input (lexer error)";
      lexerMessage = lexerError.message;
    }
    return this.reportErrorCommon(lexerMessage, tokenDisplay);
  }

  reportErrorCommon(lexerMessage, tokenDisplay) {
    var lines = [];
    lines.add(lexerMessage);
    var lastColumnIndex = this.table.length - 1;
    var lastColumn = this.table[lastColumnIndex];
    var expectantStates = lastColumn!.states.where((state) {
      var nextSymbol = state!.rule.symbols[state.dot] ?? null;
      return nextSymbol != null && !(nextSymbol is String);
    }).toList();

    if (expectantStates.length == 0) {
      lines.add('Unexpected ' +
          tokenDisplay +
          '. I did not expect any more input. Here is the state of my parse table:\n');
      this.displayStateStack(lastColumn.states, lines);
    } else {
      lines.add('Unexpected ' +
          tokenDisplay +
          '. Instead, I was expecting to see one of the following:\n');

      var stateStacks = expectantStates.map((state) {
        return this.buildFirstStateStack(state, []) ?? [state];
      });

      stateStacks.forEach((stateStack) {
        var state = stateStack[0];
        var nextSymbol = state.rule.symbols[state.dot];
        var symbolDisplay = this.getSymbolDisplay(nextSymbol);
        lines.add('A ' + symbolDisplay + ' based on:');
        this.displayStateStack(stateStack, lines);
      });
    }
    lines.add("");
    return lines.join("\n");
  }

  buildFirstStateStack(state, visited) {
    if (visited.indexOf(state) != -1) {
      return null;
    }
    if (state.wantedBy.length == 0) {
      return [state];
    }
    var prevState = state.wantedBy[0];
    var childVisited = [state]..addAll(visited);
    var childResult = this.buildFirstStateStack(prevState, childVisited);
    if (childResult == null) {
      return null;
    }
    return [state]..addAll(childResult);
  }

  displayStateStack(stateStack, lines) {
    var lastDisplay;
    var sameDisplayCount = 0;
    for (var j = 0; j < stateStack.length; j++) {
      var state = stateStack[j];
      var display = state.rule.toStringWithData(state.dot);
      if (display == lastDisplay) {
        sameDisplayCount++;
      } else {
        if (sameDisplayCount > 0) {
          lines.add('    ^ ' +
              sameDisplayCount.toString() +
              ' more lines identical to this');
        }
        sameDisplayCount = 0;
        lines.add('    ' + display);
      }
      lastDisplay = display;
    }
  }

  getSymbolDisplay(symbol) {
    return getSymbolLongDisplay(symbol);
  }

  getSymbolLongDisplay(symbol) {
    if (symbol is String) {
      return symbol;
    } else if (symbol is Map) {
      return jsonEncode(symbol['literal'] ?? '');
    } else if (symbol is RegExp) {
      return 'character matching ' + symbol.toString();
    } else if (symbol['type'] != null) {
      return symbol['type'] + ' token';
    } else if (symbol['test'] ?? null != null) {
      return 'token matching ' + symbol['test'].toString();
    } else {
      throw new Exception('Unknown symbol type: ' + symbol.toString());
    }
  }
}

class Column {
  NearleyGrammar? grammar;
  int index;

  late List<ColumnState?> states;
  Map? wants;
  late List<ColumnState> scannable;
  Map? completed;

  Column(this.grammar, this.index) {
    states = [];
    wants = {};
    scannable = [];
    completed = {};
  }

  void predict(exp) {
    var rules = grammar!.byName[exp] ?? [];
    for (var i = 0; i < rules.length; i++) {
      var r = rules[i];
      var wantBy = wants![exp];
      var s = new ColumnState(r, 0, index, wantBy);
      states.add(s);
    }
  }

  void process() {
    var _states = this.states;
    var _wants = this.wants;
    var _completed = this.completed;

    for (var w = 0; w < _states.length; w++) {
      var state = _states[w]!;
      if (state.isComplete) {
        state.finish();
        if (state.data != fail) {
          var wantBy = state.wantedBy;

          for (var i = wantBy.length - 1; 0 <= i; i--) {
            var left = wantBy[i];
            this.complete(left, state);
          }

          if (state.reference == index) {
            var exp = state.rule.name;
            this.completed![exp] = this.completed![exp] ?? [];
            this.completed![exp].add(state);
          }
        }
      } else {
        var exp = state.rule.symbols[state.dot];
        if (!(exp is String)) {
          this.scannable.add(state);
        }

        if (_wants![exp] != null) {
          _wants[exp].add(state);
          if (_completed!.containsKey(exp)) {
            var nulls = _completed[exp];
            for (var i = 0; i < nulls.length; i++) {
              var right = nulls[i];
              complete(state, right);
            }
          }
        } else {
          _wants[exp] = _wants[exp] is List ? _wants[exp].add(state) : [state];
          this.predict(exp);
        }
      }
    }
  }

  void complete(left, right) {
    var copy = left.nextState(right);
    states.add(copy);
  }
}

class ColumnState {
  Rule rule;
  var dot;
  var reference;
  var data;
  var wantedBy;
  late var isComplete;

  ColumnState? left;
  late var right;

  ColumnState(this.rule, this.dot, this.reference, this.wantedBy) {
    data = [];
    isComplete = dot == rule.symbols.length;
  }

  void finish() {
    if (rule.postprocess != null) {
      try {
        data = rule.postprocess(data);
      } catch (e) {
        print("Error In ===> " + rule.name + "  " + rule.symbols.toString());
      }
    }
  }

  ColumnState nextState(child) {
    var state = new ColumnState(rule, dot + 1, reference, wantedBy);
    state.left = this;
    state.right = child;
    if (state.isComplete) {
      state.data = state.build();
    }
    return state;
  }

  List build() {
    var children = [];
    ColumnState node = this;
    do {
      children.add(
          node.right is ColumnState ? node.right.data : node.right['data']);
      node = node.left!;
    } while (node.left != null);
    children = children.reversed.toList();
    return children;
  }
}

class NearleyError {
  var error;
  int? offset;
  var token;
  NearleyError(this.error);
}

class NearleyGrammar {
  List<Rule>? rules;
  var start;

  var lexer;
  late var byName;

  NearleyGrammar(this.rules, this.start) {
    this.rules = rules;
    this.start = start ?? this.rules![0].name;
    var byName = this.byName = {};
    this.rules!.forEach((rule) {
      if (!byName.containsKey(rule.name)) {
        byName[rule.name] = [];
      }
      byName[rule.name].add(rule);
    });
  }
}

class Rule {
  var name;
  var symbols;
  var postprocess;
  Rule(this.name, this.symbols, this.postprocess);

  toStringWithData(withCursorAt) {
    stringifySymbolSequence(e) {
      return e is Map && e.containsKey('literal')
          ? jsonEncode(e['literal'])
          : e is Map && e.containsKey('type')
              ? '%' + e['type']
              : e.toString();
    }

    var symbolSequence = (withCursorAt == null)
        ? this.symbols.map(stringifySymbolSequence).join(' ')
        : (this
                .symbols
                .take(withCursorAt - 1 < 0 ? 0 : withCursorAt - 1)
                .map(stringifySymbolSequence)
                .join(' ') +
            " ● " +
            this
                .symbols
                .take(withCursorAt)
                .map(stringifySymbolSequence)
                .join(' '));
    return this.name + " → " + symbolSequence;
  }
}

class StreamLexer {
  var buffer;
  late int index;
  var line;
  var lastLineBreak;

  StreamLexer() {
    this.reset("");
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
    pad(n, length) {
      var s = (n).toString();
      return List.generate(length - s.length + 1, (index) => '').join(" ") + s;
    }

    var buffer = this.buffer;
    if (buffer is String) {
      var lines = buffer.split("\n").sublist(max(0, this.line - 5), this.line);

      var nextLineBreak = buffer.indexOf('\n', this.index);
      if (nextLineBreak == -1) nextLineBreak = buffer.length;
      var col = this.index - this.lastLineBreak;
      var lastLineDigits = (this.line).toString().length;
      message += " at line " +
          this.line.toString() +
          " col " +
          col.toString() +
          ":\n\n";
      var msg = [];
      for (var i = 0; i < lines.length; i++) {
        var line = lines[i];
        msg.add(
            pad(this.line - lines.length + i + 1, lastLineDigits) + " " + line);
      }
      message += msg.join("\n");
      message += "\n" + pad("", lastLineDigits + col) + "^\n";
      return message;
    } else {
      return message + " at index " + (this.index - 1);
    }
  }
}
