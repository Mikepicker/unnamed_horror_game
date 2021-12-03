#ifndef particle_generator_h
#define particle_generator_h

#include "engine.h"
#include "data/camera.h"

#define PARTICLE_GENERATOR_SIZE 2048

typedef struct {
  vec3 position;
  vec3 velocity;
  float size;
  float alpha;
  float life;
} particle;

typedef struct {
  vec3 position;
  vec3 velocity;
  vec3 color;
  float size;
  float alpha;
  float life;
  float min_spread_x;
  float max_spread_x;
  float min_spread_y;
  float max_spread_y;
  float min_spread_z;
  float max_spread_z;
  char sprite_path[256];
} particle_config;

typedef struct {
  particle particles[PARTICLE_GENERATOR_SIZE];
  int* particles_indices_ordered; // indices ordered wrt camera (used for alpha blending)
  int amount;
  particle_config pc;
  GLuint sprite_id;
  GLuint vao;
  GLuint vbo_quad;
  GLuint vbo_pos;
  GLuint vbo_color_alpha;
} particle_generator;

particle_generator* particle_generator_new(particle_config pc, int amount);
void particle_generator_update(particle_generator* pg, float dt, unsigned int new_particles, camera* cam);
void particle_generator_free(particle_generator* pg);

#endif
