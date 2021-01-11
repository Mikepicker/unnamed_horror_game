#ifndef dungeon_h
#define dungeon_h

#include "../engine/seaengine.h"

#define NUM_PORTALS 2

typedef struct {
  int x;
  int y;
  light l;
  object* m;
} portal;

typedef struct {
  int w;
  int h;
  portal portals[NUM_PORTALS];
} room;

void dungeon_generate();
void dungeon_render(render_list* rl, light** lights);
void dungeon_free();

#endif
