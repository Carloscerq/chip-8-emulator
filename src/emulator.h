#pragma once
#include "log.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOTAL_MEMORY 4096
#define MEMORY_OFFSET 512
#define AMOUNT_OF_REGISTERS 16
#define STACK_NESTING 16
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define KEYPAD_SIZE 16
#define INSTRUCTIONS_PER_FRAME 15

// Standard CHIP-8 Font set (0-9, A-F)
static const uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

enum INSTRUCTIONS {
  OP_CALL,                               // 0NNN
  OP_CLEAR_DISP,                         // 00E0
  OP_SUB_RETURN,                         // 00EE
  OP_JUMP,                               // 1NNN
  OP_SUB_CALL,                           // 2NNN
  OP_SKIP_NEXT_INST_IF_EQ,               // 3XNN
  OP_SKIP_NEXT_INST_IF_NOT_EQ,           // 4XNN
  OP_SKIP_NEXT_INST_IF_VX_EQ_VY,         // 5XY0
  OP_SET_VX,                             // 6XNN
  OP_ADD_VX,                             // 7XNN
  OP_SET_VX_VY,                          // 8XY0
  OP_BITWISE_OR_VX_VY,                   // 8XY1
  OP_BITWISE_AND_VX_VY,                  // 8XY2
  OP_BITWISE_XOR_VX_VY,                  // 8XY3
  OP_ADD_VX_VY,                          // 8XY4
  OP_SUB_VX_VY,                          // 8XY5
  OP_RIGHT_SHIFT_VX,                     // 8XY6
  OP_SET_VX_VY_MINUS_VX,                 // 8XY7
  OP_LEFT_SHIFT_VX,                      // 8XYE
  OP_SKIP_NEXT_INST_IF_VX_NOT_EQ_VY,     // 9XY0
  OP_SET_I,                              // ANNN
  OP_JUMP_PLUS_V0,                       // BNNN
  OP_SET_VX_BITWISE_RAND,                // CXNN
  OP_DRAWS_SPRITE,                       // DXYN
  OP_SKIP_NEXT_INST_IF_VX_PRESSED,       // EX9E
  OP_SKIP_NEXT_INST_IF_VX_NOT_PRESSED,   // EXA1
  OP_SET_VX_DELAY_TIME,                  // FX07
  OP_STORE_PRESS_VX,                     // FX0A
  OP_SET_DELAY_TIMER_TO_VX,              // FX15
  OP_SET_SOUND_TIMER_TO_VX,              // FX18
  OP_ADD_VX_TO_I,                        // FX1E
  OP_SET_I_SPRITE_LOCATION,              // FX29
  OP_STORE_BINARY_CODED_DECIMAL_I_IN_VX, // FX33
  OP_STORE_V0_TO_VX_IN_MEMORY,           // FX55
  OP_FILL_V0_TO_VX_WITH_MEM              // FX65
};

struct instruction_info {
  enum INSTRUCTIONS type;
  uint16_t opcode;
  uint8_t x;
  uint8_t y;
  uint16_t nnn;
  uint8_t nn;
  uint8_t n;
};

void init_cpu(void);

void configure_rom_file(char *filepath);

void read_rom_file(void);

uint8_t get_sound_timer(void);
void update_timers(void);
void cpu_step(void);

struct instruction_info decode_instruction(uint16_t opcode);
void execute_instruction(struct instruction_info instruction);

void clear_draw_flag(void);
bool get_draw_flag(void);
const uint8_t *get_display_buffer(void);

void set_key_state(uint8_t key, bool is_pressed);

void op_call(struct instruction_info instruction);
void op_clear_disp();
void op_sub_return();
void op_jump(struct instruction_info instruction);
void op_sub_call(struct instruction_info instruction);
void op_skip_next_inst_if_eq(struct instruction_info instruction);
void op_skip_next_inst_if_not_eq(struct instruction_info instruction);
void op_skip_next_inst_if_vx_eq_vy(struct instruction_info instruction);
void op_set_vx(struct instruction_info instruction);
void op_add_vx(struct instruction_info instruction);
void op_set_vx_vy(struct instruction_info instruction);
void op_bitwise_or_vx_vy(struct instruction_info instruction);
void op_bitwise_and_vx_vy(struct instruction_info instruction);
void op_bitwise_xor_vx_vy(struct instruction_info instruction);
void op_add_vx_vy(struct instruction_info instruction);
void op_sub_vx_vy(struct instruction_info instruction);
void op_right_shift_vx(struct instruction_info instruction);
void op_set_vx_vy_minus_vx(struct instruction_info instruction);
void op_left_shift_vx(struct instruction_info instruction);
void op_skip_next_inst_if_vx_not_eq_vy(struct instruction_info instruction);
void op_set_i(struct instruction_info instruction);
void op_jump_plus_v0(struct instruction_info instruction);
void op_set_vx_bitwise_rand(struct instruction_info instruction);
void op_draws_sprite(struct instruction_info instruction);
void op_skip_next_inst_if_vx_pressed(struct instruction_info instruction);
void op_skip_next_inst_if_vx_not_pressed(struct instruction_info instruction);
void op_set_vx_delay_time(struct instruction_info instruction);
void op_store_press_vx(struct instruction_info instruction);
void op_set_delay_timer_to_vx(struct instruction_info instruction);
void op_set_sound_timer_to_vx(struct instruction_info instruction);
void op_add_vx_to_i(struct instruction_info instruction);
void op_set_i_sprite_location(struct instruction_info instruction);
void op_store_binary_coded_decimal_i_in_vx(struct instruction_info instruction);
void op_store_v0_to_vx_in_memory(struct instruction_info instruction);
void op_fill_v0_to_vx_with_mem(struct instruction_info instruction);
