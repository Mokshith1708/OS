#!/bin/bash
set -e

# 1. Build the tools
rm -rf build
mkdir build
cd build
cmake ..
make pack_app_target
make mkfs_target
cd ..

# 2. Build libuser.a
cd build
make user
cd ..

# 3. Build the apps
bash apps/shell/build_app.sh
bash apps/hello/build_app.sh
bash apps/test_write/build_app.sh

# 4. Create the ramdisk image
./build/mkfs build/ramdisk.img apps/shell/build/shell_app.proc apps/hello/build/hello_app.proc apps/test_write/build/test_write.proc

# 5. Convert ramdisk.img to ramdisk.o
ld -r -b binary build/ramdisk.img -o build/ramdisk.o

# 6. Re-run make to build the final kernel
cd build
make
cd ..
