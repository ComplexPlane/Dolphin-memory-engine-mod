#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

mkdir -p build && cd build
cmake ..
make -j$(nproc)
sudo setcap cap_sys_ptrace=eip Dolphin-memory-engine
./Dolphin-memory-engine
