#include "animator.h"

void animator_update(object* o, float dt) {
  animation* a = o->anim;
  skeleton* s = o->skel;

  // frame_copy_to(&s->rest_pose, &s->current_frame);

  animation_sample_to(a, dt, &s->current_frame);
  // frame_copy_to(&a->frames[1], &s->current_frame);
  // frame_copy_to(&a->frames[(int)(dt / 2)], &s->current_frame);

  frame* f = &s->current_frame;
  frame_gen_transforms(f);
  for (int i = 0; i < f->joint_count; i++) {
    mat4_mul(f->transforms[i], f->transforms[i], s->rest_pose.transforms_inv[i]);
  }

  // frame_copy_to(&a->frames[1], &s->current_frame);
}
