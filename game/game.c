#include "game.h"

object* character;

void game_init(GLFWwindow* w) {
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
  
  window = w;
  delta_time = 0.0f;
  last_frame = 0.0f;

  game_render_list = render_list_new();

  // audio
  // audio_load_sound("assets/audio/test.wav", &microdrag.sound_car);

  // init input
  input_init();

  // init ui
  ui_init();

  // lights
  lights = malloc(MAX_LIGHTS * sizeof(light));
  light l1;
  l1.position[0] = 0.0f;
  l1.position[1] = 4.0f;
  l1.position[2] = 0.0f;
  l1.color[0] = 1.0f;
  l1.color[1] = 1.0f;
  l1.color[2] = 1.0f;

  lights[0] = l1;
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

  // plane
  plane = factory_create_plane(80, 80);
  plane->position[1] = -0.001;
  plane->meshes[0].mat = mat_wood;
  plane->receive_shadows = 1;
  object_set_center(plane);
  mesh_compute_tangent(&plane->meshes[0]);
  renderer_init_object(plane);

  // wall mat
  material_init(&mat_stone);
  strcpy(mat_stone.name, "mat_stone");
  strcpy(mat_stone.texture_path, "assets/textures/Stone_Wall_013_Albedo.jpg");
  strcpy(mat_stone.normal_map_path, "assets/textures/Stone_Wall_013_Normal.jpg");
  strcpy(mat_stone.specular_map_path, "assets/textures/Stone_Wall_013_Roughness.jpg");
  mat_stone.texture_subdivision = 1;

  // sample cube to pre-allocate others
  sample_cube.o = factory_create_box(1, 1, 1);
  sample_cube.o->receive_shadows = 0;
  sample_cube.o->meshes[0].mat = mat_stone;
  object_set_center(sample_cube.o);
  mesh_compute_tangent(&sample_cube.o->meshes[0]);
  renderer_init_object(sample_cube.o);

  // pre-allocate cubes
  for (int i = 0; i < MAX_CUBES; i++) {
    object* o = malloc(sizeof(*sample_cube.o));
    memcpy(o, sample_cube.o, sizeof(*sample_cube.o));
    cubes[i].o = o;
    cubes[i].alive = 0;
  }

  // selection cube
  select_cube = malloc(sizeof(*sample_cube.o));
  memcpy(select_cube, sample_cube.o, sizeof(*sample_cube.o));

  // load character
  character = importer_load_obj("assets/character/character.obj");
  character->scale = 0.01f;

  vec3 z_axis = { 1, 0, 0 };
  // quat_rotate(character->rotation, to_radians(-90), z_axis);
  // character->position[1] += 50;
  renderer_init_object(character);

  state = MENU;
}

void game_start() {

}

void game_update() {
  float current_frame = glfwGetTime();
  delta_time = current_frame - last_frame;
  last_frame = current_frame;

  input_update();

  // lights[0].position[1] = 10 + sinf(current_frame) * 5;
  lights[0].position[0] = game_camera.pos[0];
  lights[0].position[1] = game_camera.pos[1] + 2.0f;
  lights[0].position[2] = game_camera.pos[2];
  // cube->position[1] = 4 + sinf(current_frame);
  // microdrag.cars[0].obj->position[1] = 1.0f + sinf(2.0f * current_frame);
  // microdrag.lights[0].position[0] =  24 * sinf(0.5f * current_frame);

  vec3 y = { 0, 1, 0 };
  quat_rotate(character->skel->rest_pose->joint_rotations[30], current_frame, y);
  // character->skel->rest_pose->joint_positions[34][0] += 2;

  // audio
  audio_move_listener(game_camera.pos);
}

void game_render() {
  // render entities
  render_list_clear(game_render_list);

  // render_list_add(microdrag.game_render_list, sphere);
  render_list_add(game_render_list, plane);

  // TODO: improve performance for alive lookup
  for (int i = 0; i < MAX_CUBES; i++) {
    if (cubes[i].alive) {
      vec3 dist;
      vec3_sub(dist, cubes[i].o->position, game_camera.pos);
      if (vec3_len(dist) < FOV) {
        render_list_add(game_render_list, cubes[i].o);
      }
    }
  }

  // select cube
  vec3_scale(place_target, game_camera.front, 4);
  vec3_add(place_target, place_target, game_camera.pos);
  place_target[0] = round(place_target[0]);
  place_target[1] = round(place_target[1]);
  place_target[2] = round(place_target[2]);
  vec3_copy(select_cube->position, place_target);
  // render_list_add(game_render_list, select_cube);

  // render character
  render_list_add(game_render_list, character);

  // render
  renderer_render_objects(game_render_list->objects, game_render_list->size, &lights, num_lights, &game_camera, ui_render, &sky);
}

void game_free() {
  render_list_free(game_render_list);
  /* renderer_free_object(microdrag.cars[0].obj);
  audio_free_object(microdrag.cars[0].obj); */
  object_free(sample_cube.o);
  object_free(plane);
  free(lights);

  ui_free();
  skybox_free(&sky);

  // cleanup engine modules
  audio_free();
  renderer_cleanup();
}
