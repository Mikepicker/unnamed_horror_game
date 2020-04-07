#ifndef skeleton_h
#define skeleton_h

#include "../engine.h"
#include "frame.h"

typedef struct {
  int joint_count;
  char joint_names[256];
  frame rest_pose;
  frame current_frame;
} skeleton;

skeleton* skeleton_create();
void skeleton_free(skeleton* s);
void skeleton_joint_add(skeleton* s, int joint_id, char* name, int parent, mat4 transform);
int skeleton_joint_id(skeleton* s, char* name);

#endif
