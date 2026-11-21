#include "log.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  if (filepath == NULL) {
    LOG_ERROR("filepath is required.\n\n");
    print_help(stderr);
    return EXIT_FAILURE;
  }

  LOG_DEBUG("Filepath: %s\n", filepath);

  return EXIT_SUCCESS;
}
