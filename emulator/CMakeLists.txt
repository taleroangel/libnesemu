# This emulator uses the nesemu library
message(NOTICE "NESEMU: Emulator will be build!")
message(NOTICE "NESEMU_EMULATOR: raylib will be downloaded")

# Download raylib
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)

# Raylib setup
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

if (LINUX)
    set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)
endif()

# Create the executable
add_executable(nesemu_exe src/main.c)

# Link nesemu
target_link_libraries(nesemu_exe PUBLIC nesemu)

# Link raylib
FetchContent_MakeAvailable(raylib)
target_link_libraries(nesemu_exe PRIVATE raylib)
