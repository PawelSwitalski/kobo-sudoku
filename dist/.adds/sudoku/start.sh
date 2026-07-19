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
# export SUDOKU_IDLE_EXIT_SEC=300   # auto-exit after N seconds idle, 0 disables

# Nickel keeps running in the background otherwise (we only take over touch
# input, not the screen): it periodically repaints its own UI (e.g. the
# status-bar clock) over ours, and never repaints on our exit, leaving our
# last frame stuck on screen. Freeze it for the duration and let it resume
# (and repaint itself) once we're done -- the standard Kobo homebrew pattern.
# NOTE: while frozen, Nickel can't react to the power button or its own
# inactivity timer, so the game auto-exits after 5 min idle (see above) to
# hand control back before some other watchdog forces a hard power-off.
NICKEL_PID=$(pidof nickel)
[ -n "$NICKEL_PID" ] && kill -STOP "$NICKEL_PID"

./sudoku 2> crash.log

[ -n "$NICKEL_PID" ] && kill -CONT "$NICKEL_PID"
exit 0
