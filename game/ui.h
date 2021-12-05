#ifndef ui_h
#define ui_h

#include "../engine/seaengine.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "game.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

void ui_init(SDL_Window* window);
void ui_input_begin();
void ui_input(SDL_Event* event);
void ui_input_end();
void ui_render();
void ui_free();

#endif
