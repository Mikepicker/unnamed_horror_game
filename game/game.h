#ifndef game_h
#define game_h

#include "../engine/seaengine.h"
#include "ui.h"
#include "input.h"

#define GAME_WIDTH 800
#define GAME_HEIGHT 600

#define MAX_CUBES 128
#define MAX_LIGHTS 128
#define FOV 100

enum game_state { MENU, GAME };

// cubes
typedef struct {
  object* o;
  int alive;
} cube;

GLFWwindow* window;
camera game_camera;

render_list* game_render_list;
float delta_time;
float last_frame;
light* lights;
int num_lights;
// ALuint sound_car;
enum game_state state;

// skybox
skybox sky;

// plane
object* plane;

// cubes
cube sample_cube;
cube cubes[MAX_CUBES];
object* select_cube;

// where to place next cube
vec3 place_target;

// materials
static material mat_stone;

void game_init(GLFWwindow* window);
void game_start();
void game_update();
void game_render();
void game_free();

#endif
