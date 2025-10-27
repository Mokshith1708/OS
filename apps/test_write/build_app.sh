#!/usr/bin/env bash
set -euo pipefail

APP_DIR="$(cd "$(dirname "$0")" && pwd)"
OUT_DIR="$APP_DIR/build"
TOOLS_DIR="$APP_DIR/../../tools"
mkdir -p "$OUT_DIR"

arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Os -ffreestanding -nostdlib -nostartfiles \
  -T "$APP_DIR/test_write.ld" \
  "$APP_DIR/vectors.s" "$APP_DIR/test_write.c" \
  -L"$APP_DIR/../../build" -luser \
  -o "$OUT_DIR/test_write.elf"

arm-none-eabi-objcopy -O binary "$OUT_DIR/test_write.elf" "$OUT_DIR/test_write.bin"



# pad to 60 KiB = 61440
truncate -s 30720 "$OUT_DIR/test_write.bin"

# build packer if needed
cc -O2 -o "$TOOLS_DIR/pack_app" "$TOOLS_DIR/pack_app.c"

# wrap as .proc (header + body)
"$TOOLS_DIR/pack_app" "$OUT_DIR/test_write.bin" "$OUT_DIR/test_write.proc"

echo "Built: $OUT_DIR/test_write.proc (60KB + 16B header)"
