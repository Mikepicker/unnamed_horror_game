#include "dungeon.h"
#include "bmp.h"

#define DUNGEON_BLOCK_SIZE 4
#define MIN_ROOM_SIZE 8
#define MAX_ROOM_SIZE 16

room dungeon[MAX_ROOMS];

object* block;
material mat_stone;

object blocks[(MAX_ROOM_SIZE + MAX_ROOM_SIZE) * 2];

object* ground;
object* roof;
object* portal_model;

static light portal_lights[NUM_PORTALS];
static object* portal_models[NUM_PORTALS];
static particle_generator* portal_pgs[NUM_PORTALS];

int current_room;

void dungeon_change_room(int next_room) {
  assert(next_room >= 0 && next_room < MAX_ROOMS);
  current_room = next_room;
}

void dungeon_update(float dt, camera* cam) {
  // particles
  for (int i = 0; i < NUM_PORTALS; i++) {
    particle_generator_update(portal_pgs[i], dt, 2, cam);
  }
}

void dungeon_render(render_list* rl, light** lights, particle_generator** pgs) {

  // render ground & roof
  render_list_add(rl, ground);
  render_list_add(rl, roof);

  // render walls
  int block_count = 0;

  // north walls
  for (int i = 0; i < dungeon[current_room].w; i++) {
    object* b = &blocks[block_count++];
    b->position[0] = i * DUNGEON_BLOCK_SIZE;
    b->position[2] = 0;
    render_list_add(rl, b); 
  }

  // west walls
  for (int i = 0; i < dungeon[current_room].h; i++) {
    object* b = &blocks[block_count++];
    b->position[0] = (dungeon[current_room].w - 1) * DUNGEON_BLOCK_SIZE;
    b->position[2] = i * DUNGEON_BLOCK_SIZE;
    render_list_add(rl, b); 
  }

  // south walls
  for (int i = 0; i < dungeon[current_room].w; i++) {
    object* b = &blocks[block_count++];
    b->position[0] = i * DUNGEON_BLOCK_SIZE;
    b->position[2] = (dungeon[current_room].h - 1) * DUNGEON_BLOCK_SIZE;
    render_list_add(rl, b); 
  }

  // east walls
  for (int i = 0; i < dungeon[current_room].h; i++) {
    object* b = &blocks[block_count++];
    b->position[0] = 0;
    b->position[2] = i * DUNGEON_BLOCK_SIZE;
    render_list_add(rl, b); 
  }

  // render portals
  for (int i = 0; i < NUM_PORTALS; i++) {
    portal* p = &dungeon[current_room].portals[i];

    // set light pos
    light* l = &portal_lights[i];
    l->position[0] = p->x;
    l->position[2] = p->y;
    lights[i] = l;

    // set model pos
    vec3 model_pos = { p->x / portal_models[i]->scale, 72, p->y / portal_models[i]->scale };
    vec3_copy(portal_models[i]->position, model_pos);
    render_list_add(rl, portal_models[i]);

    // set particle generator pos
    vec3 pg_pos = { p->x, 0.5f, p->y };
    vec3_copy(portal_pgs[i]->pc.position, pg_pos);
    pgs[i] = portal_pgs[i];
  }
}

static void generate_portals() {
  // place portals
  for (int j = 0; j < MAX_ROOMS; j++) {
    room* r = &dungeon[j];

    for (int i = 0; i < NUM_PORTALS; i++) {
      portal* p = &r->portals[i];
      p->x = random_range(2, r->w - 2) * DUNGEON_BLOCK_SIZE + ((float)DUNGEON_BLOCK_SIZE / 2);
      p->y = random_range(2, r->h - 2) * DUNGEON_BLOCK_SIZE + ((float)DUNGEON_BLOCK_SIZE / 2);
      
      if (j < MAX_ROOMS - 1) {
        room* next_r = &dungeon[j + 1];
        int rand_p = random_range(0, 1);
        p->link = &next_r->portals[rand_p];
        next_r->portals[rand_p].link = p;
      }
    }
  }

  for (int i = 0; i < NUM_PORTALS; i++) {
    // light
    light* l = &portal_lights[i];
    l->type = POINT;
    l->position[1] = 1;
    l->ambient = 0.5;
    l->constant = 1.0;
    l->linear = 0.09;
    l->quadratic = 0.032;
    l->color[0] = 0.0;
    l->color[1] = 0.0;
    l->color[2] = 1.0;
    l->cast_shadows = 1;

    // portal model
    float scale = 0.02f;
    // vec3 pos = { p->x / scale, 72, p->y / scale };
    vec3 pos = { 0, 0, 0 };
    portal_models[i] = object_create(pos, scale, portal_model->meshes, portal_model->num_meshes, 1, NULL);

    quat_identity(portal_models[i]->rotation);
    object_set_center(portal_models[i]);
    renderer_init_object(portal_models[i]);
    portal_models[i]->receive_shadows = 1;

    // random rotation
    vec3 y_axis = { 0, 1, 0 };
    quat_rotate(portal_models[i]->rotation, to_radians(random_range(0, 360)), y_axis);

    // init particle generator
    float spread = 0.015f;
    particle_config pc = {
      // .position = { p->x, 0.5f, p->y },
      .position = { 0, 0.5f, 0 },
      .velocity = { 0, 0.05f, 0 },
      .color = { 0, 0, 1.0f },
      .size = 0.8f,
      .alpha = 1.0f,
      .life = 1.5f,
      .min_spread_x = -spread,
      .max_spread_x = spread,
      .min_spread_y = 0,
      .max_spread_y = 0,
      .min_spread_z = -spread,
      .max_spread_z = spread,
      .sprite_path = "assets/fire/fire.jpg"
    };
    portal_pgs[i] = particle_generator_new(pc, 1000);
    renderer_init_particle_generator(portal_pgs[i]);
  }

}

void dungeon_generate() {
  current_room = 0;

  // portal model
  portal_model = importer_load("portal");

  // init sample block
  material_init(&mat_stone);
  strcpy(mat_stone.name, "mat_stone");
  strcpy(mat_stone.texture_path, "assets/textures/stone/Stone_Wall_013_Albedo.jpg");
  strcpy(mat_stone.normal_map_path, "assets/textures/stone/Stone_Wall_013_Normal.jpg");
  strcpy(mat_stone.specular_map_path, "assets/textures/stone/Stone_Wall_013_Roughness.jpg");
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

  // generate portals
  generate_portals();

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

  material mat_roof;
  material_init(&mat_roof);
  strcpy(mat_roof.name, "floor_mat");
  strcpy(mat_roof.texture_path, "assets/textures/stone/Stone_Wall_013_Albedo.jpg");
  strcpy(mat_roof.normal_map_path, "assets/textures/stone/Stone_Wall_013_Normal.jpg");
  strcpy(mat_roof.specular_map_path, "assets/textures/stone/Stone_Wall_013_Roughness.jpg");
  mat_roof.specular = 0.0f;
  mat_roof.reflectivity = 0;
  mat_roof.texture_subdivision = 300;

  ground = factory_create_plane(1000, 1000);
  ground->position[1] = -0.001;
  ground->meshes[0].mat = mat_floor;
  ground->receive_shadows = 1;
  object_set_center(ground);
  mesh_compute_tangent(&ground->meshes[0]);
  renderer_init_object(ground);

  roof = factory_create_plane(1000, 1000);
  roof->position[1] = DUNGEON_BLOCK_SIZE;
  roof->meshes[0].mat = mat_roof;
  roof->receive_shadows = 1;
  object_set_center(roof);
  mesh_compute_tangent(&roof->meshes[0]);
  renderer_init_object(roof);

  // rotate roof
  vec3 axis_x = { 1, 0, 0 };
  quat_rotate(roof->rotation, to_radians(180), axis_x);
}

void dungeon_free() {
  object_free(portal_model);

  renderer_free_object(ground);
  object_free(ground);

  renderer_free_object(roof);
  object_free(roof);

  renderer_free_object(block);
  object_free(block);

  for (int i = 0; i < NUM_PORTALS; i++) {
    particle_generator_free(portal_pgs[i]);
    renderer_free_particle_generator(portal_pgs[i]);
  }
}
