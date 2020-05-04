#include "../engine/seaengine.h"

#include "game.h"

int main()
{
  random_seed();

  // Init context
  GLFWwindow* window;
  if (renderer_init("Ada", GAME_WIDTH, GAME_HEIGHT, 0, &window) < 0) {
    printf("Error initializing renderer!\n");
    return -1;
  }

  // devmode ON
  glfwSetWindowPos(window, -1400, 100);

  // Init audio
  if (audio_init() < 0) {
    printf("Error initializing audio\n");
    return -1;
  }

  // init game
  game_init(window);

  // render & update
  while (!renderer_should_close()) {
    game_update();
    game_render();
  }

  // cleanup
  game_free();

  return 0;
}
