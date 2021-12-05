#include "game.h"
#include "ui.h"

// SDL window
SDL_Window* win;

camera game_camera;

render_list* game_render_list;
float delta_time;
float last_frame;
float fps;

light* lights[NUM_PORTALS + 1];
int num_lights;

particle_generator* pgs[NUM_PORTALS];

// ALuint sound_car;
enum game_state state;

light sun;

// skybox
skybox sky;

// mutant
entity mutant;
vec3 target_pos;

// player
object* player;

void game_init(SDL_Window* window) {
  win = window;

  // init renderer
  int width; int height;
  SDL_GetWindowSize(window, &width, &height);
  renderer_init(width, height);

  // game camera
  game_camera.front[0] = 0.0f;
  game_camera.front[1] = 0.0f;
  game_camera.front[2] = -1.0f;
  game_camera.up[0] = 0.0f;
  game_camera.up[1] = 1.0f;
  game_camera.up[2] = 0.0f;
  game_camera.pos[0] = 4.0f;
  game_camera.pos[1] = 2.0f;
  game_camera.pos[2] = 4.0f;
  game_camera.speed = 10.0f;
  
  // sunlight
  lights[NUM_PORTALS] = &sun;

  // sun
  sun.type = DIRECTIONAL;
  sun.position[0] = 0;
  sun.position[1] = 15;
  sun.position[2] = -20;
  sun.dir[0] = 0;
  sun.dir[1] = 0;
  sun.dir[2] = 0;
  sun.color[0] = 1;
  sun.color[1] = 1;
  sun.color[2] = 1;
  sun.ambient = 0.0f;

  // game loop
  delta_time = 0.0f;
  last_frame = 0.0f;

  // render list
  game_render_list = render_list_new();

  // audio
  // audio_load_sound("assets/audio/test.wav", &microdrag.sound_car);

  // init input
  input_init();

  // init ui
  ui_init(window);

  // skybox
  const char* faces[6];
  faces[0] = "assets/skybox/skybox_rt.bmp";
  faces[1] = "assets/skybox/skybox_lf.bmp";
  faces[2] = "assets/skybox/skybox_up.bmp";
  faces[3] = "assets/skybox/skybox_dn.bmp";
  faces[4] = "assets/skybox/skybox_ft.bmp";
  faces[5] = "assets/skybox/skybox_bk.bmp";
  skybox_init(&sky, faces);

  // wood material
  material mat_wood;
  material_init(&mat_wood);
  strcpy(mat_wood.name, "plane_mat");
  strcpy(mat_wood.texture_path, "assets/textures/Wood_Grain_DIFF.png");
  strcpy(mat_wood.normal_map_path, "assets/textures/Wood_Grain_NRM.png");
  strcpy(mat_wood.specular_map_path, "assets/textures/Wood_Grain_SPEC.png");
  mat_wood.texture_subdivision = 5;

  // ground / floor material
  material mat_floor;
  material_init(&mat_floor);
  strcpy(mat_floor.name, "floor_mat");
  strcpy(mat_floor.texture_path, "assets/textures/floor/PavingStones037_1K_Color.png");
  strcpy(mat_floor.normal_map_path, "assets/textures/floor/PavingStones037_1K_Normal.png");
  strcpy(mat_floor.specular_map_path, "assets/textures/floor/PavingStones037_1K_Roughness.png");
  mat_floor.specular = 0.0f;
  mat_floor.reflectivity = 0;
  mat_floor.texture_subdivision = 300;

  // load character
  mutant.state = MOVE;
  mutant.o = importer_load("mutant");
  mutant.o->scale = 0.015f;
  mutant.o->receive_shadows = 0;
  physics_compute_aabb(mutant.o);
  mutant.run_speed = 4;
  mutant.dir[0] = 0;
  mutant.dir[1] = 0;
  mutant.dir[2] = 1;
  vec3_zero(mutant.o->position);
  vec3_zero(target_pos);
  renderer_init_object(mutant.o);
  animator_play(mutant.o, "walk", 1);

  // player as cube (need it only for collisions)
  player = factory_create_box(2, 2, 2);
  physics_compute_aabb(player);

  // game state
  state = MENU;

  // dungeon
  dungeon_generate();
}

void game_resize(SDL_Window* window) {
  int width; int height;
  SDL_GetWindowSize(window, &width, &height);
  renderer_init(width, height);
}

void game_start() {

}

void game_input(SDL_Event* event) {
  ui_input(event);
  input_event(event);
}

void update_mutant() {
  // update animation
  animator_update(mutant.o, delta_time);

  enum entity_state state = mutant.state;

  // attacking
  /* if (state == ATTACK) {
    int key = animator_current_keyframe(mutant.o);
    if (enemy.state != DIE && key > 18 && key < 22) {
      animator_play(enemy.o, "die", 0);
      enemy.state = DIE;
    } else if (animator_finished(mutant.o)) {
      animator_play(mutant.o, "idle", 1);
      mutant.state = IDLE;
    }
  }

  if (state == IDLE) {
    vec3 dist_to_enemy;
    vec3_sub(dist_to_enemy, mutant.o->position, enemy.o->position);
    if (enemy.state != DIE && vec3_len(dist_to_enemy) < 2 * 1/enemy.o->scale) {
      animator_play(mutant.o, "attack", 0);
      mutant.state = ATTACK;
      return;
    }
  } */

  // move character to target
  vec3 dir, dist, scaled_pos;
  vec3_copy(scaled_pos, game_camera.pos);
  vec3_scale(scaled_pos, scaled_pos, 1 / mutant.o->scale);
  vec3_sub(dist, scaled_pos, mutant.o->position);

  if (vec3_len(dist) > 2) {
    if (strcmp(mutant.o->current_anim->name, "run") != 0)
      animator_play(mutant.o, "run", 1);

    // position
    vec3_norm(dir, dist);
    vec3_scale(dir, dir, mutant.run_speed);
    dir[1] = 0;
    vec3_add(mutant.o->position, mutant.o->position, dir);

    // rotation
    vec3 front = { 0.0f, 0.0f, 1.0f };
    vec3 y_axis = { 0, 1, 0 };
    float angle = vec3_angle_between(front, dir, y_axis);
    quat_rotate(mutant.o->rotation, angle, y_axis);
  } else {
    animator_play(mutant.o, "idle", 1);
    mutant.state = IDLE;
  }

}

void update_player() {

  // collide with walls
  if (game_camera.pos[0] <= -10) {
    game_camera.pos[0] = -10;
  }
}

void game_update() {
  unsigned int current_frame = SDL_GetTicks();
  delta_time = (current_frame - last_frame) * 0.001f;
  last_frame = current_frame;
  fps = 1 / delta_time;

  // input
  input_update(delta_time);

  // mutant
  update_mutant();

  // player
  update_player();

  // audio
  audio_move_listener(game_camera.pos);

  // dungeon
  dungeon_update(delta_time, &game_camera);
}

void game_render() {
  // render entities
  render_list_clear(game_render_list);

  // render character
  // render_list_add(game_render_list, mutant.o);

  // render room
  dungeon_render(game_render_list, lights, pgs);

  // render
  int width; int height;
  SDL_GetWindowSize(win, &width, &height);

  renderer_render_objects(width, height, game_render_list->objects, game_render_list->size, NULL, 0, lights, NUM_PORTALS + 1, &game_camera, ui_render, &sky, pgs, NUM_PORTALS);
}

void game_free() {
  render_list_free(game_render_list);

  // free dungeon
  dungeon_free();

  ui_free();

  skybox_free(&sky);

  // cleanup engine modules
  audio_free();
  renderer_free();
}
