#ifndef dungeon_h
#define dungeon_h

#include "../engine/seaengine.h"

#define MAX_ROOMS 3
#define NUM_PORTALS 2

typedef struct portal portal;

struct portal {
  int x;
  int y;
  portal* link;
};

typedef struct {
  int w;
  int h;
  portal portals[NUM_PORTALS];
} room;

extern room dungeon[MAX_ROOMS];
extern int current_room;

void dungeon_change_room(int next_room);

void dungeon_generate();
void dungeon_update(float dt, camera* cam);
void dungeon_render(render_list* rl, light** lights, particle_generator** pgs);
void dungeon_free();

#endif
