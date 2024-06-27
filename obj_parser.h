#ifndef OBJ_PARSER_H_
#define OBJ_PARSER_H_
#include "renderer.h"
#include <glad/gl.h>

typedef struct ObjStats {
	int v, vn, f;
} ObjStats;

void load_obj_model(const char *filename, GLuint shader_program, ModelsBuffer *mb);
void set_material(ModelsBuffer *mb, unsigned int index, Material *mat);

#endif // OBJ_PARSER_H_
