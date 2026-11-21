#pragma once

#define DEFAULT_LOG_LEVEL WARN

enum LOG_LEVEL { DEBUG, INFO, WARN, ERROR };

enum LOG_LEVEL get_log_level_from_string(char *log_level);

void log_init(enum LOG_LEVEL lvl, const char *file_path);
void log_log(enum LOG_LEVEL lvl, const char *file, const char *fmt, ...);
void log_close(void);

#define LOG_DEBUG(...) log_log(DEBUG, __FILE__, __VA_ARGS__)
#define LOG_INFO(...) log_log(INFO, __FILE__, __VA_ARGS__)
#define LOG_WARN(...) log_log(WARN, __FILE__, __VA_ARGS__)
#define LOG_ERROR(...) log_log(ERROR, __FILE__, __VA_ARGS__)
