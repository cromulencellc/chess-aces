#!/bin/bash

if [ "$TRAVIS_OS_NAME" == "osx" ]; then
	cmake -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl .
fi
