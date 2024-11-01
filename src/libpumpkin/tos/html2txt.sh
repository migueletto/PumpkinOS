#!/bin/sh

# Opcode:   47
# Syntax:   DTA *Fgetdta ( void );


awk '
BEGIN {
 s = 0;
}
$1 == "Opcode:" {
  opcode = $2;
  next;
}
$1 == "Syntax:" {
  proto = $2;
  for (i = 3; i <= NF; i++) {
    if ($i == "(" || $i == ");") continue;
    proto = proto " " $i;
  }
  print opcode,proto;
}
' | sort -k 1n

exit 0
