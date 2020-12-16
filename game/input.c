#include "input.h"

// TODO: improve performance
int get_dead_cube_index() {
  for (int i = 0; i < MAX_CUBES; i++) {
    if (!cubes[i].alive) return i;
  } 

  return -1;
}

void place_cube() {
  int i = get_dead_cube_index();

  if (i >= 0) {
    cubes[i].alive = 1;
    vec3_copy(cubes[i].o->position, place_target);
  } else {
    printf("Too many cubes!");
  }
}

void input_init() {
  game_input.yaw = -90.0f;
  game_input.pitch = 0.0f;
  game_input.mouse_last_x = GAME_WIDTH / 2.0f;
  game_input.mouse_last_y = GAME_HEIGHT / 2.0f;
  game_input.fov = 45.0f;
  game_input.capture_cursor = 1;
  game_input.first_mouse = 1;
  game_input.sensitivity = 0.01f;

  glfwSetKeyCallback(window, input_key_callback);
  glfwSetCursorPosCallback(window, input_mouse_callback);
  glfwSetMouseButtonCallback(window, input_mouse_button_callback);
  glfwSetJoystickCallback(input_joystick_callback);

  // joysticks
  game_input.joystick_1_present = glfwJoystickPresent(GLFW_JOYSTICK_1);
  printf("[input_init] joystick 1 present: %d\n", game_input.joystick_1_present);

  game_input.joystick_2_present = glfwJoystickPresent(GLFW_JOYSTICK_2);
  printf("[input_init] joystick 2 present: %d\n", game_input.joystick_2_present);
}

void input_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);

  if (key == GLFW_KEY_H && action == GLFW_PRESS)
    place_cube();

  // debug shadows
  if (key == GLFW_KEY_J && action == GLFW_PRESS)
    renderer_shadows_debug_enabled = renderer_shadows_debug_enabled == 0 ? 1 : 0;

  // debug ssao
  if (key == GLFW_KEY_K && action == GLFW_PRESS)
    renderer_ssao_debug_on = renderer_ssao_debug_on == 0 ? 1 : 0;

  // debug fxaa
  if (key == GLFW_KEY_L && action == GLFW_PRESS)
    renderer_fxaa_enabled = renderer_fxaa_enabled == 0 ? 1 : 0;
  
  
  // capture mouse
  if (action == GLFW_RELEASE) {
    if (key == GLFW_KEY_C) {
      game_input.capture_cursor = !(glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL);
      glfwSetInputMode(window, GLFW_CURSOR, game_input.capture_cursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
  }

  // recompile shader
  if (action == GLFW_RELEASE && key == GLFW_KEY_O) {
    renderer_recompile_shader(); 
  }

  if (key == GLFW_KEY_B && action == GLFW_PRESS) {
    garand->position[0] += 0.1f;
    printf("%f\n", garand->position[0]);
  }

  if (key == GLFW_KEY_V && action == GLFW_PRESS) {
    garand->position[0] -= 0.1f;
    printf("%f\n", garand->position[0]);
  }
}

void input_mouse_callback(GLFWwindow* window, double x_pos, double y_pos)
{
  if (game_input.capture_cursor != 0) return;

  if (game_input.first_mouse == 1) {
    game_input.mouse_last_x = x_pos;
    game_input.mouse_last_y = y_pos;
    game_input.first_mouse = 0;
  }

  float x_offset = x_pos - game_input.mouse_last_x;
  float y_offset = game_input.mouse_last_y - y_pos; // reversed since y-coordinates go from bottom to top
  game_input.mouse_last_x = x_pos;
  game_input.mouse_last_y = y_pos;

  x_offset *= game_input.sensitivity;
  y_offset *= game_input.sensitivity;

  game_input.yaw += x_offset;
  game_input.pitch += y_offset;

  // make sure that when pitch is out of bounds, screen doesn't get flipped
  if (game_input.pitch > 89.0f)
    game_input.pitch = 89.0f;
  if (game_input.pitch < -89.0f)
    game_input.pitch = -89.0f;

  game_camera.front[0] = cosf(game_input.yaw) * cosf(game_input.pitch);
  game_camera.front[1] = sinf(game_input.pitch);
  game_camera.front[2] = sinf(game_input.yaw) * cosf(game_input.pitch);
  //debug_print_vec3(microdrag.game_camera.front);
}

void input_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    ray r = renderer_raycast(&game_camera, x, y, 10000);

    plane p;
    vec3_zero(p.p);
    p.n[0] = 0; p.n[1] = 1; p.n[2] = 0;

    vec3 intersection;
    if (physics_ray_hit_plane(r, p, intersection)) {
      vec3_scale(target_pos, intersection, 1/character.o->scale);
      character.state = MOVE;
    }
  }
}

void input_joystick_callback(int joy, int event) {
  if (event == GLFW_CONNECTED) {
    if (joy == GLFW_JOYSTICK_1) {
      game_input.joystick_1_present = 1;
      printf("[input_joystick_callback] joystick 1 connected\n");
    } else if (joy == GLFW_JOYSTICK_2) {
      game_input.joystick_2_present = 1;
      printf("[input_joystick_callback] joystick 2 connected\n");
    }
  }
  else if (event == GLFW_DISCONNECTED) {
    if (joy == GLFW_JOYSTICK_1) {
      game_input.joystick_1_present = 0;
      printf("[input_joystick_callback] joystick 1 disconnected\n");
    } else if (joy == GLFW_JOYSTICK_2) {
      game_input.joystick_2_present = 0;
      printf("[input_joystick_callback] joystick 2 disconnected\n");
    }
  }
}

void input_update() {
  // camera
  float camera_delta = game_camera.speed * delta_time;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    vec3 vec3_scaled;
    vec3_scale(vec3_scaled, game_camera.front, camera_delta);
    vec3_add(game_camera.pos, game_camera.pos, vec3_scaled);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    vec3 vec3_temp;
    vec3_scale(vec3_temp, game_camera.front, camera_delta);
    vec3_sub(game_camera.pos, game_camera.pos, vec3_temp);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    vec3 vec3_crossed, vec3_scaled, vec3_normalized;
    vec3_mul_cross(vec3_crossed, game_camera.front, game_camera.up);
    vec3_norm(vec3_normalized, vec3_crossed);
    vec3_scale(vec3_scaled, vec3_normalized, camera_delta);
    vec3_sub(game_camera.pos, game_camera.pos, vec3_scaled);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    vec3 vec3_crossed, vec3_scaled, vec3_normalized;
    vec3_mul_cross(vec3_crossed, game_camera.front, game_camera.up);
    vec3_norm(vec3_normalized, vec3_crossed);
    vec3_scale(vec3_scaled, vec3_normalized, camera_delta);
    vec3_add(game_camera.pos, game_camera.pos, vec3_scaled);
  }
}
