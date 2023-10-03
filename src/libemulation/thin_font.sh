#!/bin/sh

#  // 0x00
#  0b00000000,

awk '
BEGIN {
  s = 1;
}
/^  \/\// {
  print;
  i = 0;
  s = 2;
  next;
}
s == 2 {
  if (i % 2) print;
  i++;
  if (i == 16) s = 1;
  next;
}
{
  print;
}
'

exit 0
