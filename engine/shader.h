#ifndef shader_h
#define shader_h

#include "engine.h"

void shader_compile(const GLchar* vertex_path, const GLchar* fragment_path, const GLchar* geometry_path, GLuint* shader_id);

#endif
