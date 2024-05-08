#!/usr/bin/env bash
script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PATH=./bin:$PATH
export LD_LIBRARY_PATH=./bin
"${script_dir}/pumpkin" -d 1 -f pumpkin.log -s libscriptlua.so ./script/pumpkin_linux.lua
