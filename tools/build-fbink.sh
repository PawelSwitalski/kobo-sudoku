#!/bin/sh
# Fetches (pinned) and cross-builds FBInk as a static library for the Kobo
# flavor (T023). Run inside WSL2 with koxtoolchain on PATH or at the default
# ~/x-tools location. Produces third_party/FBInk/Release/libfbink.a.
set -e

FBINK_TAG="${FBINK_TAG:-v1.25.5}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DIR="$ROOT/third_party/FBInk"
TC_TRIPLE=arm-kobo-linux-gnueabihf
TC_ROOT="${CROSS_TC_ROOT:-$HOME/x-tools/$TC_TRIPLE}"

if [ ! -d "$DIR/.git" ]; then
    echo ">> cloning FBInk $FBINK_TAG"
    git clone --recurse-submodules --depth 1 --branch "$FBINK_TAG" \
        https://github.com/NiLuJe/FBInk.git "$DIR"
fi

export PATH="$TC_ROOT/bin:$PATH"
command -v "$TC_TRIPLE-gcc" >/dev/null 2>&1 || {
    echo "!! $TC_TRIPLE-gcc not found — install koxtoolchain first (see cmake/kobo-toolchain.cmake)"
    exit 1
}

echo ">> building libfbink.a (Kobo, minimal + image for fbink_print_raw_data)"
make -C "$DIR" clean
CROSS_TC="$TC_TRIPLE" make -C "$DIR" staticlib KOBO=true MINIMAL=1 IMAGE=1

ls -l "$DIR/Release/libfbink.a"
echo ">> done"
