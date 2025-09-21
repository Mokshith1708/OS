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
  -M mps2-an385 \
  -cpu cortex-m3 \
  -m 16M \
  -nographic \
  -serial null -serial mon:stdio \
  -semihosting \
  -kernel build/OS.elf | tee "$LOG_FILE"

echo
echo "--- QEMU has exited. Full log saved to '$LOG_FILE' ---"


# #!/bin/bash
# set -e

# # --- Build the Project ---
# # Note: You have this script in your OS repo, but it should be named setup.sh
# # The run.sh script should ideally just run, not build. Let's combine for now.
# echo "--- Building project ---"
# rm -rf build
# mkdir build
# cd build
# cmake ..
# make
# cd ..
# echo "--- Build complete ---"

# DISK_IMAGE="backing_store.img"
# LOG_FILE="qemu_full_output.log"

# echo
# echo "--- Starting QEMU ---"
# echo "The FULL output will be saved to the file: '$LOG_FILE'"
# echo "Interact with QEMU as normal. Quit with [Ctrl]+[A], then [X]"
# echo

# # --- The Fix is Here ---
# # Use a machine that supports Cortex-M3 and has a simpler memory map.
# # We will use the -kernel option which is simpler than a full bootloader.

# qemu-system-arm \
#   -M virt \
#   -cpu cortex-a9 \
#   -m 256M \
#   -nographic \
#   -kernel build/OS.elf \
#   -drive file=backing_store.img,format=raw,if=none,id=disk \
#   -device virtio-blk-device,drive=disk
# echo
# echo "--- QEMU has exited. Full log saved to '$LOG_FILE' ---"