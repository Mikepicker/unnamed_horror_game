#ifndef dungeon_h
#define dungeon_h

#include "../engine/seaengine.h"

#define DUNGEON_SIZE 30

typedef enum { EMPTY, ROOM, NEXT_TO_ROOM } dungeon_block;

extern dungeon_block dungeon[DUNGEON_SIZE][DUNGEON_SIZE];

void dungeon_generate();

#endif
