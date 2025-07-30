# Build an emulator with SDL to run on Linux
# This emulator uses the nesemu library
message(NOTICE "NESEMU: linux emulator will be build!. This will require you to have SDL libraries.")

# Create the executable
add_executable(nesemu_linux
    main.c
)

# Link nesemu
target_link_libraries(nesemu_linux PUBLIC nesemu)
