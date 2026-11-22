cmake_minimum_required(VERSION 3.14)

include(FetchContent)

project(chip-8-emulator C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

option(DUMP_ROM "Enable ROM debug dumping on startup" OFF)
option(DO_UNIT_TEST "Enable Unit tests" OFF)

# Use a system-installed raylib (e.g. the one baked into the devcontainer),
# otherwise download and build it as part of the project.
find_package(raylib 6.2 QUIET)
if(NOT raylib_FOUND)
  set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

  FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/6.2.tar.gz
  )
  FetchContent_MakeAvailable(raylib)
endif()

set(SOURCES
    src/main.c
    src/log.h
    src/log.c
    src/emulator.h
    src/emulator.c
)

add_executable(chip8_emulator ${SOURCES})
target_compile_options(chip8_emulator PRIVATE -Wall -Wextra -pedantic)
target_link_libraries(chip8_emulator PRIVATE raylib)

if(DUMP_ROM)
  target_compile_definitions(chip8_emulator PRIVATE DUMP_ROM)
endif()

if(DO_UNIT_TEST)
  target_compile_definitions(chip8_emulator PRIVATE DO_UNIT_TEST)
endif()
