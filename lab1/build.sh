#!/bin/sh -e

File="$1"

[ -n "$File" ]
[ -f "$File" ]

Output=$(grep '&Output' "$File" | cut -d: -f2- | xargs)

TmpDir=$(mktemp -d)
Path=$(pwd)

cleanup() { rm -rf "$TmpDir"; }
trap cleanup EXIT HUP INT PIPE TERM

cp "$File" "$TmpDir/"
cd "$TmpDir"
g++ "$(basename "$File")" -o "$Output"

mv "$Output" "$Path/"

echo "Success: $Path/$Output"
