#include "../engine/seaengine.h"

#include "game.h"

typedef int32_t i32;
typedef uint32_t u32;
typedef int32_t b32;

int main()
{
  random_seed();

  // initialize SDL2
  assert(SDL_Init(SDL_INIT_VIDEO) >= 0);

  // set default OpenGL
  SDL_GL_LoadLibrary(NULL);

  // init opengl configs
  SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS);
  SDL_GL_SetAttribute (SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  // init context
  u32 window_flags = SDL_WINDOW_OPENGL;
  SDL_Window *window = SDL_CreateWindow("OpenGL Test", 0, 0, GAME_WIDTH, GAME_HEIGHT, window_flags);
  assert(window);

  // create OpenGL context
  SDL_GLContext context = SDL_GL_CreateContext(window);
  assert(context);

  // load OpenGL functions
  gladLoadGLLoader(SDL_GL_GetProcAddress);
  printf("Vendor:   %s\n", glGetString(GL_VENDOR));
  printf("Renderer: %s\n", glGetString(GL_RENDERER));
  printf("Version:  %s\n", glGetString(GL_VERSION));

  // Use v-sync
  SDL_GL_SetSwapInterval(1);

  b32 running = 1;
  b32 fullScreen = 0;

  // init renderer
  renderer_init(GAME_WIDTH, GAME_HEIGHT);

  // init audio
  if (audio_init() < 0) {
    printf("Error initializing audio\n");
    return -1;
  }

  // init game
  game_init(window);
  while (running)
  {
    SDL_Event event;
    ui_input_begin();
    while (SDL_PollEvent(&event)) {
      game_input(&event);
      if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
            running = 0;
            break;
          case 'f':
            fullScreen = !fullScreen;
            if (fullScreen) {
              SDL_SetWindowFullscreen(window, window_flags | SDL_WINDOW_FULLSCREEN);
            }
            else {
              SDL_SetWindowFullscreen(window, window_flags);
            }
            break;
          default:
            break;
        }
      }
    }
    ui_input_end();

    // render & update
    game_update();
    game_render();

    SDL_GL_SwapWindow(window);
  }

  // cleanup
  game_free();
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
