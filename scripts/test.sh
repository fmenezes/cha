#!/bin/bash

result=0
expected=209
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' exit
build/ni -asm examples/test.ni "$tmp/output.s"
if [[ $OSTYPE == 'darwin'* ]]; then
    cc "$tmp/output.s" -o "$tmp/a.out" -nostdlib -lSystem
else
    cc "$tmp/output.s" -o "$tmp/a.out" -nostdlib
fi
set -e
"$tmp/a.out" || result=$?
set +e
if [ $result -ne $expected ]; then
    >&2 echo "error: expected $expected got $result"
    exit 1
fi
