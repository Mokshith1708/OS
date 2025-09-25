#!/usr/bin/env bash
set -euo pipefail

APP_DIR="$(cd "$(dirname "$0")" && pwd)"
OUT_DIR="$APP_DIR/build"
TOOLS_DIR="$APP_DIR/../../tools"
mkdir -p "$OUT_DIR"

arm-none-eabi-gcc -mcpu=cortex-a9 -Os -ffreestanding -nostdlib -nostartfiles \
  -T "$APP_DIR/hello_app.ld" \
  "$APP_DIR/vectors.s" "$APP_DIR/hello_app.c" \
  -o "$OUT_DIR/hello_app.elf"

arm-none-eabi-objcopy -O binary "$OUT_DIR/hello_app.elf" "$OUT_DIR/hello_app.bin"



# pad to 60 KiB = 61440
truncate -s 30720 "$OUT_DIR/hello_app.bin"

# build packer if needed
cc -O2 -o "$TOOLS_DIR/pack_app" "$TOOLS_DIR/pack_app.c"

# wrap as .proc (header + body)
"$TOOLS_DIR/pack_app" "$OUT_DIR/hello_app.bin" "$OUT_DIR/hello_app.proc"

echo "Built: $OUT_DIR/hello_app.proc (60KB + 16B header)"
