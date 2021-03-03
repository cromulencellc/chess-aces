#!/bin/bash

if [ ! -f sploit-combined.s ]; then
    pushd shellcode
    ./build.sh
    python3 encode.py binsh > encoded.s
    popd
    ./combine.sh
fi