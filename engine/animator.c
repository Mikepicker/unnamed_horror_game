#include "animator.h"

int animator_play(object* o, const char* name) {
  for (int i = 0; i < o->anim_count; i++) {
    if (strcmp(o->anims[i]->name, name) == 0) {
      o->current_anim = o->anims[i];
      return 1;
    }
  }

  printf("[animator] unable to find animation %s\n", name);
  return 0;
}

void animator_update(object* o, float time) {
  animation* a = o->current_anim;
  skeleton* s = o->skel;

  // frame_copy_to(&s->rest_pose, &s->current_frame);

  animation_sample_to(a, time, &s->current_frame);
  // frame_copy_to(&a->frames[0], &s->current_frame);
  // frame_copy_to(&a->frames[(int)(dt / 2)], &s->current_frame);

  frame* f = &s->current_frame;
  frame_gen_transforms(f);
  for (int i = 0; i < f->joint_count; i++) {
    mat4_mul(f->transforms[i], f->transforms[i], s->rest_pose.transforms_inv[i]);
  }
}
