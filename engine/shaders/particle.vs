#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 tex_coords>
layout (location = 1) in vec4 pos_size; // <vec3 position, float size>
layout (location = 2) in vec4 color_alpha; // <vec3 color, float alpha>

uniform mat4 view;
uniform mat4 projection;

out vec2 tex_coords;
out vec4 view_pos;
out vec4 particle_color;

void main() {
  tex_coords = vertex.zw;

  view_pos = view * vec4(pos_size.xyz, 1.0);
  view_pos.xy += pos_size.w * (vertex.xy - vec2(0.5));

  particle_color = color_alpha;
  gl_Position = projection * view_pos;
}
