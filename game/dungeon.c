#include "dungeon.h"
#include "bmp.h"

#define DUNGEON_BLOCK_SIZE 4
#define MAX_ROOMS 1
#define MIN_ROOM_SIZE 4
#define MAX_ROOM_SIZE 16

room dungeon[MAX_ROOMS];

object* block;
material mat_stone;

object blocks[(MAX_ROOM_SIZE + MAX_ROOM_SIZE) * 2];

object* ground;
object* roof;

// TODO: for now, render first room
void dungeon_render(render_list* rl) {
   
  // render ground & roof
  render_list_add(rl, ground);
  render_list_add(rl, roof);

  // render walls
  int block_count = 0;

  // north walls
  for (int i = 0; i < dungeon[0].w; i++) {
    object* b = &blocks[block_count++];
    b->position[0] = i * DUNGEON_BLOCK_SIZE;
    b->position[2] = 0;
    render_list_add(rl, b); 
  }

  // west walls
  for (int i = 0; i < dungeon[0].h; i++) {
    object* b = &blocks[block_count++];
    b->position[0] = (dungeon[0].w - 1) * DUNGEON_BLOCK_SIZE;
    b->position[2] = i * DUNGEON_BLOCK_SIZE;
    render_list_add(rl, b); 
  }

  // south walls
  for (int i = 0; i < dungeon[0].w; i++) {
    object* b = &blocks[block_count++];
    b->position[0] = i * DUNGEON_BLOCK_SIZE;
    b->position[2] = (dungeon[0].h - 1) * DUNGEON_BLOCK_SIZE;
    render_list_add(rl, b); 
  }

  // east walls
  for (int i = 0; i < dungeon[0].h; i++) {
    object* b = &blocks[block_count++];
    b->position[0] = 0;
    b->position[2] = i * DUNGEON_BLOCK_SIZE;
    render_list_add(rl, b); 
  }
}

void dungeon_generate() {
  // init sample block
  material_init(&mat_stone);
  strcpy(mat_stone.name, "mat_stone");
  strcpy(mat_stone.texture_path, "assets/textures/Stone_Wall_013_Albedo.jpg");
  strcpy(mat_stone.normal_map_path, "assets/textures/Stone_Wall_013_Normal.jpg");
  strcpy(mat_stone.specular_map_path, "assets/textures/Stone_Wall_013_Roughness.jpg");
  mat_stone.texture_subdivision = 1;

  block = factory_create_box(DUNGEON_BLOCK_SIZE, DUNGEON_BLOCK_SIZE, DUNGEON_BLOCK_SIZE);
  block->receive_shadows = 1;
  block->meshes[0].mat = mat_stone;
  block->position[1] = (float)DUNGEON_BLOCK_SIZE / 2;
  renderer_init_object(block);

  // pre-allocate blocks
  for (int i = 0; i < (MAX_ROOM_SIZE + MAX_ROOM_SIZE) * 2; i++) {
    memcpy(&blocks[i], block, sizeof(object));
  }

  // generate rooms
  for (int i = 0; i < MAX_ROOMS; i++) {
    dungeon[i].w = random_range(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
    dungeon[i].h = random_range(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
  }

  // ground & floor
  material mat_floor;
  material_init(&mat_floor);
  strcpy(mat_floor.name, "floor_mat");
  strcpy(mat_floor.texture_path, "assets/textures/floor/PavingStones037_1K_Color.png");
  strcpy(mat_floor.normal_map_path, "assets/textures/floor/PavingStones037_1K_Normal.png");
  strcpy(mat_floor.specular_map_path, "assets/textures/floor/PavingStones037_1K_Roughness.png");
  mat_floor.specular = 0.0f;
  mat_floor.reflectivity = 0;
  mat_floor.texture_subdivision = 300;

  ground = factory_create_plane(1000, 1000);
  ground->position[1] = -0.001;
  ground->meshes[0].mat = mat_floor;
  ground->receive_shadows = 1;
  object_set_center(ground);
  mesh_compute_tangent(&ground->meshes[0]);
  renderer_init_object(ground);

  roof = factory_create_plane(1000, 1000);
  roof->position[1] = DUNGEON_BLOCK_SIZE;
  roof->meshes[0].mat = mat_floor;
  roof->receive_shadows = 1;
  object_set_center(roof);
  mesh_compute_tangent(&roof->meshes[0]);
  renderer_init_object(roof);

  // rotate roof
  vec3 axis_x = { 1, 0, 0 };
  quat_rotate(roof->rotation, to_radians(180), axis_x);
}

void dungeon_free() {
  renderer_free_object(ground);
  object_free(ground);

  renderer_free_object(roof);
  object_free(roof);

  renderer_free_object(block);
  for (int i = 0; i < MAX_ROOMS; i++) {
    object_free(&blocks[i]);
  }
}
