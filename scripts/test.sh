#!/bin/bash

result=0
expected=209
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' exit
build/ni -c "$tmp/output.o" examples/test.ni
if [[ $OSTYPE == 'darwin'* ]]; then
    cc "$tmp/output.o" -o "$tmp/a.out" -nostdlib -lSystem
else
    cc "$tmp/output.o" -o "$tmp/a.out" -nostdlib
fi
set -e
"$tmp/a.out" || result=$?
set +e
if [ $result -ne $expected ]; then
    >&2 echo "error: expected $expected got $result"
    exit 1
fi
