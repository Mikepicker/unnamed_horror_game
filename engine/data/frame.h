#ifndef frame_h
#define frame_h

#include "../engine.h"

#define MAX_JOINTS 128

typedef struct {
  int joint_count;
  int joint_parents[MAX_JOINTS];
  vec3 joint_positions[MAX_JOINTS];
  quat joint_rotations[MAX_JOINTS];
  mat4 transforms[MAX_JOINTS];
  mat4 transforms_inv[MAX_JOINTS];
} frame;

frame* frame_create();
frame* frame_copy(frame* f);
frame* frame_interpolate(frame* f0, frame* f1, float amount);
void frame_copy_to(frame* f, frame* out);
void frame_interpolate_to(frame* f0, frame* f1, float amount, frame* out);
void frame_descendants_to(frame* f0, frame* f1, float amount, int joint, frame* out);

void frame_joint_transform(mat4 ret, frame* f, int i);
void frame_joint_add(frame* f, int joint_id, int parent, vec3 position, quat rotation);

void frame_gen_transforms(frame* f);
void frame_gen_inv_transforms(frame* f);

#endif
