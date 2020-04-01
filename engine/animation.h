#ifndef animation_h
#define animation_h

#include "engine.h"
#include "data/frame.h"

#define MAX_KEYFRAMES 32

typedef struct {
  float keyframes[MAX_KEYFRAMES];
  float keyframe_count;
  frame* frames[MAX_KEYFRAMES];
  int frame_count;
  int frame_time;
  int duration;
} animation;

animation* animation_create();
void animation_free(animation* a);

void animation_add_keyframe(animation* a, float k);
void animation_add_frame(animation* a, frame* f, float keyframe_id);

#endif
