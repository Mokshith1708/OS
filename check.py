# print_vm_header.py
import sys

file_path = "/home/mokshith/Desktop/VM/vm.bin"

with open(file_path, "rb") as f:
    header = f.read(64)  # read first 64 bytes

# Print hex + ASCII side by side
for i in range(0, len(header), 16):
    chunk = header[i:i+16]
    hex_bytes = " ".join(f"{b:02X}" for b in chunk)
    ascii_bytes = "".join((chr(b) if 32 <= b <= 126 else ".") for b in chunk)
    print(f"{i:04X}: {hex_bytes:<48}  {ascii_bytes}")
