#!/bin/sh

MAJOR=2
MINOR=0
REVISION=14

sed -i "s/^VERSION=.*/VERSION=${MAJOR}.${MINOR}.${REVISION}/" config.mk

sed -i "s/^#define LIBMOSQUITTO_MAJOR .*/#define LIBMOSQUITTO_MAJOR ${MAJOR}/" include/mosquitto.h
sed -i "s/^#define LIBMOSQUITTO_MINOR .*/#define LIBMOSQUITTO_MINOR ${MINOR}/" include/mosquitto.h
sed -i "s/^#define LIBMOSQUITTO_REVISION .*/#define LIBMOSQUITTO_REVISION ${REVISION}/" include/mosquitto.h

sed -i "s/^set (VERSION .*)/set (VERSION ${MAJOR}.${MINOR}.${REVISION})/" CMakeLists.txt

sed -i "s/^!define VERSION .*/!define VERSION ${MAJOR}.${MINOR}.${REVISION}/" installer/*.nsi

sed -i "s/^version: .*/version: ${MAJOR}.${MINOR}.${REVISION}/" snap/snapcraft.yaml
