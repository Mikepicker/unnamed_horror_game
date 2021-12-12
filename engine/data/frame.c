#include "frame.h"

frame* frame_create() {
  frame* f = malloc(sizeof(frame));
  f->joint_count = 0;
  return f;
}

frame* frame_copy(frame* f) {
  
  frame* fn = frame_create();
  
  for (int i = 0; i < f->joint_count; i++) {
    frame_joint_add(fn, i, f->joint_parents[i], f->joint_positions[i], f->joint_rotations[i]);
  }
  
  return fn;

}

frame* frame_interpolate(frame* f0, frame* f1, float amount) {

  frame* fn = frame_copy(f0);
  frame_interpolate_to(f0, f1, amount, fn);
  return fn;

}

void frame_interpolate_to(frame* f0, frame* f1, float amount, frame* out) {
  
  for (int i = 0; i < out->joint_count; i++) {
    vec3_lerp(out->joint_positions[i], f0->joint_positions[i], f1->joint_positions[i], amount);
    quat_slerp(out->joint_rotations[i], f0->joint_rotations[i], f1->joint_rotations[i], amount);
  }
  
}

void frame_copy_to(frame* f, frame* out) {
  memcpy(out, f, sizeof(frame));
}

static bool frame_descendant_of(frame* f, int decendent, int joint) {
  
  if (f->joint_parents[decendent] == joint) { return true;  }
  if (f->joint_parents[decendent] == -1)    { return false; }
  return frame_descendant_of(f, f->joint_parents[decendent], joint);
  
}

void frame_descendants_to(frame* f0, frame* f1, float amount, int joint, frame* out) {

  for (int i = 0; i < out->joint_count; i++) {
    if (frame_descendant_of(out, i, joint) || (joint == i)) {
      vec3_lerp(out->joint_positions[i], f0->joint_positions[i], f1->joint_positions[i], amount);
      quat_slerp(out->joint_rotations[i], f0->joint_rotations[i], f1->joint_rotations[i], amount);
    } else {
      vec3_lerp(out->joint_positions[i], f0->joint_positions[i], f1->joint_positions[i], 0.0);
      quat_slerp(out->joint_rotations[i], f0->joint_rotations[i], f1->joint_rotations[i], 0.0);
    }
  }

}

void frame_joint_transform(mat4 ret, frame* f, int i) {
  
  if (f->joint_transforms_computed[i]) {
    mat4_copy(ret, f->joint_transforms[i]);
    return;
  }

  mat4 rot;
  mat4_identity(ret);

  if (f->joint_parents[i] != -1) {
    mat4 prev;
    frame_joint_transform(prev, f, f->joint_parents[i]);
    mat4_mul(ret, ret, prev);
  }  

  mat4 t;
  mat4_translate(t, f->joint_positions[i][0], f->joint_positions[i][1], f->joint_positions[i][2]);

  mat4_from_quat(rot, f->joint_rotations[i]);
  mat4_mul(ret, ret, t);
  mat4_mul(ret, ret, rot);
  mat4_copy(f->joint_transforms[i], ret);
  f->joint_transforms_computed[i] = 1;

}

void frame_joint_add(frame* f, int joint_id, int parent, vec3 position, quat rotation) {
  
  f->joint_count++;
  assert(f->joint_count < MAX_JOINTS);

  f->joint_parents[joint_id] = parent;
  vec3_copy(f->joint_positions[joint_id], position);
  vec3_copy(f->joint_rotations[joint_id], rotation);
  mat4_identity(f->transforms[joint_id]);
  mat4_identity(f->transforms_inv[joint_id]);

}

void frame_gen_transforms(frame* f) {
  
  for (int i = 0; i < f->joint_count; i++) {
    f->joint_transforms_computed[i] = 0;
  }

  for (int i = 0; i < f->joint_count; i++) {
    frame_joint_transform(f->transforms[i], f, i);
  }
  
}

void frame_gen_inv_transforms(frame* f) {

  for (int i = 0; i < f->joint_count; i++) {
    mat4_invert(f->transforms_inv[i], f->transforms[i]);
  }

}

