# Build an emulator with SDL to run on Linux
# This emulator uses the nesemu library
message(NOTICE "NESEMU: linux emulator will be build!. This will require you to have SDL libraries.")

# Dependencies
find_package(SDL3 REQUIRED CONFIG)

# Create the executable
add_executable(nesemu_linux
    src/main.c
)

# Link nesemu
target_link_libraries(nesemu_linux PUBLIC nesemu)

# Link SDL3
target_link_libraries(nesemu_linux PRIVATE SDL3::SDL3)
