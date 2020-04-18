#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform sampler2D texture_specular;

uniform int hasNormalMap;
uniform int hasSpecularMap;

uniform int texture_subdivision;

// material
struct Material {
  vec3 diffuse;
  float specular;
  float reflectivity; // TODO: use it somehow
}; 
uniform Material material;

vec3 computeNormal()
{
  // obtain normal from normal map in range [0,1]
  vec3 normal = texture(texture_normal, TexCoords * texture_subdivision).rgb;

  // transform normal vector to range [-1,1]
  normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

  return TBN * normal;
  // return normal;
}

void main() {    
  // store the fragment position vector in the first gbuffer texture
  gPosition = FragPos;
  // also store the per-fragment normals into the gbuffer
  gNormal = hasNormalMap == 1 ? computeNormal() : normalize(Normal);
  // and the diffuse per-fragment color
  gAlbedoSpec.rgb = texture(texture_diffuse, TexCoords * texture_subdivision).rgb;
  gAlbedoSpec.a = material.specular;

  if (hasSpecularMap > 0) {
    gAlbedoSpec.a *= texture(texture_specular, TexCoords * texture_subdivision).r;
  }

}
