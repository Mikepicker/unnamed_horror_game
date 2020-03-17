#ifndef factory_h
#define factory_h

#include "engine.h"
#include "data/object.h"

object* factory_create_sphere(float radius, int sector_count, int stack_count);
object* factory_create_plane(float width, float height);
object* factory_create_box(float width, float height, float depth);

#endif
