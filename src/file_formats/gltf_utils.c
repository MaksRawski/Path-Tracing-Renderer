#include "file_formats/gltf_utils.h"
#include "asserts.h"
#include "scene/bvh.h"
#include "scene/material.h"
#include "scene/mesh.h"

#include <stdlib.h>
#include <string.h>

const char *cgltf_result_str(cgltf_result res) {
  switch (res) {
  case cgltf_result_success:
    return "success";
  case cgltf_result_invalid_json:
    return "invalid JSON";
  case cgltf_result_invalid_gltf:
    return "invalid GLTF";
  case cgltf_result_invalid_options:
    return "invalid options";
  case cgltf_result_file_not_found:
    return "file not found";
  case cgltf_result_io_error:
    return "IO error";
  case cgltf_result_out_of_memory:
    return "out of memory";
  case cgltf_result_legacy_gltf:
    return "legacy GLTF";
  default:
    return "unknown error";
  }
}

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
unsigned int next_power_of_2(unsigned int x) {
  // NOTE: ignoring the case where x is 0
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  ++x;
  return x;
}

void gltf_assert(bool cond, const char *path, const char *err_msg_fmt, ...) {
  if (!cond) {
    char msg[256];
    va_list args;
    va_start(args, err_msg_fmt);
    vsnprintf(msg, 256, err_msg_fmt, args);
    va_end(args);
    ERROR_FMT("Failed to load %s: %s", path, msg);
  }
}

static unsigned int *swaps_lut;
static size_t swaps_lut_capacity = 0;

void build_bvh(Scene *scene, unsigned int tri_first, unsigned int tri_count) {
  if (swaps_lut_capacity < tri_count) {
    swaps_lut_capacity = next_power_of_2(tri_count);
    swaps_lut = malloc(tri_count * sizeof(BVHTriCount));
  }

  BVH_build(scene->bvh_nodes, &scene->bvh_nodes_count, swaps_lut,
            scene->triangles, tri_first, tri_count);
  // NOTE: ignoring swaps_lut for now since we're not using e.g. vertex normals
}

// returns mat_index in Scene->mats for the provided mat
unsigned int set_material(const char *path, const cgltf_data *data,
                          const cgltf_material *mat, Scene *scene) {
  unsigned int mat_index = cgltf_material_index(data, mat);
  gltf_assert(
      mat->has_pbr_metallic_roughness, path,
      "Material %s (index: %d) doesn't have a PBR metallic roughness model "
      "defined!",
      mat->name, mat_index);

  Material material = Material_default();
  memcpy(material.base_color_factor,
         mat->pbr_metallic_roughness.base_color_factor, 4);
  material.metallic_factor = mat->pbr_metallic_roughness.metallic_factor;
  material.roughness_factor = mat->pbr_metallic_roughness.roughness_factor;
  memcpy(material.emissive_factor, mat->emissive_factor, 3);

  scene->mats[mat_index + 1] = material;
  ++scene->mats_count;
  return mat_index + 1;
}

// returns false in case of an invalid primitve
bool append_mesh_primitive(const char *path, const cgltf_data *data,
                           const cgltf_primitive *gltf_prim, Scene *scene) {
  cgltf_accessor *pos_accessor = NULL;
  cgltf_accessor *norm_accessor = NULL;
  cgltf_size attrs_count = gltf_prim->attributes_count;
  for (cgltf_size a = 0; a < attrs_count; ++a) {
    cgltf_attribute attr = gltf_prim->attributes[a];
    if (attr.type == cgltf_attribute_type_position)
      pos_accessor = attr.data;
    else if (attr.type == cgltf_attribute_type_normal)
      norm_accessor = attr.data;
  }

  if (pos_accessor == NULL || norm_accessor == NULL)
    return false;

  gltf_assert(pos_accessor->type == cgltf_type_vec3, path,
              "POSITION attribute should have type vec3 but has: %d",
              pos_accessor->type);

  gltf_assert(norm_accessor->type == cgltf_type_vec3, path,
              "NORMAL attribute should have type vec3 but has: %d",
              pos_accessor->type);

  cgltf_accessor *idx_accessor = gltf_prim->indices;
  gltf_assert(idx_accessor != NULL, path,
              "NOT YET IMPLEMENTED: primitive is not indexed");

  unsigned int triangles_first = scene->triangles_count;

  for (cgltf_size t = 0; t < idx_accessor->count / 3; ++t) {
    for (cgltf_size v = 0; v < 3; ++v) {
      cgltf_size index = cgltf_accessor_read_index(idx_accessor, 3 * t + v);

      vec3 *tv =
          Triangle_get_vertex(&scene->triangles[scene->triangles_count], v);
      float tvf[3];
      cgltf_accessor_read_float(pos_accessor, index, tvf, 3);
      vec3_copy_from_float3(tv, tvf);
    }
    ++scene->triangles_count;
  }
  unsigned int triangles_count = scene->triangles_count - triangles_first;

  unsigned int bvh_idnex = scene->bvh_nodes_count;
  build_bvh(scene, triangles_first, triangles_count);

  // HACK: no good way to detect whether the material was defined so
  // we set them every time we encounter them
  unsigned int mat_index = set_material(path, data, gltf_prim->material, scene);

  scene->mesh_primitives[scene->mesh_primitives_count++] =
      (MeshPrimitive){.BVH_index = bvh_idnex, .mat_index = mat_index};

  return true;
}

// returns false in case a mesh doesn't have any valid primitives
bool set_mesh(const char *path, const cgltf_data *data,
              const cgltf_mesh *gltf_mesh, unsigned int index, Scene *scene) {
  cgltf_size first = scene->mesh_primitives_count;
  cgltf_size count = 0;
  for (cgltf_size p = 0; p < gltf_mesh->primitives_count; ++p) {
    if (append_mesh_primitive(path, data, &gltf_mesh->primitives[p], scene)) {
      ++count;
    }
  }
  if (count == 0)
    return false;

  // NOTE: because indices have to be the same as in the glTF file, the count is
  // NOT the same as the index of the first free element
  scene->meshes[index] =
      (Mesh){.mesh_primitive_first = first, .mesh_primitive_count = count};
  ++scene->meshes_count;

  return true;
}

bool is_mesh_initialized(const Scene *scene, unsigned int mesh_index) {
  return scene->meshes[mesh_index].mesh_primitive_count > 0;
}

void handle_mesh_instance(const char *path, const cgltf_data *data,
                          const cgltf_node *node, Scene *scene) {
  cgltf_mesh *mesh = node->mesh;
  unsigned int mesh_index = cgltf_mesh_index(data, node->mesh);

  if (!is_mesh_initialized(scene, mesh_index))
    set_mesh(path, data, mesh, mesh_index, scene);

  Mat4 transform;
  cgltf_node_transform_world(node, transform);
  MeshInstance mi = {0};
  memcpy(&mi.transform, transform, sizeof(mi) / sizeof(float));
  scene->mesh_instances[scene->mesh_instances_count++] = mi;
}

void alloc_if_necessary(void **dst, size_t count, size_t element_size,
                        size_t *capacity, bool should_zero) {
  if (*capacity < count) {
    if (*dst != NULL)
      free(*dst);
    *capacity = next_power_of_2(count);
    if (should_zero)
      *dst = calloc(count, element_size);
    else
      *dst = malloc(count * element_size);

    if (dst == NULL) {
      ERROR_FMT("Failed to allocate %lu bytes of memory",
                *capacity * element_size);
    }
  }
}

void traverse_nodes_children(const char *path, const cgltf_data *data,
                             const cgltf_node *node, Scene *scene,
                             HandleNodeFn handle_node_fn) {
  for (cgltf_size c = 0; c < node->children_count; ++c) {
    handle_node_fn(path, data, node->children[c], scene);
    traverse_nodes_children(path, data, node->children[c], scene,
                            handle_node_fn);
  }
}

void traverse_nodes(const char *path, const cgltf_data *data, Scene *scene,
                    HandleNodeFn handle_node_fn) {
  for (cgltf_size n = 0; n < data->nodes_count; ++n) {
    cgltf_node *node = data->nodes + n;
    handle_node_fn(path, data, node, scene);
    traverse_nodes_children(path, data, node, scene, handle_node_fn);
  }
}

void handle_camera(const char *path, const cgltf_node *node, Scene *scene);
void handle_mesh_instance(const char *path, const cgltf_data *data,
                          const cgltf_node *node, Scene *scene);

void count_mesh_instances(const char *path, const cgltf_data *data,
                          const cgltf_node *node, Scene *scene) {
  UNUSED(path, data);
  if (node->mesh != NULL)
    ++scene->mesh_instances_count;
}

void handle_camera(const char *path, const cgltf_node *node, Scene *scene) {
  gltf_assert(node->camera->type == cgltf_camera_type_perspective, path,
              "Only perspective camera type is supported! Got type %d\n",
              node->camera->type);
  // From glTF 2.0 Specification
  // (https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#cameras-overview):
  // "A camera object defines the projection matrix that transforms scene
  // coordinates from the view space to the clip space.
  // A node containing the camera instance defines the view matrix that
  // transforms scene coordinates from the global space to the view space."

  cgltf_camera_perspective cam = node->camera->data.perspective;
  Mat4 view_matrix;
  cgltf_node_transform_world(node, view_matrix);
  vec3 pos = Mat4_mul_vec3(view_matrix, DEFAULT_CAM_POS);
  vec3 lookat =
      Mat4_mul_vec3(view_matrix, vec3_add(DEFAULT_CAM_POS, DEFAULT_CAM_DIR));
  vec3 dir = vec3_sub(lookat, pos);

  scene->camera =
      Camera_new(pos, dir, DEFAULT_CAM_UP, cam.yfov, DEFAULT_CAM_FOCAL_LENGTH);
}

void handle_node(const char *path, const cgltf_data *data,
                 const cgltf_node *node, Scene *scene) {
  if (node->mesh)
    handle_mesh_instance(path, data, node, scene);
  if (node->camera)
    handle_camera(path, node, scene);
}
