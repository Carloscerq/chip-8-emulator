#include "emulator.h"

static struct emulator_config {
  FILE *rom;
  size_t rom_size;
} emulator_config;

static struct cpu_state {
  uint8_t memory[TOTAL_MEMORY];
  uint8_t registers[AMOUNT_OF_REGISTERS];
  uint16_t I_register;
  uint16_t stack[STACK_NESTING];
  uint16_t program_counter;
  uint8_t stack_pointer;
  uint8_t delay_timer;
  uint8_t sound_timer;
  uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
  uint8_t keypad[KEYPAD_SIZE];
} cpu_state;

void debug_print_rom() {
  LOG_DEBUG("--- ROM DUMP ---");

  for (size_t i = MEMORY_OFFSET; i < MEMORY_OFFSET + emulator_config.rom_size;
       i += 2) {
    LOG_DEBUG("Address: 0x%04X | Opcode: %02x%02X", i, cpu_state.memory[i],
              cpu_state.memory[i + 1]);
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

  size_t bytes_read = fread(&cpu_state.memory[MEMORY_OFFSET], sizeof(uint8_t),
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

struct instruction_info decode_instruction(uint16_t opcode) {
  struct instruction_info info;
  info.opcode = opcode;

  info.x = (opcode & 0x0F00) >> 8;
  info.y = (opcode & 0x00F0) >> 4;
  info.nnn = (opcode & 0x0FFF);
  info.nn = (opcode & 0x00FF);
  info.n = (opcode & 0x000F);

  switch (opcode & 0xF000) {
  case 0x0000:
    if (info.nn == 0xE0) {
      info.type = OP_CLEAR_DISP;
    } else if (info.nn == 0xEE) {
      info.type = OP_SUB_RETURN;
    } else {
      info.type = OP_CALL;
    }
    break;

  case 0x1000:
    info.type = OP_JUMP;
    break;
  case 0x2000:
    info.type = OP_SUB_CALL;
    break;
  case 0x3000:
    info.type = OP_SKIP_NEXT_INST_IF_EQ;
    break;
  case 0x4000:
    info.type = OP_SKIP_NEXT_INST_IF_NOT_EQ;
    break;
  case 0x5000:
    info.type = OP_SKIP_NEXT_INST_IF_VX_EQ_VY;
    break;
  case 0x6000:
    info.type = OP_SET_VX;
    break;
  case 0x7000:
    info.type = OP_ADD_VX;
    break;

  case 0x8000:
    // The 8-series has multiple operations determined by the last nibble (n)
    switch (info.n) {
    case 0x0:
      info.type = OP_SET_VX_VY;
      break;
    case 0x1:
      info.type = OP_BITWISE_OR_VX_VY;
      break;
    case 0x2:
      info.type = OP_BITWISE_AND_VX_VY;
      break;
    case 0x3:
      info.type = OP_BITWISE_XOR_VX_VY;
      break;
    case 0x4:
      info.type = OP_ADD_VX_VY;
      break;
    case 0x5:
      info.type = OP_SUB_VX_VY;
      break;
    case 0x6:
      info.type = OP_RIGHT_SHIFT_VX;
      break;
    case 0x7:
      info.type = OP_SET_VX_VY_MINUS_VX;
      break;
    case 0xE:
      info.type = OP_LEFT_SHIFT_VX;
      break;
    }
    break;

  case 0x9000:
    info.type = OP_SKPI_NEXT_INST_IF_VX_NOT_EQ_VY;
    break;
  case 0xA000:
    info.type = OP_SET_I;
    break;
  case 0xB000:
    info.type = OP_JUMP_PLUS_V0;
    break;
  case 0xC000:
    info.type = OP_SET_VX_BITWISE_RAND;
    break;
  case 0xD000:
    info.type = OP_DRAWS_SPRIT;
    break;

  case 0xE000:
    // The E-series is determined by the last byte (nn)
    if (info.nn == 0x9E) {
      info.type = OP_SKIP_NEXT_INST_IF_VX_PRESSED;
    } else if (info.nn == 0xA1) {
      info.type = OP_SKIP_NEXT_INST_IF_VX_NOT_PRESSED;
    }
    break;

  case 0xF000:
    // The F-series is determined by the last byte (nn)
    switch (info.nn) {
    case 0x07:
      info.type = OP_SET_VX_DELAY_TIME;
      break;
    case 0x0A:
      info.type = OP_STORE_PRESS_VX;
      break;
    case 0x15:
      info.type = OP_SET_DELAY_TIMER_TO_VX;
      break;
    case 0x18:
      info.type = OP_SET_SOUND_TIMER_TO_VX;
      break;
    case 0x1E:
      info.type = OP_ADD_VX_TO_I;
      break;
    case 0x29:
      info.type = OP_SET_I_SPRITE_LOCATION;
      break;
    case 0x33:
      info.type = OP_STORE_BINARY_CODED_DECIMAL_I_IN_VX;
      break;
    case 0x55:
      info.type = OP_STORE_V0_TO_VX_IN_MEMORY;
      break;
    case 0x65:
      info.type = OP_FILL_V0_TO_VX_WITH_MEM;
      break;
    }
    break;
  }

  return info;
}
