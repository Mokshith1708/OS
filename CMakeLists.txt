cmake_minimum_required(VERSION 3.10)
project(OS C ASM) # We will add CXX later when cxx_support.cpp is needed

set(CMAKE_SYSTEM_NAME Generic)
set(TOOLCHAIN_PREFIX arm-none-eabi-)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size)
set(CMAKE_AR ${TOOLCHAIN_PREFIX}ar) # Add the archiver tool

set(CPU_FLAGS "-mcpu=cortex-m0 -mthumb")
set(CMAKE_C_FLAGS "${CPU_FLAGS} -g -Wall -Os -ffunction-sections -fdata-sections" CACHE INTERNAL "")
set(CMAKE_ASM_FLAGS "${CPU_FLAGS} -g -mthumb-interwork" CACHE INTERNAL "")

set(SOURCE_FILES
    src/kmain.c
    src/boot.s
    src/hal/hal_console.c
    src/pmm.c
    src/proc.c
    src/semihost.c
    src/swap.c
    src/jump_trampoline.s
    src/syscalls.c
    src/fault.c
    src/interrupt.c
    src/systick.c
    src/drivers/display.c
    src/ramdisk.c
    src/fs.c
    src/kmalloc.c
)
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/src/linker.ld)

add_executable(${PROJECT_NAME}.elf ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME}.elf PUBLIC src)

target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T ${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,-Map=${PROJECT_NAME}.map
)
# No need to explicitly link gcc, the compiler driver does that.

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMENT "--- Built OS Kernel: ${PROJECT_NAME}.elf ---"
)

# ------------------------------------------------------------------
# --- NEW SECTION: Build the User System Call Library (libuser.a) ---
# ------------------------------------------------------------------

# This is the new part. It defines a second "product" to be built.
# It tells CMake where to put the final library file (.a)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Define the source files for our user-space library
set(LIBUSER_SOURCE_FILES
    apps/libuser/user_syscalls.c
    apps/libuser/stdio.c
    apps/libuser/string.c
    apps/libuser/malloc.c
    src/libuser/user_startup.S
)

# This command creates the static library target named "user"
add_library(user STATIC ${LIBUSER_SOURCE_FILES})

# We must compile the library with the same Cortex-M0 flags
# and add include directory for the library itself
target_include_directories(user PUBLIC src/libuser)

# Apply compile flags properly for C and ASM files
target_compile_options(user PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-mcpu=cortex-m0>
    $<$<COMPILE_LANGUAGE:C>:-mthumb>
    $<$<COMPILE_LANGUAGE:C>:-ffunction-sections>
    $<$<COMPILE_LANGUAGE:C>:-fdata-sections>
    $<$<COMPILE_LANGUAGE:ASM>:-mcpu=cortex-m0>
    $<$<COMPILE_LANGUAGE:ASM>:-mthumb>
)

# A message to show where the library is built
add_custom_command(TARGET user POST_BUILD
    COMMAND echo "--- Built Cortex-M0 User Library: libuser.a in build/ ---"
)

# ------------------------------------------------------------------
# --- NEW SECTION: Build Host-side Tools (pack_app, mkfs)      ---
# ------------------------------------------------------------------

# These tools run on the build machine, not the target.
# We compile them directly using the host's native compiler.

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/pack_app
    COMMAND gcc -O2 -o ${CMAKE_BINARY_DIR}/pack_app ${CMAKE_CURRENT_SOURCE_DIR}/tools/pack_app.c
    DEPENDS tools/pack_app.c
    COMMENT "Building host tool: pack_app"
)
add_custom_target(pack_app_target ALL DEPENDS ${CMAKE_BINARY_DIR}/pack_app)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/mkfs
    COMMAND gcc -O2 -o ${CMAKE_BINARY_DIR}/mkfs ${CMAKE_CURRENT_SOURCE_DIR}/tools/mkfs.c
    DEPENDS tools/mkfs.c
    COMMENT "Building host tool: mkfs"
)
add_custom_target(mkfs_target ALL DEPENDS ${CMAKE_BINARY_DIR}/mkfs)

