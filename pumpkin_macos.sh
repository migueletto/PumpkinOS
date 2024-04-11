PATH=./bin:$PATH
export DYLD_LIBRARY_PATH=./bin
./pumpkin -d 1 -f pumpkin.log -s libscriptlua.so ./script/pumpkin_linux.lua
