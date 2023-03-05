import 'package:flutter/material.dart';

const primaryColor = Color(0xFF45C2DA);
// final background = Color(0xFF191B2A);
const background = Color(0xFF0F233D);
const cardColorLight = Colors.white;
const cardColorDark = Colors.black;
const fontColorLight = Colors.black;
const fontColorDark = Colors.white;
const fontSecondaryColorLight = Colors.black26;
const fontSecondaryColorDark = Colors.white24;
const iconColorLight = Colors.black;
const iconColorDark = Colors.white;
const fontColorDarkTitle = Color(0xFF32353E);
const grayBackground = Color(0xFF172E4D);
const whiteBacground = Color(0xFFF4F5F7);
//  final grayBackground = Color(0xFF16223A);
const blackBackground = Color(0xFF12151C);
const bottomBarDark = Color(0xFF202833);

const white = Colors.white;
const black = Colors.black;
const mask = Color.fromRGBO(0, 0, 0, 0.6);
const transparent = Color.fromRGBO(0, 0, 0, 0);
const gradient = LinearGradient(
  begin: Alignment.centerLeft,
  end: Alignment.centerRight,
  colors: [
    Color.fromRGBO(245, 250, 128, 1),
    Color.fromRGBO(9, 10, 11, 1),
  ],
);

//Primary
const primary1 = Color.fromRGBO(250, 253, 225, 1);
const primary2 = Color.fromRGBO(253, 254, 204, 1);
const primary3 = Color.fromRGBO(251, 253, 179, 1);
const primary4 = Color.fromRGBO(248, 251, 159, 1);
const primary5 = Color.fromRGBO(245, 250, 128, 1);
const primary6 = Color.fromRGBO(209, 215, 93, 1);
const primary7 = Color.fromRGBO(174, 179, 64, 1);
const primary8 = Color.fromRGBO(139, 144, 40, 1);

//Secondary
const secondary1 = Color.fromRGBO(254, 254, 229, 1);
const secondary2 = Color.fromRGBO(246, 252, 250, 1);
const secondary3 = Color.fromRGBO(238, 246, 244, 1);
const secondary4 = Color.fromRGBO(228, 238, 237, 1);
const secondary5 = Color.fromRGBO(215, 228, 227, 1);
const secondary6 = Color.fromRGBO(157, 195, 196, 1);
const secondary7 = Color.fromRGBO(108, 158, 164, 1);
const secondary8 = Color.fromRGBO(68, 119, 132, 1);

//Gray
const grey0 = Color.fromRGBO(248, 248, 249, 1);
const grey1 = Color.fromRGBO(244, 244, 246, 1);
const grey2 = Color.fromRGBO(232, 233, 237, 1);
const grey3 = Color.fromRGBO(221, 223, 228, 1);
const grey4 = Color.fromRGBO(209, 212, 219, 1);
const grey5 = Color.fromRGBO(198, 200, 210, 1);
const grey6 = Color.fromRGBO(186, 190, 201, 1);
const grey7 = Color.fromRGBO(175, 180, 192, 1);
const grey8 = Color.fromRGBO(164, 169, 183, 1);
const grey9 = Color.fromRGBO(152, 158, 174, 1);
const grey10 = Color.fromRGBO(141, 147, 165, 1);
const grey11 = Color.fromRGBO(129, 137, 156, 1);
const grey12 = Color.fromRGBO(118, 126, 147, 1);
const grey13 = Color.fromRGBO(108, 116, 137, 1);
const grey14 = Color.fromRGBO(99, 106, 126, 1);
const grey15 = Color.fromRGBO(90, 96, 114, 1);
const grey16 = Color.fromRGBO(81, 87, 103, 1);
const grey17 = Color.fromRGBO(71, 76, 90, 1);
const grey18 = Color.fromRGBO(63, 67, 80, 1);
const grey19 = Color.fromRGBO(54, 58, 69, 1);
const grey20 = Color.fromRGBO(45, 48, 57, 1);
const grey21 = Color.fromRGBO(36, 39, 46, 1);
const grey22 = Color.fromRGBO(27, 29, 34, 1);
const grey23 = Color.fromRGBO(18, 19, 23, 1);
const grey24 = Color.fromRGBO(9, 10, 11, 1);

//Blue
const blue1 = Color.fromRGBO(199, 249, 244, 1);
const blue2 = Color.fromRGBO(146, 243, 240, 1);
const blue3 = Color.fromRGBO(88, 213, 219, 1);
const blue4 = Color.fromRGBO(47, 167, 184, 1);
const blue5 = Color.fromRGBO(1, 110, 137, 1);
const blue6 = Color.fromRGBO(0, 85, 117, 1);
const blue7 = Color.fromRGBO(0, 64, 98, 1);
const blue8 = Color.fromRGBO(0, 46, 79, 1);

//Yellow
const yellow1 = Color.fromRGBO(254, 240, 203, 1);
const yellow2 = Color.fromRGBO(253, 222, 153, 1);
const yellow3 = Color.fromRGBO(251, 197, 101, 1);
const yellow4 = Color.fromRGBO(247, 172, 63, 1);
const yellow5 = Color.fromRGBO(242, 134, 2, 1);
const yellow6 = Color.fromRGBO(208, 105, 1, 1);
const yellow7 = Color.fromRGBO(174, 80, 1, 1);
const yellow8 = Color.fromRGBO(140, 58, 0, 1);

//green
const green1 = Color.fromRGBO(205, 249, 208, 1);
const green2 = Color.fromRGBO(157, 243, 171, 1);
const green3 = Color.fromRGBO(103, 220, 134, 1);
const green4 = Color.fromRGBO(62, 186, 107, 1);
const green5 = Color.fromRGBO(16, 140, 74, 1);
const green6 = Color.fromRGBO(11, 120, 73, 1);
const green7 = Color.fromRGBO(8, 100, 69, 1);
const green8 = Color.fromRGBO(5, 81, 63, 1);

//Red
const red1 = Color.fromRGBO(255, 229, 218, 1);
const red2 = Color.fromRGBO(255, 196, 182, 1);
const red3 = Color.fromRGBO(255, 157, 145, 1);
const red4 = Color.fromRGBO(255, 121, 118, 1);
const red5 = Color.fromRGBO(255, 73, 86, 1);
const red6 = Color.fromRGBO(219, 53, 80, 1);
const red7 = Color.fromRGBO(183, 36, 73, 1);
const red8 = Color.fromRGBO(147, 23, 65, 1);

// Edit 1
const appBg = Color(0xffE5E5E5);
