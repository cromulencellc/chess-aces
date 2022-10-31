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

RESULT="$(curl $HOST:$PORT -H "CrashHeader: aa{CrashHeader}aa" 2>&1)"

echo "RESULT: $RESULT"

if [[ $RESULT == *"Empty"* ]]; then
  echo "[SUCCESS] The Server should be dead"
else
  echo "[FAIL] The server is likely still up"
fi
