#include "factory.h"

object* factory_create_sphere(float radius, int sector_count, int stack_count) {
  int num_vertices = sector_count * stack_count;

  vertex* vertices = (vertex*)malloc(3 * num_vertices * sizeof(vertex));
  int vcount = 0;

  float x, y, z, xy;
  float nx, ny, nz, lengthInv = 1.0f / radius;
  float s, t;

  float sector_step = 2 * M_PI / sector_count;
  float stack_step = M_PI / stack_count;
  float sector_angle, stack_angle;

  // generate vertices
  for (int i = 0; i <= stack_count; i++) {
    stack_angle = M_PI / 2 - i * stack_step;  // starting from pi/2 to -pi/2
    xy = radius * cosf(stack_angle);          // r * cos(u)
    z = radius * sinf(stack_angle);           // r * sin(u)

    for (int j = 0; j <= sector_count; j++) {
      sector_angle = j * sector_step;         // starting from 0 to 2pi

      // vertex position (x, y, z)
      x = xy * cosf(sector_angle);
      y = xy * sinf(sector_angle);
      vertices[vcount].x = x;
      vertices[vcount].y = y;
      vertices[vcount].z = z;

      // normalized vertex normal (nx, ny, nz)
      nx = x * lengthInv;
      ny = y * lengthInv;
      nz = z * lengthInv;
      vertices[vcount].nx = nx;
      vertices[vcount].ny = ny;
      vertices[vcount].nz = nz;

      // vertex tex coord (s, t) range between [0, 1]
      s = (float)j / sector_count;
      t = (float)i / stack_count;
      vertices[vcount].u = s;
      vertices[vcount].v = t;
      vcount++;
    }
  }

  // generate indices
  GLuint* indices = (GLuint*)malloc(6 * num_vertices * sizeof(GLuint));
  int icount = 0;
  int k1, k2;
  for (int i = 0; i < stack_count; i++) {
    k1 = i * (sector_count + 1);  // beginning of current stack
    k2 = k1 + sector_count + 1;   // beginning of next stack

    for (int j = 0; j < sector_count; j++, k1++, k2++) {
      // 2 triangles per sector excluding first and last stacks
      // k1 => k2 => k1+1
      if (i != 0) {
        indices[icount++] = k1;
        indices[icount++] = k2;
        indices[icount++] = k1 + 1;
      }

      // k1+1 => k2 => k2+1
      if (i != (stack_count - 1)) {
        indices[icount++] = k1 + 1;
        indices[icount++] = k2;
        indices[icount++] = k2 + 1;
      }
    }
  }

  // prepare and return object
  mesh* m = (mesh*)malloc(sizeof(mesh));
  m->vertices = vertices;
  m->num_vertices = vcount;
  m->indices = indices;
  m->num_indices = icount;

  object* obj = object_create(NULL, 1.0f, m, 1, 1, NULL);
  return obj;
}

object* factory_create_plane(float width, float height) {
  vertex* vertices = (vertex*)malloc(4 * sizeof(vertex));
  vertex v1 = { -width/2, 0, -height/2, 0, 0, 0, 1, 0 };
  vertex v2 = {  width/2, 0, -height/2, 0, 1, 0, 1, 0 };
  vertex v3 = {  width/2, 0,  height/2, 1, 1, 0, 1, 0 };
  vertex v4 = { -width/2, 0,  height/2, 1, 0, 0, 1, 0 };
  vertices[0] = v1;
  vertices[1] = v2;
  vertices[2] = v3;
  vertices[3] = v4;

  GLuint* indices = (GLuint*)malloc(6 * sizeof(GLuint));
  indices[0] = 0;
  indices[1] = 3;
  indices[2] = 1;
  indices[3] = 3;
  indices[4] = 2;
  indices[5] = 1;

  mesh* m = (mesh*)malloc(sizeof(mesh));
  m->vertices = vertices;
  m->num_vertices = 4;
  m->indices = indices;
  m->num_indices = 6;

  object* obj = object_create(NULL, 1.0f, m, 1, 1, NULL);
  return obj;
}

object* factory_create_box(float width, float height, float depth) {
  mesh* m = (mesh*)malloc(sizeof(mesh));
  m->num_vertices = 6 * 4;
  m->num_indices = 6 * 6;

  vertex* vertices = (vertex*)malloc(m->num_vertices * sizeof(vertex));

  // Front
  vertex v1 = { -width/2, -height/2, depth/2, 0, 0, 0, 0, 1 };
  vertex v2 = {  width/2, -height/2, depth/2, 0, 1, 0, 0, 1 };
  vertex v3 = {  width/2,  height/2, depth/2, 1, 1, 0, 0, 1 };
  vertex v4 = { -width/2,  height/2, depth/2, 1, 0, 0, 0, 1 };

  vertices[0] = v1;
  vertices[1] = v2;
  vertices[2] = v3;
  vertices[3] = v4;

  // Back
  vertex v5 = { -width/2, -height/2, -depth/2, 0, 0, 0, 0, -1 };
  vertex v6 = {  width/2, -height/2, -depth/2, 0, 1, 0, 0, -1 };
  vertex v7 = {  width/2,  height/2, -depth/2, 1, 1, 0, 0, -1 };
  vertex v8 = { -width/2,  height/2, -depth/2, 1, 0, 0, 0, -1 };

  vertices[4] = v5;
  vertices[5] = v6;
  vertices[6] = v7;
  vertices[7] = v8;

  // Right
  vertex v9  = { width/2,  height/2,  depth/2, 0, 0, 1, 0, 0 };
  vertex v10 = { width/2,  height/2, -depth/2, 0, 1, 1, 0, 0 };
  vertex v11 = { width/2, -height/2, -depth/2, 1, 1, 1, 0, 0 };
  vertex v12 = { width/2, -height/2,  depth/2, 1, 0, 1, 0, 0 };

  vertices[8] = v9;
  vertices[9] = v10;
  vertices[10] = v11;
  vertices[11] = v12;

  // Left
  vertex v13 = { -width/2, -height/2,  depth/2, 0, 0, -1, 0, 0 };
  vertex v14 = { -width/2, -height/2, -depth/2, 0, 1, -1, 0, 0 };
  vertex v15 = { -width/2,  height/2, -depth/2, 1, 1, -1, 0, 0 };
  vertex v16 = { -width/2,  height/2,  depth/2, 1, 0, -1, 0, 0 };

  vertices[12] = v13;
  vertices[13] = v14;
  vertices[14] = v15;
  vertices[15] = v16;

  // Top
  vertex v17 = { -width/2, height/2,  depth/2, 0, 0, 0, 1, 0 };
  vertex v18 = { -width/2, height/2, -depth/2, 0, 1, 0, 1, 0 };
  vertex v19 = {  width/2, height/2, -depth/2, 1, 1, 0, 1, 0 };
  vertex v20 = {  width/2, height/2,  depth/2, 1, 0, 0, 1, 0 };

  vertices[16] = v17;
  vertices[17] = v18;
  vertices[18] = v19;
  vertices[19] = v20;

  // Bottom
  vertex v21 = { -width/2, -height/2,  depth/2, 0, 0, 0, -1, 0 };
  vertex v22 = { -width/2, -height/2, -depth/2, 0, 1, 0, -1, 0 };
  vertex v23 = {  width/2, -height/2, -depth/2, 1, 1, 0, -1, 0 };
  vertex v24 = {  width/2, -height/2,  depth/2, 1, 0, 0, -1, 0 };

  vertices[20] = v21;
  vertices[21] = v22;
  vertices[22] = v23;
  vertices[23] = v24;

  GLuint* indices = (GLuint*)malloc(m->num_indices * sizeof(GLuint));

  // Front
  indices[0] = 0;
  indices[1] = 1;
  indices[2] = 3;
  indices[3] = 1;
  indices[4] = 2;
  indices[5] = 3;

  // Back
  indices[6] = 4;
  indices[7] = 7;
  indices[8] = 5;
  indices[9] = 7;
  indices[10] = 6;
  indices[11] = 5;

  // Right
  indices[12] = 9;
  indices[13] = 8;
  indices[14] = 11;
  indices[15] = 11;
  indices[16] = 10;
  indices[17] = 9;

  // Left
  indices[18] = 13;
  indices[19] = 12;
  indices[20] = 14;
  indices[21] = 12;
  indices[22] = 15;
  indices[23] = 14;

  // Top
  indices[24] = 17;
  indices[25] = 16;
  indices[26] = 18;
  indices[27] = 16;
  indices[28] = 19;
  indices[29] = 18;

  // Bottom
  indices[30] = 20;
  indices[31] = 21;
  indices[32] = 23;
  indices[33] = 21;
  indices[34] = 22;
  indices[35] = 23;

  m->vertices = vertices;
  m->indices = indices;

  object* obj = object_create(NULL, 1.0f, m, 1, 1, NULL);
  return obj;
}
