#include "dungeon.h"
#include "bmp.h"

#define MAX_ROOMS 12
#define MAX_ROOM_ATTEMPTS 50
#define ROOM_OPTIONS 4

typedef struct {
  int x;
  int y;
  int w;
  int h;
} room;

dungeon_block dungeon[DUNGEON_SIZE][DUNGEON_SIZE];

room room_options[ROOM_OPTIONS];
room rooms_placed[MAX_ROOMS];
int last_placed = 0;

static void print_dungeon() {
  unsigned char image[DUNGEON_SIZE][DUNGEON_SIZE][BYTES_PER_PIXEL];
  char* imageFileName = (char*) "dungeon.bmp";

  for (int y = 0; y < DUNGEON_SIZE; y++) {
    for (int x = 0; x < DUNGEON_SIZE; x++) {
      switch(dungeon[y][x]) {
        case EMPTY:
          image[y][x][2] = 0; image[y][x][1] = 0; image[y][x][0] = 0;
          break;
        case ROOM:
          image[y][x][2] = 0; image[y][x][1] = 255; image[y][x][0] = 0;
          break;
        case NEXT_TO_ROOM:
          image[y][x][2] = 255; image[y][x][1] = 0; image[y][x][0] = 0;
          break;
      }
    }
  }

  generateBitmapImage((unsigned char*) image, DUNGEON_SIZE, DUNGEON_SIZE, imageFileName);
}

static int rooms_collide(room* r1, room* r2) {
  // increase r2 size to avoid placing rooms next to each other
  return r1->x < r2->x + r2->w + 1 &&
    r1->x + r1->w > r2->x - 1 &&
    r1->y < r2->y + r2->h + 1 &&
    r1->y + r1->h > r2->y - 1;
}

static int can_place_room(room* r) {
  for (int j = 0; j < last_placed; j++) {
    if (rooms_collide(r, &rooms_placed[j])) {
      return 0;
    }
  }

  return 1;
}

static void generate_room() {
  room* r = &room_options[(int)random_range(0, ROOM_OPTIONS)];
  for (int i = 0; i < MAX_ROOM_ATTEMPTS; i++) {
    float x = random_range(4, DUNGEON_SIZE - r->w + 1);
    float y = random_range(4, DUNGEON_SIZE - r->h + 1);

    r->x = x;
    r->y = y;
    
    if (can_place_room(r)) {
      memcpy(&rooms_placed[last_placed++], r, sizeof(room));

      // set next_to_room blocks
      for (int y = clamp(r->y - 1, 0, DUNGEON_SIZE); y < clamp(r->y + r->h + 1, 0, DUNGEON_SIZE); y++) {
        for (int x = clamp(r->x - 1, 0, DUNGEON_SIZE); x < clamp(r->x + r->w + 1, 0, DUNGEON_SIZE); x++) {
          dungeon[y][x] = NEXT_TO_ROOM;
        }
      }

      // set room blocks
      for (int y = r->y; y < r->y + r->h; y++) {
        for (int x = r->x; x < r->x + r->w; x++) {
          dungeon[y][x] = ROOM;
        }
      }

      return;
    }
  }

  printf("[dungeon] generate room error: no free space found!\n");
}

static void generate_rooms() {
  for (int i = 0; i < MAX_ROOMS; i++) {
    generate_room();
  }
}

static void init_dungeon() {
  // pre-defined rooms
  room_options[0].w = 4;
  room_options[0].h = 4;

  room_options[1].w = 8;
  room_options[1].h = 4;

  room_options[2].w = 4;
  room_options[2].h = 8;

  room_options[3].w = 6;
  room_options[3].h = 3;

  // init empty blocks
  for (int y = 0; y < DUNGEON_SIZE; y++) {
    for (int x = 0; x < DUNGEON_SIZE; x++) {
      dungeon[y][x] = EMPTY;
    }
  }
}

void dungeon_generate() {
  // 1. init dungeon
  init_dungeon();

  // 2. place some rooms
  generate_rooms();
  print_dungeon();
}
