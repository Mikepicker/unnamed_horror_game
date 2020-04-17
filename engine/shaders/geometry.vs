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

out mat3 TBN_inv;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

uniform vec3 lightsPos[NR_LIGHTS]; 
uniform int hasNormalMap;

uniform mat4 boneTransforms[MAX_BONES];
uniform int hasSkeleton;

mat4 boneTransform() {
  return aWeights.x * boneTransforms[int(aJointIds.x)]
       + aWeights.y * boneTransforms[int(aJointIds.y)]
       + aWeights.z * boneTransforms[int(aJointIds.z)];
}

void main()
{
  mat4 bt = hasSkeleton == 1 ? boneTransform() : mat4(1.0);
  FragPos = vec3(M * bt * vec4(aPos, 1.0));
  Normal = mat3(transpose(inverse(M))) * (bt * vec4(aNormal, 1.0)).xyz;

  TexCoords = aUvs.st;

  // normal map
  if (hasNormalMap > 0) {
    mat3 normalMatrix = transpose(inverse(mat3(M)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = Normal;
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 TBN = transpose(mat3(T, B, N));    
    TBN_inv = inverse(TBN);
  }

  gl_Position = P * V * vec4(FragPos, 1.0);
}
