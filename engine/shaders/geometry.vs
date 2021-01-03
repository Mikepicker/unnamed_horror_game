#version 330 core
#define NR_LIGHTS 4
#define MAX_BONES 128

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUvs;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aJointIds;
layout (location = 5) in vec3 aWeights;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out vec4 FragPosLightSpace;

out mat3 TBN;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

uniform int has_normal_map;

uniform mat4 bone_transforms[MAX_BONES];
uniform int has_skeleton;

mat4 bone_transform() {
  return aWeights.x * bone_transforms[int(aJointIds.x)]
       + aWeights.y * bone_transforms[int(aJointIds.y)]
       + aWeights.z * bone_transforms[int(aJointIds.z)];
}

void main()
{
  mat4 bt = has_skeleton == 1 ? bone_transform() : mat4(1.0);

  vec4 view_pos = V * vec4(vec3(M * bt * vec4(aPos, 1.0)), 1.0);

  TexCoords = aUvs.st;

  mat3 normal_matrix = transpose(inverse(mat3(V * M * bt)));
  Normal = normal_matrix * (vec4(aNormal, 1.0)).xyz;

  // normal map
  if (has_normal_map > 0) {
    vec3 T = normalize(normal_matrix * aTangent);
    vec3 N = normalize(normal_matrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
  }

  FragPos = vec3(view_pos);
  gl_Position = P * view_pos;
}
