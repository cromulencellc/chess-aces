#!/bin/bash

docker kill `docker ps | grep riverway:challenge | cut -f1 -d' '`
