import 'package:intl/intl.dart' as intl;

formatMoney(final money) {
  // check if up to six decimal is greater than 0

  double actualMoney = double.parse(money.toString());

  if (actualMoney >= 1e6) {
    return intl.NumberFormat.compact().format(actualMoney);
  }

  // actualmoney have eight decimal place significant digit
  if (actualMoney.abs() < 0.00000001) {
    return intl.NumberFormat.decimalPattern().format(0);
  }

  if (actualMoney.abs() < 1 && actualMoney.abs() != 0) {
    return intl.NumberFormat('0.00000000').format(actualMoney);
  } else {
    return intl.NumberFormat.decimalPattern().format(actualMoney);
  }
}
