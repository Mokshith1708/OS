echo "=== Debug Information ==="

# Check if the ELF file has the right sections
echo "--- ELF file analysis ---"
arm-none-eabi-objdump -h build/OS.elf

echo -e "\n--- Symbol table ---"
arm-none-eabi-nm build/OS.elf

echo -e "\n--- Disassembly of Reset_Handler ---"
arm-none-eabi-objdump -d build/OS.elf | grep -A 10 "Reset_Handler"

echo -e "\n--- Disassembly of kmain ---"
arm-none-eabi-objdump -d build/OS.elf | grep -A 10 "kmain"

echo -e "\n--- Memory map ---"
head -20 build/OS.map

echo -e "\n=== Testing with GDB simulation ==="
echo "Try running: arm-none-eabi-gdb build/OS.elf"
echo "Then in GDB: target sim, load, run"