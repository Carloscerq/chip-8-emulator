#include "emulator.h"
#include "log.h"
#include <raylib.h>
#include <stdint.h>

#define SCREEN_NAME "chip-8 emulator"
#define SCREEN_SCALE_FACTOR 10
#define TARGET_FPS 60

void configure_gui();
void close_gui();
void clean_screen();
void draw_screen(const uint8_t *display);
void update_input(void);
