cmake_minimum_required(VERSION 3.15..4.0)

# Set the project
project(
  libneseemu
  VERSION 1.0
  LANGUAGES C)

# Setup C Version
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(
    -Wall
    -Wextra
    -pedantic
    -Wimplicit-fallthrough
)

# Project properties
if(NESEMU_DISABLE_SAFETY_CHECKS)
  add_definitions(-DCONFIG_NESEMU_DISABLE_SAFETY_CHECKS)
endif()

# Project properties
if(NESEMU_DEBUG)
    add_definitions(-DCONFIG_NESEMU_DEBUG)
    # Add sanitizers
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        add_compile_options(
            -fno-omit-frame-pointer
            -fno-optimize-sibling-calls
            -fsanitize=address,leak,undefined
        )
        add_link_options(
            -fsanitize=address,leak,undefined
        )
    elseif (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        add_compile_options(
            -fno-omit-frame-pointer
            -fno-optimize-sibling-calls
            -fsanitize=address,memory,undefined
        )
        add_link_options(
            -fsanitize=address,memory,undefined
        )
    endif()
endif()

# Build library
add_library(nesemu STATIC)

# Delegate to source
add_subdirectory(src)

# Include directories
target_include_directories(
  nesemu PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:include>)

# Build an emulator
if(NESEMU_BUILD_EMULATOR)
  add_subdirectory(emulator)
endif()

# Tests
if(NESEMU_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
