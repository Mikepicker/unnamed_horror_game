#ifndef dungeon_h
#define dungeon_h

#include "../engine/seaengine.h"

typedef struct {
  int w;
  int h;
} room;

void dungeon_generate();
void dungeon_render(render_list* rl);
void dungeon_free();

#endif
