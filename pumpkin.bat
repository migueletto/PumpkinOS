@ECHO OFF
PATH=.\bin;.\lib
START /b .\pumpkin.exe -d 1 -f pumpkin.log -s libscriptlua .\script\pumpkin.lua
