#ifndef GLTF_UTILS_H_
#define GLTF_UTILS_H_

#include "cgltf.h"
#include "scene.h"

#include <stdarg.h>

const char *cgltf_result_str(cgltf_result res);

// this compared to ASSERTQ_* handles errors that /may/ actually happen because
// of user input, whereas the latter is more for asserts that /should/ always
// hold or otherwise it's an internal error
void gltf_assert(bool cond, const char *path, const char *err_msg_fmt, ...);

void alloc_if_necessary(void **dst, size_t count, size_t element_size,
                        size_t *capacity, bool should_zero);
unsigned int next_power_of_2(unsigned int x);

typedef void(HandleNodeFn)(const char *path, const cgltf_data *data,
                           const cgltf_node *node, Scene *scene);
void traverse_nodes(const char *path, const cgltf_data *data,
                    const cgltf_node *nodes, size_t nodes_count, Scene *scene,
                    HandleNodeFn handle_node_fn);
HandleNodeFn count_mesh_instances;
HandleNodeFn handle_node;
// ===
#endif // GLTF_UTILS_H_
