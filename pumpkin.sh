PATH=./bin:$PATH
export LD_LIBRARY_PATH=./bin
export LIBRARY_PATH=$LIBRARY_PATH:%A/bin
./pumpkin -d 1 -f pumpkin.log -s libscriptlua ./script/pumpkin.lua
