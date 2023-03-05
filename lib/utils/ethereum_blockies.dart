class EthereumBlockies {
  final List randseed = []..length = 4;

  final int size = 8;
  final javaMaxInteger = 2147483647;
  final javaMinInteger = -2147483648;

  javaLongToInt(num number) {
    int count = 0;
    final javaMinIntAbs = javaMinInteger.abs();

    final isNegative = number < 0;

    number = number.abs();

    if (number == javaMinIntAbs) return -number;

    if (number > javaMinIntAbs) {
      while (number > javaMinIntAbs) {
        number -= javaMinIntAbs;
        count++;
      }
      if (count % 2 == 1) {
        number -= javaMinIntAbs;
      }
    }
    return isNegative ? -number : number;
  }

  seedrand(String seed) {
    for (int i = 0; i < randseed.length; i++) {
      randseed[i] = 0;
    }

    for (int i = 0; i < seed.length; i++) {
      int test = randseed[i % 4] << 5;
      final leftShiftMaxby1 = javaMaxInteger << 1;
      if (test > leftShiftMaxby1 || test < leftShiftMaxby1) {
        test = javaLongToInt(test);
      }
      int test2 = (test - randseed[i % 4]).toInt();

      randseed[i % 4] = test2 + seed.codeUnitAt(i);
    }

    for (int i = 0; i < randseed.length; i++) {
      randseed[i] = javaLongToInt(randseed[i]);
    }
  }

  double rand() {
    int t = javaLongToInt(randseed[0] ^ (randseed[0] << 11));

    randseed[0] = randseed[1];
    randseed[1] = randseed[2];
    randseed[2] = randseed[3];

    randseed[3] = randseed[3] ^ (randseed[3] >> 19) ^ t ^ (t >> 8);

    double t1 = randseed[3].abs().toDouble();

    return t1 / javaMaxInteger;
  }

  HSL createColor() {
    double h = (rand() * 360).floor().toDouble();
    double s = ((rand() * 60) + 40);
    double l = ((rand() + rand() + rand() + rand()) * 25);
    return HSL(h, s, l);
  }

  List<double> createImageData() {
    int width = size;
    int height = size;

    double dataWidth = (width / 2).ceil().toDouble();

    double mirrorWidth = width - dataWidth;

    List<double> data = List.filled(size * size, 0, growable: true);

    int dataCount = 0;
    for (int y = 0; y < height; y++) {
      List<double> row = List.filled(dataWidth.toInt(), 0, growable: true);

      for (int x = 0; x < dataWidth; x++) {
        row[x] = (rand() * 2.3).floor().toDouble();
      }

      List<double> r =
          row.sublist(0, (javaLongToInt(mirrorWidth) as num).toInt());

      r = r.reversed.toList();

      row = [...row, ...r];

      for (double v in row) {
        data[dataCount] = v.toDouble();
        dataCount++;
      }
    }

    return data;
  }
}

class HSL {
  double h, s, l;

  HSL(this.h, this.s, this.l);
  @override
  String toString() {
    return 'H: $h S: $s L: $l';
  }
}
