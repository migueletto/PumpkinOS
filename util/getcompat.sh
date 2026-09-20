#!/bin/bash

registry="vfs/app_storage/RegistryDB_52656769737472794442"
compat="src/compat"

for r in $registry/*.1
do
  os=`hexdump -v -C $r | awk 'NR==1 {print $2 $3}'`
  if [[ "$os" != "3600" ]]; then
    echo "os" $os $r
    cp $r $compat
  fi
done

for r in $registry/*.2
do
  flags=`hexdump -v -C $r | awk 'NR==1 {print $2 $3 $4 $5}'`
  if [[ "$flags" != "00000000" && "$flags" != "01000000" ]]; then
    echo "flags" $flags $r
    cp $r $compat
  fi
done

for r in $registry/*.5
do
  display=`hexdump -v -C $r | awk 'NR==1 {print $2 $3 $4 $5}'`
  if [[ "$display" != "90001000" ]]; then
    echo "display" $display $r
    cp $r $compat
  fi
done

for r in $registry/*.10
do
  heap=`hexdump -v -C $r | awk 'NR==1 {print $2 $3 $4 $5}'`
  if [[ "$heap" != "00000000" && "$heap" != "08000000" ]]; then
    echo "heap" $heap $r
    cp $r $compat
  fi
done

exit 0
