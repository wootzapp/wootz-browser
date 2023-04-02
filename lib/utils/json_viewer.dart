import 'package:flutter/material.dart';

class JsonViewer extends StatefulWidget {
  final dynamic jsonObj;
  final double fontSize;
  const JsonViewer(this.jsonObj, {Key key, this.fontSize}) : super(key: key);
  @override
  _JsonViewerState createState() => _JsonViewerState();
}

class _JsonViewerState extends State<JsonViewer> {
  @override
  Widget build(BuildContext context) {
    return getContentWidget(widget.jsonObj, widget.fontSize);
  }

  static getContentWidget(dynamic content, double fontSize) {
    if (content == null) {
      return const Text('{}');
    } else if (content is List) {
      return JsonArrayViewer(content, notRoot: false, fontSize: fontSize);
    } else {
      return JsonObjectViewer(content, notRoot: false, fontSize: fontSize);
    }
  }
}

class JsonObjectViewer extends StatefulWidget {
  final Map<String, dynamic> jsonObj;
  final bool notRoot;
  final double fontSize;

  const JsonObjectViewer(
    this.jsonObj, {
    Key key,
    this.notRoot = false,
    this.fontSize,
  }) : super(key: key);

  @override
  JsonObjectViewerState createState() => JsonObjectViewerState();
}

class JsonObjectViewerState extends State<JsonObjectViewer> {
  Map<String, bool> openFlag = {};

  @override
  Widget build(BuildContext context) {
    if (widget.notRoot) {
      return Container(
        padding: const EdgeInsets.only(left: 20.0),
        child: Column(
            crossAxisAlignment: CrossAxisAlignment.start, children: _getList()),
      );
    }
    return Column(
        crossAxisAlignment: CrossAxisAlignment.start, children: _getList());
  }

  _getList() {
    List<Widget> list = [];
    for (MapEntry entry in widget.jsonObj.entries) {
      bool ex = isExtensible(entry.value);
      bool ink = isInkWell(entry.value);
      list.add(Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: <Widget>[
          (ex && ink)
              ? InkWell(
                  child: Row(
                    children: [
                      ((openFlag[entry.key] ?? false)
                          ? Icon(Icons.arrow_drop_down,
                              size: 20, color: Colors.grey[700])
                          : Icon(Icons.arrow_right,
                              size: 20, color: Colors.grey[700])),
                      Text(entry.key,
                          style: TextStyle(fontSize: widget.fontSize)),
                    ],
                  ),
                  onTap: () {
                    setState(() {
                      openFlag[entry.key] = !(openFlag[entry.key] ?? false);
                    });
                  })
              : Text(entry.key,
                  style: TextStyle(
                      fontSize: widget.fontSize,
                      color: entry.value == null ? Colors.grey : null)),
          Text(
            ':',
            style: TextStyle(color: Colors.grey, fontSize: widget.fontSize),
          ),
          const SizedBox(width: 3),
          getValueWidget(entry)
        ],
      ));
      list.add(const SizedBox(height: 4));
      if (openFlag[entry.key] ?? false) {
        list.add(getContentWidget(entry.value));
      }
    }
    return list;
  }

  static getContentWidget(dynamic content) {
    if (content is List) {
      return JsonArrayViewer(content, notRoot: true);
    } else {
      return JsonObjectViewer(content, notRoot: true);
    }
  }

  static isInkWell(dynamic content) {
    if (content == null) {
      return false;
    } else if (content is int) {
      return false;
    } else if (content is String) {
      return false;
    } else if (content is bool) {
      return false;
    } else if (content is double) {
      return false;
    } else if (content is List) {
      if (content.isEmpty) {
        return false;
      } else {
        return true;
      }
    }
    return true;
  }

  getValueWidget(MapEntry entry) {
    if (entry.value == null) {
      return Expanded(
          child: Text(
        'undefined',
        style: TextStyle(color: Colors.grey, fontSize: widget.fontSize),
      ));
    } else if (entry.value is int) {
      return Expanded(
          child: Text(
        entry.value.toString(),
        style: TextStyle(color: Colors.teal, fontSize: widget.fontSize),
      ));
    } else if (entry.value is String) {
      return Expanded(
          child: Text(
        '\"' + entry.value + '\"',
        style: TextStyle(color: Colors.redAccent, fontSize: widget.fontSize),
      ));
    } else if (entry.value is bool) {
      return Expanded(
          child: Text(
        entry.value.toString(),
        style: TextStyle(color: Colors.purple, fontSize: widget.fontSize),
      ));
    } else if (entry.value is double) {
      return Expanded(
          child: Text(
        entry.value.toString(),
        style: TextStyle(color: Colors.teal, fontSize: widget.fontSize),
      ));
    } else if (entry.value is List) {
      if (entry.value.isEmpty) {
        return Text(
          'Array[0]',
          style: TextStyle(color: Colors.grey, fontSize: widget.fontSize),
        );
      } else {
        return InkWell(
            child: Text(
              'Array<${getTypeName(entry.value[0])}>[${entry.value.length}]',
              style: TextStyle(color: Colors.grey, fontSize: widget.fontSize),
            ),
            onTap: () {
              setState(() {
                openFlag[entry.key] = !(openFlag[entry.key] ?? false);
              });
            });
      }
    }
    return InkWell(
        child: Text(
          'Object',
          style: TextStyle(color: Colors.grey, fontSize: widget.fontSize),
        ),
        onTap: () {
          setState(() {
            openFlag[entry.key] = !(openFlag[entry.key] ?? false);
          });
        });
  }

  static isExtensible(dynamic content) {
    if (content == null) {
      return false;
    } else if (content is int) {
      return false;
    } else if (content is String) {
      return false;
    } else if (content is bool) {
      return false;
    } else if (content is double) {
      return false;
    }
    return true;
  }

  static getTypeName(dynamic content) {
    if (content is int) {
      return 'int';
    } else if (content is String) {
      return 'String';
    } else if (content is bool) {
      return 'bool';
    } else if (content is double) {
      return 'double';
    } else if (content is List) {
      return 'List';
    }
    return 'Object';
  }
}

class JsonArrayViewer extends StatefulWidget {
  final List<dynamic> jsonArray;

  final bool notRoot;
  final double fontSize;

  const JsonArrayViewer(
    this.jsonArray, {
    Key key,
    this.notRoot = false,
    this.fontSize,
  }) : super(key: key);

  @override
  _JsonArrayViewerState createState() => _JsonArrayViewerState();
}

class _JsonArrayViewerState extends State<JsonArrayViewer> {
  List<bool> openFlag = [];

  @override
  Widget build(BuildContext context) {
    if (widget.notRoot) {
      return Container(
          padding: const EdgeInsets.only(left: 20.0),
          child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: _getList()));
    }
    return Column(
        crossAxisAlignment: CrossAxisAlignment.start, children: _getList());
  }

  @override
  void initState() {
    super.initState();
    openFlag = List.filled(widget.jsonArray.length, false);
  }

  _getList() {
    List<Widget> list = [];
    int i = 0;
    for (dynamic content in widget.jsonArray) {
      bool ex = JsonObjectViewerState.isExtensible(content);
      bool ink = JsonObjectViewerState.isInkWell(content);
      list.add(Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: <Widget>[
          (ex && ink)
              ? getInkWell(i)
              : Text('[$i]',
                  style: TextStyle(
                      color: content == null ? Colors.grey : Colors.purple[900],
                      fontSize: widget.fontSize)),
          Text(
            ':',
            style: TextStyle(color: Colors.grey, fontSize: widget.fontSize),
          ),
          const SizedBox(width: 3),
          getValueWidget(content, i)
        ],
      ));
      list.add(const SizedBox(height: 4));
      if (openFlag[i]) {
        list.add(JsonObjectViewerState.getContentWidget(content));
      }
      i++;
    }
    return list;
  }

  getInkWell(int index) {
    return InkWell(
        child: Row(
          children: [
            ((openFlag[index])
                ? Icon(Icons.arrow_drop_down, size: 20, color: Colors.grey[700])
                : Icon(Icons.arrow_right, size: 20, color: Colors.grey[700])),
            Text('[$index]', style: TextStyle(fontSize: widget.fontSize)),
          ],
        ),
        onTap: () {
          setState(() {
            openFlag[index] = !(openFlag[index]);
          });
        });
  }

  getValueWidget(dynamic content, int index) {
    if (content == null) {
      return Expanded(
          child: Text(
        'undefined',
        style: TextStyle(color: Colors.grey, fontSize: widget.fontSize),
      ));
    } else if (content is int) {
      return Expanded(
          child: Text(
        content.toString(),
        style: TextStyle(color: Colors.teal, fontSize: widget.fontSize),
      ));
    } else if (content is String) {
      return Expanded(
          child: Text(
        '\"' + content + '\"',
        style: TextStyle(color: Colors.redAccent, fontSize: widget.fontSize),
      ));
    } else if (content is bool) {
      return Expanded(
          child: Text(
        content.toString(),
        style: TextStyle(color: Colors.purple, fontSize: widget.fontSize),
      ));
    } else if (content is double) {
      return Expanded(
          child: Text(
        content.toString(),
        style: TextStyle(color: Colors.teal, fontSize: widget.fontSize),
      ));
    } else if (content is List) {
      if (content.isEmpty) {
        return Text(
          'Array[0]',
          style: TextStyle(color: Colors.grey, fontSize: widget.fontSize),
        );
      } else {
        return InkWell(
            child: Text(
              'Array<${JsonObjectViewerState.getTypeName(content)}>[${content.length}]',
              style: TextStyle(color: Colors.grey, fontSize: widget.fontSize),
            ),
            onTap: () {
              setState(() {
                openFlag[index] = !(openFlag[index]);
              });
            });
      }
    }
    return InkWell(
        child: Text(
          'Object',
          style: TextStyle(color: Colors.grey, fontSize: widget.fontSize),
        ),
        onTap: () {
          setState(() {
            openFlag[index] = !(openFlag[index]);
          });
        });
  }
}
