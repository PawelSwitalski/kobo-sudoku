#!/bin/sh
# Builds the copy-to-device zip (T029): unzip onto the Kobo's USB root, eject,
# done. Usage: tools/package.sh <build-dir>   (e.g. tools/package.sh build/kobo)
set -e

BUILD_DIR="${1:-build/kobo}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$BUILD_DIR/sudoku"
STAGE="$(mktemp -d)"
OUT="$ROOT/dist/kobo-sudoku.zip"

[ -f "$BIN" ] || { echo "!! $BIN not found — build the fbink flavor first"; exit 1; }

mkdir -p "$STAGE/.adds/sudoku/assets" "$STAGE/.adds/kfmon/config"
cp "$BIN"                                   "$STAGE/.adds/sudoku/sudoku"
cp "$ROOT/dist/.adds/sudoku/start.sh"       "$STAGE/.adds/sudoku/"
cp "$ROOT/dist/.adds/sudoku/nm-sudoku.txt"  "$STAGE/.adds/sudoku/"
cp "$ROOT"/dist/.adds/sudoku/assets/*       "$STAGE/.adds/sudoku/assets/"
cp "$ROOT/dist/kfmon/config/sudoku.ini"     "$STAGE/.adds/kfmon/config/"
cp "$ROOT/dist/kfmon-sudoku.png"            "$STAGE/kfmon-sudoku.png"
chmod +x "$STAGE/.adds/sudoku/sudoku" "$STAGE/.adds/sudoku/start.sh"

rm -f "$OUT"
(cd "$STAGE" && zip -r -X "$OUT" .adds kfmon-sudoku.png)
rm -rf "$STAGE"
echo ">> $OUT"
