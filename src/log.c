#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static struct {
  enum LOG_LEVEL log_level;
  FILE *output_stream;
} log_configs = {DEFAULT_LOG_LEVEL, NULL};

static const char *level_strings[] = {"DEBUG", "INFO", "WARN", "ERROR"};

enum LOG_LEVEL get_log_level_from_string(char *log_level) {
  if (strcmp(log_level, "DEBUG") == 0)
    return DEBUG;
  if (strcmp(log_level, "INFO") == 0)
    return INFO;
  if (strcmp(log_level, "WARN") == 0)
    return WARN;
  if (strcmp(log_level, "ERROR") == 0)
    return ERROR;

  return DEFAULT_LOG_LEVEL;
}

void set_log_configs_level(enum LOG_LEVEL log_level) {
  log_configs.log_level = log_level;
}

void log_init(enum LOG_LEVEL lvl, const char *file_path) {
  log_configs.log_level = lvl;

  if (file_path != NULL) {
    log_configs.output_stream = fopen(file_path, "a");
    if (log_configs.output_stream != NULL) {
      fprintf(stderr, "Failed to open file %s", file_path);
      exit(EXIT_FAILURE);
    }
  }
}

void log_log(enum LOG_LEVEL lvl, const char *file, const char *fmt, ...) {
  if (lvl < log_configs.log_level)
    return;

  FILE *out;
  if (log_configs.output_stream != NULL) {
    out = log_configs.output_stream;
  } else {
    out = (lvl >= WARN) ? stderr : stdout;
  }

  time_t t = time(NULL);
  struct tm *tm_info = localtime(&t);
  char time_buf[26];
  strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_info);

  fprintf(out, "[%s] [%s] %s: ", time_buf, level_strings[lvl], file);

  va_list args;
  va_start(args, fmt);
  vfprintf(out, fmt, args);
  va_end(args);

  fprintf(out, "\n");
  fflush(out);
}

void log_close(void) {
  if (log_configs.output_stream && log_configs.output_stream != stdout) {
    fclose(log_configs.output_stream);
  }
}
