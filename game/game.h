#ifndef game_h
#define game_h

#include "../engine/seaengine.h"
#include "ui.h"
#include "input.h"

#define GAME_WIDTH 1024
#define GAME_HEIGHT 768

#define MAX_CUBES 128
#define MAX_TREES 10
#define MAX_ROCKS 10
#define MAX_LIGHTS 128
#define FOV 100

enum game_state { MENU, GAME };

// cubes
typedef struct {
  object* o;
  int alive;
} cube;

// entity
enum entity_state { IDLE, MOVE, ATTACK, DIE };
typedef struct {
  object* o;
  enum entity_state state;
  vec3 dir;
  float run_speed;
} entity;

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

// ground
object* ground;

// cubes
cube sample_cube;
cube cubes[MAX_CUBES];
object* select_cube;

// where to place next cube
vec3 place_target;

// character
entity character;
vec3 target_pos;

// sample enemy
entity enemy;

// materials
static material mat_stone;

// nature
object trees[MAX_TREES];
object* tree_1;
object rocks[MAX_ROCKS];
object* rock;

void game_init(GLFWwindow* window);
void game_start();
void game_update();
void game_render();
void game_free();

#endif
