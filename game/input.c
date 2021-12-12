#include "input.h"
#include "game.h"
#include "dungeon.h"

input input_data;
float camera_anim_time;

void input_init() {
  input_data.yaw = -90.0f;
  input_data.pitch = 0.0f;
  input_data.capture_cursor = 1;
  input_data.first_mouse = 1;
  input_data.sensitivity = 0.01f;

  camera_anim_time = 0.0f;
}

static void input_mouse_callback(float x_pos, float y_pos) {
  if (input_data.capture_cursor != 0) return;

  input_data.yaw += x_pos * input_data.sensitivity;
  input_data.pitch -= y_pos * input_data.sensitivity;

  // make sure that when pitch is out of bounds, screen doesn't get flipped
  if (input_data.pitch > 89.0f)
    input_data.pitch = 89.0f;
  if (input_data.pitch < -89.0f)
    input_data.pitch = -89.0f;

  game_camera.front[0] = cosf(input_data.yaw) * cosf(input_data.pitch);
  game_camera.front[1] = sinf(input_data.pitch);
  game_camera.front[2] = sinf(input_data.yaw) * cosf(input_data.pitch);
}

void input_update(float dt) {
  const Uint8* keystate = SDL_GetKeyboardState(NULL);

  // fps camera
  int moving = 0;
  float camera_delta = game_camera.speed * dt;
  if (keystate[SDL_SCANCODE_W]) {
    vec3 vec3_scaled;
    vec3_scale(vec3_scaled, game_camera.front, camera_delta);
    vec3_scaled[1] = 0;
    vec3_add(game_camera.pos, game_camera.pos, vec3_scaled);
    moving = 1;
  }

  if (keystate[SDL_SCANCODE_S]) {
    vec3 vec3_temp;
    vec3_scale(vec3_temp, game_camera.front, camera_delta);
    vec3_temp[1] = 0;
    vec3_sub(game_camera.pos, game_camera.pos, vec3_temp);
    moving = 1;
  }

  if (keystate[SDL_SCANCODE_A]) {
    vec3 vec3_crossed, vec3_scaled, vec3_normalized;
    vec3_mul_cross(vec3_crossed, game_camera.front, game_camera.up);
    vec3_norm(vec3_normalized, vec3_crossed);
    vec3_scale(vec3_scaled, vec3_normalized, camera_delta);
    vec3_sub(game_camera.pos, game_camera.pos, vec3_scaled);
    moving = 1;
  }

  if (keystate[SDL_SCANCODE_D]) {
    vec3 vec3_crossed, vec3_scaled, vec3_normalized;
    vec3_mul_cross(vec3_crossed, game_camera.front, game_camera.up);
    vec3_norm(vec3_normalized, vec3_crossed);
    vec3_scale(vec3_scaled, vec3_normalized, camera_delta);
    vec3_add(game_camera.pos, game_camera.pos, vec3_scaled);
    moving = 1;
  }

  // camera walk
  if (moving) {
    camera_anim_time += dt;
    game_camera.pos[1] = 2 + sinf(camera_anim_time * 8) / 10;
  }
  else {
    game_camera.pos[1] = 2;
    camera_anim_time = 0;
  }
}

void input_event(SDL_Event* event) {
  if (event->type == SDL_KEYUP) {
    switch(event->key.keysym.sym) {
      case SDLK_c:
        input_data.capture_cursor = !input_data.capture_cursor;
        SDL_SetRelativeMouseMode(input_data.capture_cursor == 0 ? SDL_TRUE : SDL_FALSE);
        break;
      case SDLK_o:
        renderer_recompile_shader(); 
        break;
      case SDLK_SPACE:
        if (player.close_portal != NULL)
          dungeon_change_room(current_room + 1);
      default:
        break;
    }
  } else if (event->type == SDL_MOUSEMOTION) {
    input_mouse_callback(event->motion.xrel, event->motion.yrel);
  } else if (event->type == SDL_MOUSEBUTTONDOWN) {
    switch (event->button.button) {
      case SDL_BUTTON_LEFT:
        break;
      case SDL_BUTTON_RIGHT:
        break;
      default:
        break;
    }
  }

}
