#version 330 core
in vec2 tex_coords;
in vec4 particle_color;

out vec4 color;

uniform mat4 view;
uniform int has_sprite;
uniform sampler2D sprite;

void main() {
  if (has_sprite > 0) {
    color = texture(sprite, tex_coords) * particle_color;
    color.a = particle_color.a;
  } else {
    color = particle_color;
  }
}  
