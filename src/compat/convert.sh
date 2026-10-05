#!/bin/sh

# DRaw.44526177.2

for res in *.*.*
do
  dat=`echo $res | awk -F. '{printf("%s%04X.dat", $1, 0+$3)}'`
  if [ ! -f $dat ]; then
    echo "renaming new $res to $dat"
    mv $res $dat
  else
    diff $res $dat 2> /dev/null
    if [ "$?" -eq "0" ]; then
      echo "ignoring $res"
      rm -f $res
    else
      echo "renaming changed $res to $dat"
      mv $res $dat
    fi
  fi
done

exit 0
