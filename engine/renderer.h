#ifndef renderer_h
#define renderer_h

#include "engine.h"
#include "shader.h"
#include "skybox.h"
#include "random.h"
#include "data/object.h"
#include "data/light.h"
#include "data/camera.h"
#include "data/ray.h"
#include "data/frame.h"

extern GLFWwindow* window;

extern GLuint renderer_ssao_enabled;
extern int renderer_ssao_debug_on;
extern int renderer_fxaa_enabled;
extern int renderer_shadows_debug_enabled;
extern int renderer_render_aabb;
extern int renderer_shadow_pcf_enabled;

int renderer_init(char* title, int width, int height, int fullscreen, GLFWwindow** out_window);
void renderer_cleanup();
void renderer_recompile_shader();
int renderer_should_close();
void renderer_init_object(object* o);
void renderer_free_object(object* o);
void renderer_render_objects(object* objects[], int objects_length, object* screen_objects[], int screen_objects_length, light* sun, light* lights[], int lights_length, camera* camera, void (*ui_render_callback)(void), skybox* sky);

ray renderer_raycast(camera* camera, float x, float y, float ray_len);

#endif
