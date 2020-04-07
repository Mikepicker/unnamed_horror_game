#include "skeleton.h"

skeleton* skeleton_create() {
  
  skeleton* s = malloc(sizeof(skeleton));
  s->joint_count = 0;
  // s->joint_names = NULL;
  s->rest_pose.joint_count = 0;  
  s->current_frame.joint_count = 0;  

  return s;
  
}

void skeleton_free(skeleton* s) {
  free(s);
}

void skeleton_joint_add(skeleton* s, int joint_id, char* name, int parent, mat4 transform) {
  
  s->joint_count++;
  assert(s->joint_count < MAX_JOINTS);

  strcpy(&s->joint_names[joint_id], name);
  
  vec3 zero;
  vec3_zero(zero);

  quat q;
  quat_identity(q);
  frame_joint_add(&s->rest_pose, joint_id, parent, zero, q);

  // set rotation
  quat_from_mat4(s->rest_pose.joint_rotations[joint_id], transform);

  // set position
  vec3 position = { transform[3][0], transform[3][1], transform[3][2] };
  vec3_copy(s->rest_pose.joint_positions[joint_id], position);

  // set inverse position
  mat4_invert(s->rest_pose.transforms_inv[joint_id], transform);

}

int skeleton_joint_id(skeleton* s, char* name) {
  
  for (int i = 0; i < s->joint_count; i++) {
    if (strcmp(&s->joint_names[i], name) == 0) { return i; }
  }
  
  printf("[skeleton] Error: skeleton has no joint named '%s'", name);
  return -1;
  
}
