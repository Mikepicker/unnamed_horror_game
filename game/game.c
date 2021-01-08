#include "game.h"

camera game_camera;

render_list* game_render_list;
float delta_time;
float last_frame;
float fps;

// sun

light* lights[MAX_LIGHTS];
int num_lights;
// ALuint sound_car;
enum game_state state;

// point light
light point_light;
light point_light_2;
light point_light_3;

// skybox
skybox sky;

// mutant
entity mutant;
vec3 target_pos;

// player
object* player;

void game_init() {
  // game camera
  game_camera.front[0] = 0.0f;
  game_camera.front[1] = 0.0f;
  game_camera.front[2] = -1.0f;
  game_camera.up[0] = 0.0f;
  game_camera.up[1] = 1.0f;
  game_camera.up[2] = 0.0f;
  game_camera.pos[0] = 0.0f;
  game_camera.pos[1] = 2.0f;
  game_camera.pos[2] = 9.0f;
  game_camera.speed = 10.0f;
  
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
  ui_init();

  // point light
  point_light.type = POINT;
  point_light.position[0] = 0;
  point_light.position[1] = 1;
  point_light.position[2] = -2;
  point_light.ambient = 0.5;
  point_light.constant = 1.0;
  point_light.linear = 0.09;
  point_light.quadratic = 0.032;
  point_light.color[0] = 1.0;
  point_light.color[1] = 1.0;
  point_light.color[2] = 1.0;

  point_light_2.type = POINT;
  point_light_2.position[0] = 0;
  point_light_2.position[1] = 1;
  point_light_2.position[2] = 2;
  point_light_2.ambient = 0.5;
  point_light_2.constant = 1.0;
  point_light_2.linear = 0.09;
  point_light_2.quadratic = 0.032;
  point_light_2.color[0] = 0.0;
  point_light_2.color[1] = 1.0;
  point_light_2.color[2] = 1.0;

  point_light_3.type = POINT;
  point_light_3.position[0] = 0;
  point_light_3.position[1] = 1;
  point_light_3.position[2] = 2;
  point_light_3.ambient = 0.5;
  point_light_3.constant = 1.0;
  point_light_3.linear = 0.09;
  point_light_3.quadratic = 0.032;
  point_light_3.color[0] = 0.0;
  point_light_3.color[1] = 1.0;
  point_light_3.color[2] = 0.0;

  // lights
  lights[0] = &point_light;
  lights[1] = &point_light_2;
  lights[2] = &point_light_3;
  num_lights = 3;

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
  mutant.o->scale = 0.01f;
  mutant.o->receive_shadows = 0;
  physics_compute_aabb(mutant.o);
  mutant.run_speed = 1;
  mutant.dir[0] = 0;
  mutant.dir[1] = 0;
  mutant.dir[2] = 1;
  vec3_zero(mutant.o->position);
  vec3_zero(target_pos);
  target_pos[0] = 30;
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

void game_start() {

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
  vec3 dir, dist;
  vec3_sub(dist, target_pos, mutant.o->position);
  printf("LEN %f\n", vec3_len(dist));
  debug_print_vec3(game_camera.pos);
  debug_print_vec3(mutant.o->position);
  if (vec3_len(dist) > 2) {
    if (strcmp(mutant.o->current_anim->name, "run") != 0)
      animator_play(mutant.o, "run", 1);

    // position
    vec3_norm(dir, dist);
    vec3_scale(dir, dir, mutant.run_speed);
    vec3_add(mutant.o->position, mutant.o->position, dir);

    // rotation
    vec3 front = { 0.0f, 0.0f, 1.0f };
    vec3 y_axis = { 0, 1, 0 };
    float angle = vec3_angle_between(front, dir, y_axis);
    quat_rotate(mutant.o->rotation, angle, y_axis);
  }/* else {
    animator_play(mutant.o, "idle", 1);
    mutant.state = IDLE;
  } */

}

void update_player() {

  // collide with walls
  if (game_camera.pos[0] <= -10) {
    game_camera.pos[0] = -10;
  }
}

void game_update() {
  float current_frame = glfwGetTime();
  delta_time = current_frame - last_frame;
  last_frame = current_frame;
  fps = 1 / delta_time;

  input_update();

  // lights
  vec3_copy(point_light.position, player->position);
  lights[1]->position[0] = 8 * cosf(current_frame);
  lights[2]->position[0] = 8 * cosf(current_frame * 4);
  // sun.position[0] = 10 + sinf(current_frame) * 5;
  /* sun.position[0] = game_camera.pos[0];
  sun.position[1] = game_camera.pos[1] + 15;
  sun.position[2] = game_camera.pos[2] - 20; */

  // mutant
  update_mutant();

  // player
  update_player();

  // audio
  audio_move_listener(game_camera.pos);
}

void game_render() {
  // render entities
  render_list_clear(game_render_list);

  // render_list_add(microdrag.game_render_list, sphere);

  // sample rock
  // render_list_add(game_render_list, rock);

  // render character
  render_list_add(game_render_list, mutant.o);

  // render room
  // dungeon_render(game_render_list);

  // render
  renderer_render_objects(game_render_list->objects, game_render_list->size, NULL, 0, lights, num_lights, &game_camera, ui_render, &sky);
}

void game_free() {
  render_list_free(game_render_list);

  // free dungeon
  dungeon_free();

  ui_free();

  skybox_free(&sky);

  // cleanup engine modules
  audio_free();
  renderer_cleanup();
}
