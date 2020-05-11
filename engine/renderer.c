#define STB_IMAGE_IMPLEMENTATION
#include "renderer.h"

#define SHADOW_WIDTH 1024 * 8
#define SHADOW_HEIGHT 1024 * 8

void set_opengl_state() {
  glEnable(GL_DEPTH_TEST);
  // glEnable(GL_MULTISAMPLE);
  glEnable(GL_CULL_FACE);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

unsigned int load_image(char* filename) {
  GLuint texture = -1;

  if (strlen(filename) == 0) {
    return 0;
  }

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load image, create texture and generate mipmaps
  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
  // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
  unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
  if (data) {
    GLenum format;
    if (nrChannels == 1)
      format = GL_RED;
    if (nrChannels == 2)
      format = GL_ALPHA;
    else if (nrChannels == 3)
      format = GL_RGB;
    else if (nrChannels == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  else {
    printf("Error loading texture: %s\n", filename);
  }

  stbi_image_free(data);
  return texture;
}

static void init_depth_fbo() {
  glGenFramebuffers(1, &renderer_depth_fbo);
  // create depth texture
  glGenTextures(1, &renderer_depth_map);
  glBindTexture(GL_TEXTURE_2D, renderer_depth_map);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  // attach depth texture as FBO's depth buffer
  glBindFramebuffer(GL_FRAMEBUFFER, renderer_depth_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderer_depth_map, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void init_g_buffer(int width, int height) {
  // configure g-buffer framebuffer
  glGenFramebuffers(1, &renderer_g_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, renderer_g_buffer);
  
  // position color buffer
  glGenTextures(1, &renderer_g_position);
  glBindTexture(GL_TEXTURE_2D, renderer_g_position);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer_g_position, 0);
  // normal color buffer
  glGenTextures(1, &renderer_g_normal);
  glBindTexture(GL_TEXTURE_2D, renderer_g_normal);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderer_g_normal, 0);
  // color buffer
  glGenTextures(1, &renderer_g_albedo);
  glBindTexture(GL_TEXTURE_2D, renderer_g_albedo);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, renderer_g_albedo, 0);
  // specular
  glGenTextures(1, &renderer_g_spec);
  glBindTexture(GL_TEXTURE_2D, renderer_g_spec);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, renderer_g_spec, 0);
  // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
  unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
  glDrawBuffers(4, attachments);

  unsigned int rboDepth;
  glGenRenderbuffers(1, &rboDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
  // finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("[renderer] framebuffer not complete\n");
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void init_ssao(int width, int height) {
  // color
  glGenFramebuffers(1, &renderer_ssao_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, renderer_ssao_fbo);
  glGenTextures(1, &renderer_ssao_color);
  glBindTexture(GL_TEXTURE_2D, renderer_ssao_color);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer_ssao_color, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("[renderer] error: SSAO Framebuffer not complete\n");

  // blur
  glGenFramebuffers(1, &renderer_ssao_blur_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, renderer_ssao_blur_fbo);
  glGenTextures(1, &renderer_ssao_blur);
  glBindTexture(GL_TEXTURE_2D, renderer_ssao_blur);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer_ssao_blur, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("[renderer] error: SSAO Framebuffer not complete\n");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // init kernel
  for (int i = 0; i < SSAO_MAX_KERNEL_SIZE; i++) {
    vec3 sample = { random_range(0, 1) * 2 - 1, random_range(0, 1) * 2 - 1, random_range(0, 1) };
    vec3_norm(sample, sample);
    vec3_scale(sample, sample, random_range(0, 1));
    float scale = (float)i / SSAO_MAX_KERNEL_SIZE;

    scale = lerp(0.1f, 1.0f, scale * scale);
    vec3_scale(sample, sample, scale);
    vec3_copy(renderer_ssao_kernel[i], sample);
  }

  // init noise texture
  for (int i = 0; i < SSAO_MAX_NOISE_SIZE; i++) {
    vec3 sample = { random_range(0, 1) * 2 - 1, random_range(0, 1) * 2 - 1, 0 };
    vec3_copy(renderer_ssao_noise[i], sample);
  }

  glGenTextures(1, &renderer_ssao_noise_texture);
  glBindTexture(GL_TEXTURE_2D, renderer_ssao_noise_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &renderer_ssao_noise[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void init_post(int width, int height) {
  glGenFramebuffers(1, &renderer_post_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, renderer_post_fbo);
  glGenTextures(1, &renderer_post_texture);
  glBindTexture(GL_TEXTURE_2D, renderer_post_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer_post_texture, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("[renderer] error: FXAA Framebuffer not complete\n");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int renderer_init(char* title, int width, int height, int fullscreen, GLFWwindow** out_window) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  window = glfwCreateWindow(width, height, title, fullscreen > 0 ? glfwGetPrimaryMonitor() : NULL, NULL);
  *out_window = window;
  if (!window) {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    return -1;
  }

  // compile shaders
  renderer_recompile_shader();

  // init gbuffer
  init_g_buffer(width, height);

  // init vars
  renderer_render_aabb = 0;
  renderer_shadow_near = 1.0f;
  renderer_shadow_far = 80.0f;
  renderer_shadow_size = 100.0f;
  renderer_vao = 0;
  renderer_shadows_debug_enabled = 0;
  renderer_shadow_bias = 0.22f;
  renderer_shadow_pcf_enabled = 1;

  // fxaa
  renderer_fxaa_enabled = 1;

  // ssao
  renderer_ssao_enabled = 0;
  renderer_ssao_debug_on = 0;

  init_ssao(width, height);

  init_post(width, height);

  // init depth fbo
  init_depth_fbo();

  // set opengl state
  set_opengl_state();

  return 0;
}

void renderer_cleanup() {
  glfwTerminate();
}

void renderer_recompile_shader() {
  shader_compile("../engine/shaders/geometry.vs", "../engine/shaders/geometry.fs", &renderer_geometry_shader);
  shader_compile("../engine/shaders/lighting.vs", "../engine/shaders/lighting.fs", &renderer_lighting_shader);
  shader_compile("../engine/shaders/toon.vs", "../engine/shaders/toon.fs", &renderer_main_shader);
  shader_compile("../engine/shaders/shadow.vs", "../engine/shaders/shadow.fs", &renderer_shadow_shader);
  shader_compile("../engine/shaders/debug.vs", "../engine/shaders/debug.fs", &renderer_debug_shader);
  shader_compile("../engine/shaders/skybox.vs", "../engine/shaders/skybox.fs", &renderer_skybox_shader);
  shader_compile("../engine/shaders/ssao.vs", "../engine/shaders/ssao.fs", &renderer_ssao_shader);
  shader_compile("../engine/shaders/ssao.vs", "../engine/shaders/blur.fs", &renderer_ssao_blur_shader);
  shader_compile("../engine/shaders/post.vs", "../engine/shaders/post.fs", &renderer_post_shader);
}

int renderer_should_close() {
  return glfwWindowShouldClose(window);
}

static void add_aabb(object* o) {
  aabb* aabb = &o->box;
  if (!aabb) {
    return;
  }

  GLfloat vertices[] = {
    -0.5, -0.5, -0.5,
     0.5, -0.5, -0.5,
     0.5,  0.5, -0.5,
    -0.5,  0.5, -0.5,
    -0.5, -0.5,  0.5,
     0.5, -0.5,  0.5,
     0.5,  0.5,  0.5,
    -0.5,  0.5,  0.5,
  };

  glGenVertexArrays(1, &(aabb->vao)); // Vertex Array Object
  glGenBuffers(1, &(aabb->vbo));      // Vertex Buffer Object
  glGenBuffers(1, &(aabb->ebo));      // Element Buffer Object

  glBindVertexArray(aabb->vao);

  glBindBuffer(GL_ARRAY_BUFFER, aabb->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  GLushort elements[] = {
    0, 1, 2, 3,
    4, 5, 6, 7,
    0, 4, 1, 5, 2, 6, 3, 7
  };
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, aabb->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void renderer_init_object(object* o) {
  for (int i = 0; i < o->num_meshes; i++) {
    mesh* mesh = &o->meshes[i];
    glGenVertexArrays(1, &(mesh->vao)); // Vertex Array Object
    glGenBuffers(1, &(mesh->vbo));      // Vertex Buffer Object
    glGenBuffers(1, &(mesh->ebo));      // Element Buffer Object

    glBindVertexArray(mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->num_vertices * sizeof(vertex), mesh->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->num_indices * sizeof(GLuint), mesh->indices, GL_STATIC_DRAW);

    // sum of all vertex components
    int total_size = 17;

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, total_size * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, total_size * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // normals attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, total_size * sizeof(GLfloat), (GLvoid *)(5 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // tangents attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, total_size * sizeof(GLfloat), (GLvoid *)(8 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3);

    // joint ids attribute
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, total_size * sizeof(GLfloat), (GLvoid *)(11 * sizeof(GLfloat)));
    glEnableVertexAttribArray(4);

    // weights attribute
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, total_size * sizeof(GLfloat), (GLvoid *)(14 * sizeof(GLfloat)));
    glEnableVertexAttribArray(5);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // texture
    mesh->texture_id = load_image(mesh->mat.texture_path);
    mesh->normal_map_id = load_image(mesh->mat.normal_map_path);
    mesh->specular_map_id = load_image(mesh->mat.specular_map_path);
    mesh->mask_map_id = load_image(mesh->mat.mask_map_path);
  }

  // aabb
  add_aabb(o);
}

void renderer_free_object(object* o) {
  for (int i = 0; i < o->num_meshes; i++) {
    glDeleteVertexArrays(1, &(o->meshes[i].vao));
    glDeleteBuffers(1, &(o->meshes[i].vbo));
    glDeleteBuffers(1, &(o->meshes[i].ebo));
  }
}

static void render_aabb(object* o) {
  aabb* aabb = &o->box;
  mat4 m;
  mat4_identity(m);

  vec3 size = {aabb->max_x - aabb->min_x, aabb->max_y - aabb->min_y, aabb->max_z - aabb->min_z};
  mat4_scale_aniso(m, m, size[0], size[1], size[2]);

  vec3 center = {
    (aabb->min_x + aabb->max_x) / 2,
    (aabb->min_y + aabb->max_y) / 2,
    (aabb->min_z + aabb->max_z) / 2
  };

  mat4 translation;
  vec3 pos = {
    (o->position[0] * o->scale + center[0]) / size[0],
    (o->position[1] * o->scale + center[1]) / size[1],
    (o->position[2] * o->scale + center[2]) / size[2]
  };

  mat4_translate(translation, pos[0], pos[1], pos[2]);
  mat4_mul(m, m, translation);

  glUniformMatrix4fv(glGetUniformLocation(renderer_main_shader, "M"), 1, GL_FALSE, (const GLfloat*) m);

  glBindVertexArray(aabb->vao);
  glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
  glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4 * sizeof(GLushort)));
  glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(GLushort)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void render_object(object* o, GLuint shader_id) {
  glUniformMatrix4fv(glGetUniformLocation(shader_id, "M"), 1, GL_FALSE, (const GLfloat*) o->world_transform);

  // handle animated objects
  if (o->skel != NULL) {
    glUniformMatrix4fv(glGetUniformLocation(shader_id, "boneTransforms"), o->skel->joint_count, GL_FALSE, (const GLfloat*) o->skel->current_frame.transforms);
    glUniform1i(glGetUniformLocation(shader_id, "hasSkeleton"), 1);
  } else {
    glUniform1i(glGetUniformLocation(shader_id, "hasSkeleton"), 0);
  }

  // render params
  glUniform3fv(glGetUniformLocation(shader_id, "color_mask"), 1, o->color_mask);
  glUniform1i(glGetUniformLocation(shader_id, "glowing"), o->glowing);
  glUniform3fv(glGetUniformLocation(shader_id, "glow_color"), 1, o->glow_color);
  glUniform1i(glGetUniformLocation(shader_id, "receive_shadows"), o->receive_shadows);

  for (int i = 0; i < o->num_meshes; i++) {
    mesh* mesh = &o->meshes[i];

    // pass material
    glUniform3fv(glGetUniformLocation(shader_id, "material.diffuse"), 1, mesh->mat.diffuse);
    glUniform1f(glGetUniformLocation(shader_id, "material.specular"), mesh->mat.specular);
    glUniform1f(glGetUniformLocation(shader_id, "material.reflectivity"), mesh->mat.reflectivity);

    glUniform1i(glGetUniformLocation(shader_id, "texture_subdivision"), mesh->mat.texture_subdivision);

    // bind texture
    if (strlen(mesh->mat.texture_path) > 0) {
      glUniform1i(glGetUniformLocation(shader_id, "texture_diffuse"), 1);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, mesh->texture_id);
      glUniform1i(glGetUniformLocation(shader_id, "hasDiffuseMap"), 1);
    } else {
      glUniform1i(glGetUniformLocation(shader_id, "hasDiffuseMap"), 0);
    }

    // bind normal map
    if (strlen(mesh->mat.normal_map_path) > 0) {
      glUniform1i(glGetUniformLocation(shader_id, "texture_normal"), 2);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, mesh->normal_map_id);
      glUniform1i(glGetUniformLocation(shader_id, "hasNormalMap"), 1);
    } else {
      glUniform1i(glGetUniformLocation(shader_id, "hasNormalMap"), 0);
    }

    // bind specular map
    if (strlen(mesh->mat.specular_map_path) > 0) {
      glUniform1i(glGetUniformLocation(shader_id, "texture_specular"), 3);
      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, mesh->specular_map_id);
      glUniform1i(glGetUniformLocation(shader_id, "hasSpecularMap"), 1);
    } else {
      glUniform1i(glGetUniformLocation(shader_id, "hasSpecularMap"), 0);
    }

    // bind mask map
    if (strlen(mesh->mat.mask_map_path) > 0) {
      glUniform1i(glGetUniformLocation(shader_id, "texture_mask"), 4);
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, mesh->mask_map_id);
      glUniform1i(glGetUniformLocation(shader_id, "hasMaskMap"), 1);
    } else {
      glUniform1i(glGetUniformLocation(shader_id, "hasMaskMap"), 0);
    }

    // render the mesh
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT , 0);
  }

  if (renderer_render_aabb)
    render_aabb(o);
}

static void render_objects(object *objects[], int objects_length, GLuint shader_id) {
  for (int i = 0; i < objects_length; i++) {
    object* o = objects[i];
    render_object(o, shader_id);
  }
}

static void render_quad() {
  if (renderer_vao == 0) {
    float quadVertices[] = {
      // positions        // texture Coords
      -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // setup plane VAO
    glGenVertexArrays(1, &renderer_vao);
    glGenBuffers(1, &renderer_vbo);
    glBindVertexArray(renderer_vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  }
  glBindVertexArray(renderer_vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

static void calculate_world_transform(object* o) {
  if (!o->calculate_transform) {
    return;
  }

  mat4 parent_transform;
  mat4_identity(parent_transform);

  if (o->parent != NULL) {
    calculate_world_transform(o->parent);

    mat4_mul(parent_transform, parent_transform, o->parent->world_transform);

    if (o->parent_joint >= 0) {
      mat4_mul(parent_transform, parent_transform, o->parent->skel->current_frame.transforms[o->parent_joint]);
    }
  }

  mat4 m;

  mat4_identity(m);

  // multiply by parent
  mat4_mul(m, m, parent_transform);

  // scale
  mat4_scale(m, m, o->scale);

  // translate
  mat4 translation;
  mat4_translate(translation, o->position[0], o->position[1], o->position[2]);
  mat4_mul(m, m, translation);

  // compute rotation matrix from quaternion
  mat4 mat_rot;
  mat4_from_quat(mat_rot, o->rotation);

  // rotate around center
  mat4 t1;
  mat4_translate(t1, o->center[0], o->center[1], o->center[2]);
  mat4_mul(m, m, t1);
  mat4_mul(m, m, mat_rot);
  mat4 t2;
  mat4_translate(t2, -o->center[0], -o->center[1], -o->center[2]);
  mat4_mul(m, m, t2);

  // set world transform
  mat4_copy(o->world_transform, m);

  o->calculate_transform = 0;
}

void pass_light_uniform(int light_index, light* l, mat4 view, GLuint shader_id) {
  char uniform_light_pos[256];
  sprintf(uniform_light_pos, "lightsPos[%d]", light_index);
  char uniform_light_color[256];
  sprintf(uniform_light_color, "lightsColors[%d]", light_index);
  char uniform_light_type[256];
  sprintf(uniform_light_type, "lightsType[%d]", light_index);

  // light pos in view space
  vec4 light_pos;
  light_pos[0] = l->position[0];
  light_pos[1] = l->position[1];
  light_pos[2] = l->position[2];
  light_pos[3] = 1.0f;
  vec4 light_pos_view;
  mat4_mul_vec4(light_pos_view, view, light_pos);

  glUniform3fv(glGetUniformLocation(shader_id, uniform_light_pos), 1, (const GLfloat*) light_pos_view);
  glUniform3fv(glGetUniformLocation(shader_id, uniform_light_color), 1, (const GLfloat*) l->color);
  glUniform1i(glGetUniformLocation(shader_id, uniform_light_type), l->type);
}

void renderer_render_objects(object* objects[], int objects_length, light* sun, light* lights[], int lights_length, camera* camera, void (*ui_render_callback)(void), skybox* sky)
{
  GLint time;
  float ratio;
  int width, height;

  glfwGetFramebufferSize(window, &width, &height);

  // reset world transform calculations
  for (int i = 0; i < objects_length; i++) {
    objects[i]->calculate_transform = 1;
  }

  // calculate transforms
  for (int i = 0; i < objects_length; i++) {
    calculate_world_transform(objects[i]);
  }

  /*-------------------------------------------------------------------*/
  /*------------------------------shadows------------------------------*/
  /*-------------------------------------------------------------------*/
  glClearColor(183.0f / 255.0f, 220.0f / 255.0f, 244.0f / 255.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mat4 light_proj, light_view, light_space;
  mat4_ortho(light_proj, -renderer_shadow_size, renderer_shadow_size, -renderer_shadow_size, renderer_shadow_size, renderer_shadow_near, renderer_shadow_far);
  vec3 up = { 0.0f, 0.0f, 1.0f };

  vec3 sun_target;
  vec3_add(sun_target, sun->dir, camera->pos);

  // move sun with camera
  vec3 sun_cam_pos;
  sun_cam_pos[0] = camera->pos[0] + sun->position[0];
  sun_cam_pos[1] = sun->position[1];
  sun_cam_pos[2] = camera->pos[2] + sun->position[2];

  mat4_look_at(light_view, sun_cam_pos, sun_target, up);
  mat4_mul(light_space, light_proj, light_view);

  // render scene from light's point of view
  glUseProgram(renderer_shadow_shader);
  glUniformMatrix4fv(glGetUniformLocation(renderer_shadow_shader, "lightSpaceMatrix"), 1, GL_FALSE, (const GLfloat*) light_space);

  // reset viewport and clear color
  glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
  glBindFramebuffer(GL_FRAMEBUFFER, renderer_depth_fbo);
  glClear(GL_DEPTH_BUFFER_BIT);
  // glCullFace(GL_FRONT);
  render_objects(objects, objects_length, renderer_shadow_shader);
  // glCullFace(GL_BACK);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  /*-------------------------------------------------------------------------*/
  /*------------------------------geometry pass------------------------------*/
  /*-------------------------------------------------------------------------*/
  // 1. geometry pass: render scene's geometry/color data into gbuffer
  glBindFramebuffer(GL_FRAMEBUFFER, renderer_g_buffer);
  glUseProgram(renderer_geometry_shader);

  ratio = width / (float)height;
  glViewport(0, 0, width, height);
  glClearColor(183.0f / 255.0f, 220.0f / 255.0f, 244.0f / 255.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // compute mvp matrix
  mat4 v, p;
  vec3 camera_dir;
  vec3_add(camera_dir, camera->pos, camera->front);
  mat4_look_at(v, camera->pos, camera_dir, camera->up);
  mat4_perspective(p, to_radians(45.0f), ratio, 0.1f, 100.0f);

  // pass mvp to shader
  glUniformMatrix4fv(glGetUniformLocation(renderer_geometry_shader, "V"), 1, GL_FALSE, (const GLfloat*) v);
  glUniformMatrix4fv(glGetUniformLocation(renderer_geometry_shader, "P"), 1, GL_FALSE, (const GLfloat*) p);

  // pass time to shader
  glUniform1f(glGetUniformLocation(renderer_geometry_shader, "time"), (float)glfwGetTime());

  render_objects(objects, objects_length, renderer_geometry_shader);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  /*---------------------------------------------------------------------*/
  /*------------------------------ssao pass------------------------------*/
  /*---------------------------------------------------------------------*/
  if (renderer_ssao_enabled) {
    // generate ssao texture
    glBindFramebuffer(GL_FRAMEBUFFER, renderer_ssao_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(renderer_ssao_shader);

    // pass kernel + rotation
    for (int i = 0; i < SSAO_MAX_KERNEL_SIZE; i++) {
      char uniform_sample[256];
      sprintf(uniform_sample, "samples[%d]", i);
      glUniform3fv(glGetUniformLocation(renderer_ssao_shader, uniform_sample), 1, (const GLfloat*) renderer_ssao_kernel[i]);
    }

    glUniform1i(glGetUniformLocation(renderer_ssao_shader, "gPosition"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer_g_position);
    glUniform1i(glGetUniformLocation(renderer_ssao_shader, "gNormal"), 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, renderer_g_normal);
    glUniform1i(glGetUniformLocation(renderer_ssao_shader, "texNoise"), 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, renderer_ssao_noise_texture);

    glUniform1i(glGetUniformLocation(renderer_ssao_shader, "screenWidth"), width);
    glUniform1i(glGetUniformLocation(renderer_ssao_shader, "screenHeight"), height);
    glUniformMatrix4fv(glGetUniformLocation(renderer_ssao_shader, "projection"), 1, GL_FALSE, (const GLfloat*) p);

    render_quad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // blur ssao texture
    glBindFramebuffer(GL_FRAMEBUFFER, renderer_ssao_blur_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(renderer_ssao_blur_shader);
    glUniform1i(glGetUniformLocation(renderer_ssao_blur_shader, "texture_blur"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer_ssao_color);
    render_quad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  /*-------------------------------------------------------------------------*/
  /*------------------------------lighting pass------------------------------*/
  /*-------------------------------------------------------------------------*/
  // 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using gbuffer
  glBindFramebuffer(GL_FRAMEBUFFER, renderer_post_fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(renderer_lighting_shader);
  glUniform1i(glGetUniformLocation(renderer_lighting_shader, "gPosition"), 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, renderer_g_position);
  glUniform1i(glGetUniformLocation(renderer_lighting_shader, "gNormal"), 1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, renderer_g_normal);
  glUniform1i(glGetUniformLocation(renderer_lighting_shader, "gAlbedo"), 2);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, renderer_g_albedo);
  glUniform1i(glGetUniformLocation(renderer_lighting_shader, "gSpec"), 3);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, renderer_g_spec);

  // camera position
  GLint uniform_camera_pos = glGetUniformLocation(renderer_lighting_shader, "cameraPos");
  glUniform3fv(uniform_camera_pos, 1, (const GLfloat*) camera->pos);

  // pass sun as light
  pass_light_uniform(0, sun, v, renderer_lighting_shader);

  // lights
  glUniform1i(glGetUniformLocation(renderer_lighting_shader, "lightsNr"), lights_length + 1);
  for (int i = 1; i < lights_length; i++) {
    pass_light_uniform(i, lights[i], v, renderer_lighting_shader);
  }

  // shadow map to shader
  glUniform1f(glGetUniformLocation(renderer_lighting_shader, "shadowBias"), renderer_shadow_bias);
  glUniform1i(glGetUniformLocation(renderer_lighting_shader, "shadowPCFEnabled"), renderer_shadow_pcf_enabled);

  // pass inverse of view matrix (for shadows)
  mat4 view_inv;
  mat4_invert(view_inv, v);
  glUniformMatrix4fv(glGetUniformLocation(renderer_lighting_shader, "viewInv"), 1, GL_FALSE, (const GLfloat*) view_inv);

  // pass depth map
  glUniform1i(glGetUniformLocation(renderer_lighting_shader, "shadowMap"), 4);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, renderer_depth_map);

  // skybox to shader
  if (sky) {
    glUniform1i(glGetUniformLocation(renderer_lighting_shader, "skybox"), 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, sky->texture_id);
  }

  // pass light-space matrix to shader
  glUniformMatrix4fv(glGetUniformLocation(renderer_lighting_shader, "lightSpaceMatrix"), 1, GL_FALSE, (const GLfloat*) light_space);

  // pass time to shader
  glUniform1f(glGetUniformLocation(renderer_lighting_shader, "time"), (float)glfwGetTime());

  // pass ssao texture to shader
  glUniform1i(glGetUniformLocation(renderer_lighting_shader, "ssao"), 6);
  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_2D, renderer_ssao_blur);

  // ssao uniforms
  glUniform1i(glGetUniformLocation(renderer_lighting_shader, "ssao_enabled"), renderer_ssao_enabled);
  glUniform1i(glGetUniformLocation(renderer_lighting_shader, "ssao_debug"), renderer_ssao_debug_on);

  render_quad();
  /*-------------------------------------------------------------------------*/
  /*------------------------------fxaa pass------------------------------*/
  /*-------------------------------------------------------------------------*/
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(renderer_post_shader);

  // pass window size
  glUniform1i(glGetUniformLocation(renderer_post_shader, "fxaa_enabled"), renderer_fxaa_enabled);
  glUniform1i(glGetUniformLocation(renderer_post_shader, "width"), width);
  glUniform1i(glGetUniformLocation(renderer_post_shader, "height"), height);

  glUniform1i(glGetUniformLocation(renderer_post_shader, "frame"), 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, renderer_post_texture);

  render_quad();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  /*----------------------------------------------------------------------------------------------------*/
  /*-----------------------------blit gbuffer depth to default framebuffer------------------------------*/
  /*----------------------------------------------------------------------------------------------------*/
  glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer_g_buffer);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
  glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  /*-----------------------------------------------------------------*/
  /*-----------------------------skybox------------------------------*/
  /*-----------------------------------------------------------------*/
  if (sky) {
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    glUseProgram(renderer_skybox_shader);
    glUniform1i(glGetUniformLocation(sky->texture_id, "skybox"), 0);
    v[3][0] = 0;
    v[3][1] = 0;
    v[3][2] = 0;
    v[3][3] = 0;
    glUniformMatrix4fv(glGetUniformLocation(renderer_skybox_shader, "view"), 1, GL_FALSE, (const GLfloat*) v);
    glUniformMatrix4fv(glGetUniformLocation(renderer_skybox_shader, "projection"), 1, GL_FALSE, (const GLfloat*) p);

    // skybox cube
    glBindVertexArray(sky->vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, sky->texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default
  }
  /*-----------------------------------------------------------------*/
  /*------------------------------debug------------------------------*/
  /*-----------------------------------------------------------------*/
  glUseProgram(renderer_debug_shader);
  glUniform1f(glGetUniformLocation(renderer_debug_shader, "near_plane"), renderer_shadow_near);
  glUniform1f(glGetUniformLocation(renderer_debug_shader, "far_plane"), renderer_shadow_far);
  glUniform1i(glGetUniformLocation(renderer_debug_shader, "depthMap"), 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, renderer_depth_map);
  if (renderer_shadows_debug_enabled) render_quad();

  // ui callback
  if (ui_render_callback != NULL) {
    ui_render_callback();
  }

  // reset opengl state
  set_opengl_state();

  // swap buffers and poll events
  glfwSwapBuffers(window);

  // poll events
  glfwPollEvents();
}

ray renderer_raycast(camera* camera, float x, float y, float ray_len) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  float ratio = width / (float)height;

  // normalised device coordinates [-1:1, -1:1, -1:1]
  vec3 ray_nds = { (2*x)/width - 1, 1 - (2*y) / height, 1 };

  // 4D homogeneous clip coordinates [-1:1, -1:1, -1:1, -1:1]
  vec4 ray_clip = { ray_nds[0], ray_nds[1], -1, 1 };

  // 4D eye (camera) coordinates [-x:x, -y:y, -z:z, -w:w]
  mat4 p, p_inv;
  mat4_perspective(p, to_radians(45.0f), ratio, 0.1f, 100.0f);
  mat4_invert(p_inv, p);

  vec4 ray_eye;
  mat4_mul_vec4(ray_eye, p_inv, ray_clip);
  ray_eye[2] = -1;
  ray_eye[3] = 0;

  // 4D world coordinates [-x:x, -y:y, -z:z, -w:w]
  mat4 v, v_inv;
  vec3 camera_dir;
  vec3_add(camera_dir, camera->pos, camera->front);
  mat4_look_at(v, camera->pos, camera_dir, camera->up);
  mat4_invert(v_inv, v);
  
  vec4 ray_wor;
  mat4_mul_vec4(ray_wor, v_inv, ray_eye);
  vec4_norm(ray_wor, ray_wor);

  // build ray
  ray r;
  vec3_copy(r.o, camera->pos);
  vec3_copy(r.dir, ray_wor);
  r.length = ray_len;

  return r;
}
