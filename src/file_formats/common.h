#ifndef COMMON_H_
#define COMMON_H_

#include "../renderer.h"

/* int scene_add_triangles(Scene *scene, Triangle *triangles, int count); */
/* int scene_add_mats(Scene *scene, Material *mats, int count); */
/* int scene_add_primitives(Scene *scene, Primitive *prim, int count); */

int load_obj_model(const char *filename, RMeshBuffers *mb, vec3 origin);
void set_obj_pos(RMeshBuffers *mb, int model_id, vec3 pos);
void set_model_material(RMeshBuffers *mb, int model_id, int mat_index);

#endif // COMMON_H_
