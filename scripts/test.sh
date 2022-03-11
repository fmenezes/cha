#!/bin/bash

result=0
expected=211
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' exit
build/ni -asm examples/test.ni "$tmp/output.s"
clang++ "$tmp/output.s" -o "$tmp/a.out"
set -e
"$tmp/a.out" || result=$?
set +e
if [ $result -ne $expected ]; then
    >&2 echo "error: expected $expected got $result"
    exit 1
fi
