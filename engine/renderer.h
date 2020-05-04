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

#define SSAO_MAX_KERNEL_SIZE 64
#define SSAO_MAX_NOISE_SIZE 16

GLFWwindow* window;

GLuint renderer_geometry_shader;
GLuint renderer_lighting_shader;
GLuint renderer_main_shader;
GLuint renderer_shadow_shader;
GLuint renderer_debug_shader;
GLuint renderer_skybox_shader;
GLuint renderer_ssao_shader;
GLuint renderer_ssao_blur_shader;
GLuint renderer_fxaa_shader;

GLuint renderer_depth_fbo;
GLuint renderer_depth_map;
GLuint renderer_vao;
GLuint renderer_vbo;

// deferred rendering
GLuint renderer_g_buffer;
GLuint renderer_g_position;
GLuint renderer_g_normal;
GLuint renderer_g_albedo;
GLuint renderer_g_spec;

// ssao
GLuint renderer_ssao_enabled;
GLuint renderer_ssao_fbo;
GLuint renderer_ssao_blur_fbo;
GLuint renderer_ssao_color;
GLuint renderer_ssao_blur;
GLuint renderer_ssao_noise_texture;
vec3 renderer_ssao_kernel[SSAO_MAX_KERNEL_SIZE];
vec3 renderer_ssao_noise[SSAO_MAX_NOISE_SIZE];
int renderer_ssao_debug_on;

// fxaa
int renderer_fxaa_enabled;
GLuint renderer_fxaa_fbo;
GLuint renderer_fxaa_texture;

int renderer_shadows_debug_enabled;
int renderer_render_aabb;
float renderer_shadow_bias;
int renderer_shadow_pcf_enabled;
float renderer_shadow_near;
float renderer_shadow_far;
float renderer_shadow_size;

int renderer_init(char* title, int width, int height, int fullscreen, GLFWwindow** out_window);
void renderer_cleanup();
void renderer_recompile_shader();
int renderer_should_close();
void renderer_init_object(object* o);
void renderer_free_object(object* o);
void renderer_render_objects(object* objects[], int objects_length, light* sun, light* lights[], int lights_length, camera* camera, void (*ui_render_callback)(void), skybox* sky);

ray renderer_raycast(camera* camera, float x, float y, float ray_len);

#endif
