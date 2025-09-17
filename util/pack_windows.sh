#!/bin/sh

if [ -z "$1" ]; then
exit 0
fi

cat <<EOF > PumpkinOS/README.txt
This is a binary-only distribution of PumpkinOS for Windows 10 / Windows 11 on x86_64.
Run it with the 'pumpkin.bat' batch script.

The source code is licensed under GPLv3 and can be obtained at:
https://github.com/migueletto/PumpkinOS
EOF

unix2dos PumpkinOS/README.txt

zip PumpkinOS.$1.Windows.x86_64.zip PumpkinOS/README.txt PumpkinOS/LICENSE PumpkinOS/pumpkin.exe PumpkinOS/pumpkin.bat PumpkinOS/pumpkin.vbs PumpkinOS/bin/*.dll PumpkinOS/lib/*.dll PumpkinOS/registry PumpkinOS/script/pumpkin.lua PumpkinOS/vfs/app_storage PumpkinOS/vfs/app_install/*.prc PumpkinOS/vfs/app_card/PALM/Programs/Command

rm PumpkinOS/README.txt

exit 0
