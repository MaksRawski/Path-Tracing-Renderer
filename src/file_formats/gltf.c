#include "file_formats/gltf.h"
#include "cgltf.h"
#include "file_formats/gltf_utils.h"
#include "scene.h"

#include <stdbool.h>
#include <string.h>

/* const Material DEFAULT_MATERIAL = { */
/*     .albedo = {0.3, 0.3, 0.3}, */
/*     .emission_color = {0, 0, 0}, */
/*     .emission_strength = 0, */
/*     .specular_component = 0, */
/* }; */

void load_gltf_scene(Scene *scene, const char *path) {
  cgltf_options options = {0};
  cgltf_data *data = NULL;

  cgltf_result res = cgltf_parse_file(&options, path, &data);
  gltf_assert(res == cgltf_result_success, path, "%s\n", cgltf_result_str(res));

  res = cgltf_load_buffers(&options, data, path);
  gltf_assert(res == cgltf_result_success, path, "%s\n", cgltf_result_str(res));

  // === Preprocessing ===
  unsigned int triangles_count = 0;
  unsigned int mesh_primitives_count = 0;

  for (cgltf_size m = 0; m < data->meshes_count; ++m) {
    cgltf_mesh mesh = data->meshes[m];
    mesh_primitives_count += mesh.primitives_count;
    for (cgltf_size p = 0; p < mesh.primitives_count; ++p) {
      cgltf_primitive prim = mesh.primitives[p];
      if (prim.type == cgltf_primitive_type_triangles)
        triangles_count += prim.indices->count / 3;
    }
  }

  // there will always be a default material at index 0
  unsigned int mats_count = data->materials_count + 1;
  unsigned int meshes_count = data->meshes_count;

  // NOTE: populates scene->mesh_instances_count
  scene->mesh_instances_count = 0;
  traverse_nodes(path, data, scene, count_mesh_instances);
  unsigned int mesh_instances_count = scene->mesh_instances_count;

  scene->camera = Camera_default();

  // === Allocations ===
  alloc_if_necessary((void **)&scene->triangles, triangles_count,
                     sizeof(Triangle), &scene->triangles_capacity, false);
  alloc_if_necessary((void **)&scene->mats, mats_count, sizeof(Material),
                     &scene->mats_capacity, true);
  alloc_if_necessary((void **)&scene->mesh_primitives, mesh_primitives_count,
                     sizeof(MeshPrimitive), &scene->mesh_primitives_capacity,
                     true);
  alloc_if_necessary((void **)&scene->meshes, meshes_count, sizeof(Mesh),
                     &scene->meshes_capacity, true);
  alloc_if_necessary((void **)&scene->mesh_instances, mesh_instances_count,
                     sizeof(MeshInstance), &scene->mesh_instances_capacity,
                     true);

  // Assuming that the worst case is each leaf containing a single triangle.
  // In practice BVH_build function shouldn't even create nodes this small.
  unsigned int bvh_nodes_count = 2 * triangles_count;
  alloc_if_necessary((void **)&scene->bvh_nodes, bvh_nodes_count,
                     sizeof(BVHNode), &scene->bvh_nodes_capacity, true);

  // similarly to the above case
  unsigned int tlas_nodes_count = 2 * mesh_instances_count;
  alloc_if_necessary((void **)&scene->tlas_nodes, tlas_nodes_count,
                     sizeof(TLASNode), &scene->tlas_nodes_capacity, true);

  scene->triangles_count = 0;
  scene->bvh_nodes_count = 0;
  scene->mats_count = 0;
  scene->mesh_primitives_count = 0;
  scene->meshes_count = 0;
  scene->mesh_instances_count = 0;
  scene->tlas_nodes_count = 0;

  traverse_nodes(path, data, scene, handle_node);

  cgltf_free(data);
}
