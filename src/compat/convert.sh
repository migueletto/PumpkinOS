#!/bin/sh

# DRaw.44526177.2

for res in *.*.*
do
  dat=`echo $res | awk -F. '{printf("%s%04X.dat", $1, 0+$3)}'`
  if [ ! -f $dat ]; then
    echo "renaming $res to $dat"
    mv $res $dat
  else
    echo "ignoring $res"
    rm -f $res
  fi
done

../../tools/prcbuild -v -f compat.prc -t regt -c psys -n compat *.dat

exit 0
