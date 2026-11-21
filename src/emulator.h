#pragma once
#include "log.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define TOTAL_MEMORY 4096
#define MEMORY_OFFSET 512
#define AMOUNT_OF_REGISTERS 16
#define STACK_NESTING 16
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define KEYPAD_SIZE 16

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
  OP_SKPI_NEXT_INST_IF_VX_NOT_EQ_VY,     // 9XY0
  OP_SET_I,                              // ANNN
  OP_JUMP_PLUS_V0,                       // BNNN
  OP_SET_VX_BITWISE_RAND,                // CXNN
  OP_DRAWS_SPRIT,                        // DXYN
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

void configure_rom_file(char *filepath);

void read_rom_file();

struct instruction_info decode_instruction(uint16_t opcode);
