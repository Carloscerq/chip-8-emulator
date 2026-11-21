#pragma once
#include <stdint.h>
#include <stdio.h>

#define TOTAL_MEMORY 4096
#define MEMORY_OFFSET 512

void configure_rom_file(char *filepath);

void read_rom_file();
