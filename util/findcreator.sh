#!/bin/sh

for header in `find vfs/app_storage -name header`
do
  grep "$1" $header > /dev/null
  if [ $? -eq 0 ]; then
    echo $header
  fi
done

exit 0
