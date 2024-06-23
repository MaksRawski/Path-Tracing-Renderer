#ifndef OBJ_PARSER_H_
#define OBJ_PARSER_H_
#include <glad/gl.h>
#include "renderer.h"

void load_obj_model(const char *filename, GLuint shader_program, ModelBuffer *mb);

#endif // OBJ_PARSER_H_
