#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT="/tmp/crazypod-s5l8702-i2c-test-$$"
trap 'rm -f "$OUT"' EXIT HUP INT TERM

cc -std=c99 -Wall -Wextra \
    -I"$ROOT/tests/s5l8702-i2c-stubs" \
    -I"$ROOT/firmware/export" \
    "$ROOT/tests/s5l8702-i2c-timeout-host-test.c" \
    -o "$OUT"

"$OUT"
