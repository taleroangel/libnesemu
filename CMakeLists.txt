cmake_minimum_required(VERSION 3.15..4.0)

# Set the project
project(
  libneseemu
  VERSION 1.0
  LANGUAGES C)

# Setup C Version
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -pedantic -Wimplicit-fallthrough)

# Project properties
if(NESEMU_DISABLE_SAFETY_CHECKS)
  add_definitions(-DCONFIG_NESEMU_DISABLE_SAFETY_CHECKS)
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
  message("Nesemu Linux emulator executable will be build!")
  add_subdirectory(emulator)
endif()
