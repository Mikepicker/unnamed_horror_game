#version 330 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out float gSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform sampler2D texture_specular;

uniform int has_diffuse_map;
uniform int has_normal_map;
uniform int has_specular_map;

uniform int texture_subdivision;

uniform int receive_shadows;

// material
struct Material {
  vec3 diffuse;
  float specular;
  float reflectivity; // TODO: use it somehow
}; 
uniform Material material;

vec3 compute_normal()
{
  // obtain normal from normal map in range [0,1]
  vec3 normal = texture(texture_normal, TexCoords * texture_subdivision).rgb;

  // transform normal vector to range [-1,1]
  normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

  return TBN * normal;
}

void main() {    
  // store the fragment position vector in the first gbuffer texture
  gPosition.xyz = FragPos.xyz;

  // store receive_shadow
  gPosition.a = receive_shadows;

  // also store the per-fragment normals into the gbuffer
  gNormal = has_normal_map == 1 ? compute_normal() : normalize(Normal);
  // and the diffuse per-fragment color
  gAlbedo = has_diffuse_map == 1 ? texture(texture_diffuse, TexCoords * texture_subdivision).rgba : vec4(material.diffuse.rgb, 1.0);
  gAlbedo.rgb *= material.diffuse;

  if (gAlbedo.a < 0.1)
    discard;

  gSpec = material.specular;
  if (has_specular_map > 0) {
    gSpec *= texture(texture_specular, TexCoords * texture_subdivision).r;
  }

}
