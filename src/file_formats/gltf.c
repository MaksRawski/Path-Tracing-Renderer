#include "file_formats/gltf.h"
#include "cgltf.h"
#include "file_formats/gltf_utils.h"
#include "scene.h"
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
  unsigned int max_meshes_count = data->meshes_count;
  // there will always be a default material at index 0
  unsigned int max_mats_count = data->materials_count + 1;

  // NOTE: count_mesh_instances populates scene->mesh_instances_count
  scene->mesh_instances_count = 0;
  traverse_nodes(path, data, scene, count_mesh_instances);
  unsigned int max_mesh_instances_count = scene->mesh_instances_count;

  unsigned int max_triangles_count = 0;
  unsigned int max_mesh_primitives_count = 0;
  for (cgltf_size m = 0; m < data->meshes_count; ++m) {
    cgltf_mesh mesh = data->meshes[m];
    max_mesh_primitives_count += mesh.primitives_count;
    for (cgltf_size p = 0; p < mesh.primitives_count; ++p) {
      cgltf_primitive prim = mesh.primitives[p];
      if (prim.type == cgltf_primitive_type_triangles)
        max_triangles_count += prim.indices->count / 3;
    }
  }

  // The worst case is when each leaf contains a single triangle/mesh (assuming
  // empty nodes are impossible). In practice those functions shouldn't even
  // create nodes this small. The formula is 2 * l - 1, where l is number of
  // leaf nodes, but to avoid having the negative result in case when l is 0, 1
  // is not subtracted.
  unsigned int max_bvh_nodes_count = 2 * max_triangles_count;
  unsigned int max_tlas_nodes_count = 2 * max_mesh_instances_count;

  // === Memory allocations ===
  alloc_if_necessary((void **)&scene->triangles, max_triangles_count,
                     sizeof(Triangle), &scene->triangles_capacity, false);
  alloc_if_necessary((void **)&scene->mats, max_mats_count, sizeof(Material),
                     &scene->mats_capacity, true);
  alloc_if_necessary((void **)&scene->mesh_primitives,
                     max_mesh_primitives_count, sizeof(MeshPrimitive),
                     &scene->mesh_primitives_capacity, true);
  alloc_if_necessary((void **)&scene->meshes, max_meshes_count, sizeof(Mesh),
                     &scene->meshes_capacity, true);
  alloc_if_necessary((void **)&scene->mesh_instances, max_mesh_instances_count,
                     sizeof(MeshInstance), &scene->mesh_instances_capacity,
                     true);
  alloc_if_necessary((void **)&scene->bvh_nodes, max_bvh_nodes_count,
                     sizeof(BVHNode), &scene->bvh_nodes_capacity, true);
  alloc_if_necessary((void **)&scene->tlas_nodes, max_tlas_nodes_count,
                     sizeof(TLASNode), &scene->tlas_nodes_capacity, true);

  // === Scene initialization ===
  scene->triangles_count = 0;
  scene->bvh_nodes_count = 0;
  scene->mesh_primitives_count = 0;
  scene->meshes_count = 0;
  scene->mesh_instances_count = 0;
  scene->tlas_nodes_count = 0;

  scene->camera = Camera_default();

  if (scene->mats_count == 0) {
    scene->mats[0] = Material_default();
  }
  scene->mats_count = 1;

  // === glTF data loading ===
  traverse_nodes(path, data, scene, handle_node);

  // === Building TLAS ===
  build_tlas(scene);

  cgltf_free(data);
}
