#include "animation.h"

animation* create() {
  
  animation* a = malloc(sizeof(animation));
  
  a->frame_count = 0;
  a->frame_time = 1.0/30.0;
  
  return a;
}

void animation_free(animation* a) {
  free(a);
}

void animation_add_keyframe(animation* a, float k) {

}

void animation_add_frame(animation* a, frame* f, float keyframe_id) {

}
