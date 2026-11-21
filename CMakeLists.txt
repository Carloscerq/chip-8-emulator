cmake_minimum_required(VERSION 3.10)

project(chip-8-emulator C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

set(SOURCES
    src/main.c
)

add_executable(chip8_emulator ${SOURCES})
target_compile_options(chip8_emulator PRIVATE -Wall -Wextra -pedantic)
