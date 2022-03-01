#!/bin/bash

result=0
set -e
./a.out || result=$?
set +e
if [ $result -ne 200 ]; then
    >&2 echo "error: expected 200 got $result"
    exit 1
fi
