#include "ui.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT

#include "../libs/nuklear.h"
#include "../libs/nuklear_glfw_gl3.h"

struct nk_context *ctx;
struct nk_colorf bg;

void ui_init() {

 // ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);
 ctx = nk_glfw3_init(window, NK_GLFW3_DEFAULT);
 {struct nk_font_atlas *atlas;
   nk_glfw3_font_stash_begin(&atlas);
   nk_glfw3_font_stash_end();}

 bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;

 //ctx->style.window.background = nk_rgba(0,0,0,0);
 //ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(0,0,0,0));
 ctx->style.window.border_color = nk_rgb(255,165,0);
 ctx->style.window.combo_border_color = nk_rgb(255,165,0);
 ctx->style.window.contextual_border_color = nk_rgb(255,165,0);
 ctx->style.window.menu_border_color = nk_rgb(255,165,0);
 ctx->style.window.group_border_color = nk_rgb(255,165,0);
 ctx->style.window.tooltip_border_color = nk_rgb(255,165,0);
 ctx->style.window.scrollbar_size = nk_vec2(16,16);
 ctx->style.window.border_color = nk_rgba(0,0,0,0);
 ctx->style.window.border = 1;

 ctx->style.button.normal = nk_style_item_color(nk_rgba(0,0,0,0));
 ctx->style.button.hover = nk_style_item_color(nk_rgb(255,165,0));
 ctx->style.button.active = nk_style_item_color(nk_rgb(220,10,0));
 ctx->style.button.border_color = nk_rgb(255,165,0);
 ctx->style.button.text_background = nk_rgb(0,0,0);
 ctx->style.button.text_normal = nk_rgb(255,165,0);
 ctx->style.button.text_hover = nk_rgb(28,48,62);
 ctx->style.button.text_active = nk_rgb(28,48,62);
}

void ui_render() {

  nk_glfw3_new_frame();

  camera* cam = &game_camera;

  // debug
  if (nk_begin(ctx, "Debug", nk_rect(800, 50, 400, 500),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
        NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
  {
    nk_layout_row_static(ctx, 30, 120, 1);
    if (nk_button_label(ctx, "Toggle AABB"))
      renderer_render_aabb = renderer_render_aabb == 0 ? 1 : 0;

    nk_layout_row_static(ctx, 30, 120, 1);
    if (nk_button_label(ctx, "Reset camera")) {
      cam->pos[0] = 0.0f; 
      cam->pos[1] = 2.0f; 
      cam->pos[2] = 9.0f; 
      cam->front[0] = 0.0f;
      cam->front[1] = 0.0f;
      cam->front[2] = -1.0f;
    }

    nk_layout_row_static(ctx, 30, 120, 1);
    if (nk_button_label(ctx, "Start game")) {
      game_start();
    }

    nk_layout_row_static(ctx, 30, 120, 1);
    if (nk_button_label(ctx, "Compile shader")) {
      renderer_recompile_shader();
    }

    nk_layout_row_static(ctx, 30, 120, 1);
    if (nk_button_label(ctx, "Set light here")) {
      vec3_copy(lights[0].position, cam->pos);
    }

    nk_layout_row_static(ctx, 30, 120, 1);
    if (nk_button_label(ctx, "Toggle depth map")) {
      renderer_shadows_debug_enabled = renderer_shadows_debug_enabled == 0 ? 1 : 0;
    }

    nk_layout_row_static(ctx, 30, 120, 1);
    if (nk_button_label(ctx, "Toggle FXAA")) {
      renderer_fxaa_enabled = renderer_fxaa_enabled == 0 ? 1 : 0;
    }

    nk_layout_row_static(ctx, 30, 120, 1);
    if (nk_button_label(ctx, "Toggle SSAO")) {
      renderer_ssao_enabled = renderer_ssao_enabled == 0 ? 1 : 0;
    }

    nk_layout_row_static(ctx, 30, 120, 1);
    if (nk_button_label(ctx, "Toggle pcf")) {
      renderer_shadow_pcf_enabled = renderer_shadow_pcf_enabled == 0 ? 1 : 0;
    }

    char camera_pos[128];
    snprintf(camera_pos, 128, "camera: %f %f %f | %f %f %f\n", cam->pos[0], cam->pos[1], cam->pos[2], cam->front[0], cam->front[1], cam->front[2]);
    nk_label(ctx, camera_pos, NK_TEXT_LEFT);

    char item_pos[256];
    snprintf(item_pos, 256, "garand: %f %f %f\n", garand->position[0], garand->position[1], garand->position[2]);
    nk_label(ctx, item_pos, NK_TEXT_LEFT);

    char ui_fps[256];
    snprintf(ui_fps, 256, "fps: %f\n", fps);
    nk_label(ctx, ui_fps, NK_TEXT_LEFT);
  }
  nk_end(ctx);

  nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
}

void ui_free() {
  nk_glfw3_shutdown();
}
