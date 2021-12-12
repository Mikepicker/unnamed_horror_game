#include <stdlib.h>
#include "particle_generator.h"
#include "random.h"

unsigned int last_used_particle = 0;

static void reset_particle(particle* p, particle_config* pc, bool is_init) {
  vec3_copy(p->position, pc->position);
  vec3_copy(p->velocity, pc->velocity);
  vec3 rnd = {
    random_range(pc->min_spread_x, pc->max_spread_x),
    random_range(pc->min_spread_y, pc->max_spread_y),
    random_range(pc->min_spread_z, pc->max_spread_z)
  };
  vec3_add(p->velocity, pc->velocity, rnd);
  p->size = pc->size;
  p->alpha = 1;
  if (!is_init) p->life = pc->life; 
  else p->life = 0;
}

particle_generator* particle_generator_new(particle_config pc, int amount) {
  assert(amount < PARTICLE_GENERATOR_SIZE);

  particle_generator* pg = malloc(sizeof(particle_generator));
  pg->amount = amount;
  pg->pc = pc;

  // init indices (for alpha blending)
  pg->particles_indices_ordered = malloc(sizeof(int) * amount);
  for (int i = 0; i < amount; i++) {
    pg->particles_indices_ordered[i] = i;
  }

  for (int i = 0; i < amount; i++) {
    reset_particle(&pg->particles[i], &pc, true);
  }

  return pg;
}

static unsigned int first_unused_particle(particle_generator* pg) {
  // first search from last used particle, this will usually return almost instantly
  for (unsigned int i = last_used_particle; i < pg->amount; i++){
    if (pg->particles[i].life <= 0.0f){
      last_used_particle = i;
      return i;
    }
  }

  // otherwise, do a linear search
  for (unsigned int i = 0; i < last_used_particle; i++){
    if (pg->particles[i].life <= 0.0f){
      last_used_particle = i;
      return i;
    }
  }

  // all particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
  last_used_particle = 0;
  return 0;
}

typedef struct {
  camera* cam;
  particle* particles;
} sort_struct;

static int sort_particles(void* a, void* b, void* s) {
  camera* cam = ((sort_struct*)s)->cam;
  particle* particles = ((sort_struct*)s)->particles;
  vec3 dist_a; vec3 dist_b;
  vec3_sub(dist_a, particles[*(int*)a].position, cam->pos);
  vec3_sub(dist_b, particles[*(int*)b].position, cam->pos);
  return vec3_len(dist_b) - vec3_len(dist_a);
}

void particle_generator_update(particle_generator* pg, float dt, unsigned int new_particles, camera* cam) {
  // add new particles
  for (int i = 0; i < new_particles; i++) {
    unsigned int unused_particle = first_unused_particle(pg);
    reset_particle(&pg->particles[unused_particle], &pg->pc, false);
  }

  // update all particles
  for (int i = 0; i < pg->amount; i++) {
    particle* p = &pg->particles[i];
    p->life -= dt;
    if (p->life > 0.0f) {
      vec3 vt;
      vec3_scale(vt, p->velocity, dt);
      vec3_add(p->position, p->position, p->velocity);

      p->alpha = p->life / pg->pc.life;
    } else {
      p->alpha = 0;
      p->size = 0;
    }
  }

  // sort indices for alpha blending
  // sort_struct s = { .cam = cam, .particles = pg->particles };
  // qsort_r(pg->particles_indices_ordered, pg->amount, sizeof(int), sort_particles, &s);
}

void particle_generator_free(particle_generator* pg) {
  free(pg->particles_indices_ordered);
  free(pg);
}

