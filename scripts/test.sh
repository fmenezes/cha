#!/bin/bash

result=0
expected=200
set -e
./a.out || result=$?
set +e
if [ $result -ne $expected ]; then
    >&2 echo "error: expected $expected got $result"
    exit 1
fi
