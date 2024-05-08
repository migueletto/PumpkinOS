#!/usr/bin/env bash
script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
LD_LIBRARY_PATH="${script_dir}/bin"
export LD_LIBRARY_PATH
storage_dir="${XDG_DATA_HOME:-${HOME}/.local/share}/PumpkinOS"
mkdir -p "${storage_dir}"
if ! [ -d "${storage_dir}/vfs" ]; then
    cp -rf "${script_dir}/vfs" "${storage_dir}"
fi
mkdir -p "${storage_dir}/vfs/app_storage" "${storage_dir}/registry" "${storage_dir}/log"
cd "${storage_dir}" || exit 1
XDG_PUMPKINOS="${storage_dir}"
export XDG_PUMPKINOS
"${script_dir}/pumpkin" -d 1 -f "${storage_dir}/pumpkin.log" -s "${LD_LIBRARY_PATH}/libscriptlua.so" "${script_dir}/script/pumpkin_linux.lua"
