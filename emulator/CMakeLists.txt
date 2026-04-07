# Build a simple SDL emulator
# This emulator uses the nesemu library
message(NOTICE "NESEMU: Emulator will be build!. This will require you to have SDL libraries.")

# Dependencies
# In the future this will be migrated to FetchContent or git submodule
# instead of relying on the system's package manager
find_package(SDL3 REQUIRED CONFIG)

# Create the executable
add_executable(nesemu_exe src/main.c)

# Link nesemu
target_link_libraries(nesemu_exe PUBLIC nesemu)

# Link SDL3
target_link_libraries(nesemu_exe PRIVATE SDL3::SDL3)
