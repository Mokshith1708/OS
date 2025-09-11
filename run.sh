#!/bin/bash
set -e

# The name of the file where the full log will be saved
LOG_FILE="qemu_full_output.log"

echo "--- Building project ---"
# This part is the same as before
rm -rf build
mkdir -p build
cd build
cmake ..
make
cd ..
echo "--- Build complete ---"

echo
echo "--- Starting QEMU ---"
echo "The FULL output, including the memory dump, will be saved to the file: '$LOG_FILE'"
echo "Interact with QEMU as normal. Quit with [Ctrl]+[A], then [X]"
echo

# Run QEMU and pipe ALL its output to the 'tee' command.
# This shows the output on the screen AND saves it to the log file.
qemu-system-arm \
  -M lm3s6965evb -cpu cortex-m3 \
  -nographic \
  -semihosting-config enable=on,target=native \
  -kernel build/OS.elf | tee "$LOG_FILE"

echo
echo "--- QEMU has exited. Full log saved to '$LOG_FILE' ---"