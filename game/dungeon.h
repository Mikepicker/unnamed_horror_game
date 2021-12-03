#ifndef dungeon_h
#define dungeon_h

#include "../engine/seaengine.h"

#define NUM_PORTALS 2

typedef struct {
  int x;
  int y;
  light l;
  object* m;
  particle_generator* pg;
} portal;

typedef struct {
  int w;
  int h;
  portal portals[NUM_PORTALS];
} room;

void dungeon_generate();
void dungeon_update(float dt, camera* cam);
void dungeon_render(render_list* rl, light** lights, particle_generator** pgs);
void dungeon_free();

#endif
