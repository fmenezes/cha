#!/bin/bash

result=0
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' exit
build/ni -c "$tmp/output.o" examples/test.ni
if [[ $OSTYPE == 'darwin'* ]]; then
    cc "$tmp/output.o" -o "$tmp/a.out" -lSystem
else
    cc "$tmp/output.o" -o "$tmp/a.out"
fi
set -e
"$tmp/a.out" || result=$?
set +e
if [ $result -ne 0 ]; then
    >&2 echo "error: expected 0 got $result"
    exit 1
fi
