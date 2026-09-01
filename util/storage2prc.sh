#!/bin/sh

# tSTR.74535452.1004

if [ $# -ne 4 ]; then
  echo "usage: $0 <filename.prc> <type> <creator> <appname>"
  exit 0
fi

rm *.dat

for res in *.*.*
do
  novo=`echo $res | awk -F. '{printf("%s%04X.dat", $1, 0+$3)}'`
  mv $res $novo
done

prcbuild -v -f "$1" -t "$2" -c "$3" -n "$4" *.dat

exit 0
