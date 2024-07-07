#ifndef OBJ_PARSER_H_
#define OBJ_PARSER_H_
#include "renderer.h"
#include <glad/gl.h>

typedef struct ObjStats {
  int v, vn, f;
} ObjStats;

typedef struct {
  float l[3];
} vec3;

static const Material white_mat = {
    .albedo = {1.0, 1.0, 1.0},
    .emissionColor = {0, 0, 0},
    .emissionStrength = 0,
    .specularComponent = 0.0,
};
static const Material mirror_mat = {
    .albedo = {1.0, 1.0, 1.0},
    .emissionColor = {0, 0, 0},
    .emissionStrength = 0,
    .specularComponent = 1.0,
};
static const Material red_mat = {
    .albedo = {1.0, 0.0, 0.0},
    .emissionColor = {0, 0, 0},
    .emissionStrength = 0,
    .specularComponent = 0.2,
};
static const Material green_mat = {
    .albedo = {0.0, 1.0, 0.0},
    .emissionColor = {0, 0, 0},
    .emissionStrength = 0,
    .specularComponent = 0.2,
};
static const Material blue_mat = {
    .albedo = {0.0, 0.0, 1.0},
    .emissionColor = {0, 0, 0},
    .emissionStrength = 0,
    .specularComponent = 0.2,
};
static const Material gold_mat = {
    .albedo = {1.0, 0.84, 0.0},
    .emissionColor = {0, 0, 0},
    .emissionStrength = 0,
    .specularComponent = 0.4,
};
static const Material black_mat = {
    .albedo = {0.0, 0.0, 0.0},
    .emissionColor = {0, 0, 0},
    .emissionStrength = 0,
    .specularComponent = 0.0,
};

int load_obj_model(const char *filename, GLuint shader_program,
                   ModelsBuffer *mb, vec3 *origin);
void set_obj_pos(ModelsBuffer *mb, int model_id, vec3 pos);
void set_material_slot(ModelsBuffer *mb, int index, const Material *mat);
void set_model_material(ModelsBuffer *mb, int model_id, int mat_index);

#endif // OBJ_PARSER_H_
