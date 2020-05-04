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
  vec3 dir;
} light;

#endif
