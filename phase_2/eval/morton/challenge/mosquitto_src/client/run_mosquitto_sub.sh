#!/bin/bash
./mosquitto_sub -t "app/request" -v -d --remove-retained -i Thermometer-1
