#include "emulator.h"
#include "log.h"
#include <stdint.h>
#include <string.h>

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
  bool draw_flag;
} cpu_state;

void init_cpu(void) {
  LOG_DEBUG("Initializing the CPU State");

  memset(&cpu_state, 0, sizeof(cpu_state));
  cpu_state.program_counter = MEMORY_OFFSET;

  for (int i = 0; i < 80; i++) {
    cpu_state.memory[i] = fontset[i];
  }
}

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

void read_rom_file(void) {
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
    info.type = OP_SKIP_NEXT_INST_IF_VX_NOT_EQ_VY;
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
    info.type = OP_DRAWS_SPRITE;
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

void op_call(struct instruction_info instruction) {
  LOG_DEBUG("Ignored SYS instruction at 0x%03X", instruction.nnn);
}

void op_clear_disp() {
  LOG_DEBUG("Running OP_CLEAR instruction");
  memset(cpu_state.display, 0, sizeof(cpu_state.display));
  cpu_state.draw_flag = true;
}

void op_sub_return() {
  LOG_DEBUG("Running OP_SUB_RETURN instruction. Going back to %d",
            cpu_state.stack_pointer - 1);
  if (cpu_state.stack_pointer <= 0) {
    LOG_WARN("Got an empty stack");
    return;
  }

  cpu_state.stack_pointer--;
  cpu_state.program_counter = cpu_state.stack[cpu_state.stack_pointer];
}

void op_jump(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_JUMP instruction. Going to: %d", instruction.nnn);

  cpu_state.program_counter = instruction.nnn;
}

void op_sub_call(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SUB_CALL instruction. Going to %d", instruction.nnn);
  if (cpu_state.stack_pointer >= STACK_NESTING) {
    LOG_WARN("Got a full stack");
    return;
  }

  cpu_state.stack[cpu_state.stack_pointer] = cpu_state.program_counter;
  cpu_state.stack_pointer++;
  cpu_state.program_counter = instruction.nnn;
}

void op_skip_next_inst_if_eq(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SKIP_NEXT_INST_IF_EQ instruction. V%X == 0x%02X?",
            instruction.x, instruction.nn);
  if (cpu_state.registers[instruction.x] == instruction.nn) {
    cpu_state.program_counter += 2;
  }
}

void op_skip_next_inst_if_not_eq(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SKIP_NEXT_INST_IF_NOT_EQ instruction. V%X != 0x%02X?",
            instruction.x, instruction.nn);
  if (cpu_state.registers[instruction.x] != instruction.nn) {
    cpu_state.program_counter += 2;
  }
}

void op_skip_next_inst_if_vx_eq_vy(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SKIP_NEXT_INST_IF_VX_EQ_VY instruction. V%X == V%X?",
            instruction.x, instruction.y);
  if (cpu_state.registers[instruction.x] ==
      cpu_state.registers[instruction.y]) {
    cpu_state.program_counter += 2;
  }
}

void op_skip_next_inst_if_vx_not_eq_vy(struct instruction_info instruction) {
  LOG_DEBUG(
      "Running OP_SKIP_NEXT_INST_IF_VX_NOT_EQ_VY instruction. V%X != V%X?",
      instruction.x, instruction.y);
  if (cpu_state.registers[instruction.x] !=
      cpu_state.registers[instruction.y]) {
    cpu_state.program_counter += 2;
  }
}

void op_set_vx(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SET_VX instruction. V%X = 0x%02X", instruction.x,
            instruction.nn);
  cpu_state.registers[instruction.x] = instruction.nn;
}

void op_add_vx(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_ADD_VX instruction. V%X += 0x%02X", instruction.x,
            instruction.nn);
  cpu_state.registers[instruction.x] += instruction.nn;
}

void op_set_vx_vy(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SET_VX_VY instruction. V%X = V%X", instruction.x,
            instruction.y);
  cpu_state.registers[instruction.x] = cpu_state.registers[instruction.y];
}

void op_bitwise_or_vx_vy(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_BITWISE_OR_VX_VY instruction. V%X |= V%X",
            instruction.x, instruction.y);
  cpu_state.registers[instruction.x] =
      cpu_state.registers[instruction.x] | cpu_state.registers[instruction.y];
}

void op_bitwise_and_vx_vy(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_BITWISE_AND_VX_VY instruction. V%X &= V%X",
            instruction.x, instruction.y);
  cpu_state.registers[instruction.x] =
      cpu_state.registers[instruction.x] & cpu_state.registers[instruction.y];
}

void op_bitwise_xor_vx_vy(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_BITWISE_XOR_VX_VY instruction. V%X ^= V%X",
            instruction.x, instruction.y);
  cpu_state.registers[instruction.x] =
      cpu_state.registers[instruction.x] ^ cpu_state.registers[instruction.y];
}

void op_add_vx_vy(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_ADD_VX_VY instruction");

  uint16_t sum =
      cpu_state.registers[instruction.x] + cpu_state.registers[instruction.y];
  cpu_state.registers[instruction.x] = sum & 0xFF;

  if (sum > 255) {
    cpu_state.registers[0xF] = 1;
  } else {
    cpu_state.registers[0xF] = 0;
  }
}

void op_sub_vx_vy(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SUB_VX_VY instruction");

  uint8_t flag =
      (cpu_state.registers[instruction.x] >= cpu_state.registers[instruction.y])
          ? 1
          : 0;
  cpu_state.registers[instruction.x] -= cpu_state.registers[instruction.y];
  cpu_state.registers[0xF] = flag;
}

void op_right_shift_vx(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_RIGHT_SHIFT_VX instruction. V%X >>= 1", instruction.x);

  uint8_t vx = cpu_state.registers[instruction.x];
  cpu_state.registers[instruction.x] = vx >> 1;
  cpu_state.registers[0xF] = vx & 0x1;
}

void op_set_vx_vy_minus_vx(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SET_VX_VY_MINUS_VX instruction. V%X = V%X - V%X",
            instruction.x, instruction.y, instruction.x);

  uint8_t vx = cpu_state.registers[instruction.x];
  uint8_t vy = cpu_state.registers[instruction.y];
  cpu_state.registers[instruction.x] = vy - vx;
  cpu_state.registers[0xF] = vy >= vx ? 1 : 0;
}

void op_left_shift_vx(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_LEFT_SHIFT_VX instruction. V%X <<= 1", instruction.x);

  uint8_t vx = cpu_state.registers[instruction.x];
  cpu_state.registers[instruction.x] = (uint8_t)(vx << 1);
  cpu_state.registers[0xF] = (vx & 0x80) >> 7;
}

void op_set_i(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SET_I instruction. I = 0x%03X", instruction.nnn);

  cpu_state.I_register = instruction.nnn;
}

void op_jump_plus_v0(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_JUMP_PLUS_V0 instruction. Going to 0x%03X + V0",
            instruction.nnn);

  cpu_state.program_counter =
      (instruction.nnn + cpu_state.registers[0]) & 0xFFF;
}

void op_set_vx_bitwise_rand(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SET_VX_BITWISE_RAND instruction. V%X = rand() & 0x%02X",
            instruction.x, instruction.nn);

  cpu_state.registers[instruction.x] =
      (uint8_t)(rand() & 0xFF) & instruction.nn;
}

void op_draws_sprite(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_DRAWS_SPRITE instruction. Sprite of %d rows at "
            "(V%X, V%X)",
            instruction.n, instruction.x, instruction.y);

  uint8_t x_start = cpu_state.registers[instruction.x] % DISPLAY_WIDTH;
  uint8_t y_start = cpu_state.registers[instruction.y] % DISPLAY_HEIGHT;

  cpu_state.registers[0xF] = 0;

  for (uint8_t row = 0; row < instruction.n; row++) {
    uint8_t y = y_start + row;
    if (y >= DISPLAY_HEIGHT) {
      break; // Sprites are clipped, not wrapped
    }

    uint8_t sprite_byte =
        cpu_state.memory[(cpu_state.I_register + row) % TOTAL_MEMORY];

    for (uint8_t col = 0; col < 8; col++) {
      uint8_t x = x_start + col;
      if (x >= DISPLAY_WIDTH) {
        break;
      }

      if (sprite_byte & (0x80 >> col)) {
        uint8_t *pixel = &cpu_state.display[y * DISPLAY_WIDTH + x];
        if (*pixel) {
          cpu_state.registers[0xF] = 1;
        }
        *pixel ^= 1;
      }
    }
  }

  cpu_state.draw_flag = true;
}

void op_skip_next_inst_if_vx_pressed(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SKIP_NEXT_INST_IF_VX_PRESSED instruction. V%X pressed?",
            instruction.x);

  if (cpu_state.keypad[cpu_state.registers[instruction.x] & 0xF]) {
    cpu_state.program_counter += 2;
  }
}

void op_skip_next_inst_if_vx_not_pressed(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SKIP_NEXT_INST_IF_VX_NOT_PRESSED instruction. V%X not "
            "pressed?",
            instruction.x);

  if (!cpu_state.keypad[cpu_state.registers[instruction.x] & 0xF]) {
    cpu_state.program_counter += 2;
  }
}

void op_set_vx_delay_time(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SET_VX_DELAY_TIME instruction. V%X = delay timer",
            instruction.x);

  cpu_state.registers[instruction.x] = cpu_state.delay_timer;
}

void op_store_press_vx(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_STORE_PRESS_VX instruction. Waiting for a key press");

  for (uint8_t key = 0; key < KEYPAD_SIZE; key++) {
    if (cpu_state.keypad[key]) {
      cpu_state.registers[instruction.x] = key;
      return;
    }
  }

  // No key pressed: repeat this instruction on the next cycle
  cpu_state.program_counter -= 2;
}

void op_set_delay_timer_to_vx(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SET_DELAY_TIMER_TO_VX instruction. Delay timer = V%X",
            instruction.x);

  cpu_state.delay_timer = cpu_state.registers[instruction.x];
}

void op_set_sound_timer_to_vx(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SET_SOUND_TIMER_TO_VX instruction. Sound timer = V%X",
            instruction.x);

  cpu_state.sound_timer = cpu_state.registers[instruction.x];
}

void op_add_vx_to_i(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_ADD_VX_TO_I instruction. I += V%X", instruction.x);

  cpu_state.I_register += cpu_state.registers[instruction.x];
}

void op_set_i_sprite_location(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_SET_I_SPRITE_LOCATION instruction. I = sprite of V%X",
            instruction.x);

  // Each font glyph is 5 bytes long and the fontset is stored at address 0
  cpu_state.I_register = (cpu_state.registers[instruction.x] & 0xF) * 5;
}

void op_store_binary_coded_decimal_i_in_vx(
    struct instruction_info instruction) {
  LOG_DEBUG("Running OP_STORE_BINARY_CODED_DECIMAL_I_IN_VX instruction. BCD of "
            "V%X at I",
            instruction.x);

  if (cpu_state.I_register + 2 >= TOTAL_MEMORY) {
    LOG_WARN("BCD write out of memory bounds");
    return;
  }

  uint8_t value = cpu_state.registers[instruction.x];
  cpu_state.memory[cpu_state.I_register] = value / 100;
  cpu_state.memory[cpu_state.I_register + 1] = (value / 10) % 10;
  cpu_state.memory[cpu_state.I_register + 2] = value % 10;
}

void op_store_v0_to_vx_in_memory(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_STORE_V0_TO_VX_IN_MEMORY instruction. V0..V%X to I",
            instruction.x);

  if (cpu_state.I_register + instruction.x >= TOTAL_MEMORY) {
    LOG_WARN("Register dump out of memory bounds");
    return;
  }

  for (uint8_t i = 0; i <= instruction.x; i++) {
    cpu_state.memory[cpu_state.I_register + i] = cpu_state.registers[i];
  }
}

void op_fill_v0_to_vx_with_mem(struct instruction_info instruction) {
  LOG_DEBUG("Running OP_FILL_V0_TO_VX_WITH_MEM instruction. V0..V%X from I",
            instruction.x);

  if (cpu_state.I_register + instruction.x >= TOTAL_MEMORY) {
    LOG_WARN("Register load out of memory bounds");
    return;
  }

  for (uint8_t i = 0; i <= instruction.x; i++) {
    cpu_state.registers[i] = cpu_state.memory[cpu_state.I_register + i];
  }
}

void execute_instruction() {
  // TODO: Implement this
}
