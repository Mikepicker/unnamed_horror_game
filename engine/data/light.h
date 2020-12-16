#ifndef light_h
#define light_h

#include "../engine.h"

enum light_type {
  DIRECTIONAL,
  POINT
};

typedef struct {
  enum light_type type;
  vec3 position;
  vec3 color;

  // directional
  float ambient;
  vec3 dir;

  // point lights
  float constant;
  float linear;
  float quadratic;
} light;

#endif
