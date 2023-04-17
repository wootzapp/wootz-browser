import 'package:dartez/types/tezos/query_types.dart';

class ConseilQueryBuilder {
  static blankQuery() {
    return {
      'fields': [],
      'predicates': [],
      'orderBy': [],
      'aggregation': [],
      'limit': 100
    };
  }

  static setLimit(query, limit) {
    if (limit < 1) {
      throw new Exception('Limit cannot be less than one.');
    }
    var q = query;
    q['limit'] = limit;
    return q;
  }

  static addOrdering(query, field, {direction}) {
    if (direction == null) direction = QueryTypes.conseilSortDirection['ASC'];
    var q = query;
    q['orderBy'].add({'field': field, 'direction': direction});
    return q;
  }

  static addPredicate(query, field, operation, values,
      {invert = false, group = ''}) {
    if (group == null) group = '';
    if (operation == QueryTypes.conseilOperator['BETWEEN'] &&
        values.length != 2) {
      throw new Exception('BETWEEN operation requires a list of two values.');
    } else if (operation == QueryTypes.conseilOperator['IN'] &&
        values.length < 2) {
      throw new Exception(
          'IN operation requires a list of two or more values.');
    } else if (values.length != 1 &&
        operation != QueryTypes.conseilOperator['IN'] &&
        operation != QueryTypes.conseilOperator['BETWEEN'] &&
        operation != QueryTypes.conseilOperator['ISNULL']) {
      throw new Exception('invalid values list for $operation.');
    }
    var q = query;
    q['predicates'].add({
      'field': field,
      'operation': operation ?? '',
      'set': values,
      'inverse': invert,
      'group': group ?? '',
    });
    return q;
  }
}
