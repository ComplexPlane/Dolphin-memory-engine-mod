#!/usr/bin/env bash

mkdir -p build && cd build
cmake ..
make -j$(nproc)
sudo setcap cap_sys_ptrace=eip Dolphin-memory-engine
./Dolphin-memory-engine
