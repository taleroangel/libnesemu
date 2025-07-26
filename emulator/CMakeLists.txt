# Build an emulator with SDL to run on Linux
# This emulator uses the nesemu library

# Create the executable
add_executable(nesemu_linux
    main.c
)

# Link nesemu
target_link_libraries(nesemu_linux PUBLIC nesemu)
