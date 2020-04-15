#ifndef animator_h
#define animator_h

#include "engine.h"
#include "data/object.h"
#include "data/skeleton.h"
#include "data/frame.h"

int animator_play(object* o, const char* name, int loop);
void animator_update(object* o, float dt);
int animator_current_keyframe(object* o);
int animator_total_keyframes(object* o);
int animator_finished(object* o);

#endif
