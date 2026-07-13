#!/bin/sh
# KFMon launch wrapper (contracts/install-layout.md).
# stderr goes to crash.log (truncated each run) for field debugging.
cd /mnt/onboard/.adds/sudoku || exit 1

# Touch-panel calibration overrides, if this model needs them (see README):
# export SUDOKU_TOUCH_SWAP_XY=1
# export SUDOKU_TOUCH_MIRROR_X=1
# export SUDOKU_TOUCH_MIRROR_Y=1
# export SUDOKU_TOUCH_DEBUG=1
# export SUDOKU_GHOSTING_N=12

./sudoku 2> crash.log
exit 0
