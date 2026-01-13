#include "file_formats/gltf_utils.h"
#include "asserts.h"
#include "mat4.h"
#include "scene/bvh.h"
#include "scene/material.h"
#include "scene/mesh.h"
#include "scene/tlas.h"
#include "vec3.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(_x, _y) _x > _y ? _x : _y

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
// HACK: returns 0 when x is 0
unsigned int next_power_of_2(unsigned int x) {
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

static void build_bvh(Scene *scene, unsigned int tri_first,
                      unsigned int tri_count) {
  if (swaps_lut_capacity < tri_count) {
    swaps_lut_capacity = next_power_of_2(tri_count);
    swaps_lut = malloc(tri_count * sizeof(BVHTriCount));
  }

  BVH_build(scene->bvh_nodes, &scene->bvh_nodes_count, swaps_lut,
            scene->triangles, tri_first, tri_count);
  // NOTE: ignoring swaps_lut for now since we're not using e.g. vertex normals
}

// returns mat_index in Scene->mats for the provided mat
static unsigned int set_material(const char *path, const cgltf_data *data,
                                 const cgltf_material *mat, Scene *scene) {
  // NOTE: must offset by 1 as we have a default material at index 0
  unsigned int mat_index = cgltf_material_index(data, mat) + 1;

  ASSERTQ_COND(mat_index < scene->mats_capacity, mat_index);
  if (scene->mats[mat_index]._set)
    return mat_index;

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

  scene->mats[mat_index] = material;
  scene->last_mat_index = MAX(scene->last_mat_index, mat_index);
  return mat_index;
}

// returns false in case of an invalid primitve
static bool append_mesh_primitive(const char *path, const cgltf_data *data,
                                  const cgltf_primitive *gltf_prim,
                                  Scene *scene) {
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

  unsigned int bvh_index = scene->bvh_nodes_count;
  build_bvh(scene, triangles_first, triangles_count);

  unsigned int mat_index;
  if (gltf_prim->material != NULL)
    mat_index = set_material(path, data, gltf_prim->material, scene);
  else
    mat_index = 0;

  scene->mesh_primitives[scene->mesh_primitives_count++] =
      (MeshPrimitive){.BVH_index = bvh_index, .mat_index = mat_index};

  return true;
}

static void set_mesh_bounds(Mesh *mesh, const Scene *scene) {
  mesh->aabbMin = vec3_new(INFINITY, INFINITY, INFINITY);
  mesh->aabbMax = vec3_new(-INFINITY, -INFINITY, -INFINITY);

  for (cgltf_size p = mesh->mesh_primitive_first;
       p < (mesh->mesh_primitive_first + mesh->mesh_primitive_count); ++p) {
    BVHNode *bvh = &scene->bvh_nodes[scene->mesh_primitives[p].BVH_index];
    mesh->aabbMin = vec3_min(mesh->aabbMin, bvh->bound_min);
    mesh->aabbMax = vec3_max(mesh->aabbMax, bvh->bound_max);
  }
}

// returns false in case a mesh doesn't have any valid primitives
static bool set_mesh(const char *path, const cgltf_data *data,
                     const cgltf_mesh *gltf_mesh, unsigned int index,
                     Scene *scene) {
  cgltf_size first = scene->mesh_primitives_count;
  cgltf_size count = 0;
  for (cgltf_size p = 0; p < gltf_mesh->primitives_count; ++p) {
    if (append_mesh_primitive(path, data, &gltf_mesh->primitives[p], scene)) {
      ++count;
    }
  }
  if (count == 0)
    return false;

  ASSERTQ_COND(index < scene->meshes_capacity, index);
  Mesh mesh = {.mesh_primitive_first = first, .mesh_primitive_count = count};
  set_mesh_bounds(&mesh, scene);
  scene->meshes[index] = mesh;
  scene->last_mesh_index = MAX(scene->last_mesh_index, index);

  return true;
}

static bool is_mesh_initialized(const Scene *scene, unsigned int mesh_index) {
  return scene->meshes[mesh_index].mesh_primitive_count > 0;
}

static void handle_mesh_instance(const char *path, const cgltf_data *data,
                                 const cgltf_node *node, Scene *scene) {
  cgltf_mesh *mesh = node->mesh;
  unsigned int mesh_index = cgltf_mesh_index(data, node->mesh);

  if (!is_mesh_initialized(scene, mesh_index))
    set_mesh(path, data, mesh, mesh_index, scene);

  Mat4 transform;
  cgltf_node_transform_world(node, transform);
  MeshInstance mi = {0};
  memcpy(&mi.transform, transform, 16 * sizeof(float));
  Mat4_trs_inverse(mi.transform, mi.inv_transform);
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

static void traverse_nodes_children(const char *path, const cgltf_data *data,
                                    const cgltf_node *node, Scene *scene,
                                    HandleNodeFn handle_node_fn) {
  for (cgltf_size c = 0; c < node->children_count; ++c) {
    handle_node_fn(path, data, node->children[c], scene);
    traverse_nodes_children(path, data, node->children[c], scene,
                            handle_node_fn);
  }
}

// NOTE: the `scene` root-level property is ignored and all the `scenes` are loaded
void traverse_nodes(const char *path, const cgltf_data *data, Scene *scene,
                    HandleNodeFn handle_node_fn) {
  for (cgltf_size s = 0; s < data->scenes_count; ++s) {
    cgltf_scene *gltf_scene = &data->scenes[s];
    for (cgltf_size n = 0; n < gltf_scene->nodes_count; ++n) {
      cgltf_node *node = gltf_scene->nodes[n];
      handle_node_fn(path, data, node, scene);
      traverse_nodes_children(path, data, node, scene, handle_node_fn);
    }
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

static void handle_camera(const char *path, const cgltf_node *node,
                          Scene *scene) {
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

// NOTE: only sets bounds_min and bounds_max on the returned node
static BVHNode transform_bvh_node_bounds(const BVHNode *node, Mat4 transform) {
  vec3 min = node->bound_min;
  vec3 max = node->bound_max;

  vec3 corners[8] = {
      vec3_new(min.x, min.y, min.z), vec3_new(min.x, min.y, max.z),
      vec3_new(min.x, max.y, min.z), vec3_new(min.x, max.y, max.z),
      vec3_new(max.x, min.y, min.z), vec3_new(max.x, min.y, max.z),
      vec3_new(max.x, max.y, min.z), vec3_new(max.x, max.y, max.z),
  };

  vec3 new_min = vec3_new(INFINITY, INFINITY, INFINITY);
  vec3 new_max = vec3_new(-INFINITY, -INFINITY, -INFINITY);
  for (unsigned int c = 0; c < 8; ++c) {
    vec3 transformed = Mat4_mul_vec3(transform, corners[c]);
    new_min = vec3_min(new_min, transformed);
    new_max = vec3_max(new_max, transformed);
  }

  return (BVHNode){.bound_min = new_min, .bound_max = new_max};
}

static void set_tlas_node_bounds(const Scene *scene, TLASNode *node) {
  node->aabbMin.x = INFINITY;
  node->aabbMin.y = INFINITY;
  node->aabbMin.z = INFINITY;

  node->aabbMax.x = -INFINITY;
  node->aabbMax.y = -INFINITY;
  node->aabbMax.z = -INFINITY;

  if (node->isLeaf) {
    ASSERTQ_COND(node->first < scene->mesh_instances_count, node->first);
    MeshInstance *mesh_instance = &scene->mesh_instances[node->first];
    ASSERTQ_COND(mesh_instance->mesh_index <= scene->last_mesh_index,
                 mesh_instance->mesh_index);
    Mesh *mesh = &scene->meshes[mesh_instance->mesh_index];

    for (unsigned int p = mesh->mesh_primitive_first;
         p < mesh->mesh_primitive_count; ++p) {
      MeshPrimitive *mp = &scene->mesh_primitives[p];
      ASSERTQ_COND(mp->BVH_index < scene->bvh_nodes_count, mp->BVH_index);
      BVHNode *bvh = &scene->bvh_nodes[mp->BVH_index];
      BVHNode transformed_bvh =
          transform_bvh_node_bounds(bvh, mesh_instance->transform);
      node->aabbMin = vec3_min(node->aabbMin, transformed_bvh.bound_min);
      node->aabbMax = vec3_max(node->aabbMax, transformed_bvh.bound_max);
    }
  }
}

void build_tlas(Scene *scene) {
  // TODO: do it properly
  TLASNode root = {0};
  root.first = 0;
  root.isLeaf = 1;
  set_tlas_node_bounds(scene, &root);

  scene->tlas_nodes[scene->tlas_nodes_count++] = root;
}
