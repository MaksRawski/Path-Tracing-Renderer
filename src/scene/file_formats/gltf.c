#include "scene/file_formats/gltf.h"
#include "cgltf.h"
#include "scene.h"
#include "scene/file_formats/gltf_utils.h"
#include "scene/material.h"

#include <stdbool.h>
#include <string.h>

void load_gltf_scene(Scene *scene, const char *path) {
  cgltf_options options = {0};
  cgltf_data *data = NULL;

  cgltf_result res = cgltf_parse_file(&options, path, &data);
  gltf_assert(res == cgltf_result_success, path, "%s\n", cgltf_result_str(res));

  res = cgltf_load_buffers(&options, data, path);
  gltf_assert(res == cgltf_result_success, path, "%s\n", cgltf_result_str(res));

  // === Required capacity calculation ===
  // there will always be a default material at index 0
  size_t max_mats_count = data->materials_count + 1;
  size_t max_triangles_count = 0;

  for (cgltf_size m = 0; m < data->meshes_count; ++m) {
    cgltf_mesh mesh = data->meshes[m];
    for (cgltf_size p = 0; p < mesh.primitives_count; ++p) {
      cgltf_primitive prim = mesh.primitives[p];
      if (prim.type == cgltf_primitive_type_triangles)
        max_triangles_count += prim.indices->count / 3;
    }
  }

  // The worst case is when each leaf contains a single triangle (assuming
  // empty nodes are impossible). In practice those functions shouldn't even
  // create nodes this small. The formula is 2 * l - 1, where l is number of
  // leaf nodes, but to avoid having the negative result in case when l is 0, 1
  // is not subtracted.
  size_t max_bvh_nodes_count = 2 * max_triangles_count;

  // === Memory allocations ===
  alloc_if_necessary((void **)&scene->triangles, max_triangles_count,
                     sizeof(Triangle), &scene->triangles_capacity, false);
  alloc_if_necessary((void **)&scene->triangles_data, max_triangles_count,
                     sizeof(TriangleEx), &scene->triangles_data_capacity,
                     false);
  alloc_if_necessary((void **)&scene->mats, max_mats_count, sizeof(Material),
                     &scene->mats_capacity, true);
  alloc_if_necessary((void **)&scene->bvh_nodes, max_bvh_nodes_count,
                     sizeof(BVHnode), &scene->bvh_nodes_capacity, true);

  // === Scene initialization ===
  scene->triangles_count = 0;
  scene->bvh_nodes_count = 0;
  scene->camera = Camera_default();
  scene->mats[0] = Material_default();
  scene->mats_count = 1;

  // === glTF data loading ===
  traverse_nodes(path, data, scene, handle_node);

  cgltf_free(data);
}
