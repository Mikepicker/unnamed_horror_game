#version 330 core

#define MAX_BONES 128

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUvs;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aJointIds;
layout (location = 5) in vec3 aWeights;

uniform mat4 bone_transforms[MAX_BONES];
uniform int has_skeleton;

uniform mat4 M;

mat4 bone_transform() {
  return aWeights.x * bone_transforms[int(aJointIds.x)]
       + aWeights.y * bone_transforms[int(aJointIds.y)]
       + aWeights.z * bone_transforms[int(aJointIds.z)];
}

void main() {
  mat4 bt = has_skeleton == 1 ? bone_transform() : mat4(1.0);
  vec3 FragPos = vec3(M * bt * vec4(aPos, 1.0));

  gl_Position = vec4(FragPos, 1.0);
}  
