#ifndef GLTF_H_
#define GLTF_H_

#include "cgltf.h"
#include "scene.h"

void load_gltf_scene(Scene *scene, const char *filename);

void handle_mesh(cgltf_data *data, const char *path, cgltf_node *node,
                 Scene *scene, int m, cgltf_size *const t_counter);
void handle_camera(const char *path, cgltf_node *node, Scene *scene);

#endif // GLTF_H_
