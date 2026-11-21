#include "emulator.h"
#include "log.h"
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DO_UNIT_TEST
void test_decode_instruction() {
  struct instruction_info info;

  // Test 1: OP_CLEAR_DISP (0x00E0)
  info = decode_instruction(0x00E0);
  assert(info.type == OP_CLEAR_DISP);
  assert(info.nn == 0xE0);

  // Test 2: OP_JUMP (0x1234)
  // 1NNN means jump to NNN. So NNN should be 0x234.
  info = decode_instruction(0x1234);
  assert(info.type == OP_JUMP);
  assert(info.nnn == 0x234);

  // Test 3: OP_ADD_VX_VY (0x8AB4)
  // 8XY4 means add Vy to Vx. X = A, Y = B, N = 4.
  info = decode_instruction(0x8AB4);
  assert(info.type == OP_ADD_VX_VY);
  assert(info.x == 0x0A);
  assert(info.y == 0x0B);
  assert(info.n == 0x04);

  // Test 4: OP_SET_I (0xAFFF)
  // ANNN means set I to NNN. NNN should be 0xFFF.
  info = decode_instruction(0xAFFF);
  assert(info.type == OP_SET_I);
  assert(info.nnn == 0xFFF);

  // Test 5: OP_STORE_PRESS_VX (0xF30A)
  // FX0A means wait for key press and store in Vx. X = 3.
  info = decode_instruction(0xF30A);
  assert(info.type == OP_STORE_PRESS_VX);
  assert(info.x == 0x03);
  assert(info.nn == 0x0A);

  LOG_INFO("All decode tests passed successfully!");
}
#endif

void print_help(FILE *stream) {
  fprintf(stream, "Usage: chip8_emulator [OPTIONS]\n\n");
  fprintf(stream, "Options:\n");
  fprintf(
      stream,
      "  -f, --filepath <path>   Specify the target ROM filepath (Required)\n");
  fprintf(stream,
          "  -l, --level <level>     Set the logging level (Default: WARN)\n");
  fprintf(stream, "  -o, --logfile <path>    Specify a file to write logs to "
                  "(Default: stdout)\n");
  fprintf(stream,
          "  -h, --help              Display this help message and exit\n");
}

int main(int argc, char *argv[]) {
  int opt;
  char *filepath = NULL;
  enum LOG_LEVEL log_level = DEFAULT_LOG_LEVEL;
  char *log_filepath = NULL;

  struct option long_options[] = {{"filepath", required_argument, 0, 'f'},
                                  {"level", required_argument, 0, 'l'},
                                  {"logfile", required_argument, 0, 'o'},
                                  {"help", no_argument, 0, 'h'},
                                  {0, 0, 0, 0}};

  while ((opt = getopt_long(argc, argv, "f:l:h", long_options, NULL)) != -1) {
    switch (opt) {
    case 'f':
      filepath = optarg;
      break;
    case 'l':
      log_level = get_log_level_from_string(optarg);
      break;
    case 'h':
      print_help(stdout);
      return EXIT_SUCCESS;
    case '?':
      print_help(stderr);
      return EXIT_FAILURE;
    }
  }

  log_init(log_level, log_filepath);

#ifdef DO_UNIT_TEST
  test_decode_instruction();
#endif

  if (filepath == NULL) {
    LOG_ERROR("filepath is required.\n\n");
    print_help(stderr);
    return EXIT_FAILURE;
  }

  LOG_DEBUG("Filepath: %s\n", filepath);

  configure_rom_file(filepath);
  read_rom_file();

  return EXIT_SUCCESS;
}
