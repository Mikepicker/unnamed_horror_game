#ifndef renderer_h
#define renderer_h

#include "engine.h"
#include "shader.h"
#include "skybox.h"
#include "random.h"
#include "particle_generator.h"
#include "data/object.h"
#include "data/light.h"
#include "data/camera.h"
#include "data/ray.h"
#include "data/frame.h"

extern GLuint renderer_ssao_enabled;
extern int renderer_ssao_debug_on;
extern int renderer_fxaa_enabled;
extern int renderer_shadows_debug_enabled;
extern int renderer_render_aabb;
extern int renderer_shadow_pcf_enabled;

int renderer_init(int width, int height);
void renderer_free();
void renderer_recompile_shader();
void renderer_init_object(object* o);
void renderer_free_object(object* o);
void renderer_init_particle_generator(particle_generator* pg);
void renderer_free_particle_generator(particle_generator* pg);
void renderer_render_objects(int width, int height, object* objects[], int objects_length, object* screen_objects[], int screen_objects_length, light* lights[], int lights_length, camera* camera, void (*ui_render_callback)(void), skybox* sky, particle_generator* particle_generators[], int particle_generators_length);

ray renderer_raycast(int width, int height, camera* camera, float x, float y, float ray_len);

#endif
