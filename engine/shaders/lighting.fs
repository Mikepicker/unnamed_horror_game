#version 330 core
#define NR_LIGHTS 4

out vec4 FragColor;

in vec2 TexCoords;

// gbuffer
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssao;

// lights
uniform vec3 lightsPos[NR_LIGHTS]; 
uniform vec3 lightsColors[NR_LIGHTS];
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
uniform int receive_shadows;

uniform mat4 viewInv;

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

void main()
{             
  // retrieve data from gbuffer
  vec3 FragPos = texture(gPosition, TexCoords).rgb;
  vec3 Normal = texture(gNormal, TexCoords).rgb;
  vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
  float Specular = texture(gAlbedoSpec, TexCoords).a;

  vec4 fragPosLightSpace = lightSpaceMatrix * viewInv * vec4(FragPos, 1.0);

  float ao = texture(ssao, TexCoords).r;
  /* FragColor = vec4(ao, ao, ao, 1);
  return;*/

  // then calculate lighting as usual
  vec3 ambient = vec3(1 * Diffuse * ao);
  vec3 lighting  = ambient; 
  vec3 viewDir  = normalize(-FragPos); // viewpos is (0.0.0)

  for (int i = 0; i < NR_LIGHTS; i++) {
    // diffuse
    vec3 lightDir = normalize(lightsPos[i] - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lightsColors[i];
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = Specular * lightsColors[i] * spec;

    // attenuation
    /* float distance = length(light.Position - FragPos);
       float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);
       diffuse *= attenuation;
       specular *= attenuation; */

    // shadows
    float shadow = 0.0;
    shadow = shadowCalculation(fragPosLightSpace, lightDir, Normal);

    lighting += (1.0 - shadow) * (diffuse + specular);
  }

  FragColor = vec4(lighting, 1.0);
}
