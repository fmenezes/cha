#!/bin/bash

result=0
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' exit
build/ni -o "$tmp/test" examples/test.ni
set -e
"$tmp/test" || result=$?
set +e
if [ $result -ne 0 ]; then
    >&2 echo "error: expected 0 got $result"
    exit 1
fi
