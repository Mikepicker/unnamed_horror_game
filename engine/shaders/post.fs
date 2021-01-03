#version 330 core

#define LUMA_THRESHOLD 0.5
#define MUL_REDUCE (1 / 8.0)
#define MIN_REDUCE (1 / 128.0)
#define MAX_SPAN 8
#define GAMMA 2.2

in vec2 TexCoords;
out vec3 FragColor;

uniform sampler2D frame;

uniform int width;
uniform int height;

uniform int fxaa_enabled;

vec3 fxaa() {
  vec2 u_texelStep = vec2(1 / width, 1 / height);

  vec3 rgb_m = texture(frame, TexCoords).rgb;

  // Sampling neighbour texels. Offsets are adapted to OpenGL texture coordinates. 
  vec3 rgb_nw = textureOffset(frame, TexCoords, ivec2(-1, 1)).rgb;
  vec3 rgb_ne = textureOffset(frame, TexCoords, ivec2(1, 1)).rgb;
  vec3 rgb_sw = textureOffset(frame, TexCoords, ivec2(-1, -1)).rgb;
  vec3 rgb_se = textureOffset(frame, TexCoords, ivec2(1, -1)).rgb;

  // see http://en.wikipedia.org/wiki/Grayscale
  const vec3 to_luma = vec3(0.299, 0.587, 0.114);

  // Convert from RGB to luma.
  float luma_nw = dot(rgb_nw, to_luma);
  float luma_ne = dot(rgb_ne, to_luma);
  float luma_sw = dot(rgb_sw, to_luma);
  float luma_se = dot(rgb_se, to_luma);
  float luma_m = dot(rgb_m, to_luma);

  // Gather minimum and maximum luma.
  float luma_min = min(luma_m, min(min(luma_nw, luma_ne), min(luma_sw, luma_se)));
  float luma_max = max(luma_m, max(max(luma_nw, luma_ne), max(luma_sw, luma_se)));

  // If contrast is lower than a maximum threshold ...
  if (luma_max - luma_min <= luma_max * LUMA_THRESHOLD) {
    // ... do no AA and return.
    return rgb_m;
  }

  // Sampling is done along the gradient.
  vec2 sampling_direction;	
  sampling_direction.x = -((luma_nw + luma_ne) - (luma_sw + luma_se));
  sampling_direction.y =  ((luma_nw + luma_sw) - (luma_ne + luma_se));

  // Sampling step distance depends on the luma: The brighter the sampled texels, the smaller the final sampling step direction.
  // This results, that brighter areas are less blurred/more sharper than dark areas.  
  float sampling_direction_reduce = max((luma_nw + luma_ne + luma_sw + luma_se) * 0.25 * MUL_REDUCE, MIN_REDUCE);

  // Factor for norming the sampling direction plus adding the brightness influence. 
  float min_sampling_direction_factor = 1.0 / (min(abs(sampling_direction.x), abs(sampling_direction.y)) + sampling_direction_reduce);

  // Calculate final sampling direction vector by reducing, clamping to a range and finally adapting to the texture size. 
  sampling_direction = clamp(sampling_direction * min_sampling_direction_factor, vec2(-MAX_SPAN), vec2(MAX_SPAN)) * u_texelStep;

  // Inner samples on the tab.
  vec3 rgb_sample_neg = texture(frame, TexCoords + sampling_direction * (1.0/3.0 - 0.5)).rgb;
  vec3 rgb_sample_pos = texture(frame, TexCoords + sampling_direction * (2.0/3.0 - 0.5)).rgb;

  vec3 rgb_two_tab = (rgb_sample_pos + rgb_sample_neg) * 0.5;  

  // Outer samples on the tab.
  vec3 rgb_sample_neg_outer = texture(frame, TexCoords + sampling_direction * (0.0/3.0 - 0.5)).rgb;
  vec3 rgb_sample_pos_outer = texture(frame, TexCoords + sampling_direction * (3.0/3.0 - 0.5)).rgb;

  vec3 rgb_four_tab = (rgb_sample_pos_outer + rgb_sample_neg_outer) * 0.25 + rgb_two_tab * 0.5;   

  // Calculate luma for checking against the minimum and maximum value.
  float luma_four_tab = dot(rgb_four_tab, to_luma);

  // Are outer samples of the tab beyond the edge ... 
  if (luma_four_tab < luma_min || luma_four_tab > luma_max) {
    // ... yes, so use only two samples.
    // rgb_two_tab.r = 1.0;
    return rgb_two_tab; 
  }
  else {
    // ... no, so use four samples. 
    // rgb_four_tab.r = 1.0;
    return rgb_four_tab;
  }
}

vec3 uncharted2_tonemap(vec3 x) {
	const float A = 0.15;
	const float B = 0.50;
	const float C = 0.10;
	const float D = 0.20;
	const float E = 0.02;
	const float F = 0.30;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 tonemap_filmic(vec3 color) {
	vec3 x = max(vec3(0.0), color - 0.004);
	return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}

vec3 aces_film_tonemap(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d ) + e), 0.0, 1.0);
}

vec3 reinhard_tonemap(vec3 x) {
  return x / (x + vec3(1.0));
  // return vec3(1.0) - exp(-x);
}

void main() {
  FragColor = texture(frame, TexCoords).rgb;

  // Toggle FXAA on and off.
  if (fxaa_enabled == 1) {
    FragColor = fxaa(); 
  }

  // Tonemap
  // FragColor = aces_film_tonemap(FragColor);
  // FragColor = tonemap_filmic(FragColor);
  
  // Gamma correction
  // FragColor = pow(FragColor, vec3(1.0 / GAMMA));
}
