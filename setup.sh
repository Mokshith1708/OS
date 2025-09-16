#!/bin/bash
set -e

# The name of our virtual disk for the backing store
DISK_IMAGE="backing_store.img"

# --- Create the 1GB Virtual Disk ---
echo "--- Creating 1GB virtual disk image: $DISK_IMAGE ---"
if [ -f "$DISK_IMAGE" ]; then
    echo "$DISK_IMAGE already exists. Skipping creation."
else
    qemu-img create -f raw "$DISK_IMAGE" 1G
    echo "Successfully created $DISK_IMAGE."
fi

echo
echo "--- Building project with CMake ---"
# Clean and build the OS kernel
rm -rf build
mkdir -p build
cd build
cmake ..
make
cd ..
echo "--- Build complete ---"

echo
echo "Setup is complete. You can now:"
echo "1. Prepare the disk image with user programs (using a 'mkfs' tool)."
echo "2. Run the OS using the './run.sh' script."