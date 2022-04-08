#!/bin/bash

docker build -t cross-rv32im cross-rv32im
docker run --rm -ti -v $(pwd):/jaeger cross-rv32im /jaeger/build_scripts/devices.sh
docker build -t emulator-builder challenge/emulator
docker run --rm -ti -v $(pwd):/jaeger challenge/emulator