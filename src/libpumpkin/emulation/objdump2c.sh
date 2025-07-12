#!/bin/sh

 # 0000 4e56ffc8 48e71f30 2e2e0008 2c2e000c  NV..H..0....,...
 # 0010 266e0010 363c050a b6fc0000 670436bc  &n..6<......g.6.
 # 0020 ffff486e ffcc486e ffca4227 2f062f07  ..Hn..Hn..B'/./.
 # 0030 486effe0 1f3c0001 4e4fa078 4fef0018  Hn...<..NO.xO...
 # 0040 4a406600 00b63f3c 00012f2e ffcc3f2e  J@f...?<../...?.
 # 0050 ffca4e4f a0492a08 508f6700 009e4267  ..NO.I*.P.g...Bg
 # 0060 2f3c6c69 62724e4f a0602808 5c8f6700  /<librNO.`(.\.g.
 # 0070 00842f04 4e4fa021 2448588f b4fc0000  ../.NO.!$HX.....
 # 0080 676a2f0b 2f062f07 4e4fa503 4fef000c  gj/././.NO..O...
 # 0090 4a006706 42436000 004c4227 48780010  J.g.BC`..LB'Hx..
 # 00a0 76d0d68e 2f034e4f a0272f03 3f134e92  v.../.NO.'/.?.N.
 # 00b0 36004fef 00106624 2f2effd4 2f2effd0  6.O...f$/.../...
 # 00c0 2f044e4f a02d2e80 2f0a2f2e ffcc3f13  /.NO.-.././...?.
 # 00d0 4e4fa504 36004fef 00166708 3f134e4f  NO..6.O...g.?.NO
 # 00e0 a505548f 2f044e4f a022588f 2f044e4f  ..T./.NO."X./.NO
 # 00f0 a061588f 2f054e4f a04a3003 4cee0cf8  .aX./.NO.J0.L...
 # 0100 ffac4e5e 4e75                        ..N^Nu  

cut -c2-41 |
awk '
$1 ~ /^[0-9a-f][0-9a-f][0-9a-f][0-9a-f]$/ {
  line = $2 $3 $4 $5;
  len = length(line);
  s = " ";
  for (i = 1; i <= len; i += 2) {
    hex = " 0x" substr(line, i, 2) ",";
    s = s hex;
  }
  print s;
}
'

exit 0
