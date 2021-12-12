#ifndef input_h
#define input_h

#include "../engine/seaengine.h"

typedef struct {
  int first_mouse;
  float yaw;
  float pitch;
  float sensitivity;
  int capture_cursor;
  int joystick_1_present;
  int joystick_2_present;
} input;

void input_init();
void input_update(float dt);
void input_event(SDL_Event* event);

#endif
