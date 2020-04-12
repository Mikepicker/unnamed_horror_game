#ifndef animator_h
#define animator_h

#include "engine.h"
#include "data/object.h"
#include "data/skeleton.h"
#include "data/frame.h"

int animator_play(object* o, const char* name);
void animator_update(object* o, float time);

#endif
