#version 330 core

in vec2 Uvs;

// mask map
uniform sampler2D texture_diffuse;

void main() {             
  // mask map
  vec4 alpha = texture(texture_diffuse, Uvs).rgba;
  if (alpha.a < 0.1) {
    discard;
  }

  gl_FragDepth = gl_FragCoord.z;
} 
