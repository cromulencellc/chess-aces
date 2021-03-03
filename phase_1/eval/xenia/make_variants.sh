#!/bin/sh

docker run --rm -it -v $(pwd):/mnt aces_preproc aces_preproc -d /mnt/challenge -b /mnt/variants --fix-chown
cp challenge/Dockerfile variants/unpatched
cp challenge/Dockerfile variants/fully_patched