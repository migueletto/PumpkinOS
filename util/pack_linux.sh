#!/bin/sh

if [ -z "$1" ]; then
exit 0
fi

cat <<EOF > PumpkinOS/README.txt
This is a binary-only distribution of PumpkinOS for GNU/Linux on x86_64.
Run it with the 'pumpkin.sh' shell script.

The source code is licensed under GPLv3 and can be obtained at:
https://github.com/migueletto/PumpkinOS
EOF

tar --no-recursion -c -v -f PumpkinOS.$1.Linux.x86_64.tar PumpkinOS/README.txt PumpkinOS/LICENSE PumpkinOS/pumpkin PumpkinOS/pumpkin.sh PumpkinOS/bin/*.so PumpkinOS/registry PumpkinOS/script/pumpkin.lua PumpkinOS/vfs/app_storage PumpkinOS/vfs/app_install/*.prc PumpkinOS/vfs/app_card/PALM/Programs/Command
gzip PumpkinOS.$1.Linux.x86_64.tar

rm PumpkinOS/README.txt

exit 0
