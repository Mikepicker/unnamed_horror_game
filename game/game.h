#ifndef game_h
#define game_h

#include "../engine/seaengine.h"
#include "dungeon.h"

#define GAME_WIDTH 1080
#define GAME_HEIGHT 720

#define MAX_CUBES 128
#define MAX_TREES 10
#define MAX_ROCKS 10
#define MAX_LIGHTS 128
#define FOV 100

enum game_state { MENU, GAME };

// entity
enum entity_state { IDLE, MOVE, ATTACK, DIE };
typedef struct {
  object* o;
  enum entity_state state;
  vec3 dir;
  float run_speed;
  int current_room;
} entity;

// player
typedef struct {
  int health;
  portal* close_portal;
} player_entity;

extern float delta_time;
extern float fps;
extern camera game_camera;
extern vec3 target_pos;
extern light* lights[];
extern player_entity player;
extern entity monster;
extern int key_rot_x_debug;

void game_init();
void game_resize(SDL_Window* window);
void game_start();
void game_input(SDL_Event* event);
void game_update();
void game_render();
void game_free();

#endif
