#!/bin/bash

result=0
set -e
./a.out || result=$?
set +e
if [ $result -ne 100 ]; then
    exit 1
fi
