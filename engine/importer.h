#ifndef importer_h
#define importer_h

#include "engine.h"
#include "data/object.h"
#include "data/skeleton.h"
#include "data/frame.h"
#include "data/animation.h"

object* importer_load(const char *filename);

#endif
