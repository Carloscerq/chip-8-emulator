cmake_minimum_required(VERSION 3.10)

project(chip-8-emulator C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

option(DUMP_ROM "Enable ROM debug dumping on startup" OFF)
option(DO_UNIT_TEST "Enable Unit tests" OFF)

set(SOURCES
    src/main.c
    src/log.h
    src/log.c
    src/emulator.h
    src/emulator.c
)

add_executable(chip8_emulator ${SOURCES})
target_compile_options(chip8_emulator PRIVATE -Wall -Wextra -pedantic)

if(DUMP_ROM)
  target_compile_definitions(chip8_emulator PRIVATE DUMP_ROM)
endif()

if(DO_UNIT_TEST)
  target_compile_definitions(chip8_emulator PRIVATE DO_UNIT_TEST)
endif()
