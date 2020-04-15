#include "animation.h"

animation* animation_create(const char* name) {
  
  animation* a = malloc(sizeof(animation));
  
  strcpy(a->name, name);
  a->keyframe_count = 0;
  a->frame_count = 0;
  a->frame_time = 0;
  a->frame_speed = 1.0/30.0;
  a->loop = 1;
  a->finished = 0;
  
  for (int i = 0; i < MAX_KEYFRAMES; i++) {
    a->frames[i].joint_count = 0;  
  }

  return a;
}

void animation_free(animation* a) {
  free(a);
}

void animation_add_keyframe(animation* a, float k) {
  assert(a->keyframe_count < MAX_KEYFRAMES);
  a->keyframes[a->keyframe_count++] = k; 
  a->duration = k;
}

static frame* animation_frame(animation* a, int i) {
  i = i < 0 ? 0 : i;
  i = i > (a->frame_count-1) ? (a->frame_count-1) : i;
  return &a->frames[i];
}

void animation_sample_to(animation* a, float dt, frame* out) {

  assert(a->frame_count > 0);

  if (a->frame_count == 1) {
    frame_copy_to(&a->frames[0], out);
  }

  a->frame_time += dt;
  if (a->loop)
    a->frame_time = fmodf(a->frame_time, a->frame_speed * (a->frame_count-1));
  else
    a->finished = (a->frame_time / a->frame_speed) > (a->frame_count-1);

  frame* frame0 = animation_frame(a, (a->frame_time / a->frame_speed) + 0);
  frame* frame1 = animation_frame(a, (a->frame_time / a->frame_speed) + 1);
  float amount = fmod(a->frame_time / a->frame_speed, 1.0);

  frame_interpolate_to(frame0, frame1, amount, out);

}
