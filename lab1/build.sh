#!/bin/sh -e

File="$1"

if [ -z "$File" ]; then
    echo "Error: no source file specified" >&2
    exit 1
fi

if [ ! -f "$File" ]; then
    echo "Error: file '$File' not found" >&2
    exit 2
fi

Output=$(grep '&Output:' "$File" | cut -d: -f2- | xargs || true)

if [ -z "$Output" ]; then
    echo "Error: &Output: not found in file" >&2
    exit 3
fi

TmpDir=$(mktemp -d)
Path=$(pwd)

cleanup() { rm -rf "$TmpDir"; }
trap cleanup EXIT HUP INT PIPE TERM

cp "$File" "$TmpDir/"
cd "$TmpDir"
g++ "$(basename "$File")" -o "$Output"

mv "$Output" "$Path/"

echo "Success: $Path/$Output"
