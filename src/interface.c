#include "interface.h"
#include "log.h"
#include <raylib.h>

static struct gui_config {
  AudioStream beepStream;
} gui_config;

void audio_callback(void *bufferData, unsigned int frames) {
  short *buffer = (short *)bufferData;
  static float phase = 0.0f;
  float frequency = 440.0f; // 440 Hz (A4 note)
  float sampleRate = 44100.0f;

  for (unsigned int i = 0; i < frames; i++) {
    phase += frequency / sampleRate;
    if (phase >= 1.0f)
      phase -= 1.0f;

    // Square wave: positive for first half of the cycle, negative for the
    // second 8000 is the volume amplitude (approx 25% of the max short value)
    buffer[i] = (phase < 0.5f) ? 8000 : -8000;
  }
}

void configure_gui() {
  LOG_INFO("Starting the GUI");
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  InitWindow(DISPLAY_WIDTH * SCREEN_SCALE_FACTOR,
             DISPLAY_HEIGHT * SCREEN_SCALE_FACTOR, SCREEN_NAME);

  SetTargetFPS(TARGET_FPS);
  InitAudioDevice();
  // SetExitKey(0);

  // Create an audio stream: 44100 Hz, 16-bit, 1 channel (Mono)
  SetAudioStreamBufferSizeDefault(1024);
  gui_config.beepStream = LoadAudioStream(44100, 16, 1);
  SetAudioStreamCallback(gui_config.beepStream, audio_callback);

  // Start the stream, then immediately pause it so it doesn't beep continuously
  // on launch
  PlayAudioStream(gui_config.beepStream);
  PauseAudioStream(gui_config.beepStream);
}

void close_gui() {
  LOG_DEBUG("Closing the screen");
  CloseWindow();
  CloseAudioDevice();
}

void clean_screen() {
  LOG_DEBUG("Clearing the screen");
  ClearBackground(BLACK);
}

void draw_screen(const uint8_t *display) {
  LOG_DEBUG("Starting to draw");
  BeginDrawing();
  clean_screen();

  for (int y = 0; y < DISPLAY_HEIGHT; y++) {
    for (int x = 0; x < DISPLAY_WIDTH; x++) {
      if (display[y * DISPLAY_WIDTH + x]) {
        DrawRectangle(x * SCREEN_SCALE_FACTOR, y * SCREEN_SCALE_FACTOR,
                      SCREEN_SCALE_FACTOR, SCREEN_SCALE_FACTOR, WHITE);
      }
    }
  }

  LOG_DEBUG("Finish the drawing");
  EndDrawing();
}

void update_input(void) {
  // Map standard QWERTY grid (1234, QWER, ASDF, ZXCV) to CHIP-8 Hex (123C,
  // 456D, 789E, A0BF)
  LOG_DEBUG("GETTING KEY STATE");
  set_key_state(0x1, IsKeyDown(KEY_ONE));
  set_key_state(0x2, IsKeyDown(KEY_TWO));
  set_key_state(0x3, IsKeyDown(KEY_THREE));
  set_key_state(0xC, IsKeyDown(KEY_FOUR));

  set_key_state(0x4, IsKeyDown(KEY_Q));
  set_key_state(0x5, IsKeyDown(KEY_W));
  set_key_state(0x6, IsKeyDown(KEY_E));
  set_key_state(0xD, IsKeyDown(KEY_R));

  set_key_state(0x7, IsKeyDown(KEY_A));
  set_key_state(0x8, IsKeyDown(KEY_S));
  set_key_state(0x9, IsKeyDown(KEY_D));
  set_key_state(0xE, IsKeyDown(KEY_F));

  set_key_state(0xA, IsKeyDown(KEY_Z));
  set_key_state(0x0, IsKeyDown(KEY_X));
  set_key_state(0xB, IsKeyDown(KEY_C));
  set_key_state(0xF, IsKeyDown(KEY_V));
}

void play_audio(void) {
  if (!IsAudioStreamPlaying(gui_config.beepStream)) {
    ResumeAudioStream(gui_config.beepStream);
  }
}

void stop_audio(void) {
  if (IsAudioStreamPlaying(gui_config.beepStream)) {
    PauseAudioStream(gui_config.beepStream);
  }
}
