#include "emulator.h"
#include "log.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static struct emulator_config {
  FILE *rom;
  uint8_t memory[TOTAL_MEMORY];
  size_t rom_size;
} emulator_config;

void debug_print_rom() {
  LOG_DEBUG("--- ROM DUMP ---");

  for (size_t i = MEMORY_OFFSET; i < MEMORY_OFFSET + emulator_config.rom_size;
       i += 2) {
    LOG_DEBUG("Address: 0x%04X | Opcode: %02x%02X", i,
              emulator_config.memory[i], emulator_config.memory[i + 1]);
  }

  LOG_DEBUG("---------------");
}

void configure_rom_file(char *filepath) {
  LOG_DEBUG("Reading rom file (%s) as binary", filepath);
  emulator_config.rom = fopen(filepath, "rb");
  if (emulator_config.rom == NULL) {
    LOG_ERROR("Error while reading rom file.");
    exit(EXIT_FAILURE);
  }
}

void read_rom_file() {
  fseek(emulator_config.rom, 0, SEEK_END);
  emulator_config.rom_size = ftell(emulator_config.rom);
  rewind(emulator_config.rom); // Rewind the pointer to the beggining
  LOG_DEBUG("ROM Size: %zu", emulator_config.rom_size);

  if (emulator_config.rom_size > (TOTAL_MEMORY - MEMORY_OFFSET)) {
    LOG_ERROR("ROM file is too large");
    fclose(emulator_config.rom);
    exit(EXIT_FAILURE);
  }

  size_t bytes_read =
      fread(&emulator_config.memory[MEMORY_OFFSET], sizeof(uint8_t),
            emulator_config.rom_size, emulator_config.rom);

  if (bytes_read != emulator_config.rom_size) {
    LOG_ERROR("Failed to read the entire ROM");
    fclose(emulator_config.rom);
    exit(EXIT_FAILURE);
  }

  fclose(emulator_config.rom);

#ifdef DUMP_ROM
  debug_print_rom();
#endif
}
