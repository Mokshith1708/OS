#!/bin/bash
set -e

echo "--- Building project with CMake ---"
# Clean and build the OS kernel
mkdir -p build
cd build
cmake ..
make
cd ..
echo "--- Build complete ---"
