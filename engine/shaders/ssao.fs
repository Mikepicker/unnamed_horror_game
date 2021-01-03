#version 330 core
#define SSAO_MAX_KERNEL_SIZE 64
out float FragColor;

in vec2 TexCoords;

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D tex_noise;

uniform int screen_width;
uniform int screen_height;

uniform vec3 samples[SSAO_MAX_KERNEL_SIZE];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
float radius = 1.5;
float bias = 0.0025;

uniform mat4 projection;

void main() {
  // tile noise texture over screen based on screen dimensions divided by noise size
  vec2 noise_scale = vec2(screen_width/4.0, screen_height/4.0); 
  // noise_scale = vec2(1, 1);

  // get input for SSAO algorithm
  vec3 frag_pos = texture(g_position, TexCoords).xyz;
  vec3 normal = normalize(texture(g_normal, TexCoords).rgb);
  vec3 random_vec = normalize(texture(tex_noise, TexCoords * noise_scale).xyz);
  // create TBN change-of-basis matrix: from tangent-space to view-space
  vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
  vec3 bitangent = cross(normal, tangent);
  mat3 TBN = mat3(tangent, bitangent, normal);
  // iterate over the sample kernel and calculate occlusion factor
  float occlusion = 0.0;
  for(int i = 0; i < SSAO_MAX_KERNEL_SIZE; i++) {
    // get sample position
    vec3 sample = TBN * samples[i]; // from tangent to view-space
    sample = frag_pos + sample * radius; 

    // project sample position (to sample texture) (to get position on screen/texture)
    vec4 offset = vec4(sample, 1.0);
    offset = projection * offset; // from view to clip-space
    offset.xyz /= offset.w; // perspective divide
    offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

    // get sample depth
    float sample_depth = texture(g_position, offset.xy).z; // get depth value of kernel sample

    // range check & accumulate
    float range_check = smoothstep(0.0, 1.0, radius / abs(frag_pos.z - sample_depth));
    occlusion += (sample_depth >= sample.z + bias ? 1.0 : 0.0) * range_check;           

  }
  occlusion = 1.0 - (occlusion / SSAO_MAX_KERNEL_SIZE);

  FragColor = occlusion;
  // FragColor = normal.z;
  // FragColor = texture(tex_noise, TexCoords * noise_scale).x;
  // FragColor = texture(tex_noise, TexCoords * noise_scale).x;
  // FragColor = frag_pos.z;
}
