# #!/bin/bash
# set -e
# rm -rf build
# mkdir -p build
# cd build
# cmake ..
# make
# cd ..
# qemu-system-arm \
#   -M lm3s6965evb -cpu cortex-m3 \
#   -nographic \
#   -semihosting-config enable=on,target=native \
#   -kernel build/OS.elf
# echo "QEMU has exited."


#!/bin/bash
set -e
rm -rf build
mkdir -p build
cd build
cmake ..
make
cd ..
qemu-system-arm \
  -M lm3s6965evb -cpu cortex-m3 \
  -nographic \
  -semihosting-config enable=on,target=native \
  -kernel build/OS.elf
echo "QEMU has exited."


  

