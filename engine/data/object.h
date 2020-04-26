#ifndef object_h
#define object_h

#include "../engine.h"
#include "mesh.h"
#include "aabb.h"
#include "skeleton.h"
#include "animation.h"

#define OBJECT_MAX_ANIMS 16

struct object {
  // parent
  struct object* parent;
  int parent_joint;

  // transform
  vec3 position;
  vec3 center;
  GLfloat scale;
  quat rotation;

  // world transform
  mat4 world_transform;
  int calculate_transform;

  // meshes
  mesh* meshes;
  int num_meshes;

  // shaders
  vec3 color_mask;
  int glowing;
  vec3 glow_color;
  int receive_shadows;

  // physics
  aabb box;

  // audio
  ALuint audio_source;

  // animations
  skeleton* skel;
  animation* anims[OBJECT_MAX_ANIMS];
  int anim_count;
  animation* current_anim;
};

typedef struct object object;

object* object_create(vec3 position, GLfloat scale, mesh* meshes, int num_meshes, int compute_center, skeleton* s);
void object_add_animation(object* o, animation* a);
void object_get_transform(const object* o, mat4 m);
void object_get_center(const object* o, vec3* out_center);
void object_set_center(object* o);
void object_vec3_to_object_space(const object* o, vec3 v);
aabb object_aabb_to_object_space(const object* o, aabb box);
void object_set_position(object* o, vec3 pos);
void object_free(object* o);

#endif
