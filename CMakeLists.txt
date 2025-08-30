cmake_minimum_required(VERSION 3.10)
project(OS C ASM)

# Toolchain
set(CMAKE_SYSTEM_NAME Generic)
set(TOOLCHAIN_PREFIX arm-none-eabi-)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size)

# CPU flags
set(CPU_FLAGS "-mcpu=cortex-m0 -mthumb")
set(CMAKE_C_FLAGS "${CPU_FLAGS} -g -Wall -Os -ffunction-sections -fdata-sections -fno-builtin -ffreestanding" CACHE INTERNAL "")
set(CMAKE_ASM_FLAGS "${CPU_FLAGS} -g" CACHE INTERNAL "")

# Sources
set(SOURCE_FILES
    src/kmain.c
    src/boot.s
    src/hal/hal_console.c
)

# Linker script
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/src/linker.ld)

add_executable(${PROJECT_NAME}.elf ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}.elf PUBLIC src)

# Linker flags
target_link_options(${PROJECT_NAME}.elf PRIVATE
    -T ${LINKER_SCRIPT}
    -Wl,--gc-sections
    -nostdlib
    --specs=nosys.specs
    -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
)

# Post-build steps
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMENT "--- Built target: ${PROJECT_NAME}.elf ---"
)
