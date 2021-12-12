#version 330 core
#define MAX_LIGHTS 4
#define MAX_OMNI_SHADOWS 4

out vec4 FragColor;

in vec2 TexCoords;

// gbuffer
uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_albedo;
uniform sampler2D g_spec;
uniform sampler2D ssao;

// lights
struct Light {
  int type;
  vec3 position;
  vec3 color;
  vec3 dir;
  float ambient;
  float constant;
  float linear;
  float quadratic;
  int cast_shadows;
  mat4 light_space_matrix;
};

uniform Light lights[MAX_LIGHTS];
uniform int lights_nr;

// camera
uniform vec3 camera_pos; 

// shadow map
uniform sampler2D shadow_map;
uniform float shadow_bias;
uniform int shadow_pcf_enabled;

// omni shadow map
uniform samplerCube omni_shadow_map_0;
uniform samplerCube omni_shadow_map_1;
uniform samplerCube omni_shadow_map_2;
uniform samplerCube omni_shadow_map_3;
uniform float omni_shadow_far_plane;

// render params
uniform vec3 color_mask;
uniform int glowing;
uniform vec3 glow_color;

// inverse of view matrix
uniform mat4 view_inv;

// ssao uniforms
uniform int ssao_enabled;
uniform int ssao_debug;

// samples for omni-directional shadows
vec3 sample_offset_directions[20] = vec3[] (
  vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
  vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
  vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
  vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
  vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float shadow_calculation(vec4 frag_pos_light_space, vec3 light_dir, vec3 normal) {
  // perform perspective divide
  vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
  // transform to [0,1] range
  proj_coords = proj_coords * 0.5 + 0.5;
  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closest_depth = texture(shadow_map, proj_coords.xy).r; 
  // get depth of current fragment from light's perspective
  float current_depth = proj_coords.z;
  // check whether current frag pos is in shadow
  float bias = max(0.02 * (1.0 - dot(normal, light_dir)), 0.01);

  float shadow = 0.0;
  
  if (shadow_pcf_enabled == 1) {
    vec2 texelSize = 1.0 / textureSize(shadow_map, 0);
    for (int x = -1; x <= 1; x++) {
      for (int y = -1; y <= 1; y++) {
        float pcf_depth = texture(shadow_map, proj_coords.xy + vec2(x, y) * texelSize).r;
        shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
      }
    }
    shadow /= 9.0;
  } else {
    shadow = current_depth - bias > closest_depth ? 1.0 : 0.0;
  }

  if (proj_coords.z > 1.0)
    shadow = 0.0;

  return shadow;
}

float omni_shadow_calculation(samplerCube shadow_cube, vec3 frag_pos_world_space, vec3 light_pos_world_space) {
  vec3 frag_to_light = frag_pos_world_space - light_pos_world_space;

  float closest_depth = texture(shadow_cube, frag_to_light).r;
  closest_depth *= omni_shadow_far_plane;  

  float current_depth = length(frag_to_light);  

  float shadow = 0.0;
  float bias   = 0.15;
  int samples  = 20;
  float view_distance = length(camera_pos - frag_pos_world_space);
  float disk_radius = (1.0 + (view_distance / omni_shadow_far_plane)) / 25.0;
  for (int i = 0; i < samples; i++) {
    float closest_depth = texture(shadow_cube, frag_to_light + sample_offset_directions[i] * disk_radius).r;
    closest_depth *= omni_shadow_far_plane;   // undo mapping [0;1]
    if (current_depth - bias > closest_depth) shadow += 1.0;
  }

  shadow /= float(samples); 
  return shadow;
}

vec3 calc_dir_light(Light l, vec3 diffuse, float specular, vec3 normal, float ao, vec3 view_dir, vec4 frag_pos_light_space, float receive_shadows) {
  vec3 light_dir = normalize(-l.dir);
  
  // ambient
  vec3 l_ambient = vec3(l.ambient * diffuse * ao);

  // diffuse
  vec3 l_diffuse = max(dot(normal, light_dir), 0.0) * diffuse * l.color;

  // specular
  vec3 halfway_dir = normalize(light_dir + view_dir);  
  float spec = pow(max(dot(normal, halfway_dir), 0.0), 8.0);
  vec3 l_specular = specular * l.color * spec;

  float shadow = 0.0;
  if (l.cast_shadows == 1 && receive_shadows > 0) {
    shadow = shadow_calculation(frag_pos_light_space, light_dir, normal);
  }

  return l_ambient + (1.0 - shadow) * (l_diffuse + l_specular);
}

vec3 calc_point_light(Light l, int shadow_map_index, vec3 diffuse, float specular, vec3 normal, float ao, vec3 view_dir, vec3 frag_pos, vec3 frag_pos_world_space, float receive_shadows) {
  vec3 light_dir = normalize(l.position - frag_pos);
  
  // ambient
  vec3 l_ambient = vec3(l.ambient * diffuse * ao);

  // diffuse
  vec3 l_diffuse = max(dot(normal, light_dir), 0.0) * diffuse * l.color;

  // specular
  vec3 halfway_dir = normalize(light_dir + view_dir);  
  float spec = pow(max(dot(normal, halfway_dir), 0.0), 8.0);
  vec3 l_specular = specular * l.color * spec;

  float distance = length(l.position - frag_pos);
  float attenuation = 1.0 / (1.0 + l.linear * distance + l.quadratic * distance * distance);
  l_ambient *= attenuation;
  l_diffuse *= attenuation;
  l_specular *= attenuation;

  float shadow = 0.0;
  if (l.cast_shadows == 1 && shadow_map_index < MAX_OMNI_SHADOWS && receive_shadows > 0) {
    vec3 light_pos_world_space = (view_inv * vec4(l.position, 1.0)).xyz;
    
    // GLSL 3 limitation
    shadow = omni_shadow_calculation(omni_shadow_map_0, frag_pos_world_space, light_pos_world_space);
    switch (shadow_map_index) {
      case 0:
        shadow = omni_shadow_calculation(omni_shadow_map_0, frag_pos_world_space, light_pos_world_space);
        break;
      case 1:
        shadow = omni_shadow_calculation(omni_shadow_map_1, frag_pos_world_space, light_pos_world_space);
        break;
      case 2:
        shadow = omni_shadow_calculation(omni_shadow_map_2, frag_pos_world_space, light_pos_world_space);
        break;
      case 3:
        shadow = omni_shadow_calculation(omni_shadow_map_3, frag_pos_world_space, light_pos_world_space);
        break;
    }
  }

  return l_ambient + (1.0 - shadow) * (l_diffuse + l_specular);
}

void main() {             
  // retrieve data from gbuffer
  vec3 frag_pos = texture(g_position, TexCoords).rgb; // FragPos in view space!
  vec3 normal = texture(g_normal, TexCoords).rgb;
  vec3 diffuse = texture(g_albedo, TexCoords).rgb;
  float specular = texture(g_spec, TexCoords).r;

  // FragColor = vec4(Normal, 1.0); return;
  // FragColor = vec4(frag_pos, 1.0);
  // FragColor = vec4(1.0) * Specular;

  // receive shadow in the gBuffer (g_position, alpha channel)
  float receive_shadows = texture(g_position, TexCoords).a;

  vec4 frag_pos_world_space = view_inv * vec4(frag_pos, 1.0);

  float ao = 1.0;
  if (ssao_enabled > 0) {
    ao = texture(ssao, TexCoords).r;
  }

  if (ssao_debug > 0) {
    FragColor = vec4(ao, ao, ao, 1);
    return;
  }

  // then calculate lighting as usual
  vec3 lighting  = vec3(0);
  vec3 view_dir  = normalize(-frag_pos); // viewpos is (0.0.0)

  int omni_light_index = 0;

  for (int l = 0; l < min(MAX_LIGHTS, lights_nr); l++) {
    Light light = lights[l];

    if (light.type == 0) { // directional light
      vec4 frag_pos_light_space = light.light_space_matrix * frag_pos_world_space;
      lighting += calc_dir_light(light, diffuse, specular, normal, ao, view_dir, frag_pos_light_space, receive_shadows);
    } else if (light.type == 1) { // point light
      lighting += calc_point_light(light, omni_light_index++, diffuse, specular, normal, ao, view_dir, frag_pos, frag_pos_world_space.xyz, receive_shadows);
    }
  }

  // Fog
  float fog_coord = abs(frag_pos.z);
  float density = 0.05;
  float fog = 1.0 - clamp(exp(-pow(density * fog_coord, 2)), 0.0, 1.0);

  vec3 fog_color = vec3(0.08, 0.08, 0.08);
  FragColor = mix(vec4(lighting, 1.0), vec4(fog_color, 1.0), fog);
}
