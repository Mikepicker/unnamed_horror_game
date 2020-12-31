#include "game.h"

#define DUNGEON_BLOCK_SIZE 4

camera game_camera;

render_list* game_render_list;
render_list* screen_render_list;
float delta_time;
float last_frame;
float fps;

// sun
//object* sun_sphere;
light sun;

light* lights;
int num_lights;
// ALuint sound_car;
enum game_state state;

// light sphere
object* sun_sphere;

// point light
light point_light;

// skybox
skybox sky;

// ground
object* ground;

// where to place next cube
vec3 place_target;

// mutant
entity mutant;
vec3 target_pos;

// sample enemy
entity enemy;

// sample item
object* garand;

// player
object* player;
object player_weapon;

// dungeon
object* block;
object blocks[DUNGEON_SIZE * DUNGEON_SIZE];

// materials
material mat_stone;

// nature
object trees[MAX_TREES];
object* tree_1;
object rocks[MAX_ROCKS];
object* rock;

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
  screen_render_list = render_list_new();

  // audio
  // audio_load_sound("assets/audio/test.wav", &microdrag.sound_car);

  // init input
  input_init();

  // init ui
  ui_init();

  // sun
  sun.type = DIRECTIONAL;
  sun.position[0] = 0;
  sun.position[1] = 60;
  sun.position[2] = -10;
  sun.dir[0] = -0.2f;
  sun.dir[1] = -1.0f;
  sun.dir[2] = -0.3f;

  float light_s = 1;
  sun.color[0] = light_s * 1.0f;
  sun.color[1] = light_s * 0.86f;
  sun.color[2] = light_s * 0.53f;
  sun.ambient = 1.0f;
  sun.ambient = 0.0f;
  sun.color[0] = 0.0;
  sun.color[1] = 0.0;
  sun.color[2] = 0.0;

  sun_sphere = factory_create_sphere(5, 10, 10);
  vec3_copy(sun_sphere->position, sun.position);
  renderer_init_object(sun_sphere);

  // point light
  point_light.type = POINT;
  point_light.position[0] = 0;
  point_light.position[1] = 0;
  point_light.position[2] = 0;
  point_light.ambient = 0.5;
  point_light.constant = 1.0;
  point_light.linear = 0.09;
  point_light.quadratic = 0.032;
  point_light.color[0] = 1.0;
  point_light.color[1] = 1.0;
  point_light.color[2] = 1.0;

  // lights
  lights = malloc(MAX_LIGHTS * sizeof(light));
  lights[0] = point_light;
  num_lights = 1;

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

  // grass material
  material mat_grass;
  material_init(&mat_grass);
  strcpy(mat_grass.name, "grass_mat");
  strcpy(mat_grass.texture_path, "assets/textures/Grass_001_COLOR.jpg");
  strcpy(mat_grass.normal_map_path, "assets/textures/Grass_001_NORM.jpg");
  mat_grass.specular = 0.0f;
  mat_grass.reflectivity = 0;
  mat_grass.texture_subdivision = 300;

  // floor material
  material mat_floor;
  material_init(&mat_floor);
  strcpy(mat_floor.name, "floor_mat");
  strcpy(mat_floor.texture_path, "assets/textures/floor/PavingStones037_1K_Color.png");
  strcpy(mat_floor.normal_map_path, "assets/textures/floor/PavingStones037_1K_Normal.png");
  strcpy(mat_floor.specular_map_path, "assets/textures/floor/PavingStones037_1K_Roughness.png");
  mat_floor.specular = 0.0f;
  mat_floor.reflectivity = 0;
  mat_floor.texture_subdivision = 300;

  // ground
  ground = factory_create_plane(1000, 1000);
  ground->position[1] = -0.001;
  ground->meshes[0].mat = mat_floor;
  ground->receive_shadows = 1;
  object_set_center(ground);
  mesh_compute_tangent(&ground->meshes[0]);
  renderer_init_object(ground);

  // wall mat
  material_init(&mat_stone);
  strcpy(mat_stone.name, "mat_stone");
  strcpy(mat_stone.texture_path, "assets/textures/Stone_Wall_013_Albedo.jpg");
  strcpy(mat_stone.normal_map_path, "assets/textures/Stone_Wall_013_Normal.jpg");
  strcpy(mat_stone.specular_map_path, "assets/textures/Stone_Wall_013_Roughness.jpg");
  mat_stone.texture_subdivision = 1;

  // load character
  mutant.state = IDLE;
  mutant.o = importer_load("mutant");
  mutant.o->scale = 0.01f;
  mutant.o->receive_shadows = 0;
  physics_compute_aabb(mutant.o);
  mutant.run_speed = 4;
  mutant.dir[0] = 0;
  mutant.dir[1] = 0;
  mutant.dir[2] = 1;
  vec3_zero(mutant.o->position);
  vec3_zero(target_pos);
  renderer_init_object(mutant.o);
  animator_play(mutant.o, "idle", 1);

  // load example item
  garand = importer_load("garand");

  // garand->scale = 1;
  garand->position[0] = -5.4;
  garand->position[1] = 7.6;
  garand->position[2] = 1;

  // rotate rifle
  vec3 y_axis = { 0, 1, 0 };
  vec3 z_axis = { 0, 0, 1 };
  quat qy;
  quat_rotate(qy, to_radians(-90), y_axis);
  quat qz;
  quat_rotate(qz, to_radians(-80), z_axis);
  quat_mul(garand->rotation, qz, qy);

  renderer_init_object(garand);

  // player as cube (need it only for collisions)
  player = factory_create_box(2, 2, 2);
  physics_compute_aabb(player);

  // load player weapon
  memcpy(&player_weapon, garand, sizeof(object));
  player_weapon.scale = 1;
  player_weapon.parent = NULL;
  
  // load enemy
  enemy.state = IDLE;
  enemy.o = importer_load("character");
  enemy.o->scale = 0.01f;
  enemy.o->receive_shadows = 0;
  enemy.run_speed = 4;
  enemy.dir[0] = 0;
  enemy.dir[1] = 0;
  enemy.dir[2] = 1;
  enemy.o->position[0] = 1000;
  enemy.o->position[1] = 0;
  enemy.o->position[2] = 1000;
  renderer_init_object(enemy.o);
  animator_play(enemy.o, "idle", 1);

  // load tree
  tree_1 = importer_load("tree");
  tree_1->receive_shadows = 0;
  renderer_init_object(tree_1);

  // spawn trees randomly
  float tree_scale = 16;
  int max = 30 * 1/tree_scale;
  int min = -30 * 1/tree_scale;
  for (int i = 0; i < MAX_TREES; i++) {
    memcpy(&trees[i], tree_1, sizeof(object));
    trees[i].scale = tree_scale;
    trees[i].position[0] = random_range(min, max);
    trees[i].position[1] = 0;
    trees[i].position[2] = random_range(min, max);
    float rot = random_range(0, 180);
    vec3 y_axis = { 0, 1, 0 };
    quat_rotate(trees[i].rotation, to_radians(rot), y_axis);
  }

  // load rock
  rock = importer_load("rock");
  rock->receive_shadows = 0;
  renderer_init_object(rock);

  max = 30;
  min = -30;
  for (int i = 0; i < MAX_ROCKS; i++) {
    memcpy(&rocks[i], rock, sizeof(object));
    rocks[i].scale = random_range(0.5, 1);
    rocks[i].position[0] = random_range(min, max);
    rocks[i].position[1] = random_range(-1, 0.5);
    rocks[i].position[2] = random_range(min, max);
    float rot = random_range(0, 180);
    vec3 y_axis = { 0, 1, 0 };
    quat_rotate(rocks[i].rotation, to_radians(rot), y_axis);
    rot = random_range(0, 180);
    vec3 z_axis = { 0, 0, 1 };
    quat_rotate(rocks[i].rotation, to_radians(rot), z_axis);
  }

  // game state
  state = MENU;

  // dungeon
  dungeon_generate();

  // doungeon block
  block = factory_create_box(DUNGEON_BLOCK_SIZE, DUNGEON_BLOCK_SIZE, DUNGEON_BLOCK_SIZE);
  block->receive_shadows = 0;
  block->meshes[0].mat = mat_stone;
  block->position[1] = (int)(DUNGEON_BLOCK_SIZE / 2);
  renderer_init_object(block);

  // popolate dungeon
  int i = 0;
  for (int y = 0; y < DUNGEON_SIZE; y++) {
    for (int x = 0; x < DUNGEON_SIZE; x++) {
      if (dungeon[y][x] == NEXT_TO_ROOM) {
        memcpy(&blocks[i], block, sizeof(object));
        blocks[i].position[0] = DUNGEON_BLOCK_SIZE * x;
        blocks[i].position[2] = DUNGEON_BLOCK_SIZE * y;
        i++;
      }
    }
  }

}

void game_start() {

}

void update_character() {
  // update animation
  animator_update(mutant.o, delta_time);

  enum entity_state state = mutant.state;

  // attacking
  if (state == ATTACK) {
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
  }

  // move character to target
  if (state == MOVE) {
    vec3 dir, dist;
    vec3_sub(dist, target_pos, mutant.o->position);
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
    } else {
      animator_play(mutant.o, "idle", 1);
      mutant.state = IDLE;
    }
  }

}

void update_enemy() {
  animator_update(enemy.o, delta_time);
  
  if (enemy.state != DIE) {
    vec3 dir;
    vec3_sub(dir, mutant.o->position, enemy.o->position);
    vec3_norm(dir, dir);

    vec3 front = { 0.0f, 0.0f, 1.0f };
    vec3 y_axis = { 0, 1, 0 };
    float angle = vec3_angle_between(front, dir, y_axis);
    quat_rotate(enemy.o->rotation, angle, y_axis);
  }
}

void update_player() {
  player_weapon.position[0] = 1;
  player_weapon.position[1] = -1;
  player_weapon.position[2] = -2;

  vec3 y_axis = { 0, 1, 0 };
  quat_rotate(player_weapon.rotation, to_radians(90), y_axis);

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
  // sun.position[0] = 10 + sinf(current_frame) * 5;
  /* sun.position[0] = game_camera.pos[0];
  sun.position[1] = game_camera.pos[1] + 15;
  sun.position[2] = game_camera.pos[2] - 20; */

  // character
  update_character();

  // enemy
  update_enemy();
  
  // player
  update_player();

  // audio
  audio_move_listener(game_camera.pos);
}

void game_render() {
  // point light
  vec3_copy(lights[0].position, game_camera.pos);

  // render entities
  render_list_clear(game_render_list);
  render_list_clear(screen_render_list);

  // render sun sphere
  render_list_add(game_render_list, sun_sphere);

  // render_list_add(microdrag.game_render_list, sphere);

  render_list_add(game_render_list, ground);

  // render character
  render_list_add(game_render_list, mutant.o);

  // render enemy
  render_list_add(game_render_list, enemy.o);

  // render player weapon
  //render_list_add(game_render_list, &player_weapon);

  // render nature
  for (int i = 0; i < MAX_TREES; i++) {
    render_list_add(game_render_list, &trees[i]);
  }

  for (int i = 0; i < MAX_ROCKS; i++) {
    render_list_add(game_render_list, &rocks[i]);
  }
  
  // render room
  for (int i = 0; i < DUNGEON_SIZE * DUNGEON_SIZE; i++) {
    render_list_add(game_render_list, &blocks[i]);
  }

  // screen objects
  render_list_add(screen_render_list, &player_weapon);

  // render
  renderer_render_objects(game_render_list->objects, game_render_list->size, screen_render_list->objects, screen_render_list->size, &sun, &lights, 1, &game_camera, ui_render, &sky);
}

void game_free() {
  render_list_free(game_render_list);
  render_list_free(screen_render_list);
  /* renderer_free_object(microdrag.cars[0].obj);
  audio_free_object(microdrag.cars[0].obj); */
  object_free(ground);

  // free dungeon
  object_free(block);

  object_free(sun_sphere);

  free(lights);

  ui_free();
  skybox_free(&sky);

  object_free(garand);

  // cleanup engine modules
  audio_free();
  renderer_cleanup();
}
