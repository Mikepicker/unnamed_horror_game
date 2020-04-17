#version 330 core

#define MAX_BONES 128

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUvs;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aJointIds;
layout (location = 5) in vec3 aWeights;

uniform mat4 boneTransforms[MAX_BONES];
uniform int hasSkeleton;

uniform mat4 lightSpaceMatrix;
uniform mat4 M;

out vec2 Uvs;

mat4 boneTransform() {
  return aWeights.x * boneTransforms[int(aJointIds.x)]
       + aWeights.y * boneTransforms[int(aJointIds.y)]
       + aWeights.z * boneTransforms[int(aJointIds.z)];
}

void main() {
  mat4 bt = hasSkeleton == 1 ? boneTransform() : mat4(1.0);
  vec3 FragPos = vec3(M * bt * vec4(aPos, 1.0));
  Uvs = aUvs.st;
  gl_Position = lightSpaceMatrix * vec4(FragPos, 1.0);
}  
