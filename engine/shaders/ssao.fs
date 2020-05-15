#version 330 core
#define SSAO_MAX_KERNEL_SIZE 64
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform int screenWidth;
uniform int screenHeight;

uniform vec3 samples[SSAO_MAX_KERNEL_SIZE];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
float radius = 1.5;
float bias = 0.0025;

uniform mat4 projection;

void main() {
  // tile noise texture over screen based on screen dimensions divided by noise size
  vec2 noiseScale = vec2(screenWidth/4.0, screenHeight/4.0); 
  // noiseScale = vec2(1, 1);

  // get input for SSAO algorithm
  vec3 fragPos = texture(gPosition, TexCoords).xyz;
  vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
  vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
  // create TBN change-of-basis matrix: from tangent-space to view-space
  vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
  vec3 bitangent = cross(normal, tangent);
  mat3 TBN = mat3(tangent, bitangent, normal);
  // iterate over the sample kernel and calculate occlusion factor
  float occlusion = 0.0;
  for(int i = 0; i < SSAO_MAX_KERNEL_SIZE; i++) {
    // get sample position
    vec3 sample = TBN * samples[i]; // from tangent to view-space
    sample = fragPos + sample * radius; 

    // project sample position (to sample texture) (to get position on screen/texture)
    vec4 offset = vec4(sample, 1.0);
    offset = projection * offset; // from view to clip-space
    offset.xyz /= offset.w; // perspective divide
    offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

    // get sample depth
    float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel sample

    // range check & accumulate
    float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
    occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;           

  }
  occlusion = 1.0 - (occlusion / SSAO_MAX_KERNEL_SIZE);

  FragColor = occlusion;
  // FragColor = normal.z;
  // FragColor = texture(texNoise, TexCoords * noiseScale).x;
  // FragColor = texture(texNoise, TexCoords * noiseScale).x;
  // FragColor = fragPos.z;
}
