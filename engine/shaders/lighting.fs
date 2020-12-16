#version 330 core
#define MAX_LIGHTS 4

out vec4 FragColor;

in vec2 TexCoords;

// gbuffer
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSpec;
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
};

uniform Light lights[MAX_LIGHTS];

uniform int lightsNr;

// camera
uniform vec3 cameraPos; 

// shadow map
uniform sampler2D shadowMap;
uniform float shadowBias;
uniform int shadowPCFEnabled;
uniform mat4 lightSpaceMatrix;

// render params
uniform vec3 color_mask;
uniform int glowing;
uniform vec3 glow_color;

// inverse of view matrix
uniform mat4 viewInv;

// ssao uniforms
uniform int ssao_enabled;
uniform int ssao_debug;

float shadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal) {
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(shadowMap, projCoords.xy).r; 
  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;
  // check whether current frag pos is in shadow
  float bias = max(0.02 * (1.0 - dot(normal, lightDir)), 0.01);

  float shadow = 0.0;
  
  if (shadowPCFEnabled == 1) {
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; x++) {
      for (int y = -1; y <= 1; y++) {
        float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
      }
    }
    shadow /= 9.0;
  } else {
    shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
  }

  if (projCoords.z > 1.0)
    shadow = 0.0;

  return shadow;
}

vec3 calcDirLight(Light l, vec3 diffuse, float specular, vec3 normal, float ao, vec3 viewDir, vec4 fragPosLightSpace, float receive_shadows) {
  vec3 lightDir = normalize(-l.dir);
  
  // ambient
  vec3 lAmbient = vec3(l.ambient * diffuse * ao);

  // diffuse
  vec3 lDiffuse = max(dot(normal, lightDir), 0.0) * diffuse * l.color;

  // specular
  vec3 halfwayDir = normalize(lightDir + viewDir);  
  float spec = pow(max(dot(normal, halfwayDir), 0.0), 8.0);
  vec3 lSpecular = specular * l.color * spec;

  float shadow = 0.0;
  if (receive_shadows > 0) {
    shadow = shadowCalculation(fragPosLightSpace, lightDir, normal);
  }

  return lAmbient + (1.0 - shadow) * (lDiffuse + lSpecular);
}

vec3 calcPointLight(Light l, vec3 diffuse, float specular, vec3 normal, float ao, vec3 viewDir, vec3 fragPos) {
  vec3 lightDir = normalize(l.position - fragPos);
  
  // ambient
  vec3 lAmbient = vec3(l.ambient * diffuse * ao);

  // diffuse
  vec3 lDiffuse = max(dot(normal, lightDir), 0.0) * diffuse * l.color;

  // specular
  vec3 halfwayDir = normalize(lightDir + viewDir);  
  float spec = pow(max(dot(normal, halfwayDir), 0.0), 8.0);
  vec3 lSpecular = specular * l.color * spec;

  float distance = length(l.position - fragPos);
  float attenuation = 1.0 / (1.0 + l.linear * distance + l.quadratic * distance * distance);
  lAmbient *= attenuation;
  lDiffuse *= attenuation;
  lSpecular *= attenuation;

  return lAmbient + lDiffuse + lSpecular;
}

void main()
{             
  // retrieve data from gbuffer
  vec3 FragPos = texture(gPosition, TexCoords).rgb; // FragPos in view space!
  vec3 Normal = texture(gNormal, TexCoords).rgb;
  vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
  float Specular = texture(gSpec, TexCoords).r;

  // FragColor = vec4(Normal, 1.0); return;
  // FragColor = vec4(FragPos, 1.0);
  // FragColor = vec4(1.0) * Specular;

  // receive shadow in the gBuffer (gPosition, alpha channel)
  float receive_shadows = texture(gPosition, TexCoords).a;

  vec4 fragPosWorldSpace = viewInv * vec4(FragPos, 1.0);
  vec4 fragPosLightSpace = lightSpaceMatrix * fragPosWorldSpace;

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
  vec3 viewDir  = normalize(-FragPos); // viewpos is (0.0.0)

  for (int i = 0; i < min(lightsNr, MAX_LIGHTS); i++) {

    if (lights[i].type == 0) { // directional light
      lighting += calcDirLight(lights[i], Diffuse, Specular, Normal, ao, viewDir, fragPosLightSpace, receive_shadows);
    } else if (lights[i].type == 1) { // point light
      lighting += calcPointLight(lights[i], Diffuse, Specular, Normal, ao, viewDir, FragPos);
    }

  }

  FragColor = vec4(lighting, 1.0);
}
