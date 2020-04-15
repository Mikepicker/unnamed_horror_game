#include "animator.h"

int animator_play(object* o, const char* name, int loop) {
  for (int i = 0; i < o->anim_count; i++) {
    if (strcmp(o->anims[i]->name, name) == 0) {
      o->current_anim = o->anims[i];
      o->current_anim->frame_time = 0;
      o->current_anim->loop = loop;
      o->current_anim->finished = 0;
      return 1;
    }
  }

  printf("[animator] unable to find animation %s\n", name);
  return 0;
}

void animator_update(object* o, float dt) {
  animation* a = o->current_anim;
  skeleton* s = o->skel;

  animation_sample_to(a, dt, &s->current_frame);

  frame* f = &s->current_frame;
  frame_gen_transforms(f);
  for (int i = 0; i < f->joint_count; i++) {
    mat4_mul(f->transforms[i], f->transforms[i], s->rest_pose.transforms_inv[i]);
  }
}

int animator_current_keyframe(object* o) {
  animation* a = o->current_anim;

  float curr_keyframe;
  float curr_time = (a->frame_time / a->frame_speed);

  curr_keyframe = curr_time < 0 ? 0 : curr_time;
  curr_keyframe = curr_keyframe > (a->frame_count-1) ? (a->frame_count-1) : curr_keyframe;

  return round(curr_keyframe);
}

int animator_total_keyframes(object* o) {
  animation* a = o->current_anim;
  return a->keyframe_count;
}

int animator_finished(object* o) {
  return o->current_anim->finished;
}
