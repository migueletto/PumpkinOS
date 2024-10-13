@ECHO OFF
PATH=.\bin;.\lib
START /b .\pumpkin.exe -d 1 -f pumpkin.log -s libscriptlua.dll .\script\pumpkin_windows.lua
