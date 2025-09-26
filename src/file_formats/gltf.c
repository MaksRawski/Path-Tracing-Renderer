#include "file_formats/gltf.h"
#include "mat4.h"
#include "vec3.h"
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define CGLTF_IMPLEMENTATION
#include "file_formats/cgltf.h"

const Material DEFAULT_MATERIAL = {
    .albedo = {0.3, 0.3, 0.3},
    .emission_color = {0, 0, 0},
    .emission_strength = 0,
    .specular_component = 0,
};

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

void gltf_assert(bool cond, const char *path, const char *err_msg_fmt, ...) {
  if (!cond) {
    char msg[256];
    va_list args;
    va_start(args, err_msg_fmt);
    vsnprintf(msg, 256, err_msg_fmt, args);
    va_end(args);
    fprintf(stderr, "Error occured while loading %s: %s", path, msg);
    exit(1);
  }
}

// vertex must be a number from 0 to 5
vec3 *vec3_get_triangle_vertex(Triangle *t, int vertex) {
  // HACK: this is technically UB but /it works/ and is soooo easy to write
  return ((vec3 *)t) + vertex;
}

// scene should be zero-intialized
// NOTE: this is very naive loading, node-level transformations are ignored
void load_gltf_scene(Scene *scene, const char *path) {
  cgltf_options options = {0};
  cgltf_data *data = NULL;

  cgltf_result res = cgltf_parse_file(&options, path, &data);
  gltf_assert(res == cgltf_result_success, path, "%s\n", cgltf_result_str(res));

  res = cgltf_load_buffers(&options, data, path);
  gltf_assert(res == cgltf_result_success, path, "%s\n", cgltf_result_str(res));

  int total_triangles = 0;
  for (cgltf_size m = 0; m < data->meshes_count; ++m) {
    cgltf_mesh mesh = data->meshes[m];
    for (cgltf_size p = 0; p < data->meshes[m].primitives_count; ++p) {
      cgltf_primitive prim = mesh.primitives[p];
      if (prim.type != cgltf_primitive_type_triangles)
        continue;
      total_triangles += prim.indices->count / 3;
    }
  }

  // there will always be a default material at 0
  scene->mats_count = data->materials_count + 1;
  scene->meshes_count = data->meshes_count;
  scene->triangles_count = total_triangles;

  scene->meshes = malloc(scene->meshes_count * sizeof(Mesh));
  scene->triangles = malloc(scene->triangles_count * sizeof(Triangle));
  scene->primitives = malloc(scene->triangles_count * sizeof(Primitive));
  scene->mats = malloc(scene->mats_count * sizeof(Material));

  // nodes
  cgltf_size t_counter = 0;
  cgltf_size meshes_counter = 0;
  for (cgltf_size n = 0; n < data->nodes_count; ++n) {
    cgltf_node *node = &data->nodes[n];

    // FIXME: technically a node can instantiate both a camera and a mesh
    if (node->mesh != NULL)
      handle_mesh(data, path, node, scene, meshes_counter++, &t_counter);
    if (node->camera != NULL)
      handle_camera(path, node, scene);
  }

  // materials
  scene->mats[0] = DEFAULT_MATERIAL;
  for (cgltf_size m = 1; m < data->materials_count + 1; ++m) {
    cgltf_material mat = data->materials[m - 1];

    memcpy(scene->mats[m].albedo, mat.pbr_metallic_roughness.base_color_factor,
           3 * sizeof(float));
    memcpy(scene->mats[m].emission_color, mat.emissive_factor,
           3 * sizeof(float));

    scene->mats[m].emission_strength = mat.emissive_strength.emissive_strength;

    // TODO: specular component seems compliated, ignoring it for now
    scene->mats[m].specular_component = 0.0;
  }

  cgltf_free(data);
}

void handle_mesh(cgltf_data *data, const char *path, cgltf_node *node,
                 Scene *scene, int m, cgltf_size *const t_counter) {
  cgltf_mesh mesh = *node->mesh;
  cgltf_size primitives_count = mesh.primitives_count;
  scene->meshes[m].index = *t_counter;

  cgltf_float node_transform_matrix[16] = {0};
  cgltf_node_transform_world(node, node_transform_matrix);

  for (cgltf_size p = 0; p < primitives_count; ++p) {
    cgltf_primitive prim = mesh.primitives[p];

    // choose default material if not specified
    cgltf_size mat_index = 0;
    if (prim.material != NULL) {
      mat_index = cgltf_material_index(data, prim.material) + 1;
    }

    cgltf_accessor *pos_accessor = NULL;
    cgltf_accessor *norm_accessor = NULL;
    cgltf_size attrs_count = prim.attributes_count;
    for (cgltf_size a = 0; a < attrs_count; ++a) {
      cgltf_attribute attr = prim.attributes[a];
      if (attr.type == cgltf_attribute_type_position)
        pos_accessor = attr.data;
      else if (attr.type == cgltf_attribute_type_normal)
        norm_accessor = attr.data;
    }
    if (pos_accessor == NULL || norm_accessor == NULL)
      continue;

    gltf_assert(pos_accessor->type == cgltf_type_vec3, path,
                "POSITION attribute should have type vec3 but has: %d",
                pos_accessor->type);

    gltf_assert(norm_accessor->type == cgltf_type_vec3, path,
                "NORMAL attribute should have type vec3 but has: %d",
                pos_accessor->type);

    cgltf_accessor *idx_accessor = prim.indices;
    gltf_assert(idx_accessor != NULL, path,
                "NOT YET IMPLEMENTED: primitive is not indexed");

    cgltf_size t_count = idx_accessor->count / 3;
    for (cgltf_size i = 0; i < t_count; ++i) {
      for (cgltf_size v = 0; v < 3; ++v) {
        vec3 *tv = (vec3 *)(scene->triangles + *t_counter) + v;
        vec3 *tn = tv + 3;
        cgltf_size index = cgltf_accessor_read_index(idx_accessor, 3 * i + v);
        cgltf_accessor_read_float(pos_accessor, index, (float *)tv, 3);
        cgltf_accessor_read_float(norm_accessor, index, (float *)tn, 3);
      }
      // NOTE: primitive in my case is a just a wrapper around a single
      // triangle
      scene->primitives[*t_counter].mat = mat_index;
      ++*t_counter;
    }
  }

  scene->meshes[m].count = *t_counter - scene->meshes[m].index;

  // apply node_transform_matrix to each triangle's vertex
  int first_tri = scene->meshes[m].index;
  int last_tri = first_tri + scene->meshes[m].count;
  for (int t = first_tri; t < last_tri; ++t) {
    for (int i = 0; i < 6; ++i) {
      vec3 *v = vec3_get_triangle_vertex(&scene->triangles[t], i);
      *v = mat4_mul_vec3(node_transform_matrix, *v);
    }
  }
}

void handle_camera(const char *path, cgltf_node *node, Scene *scene) {
  gltf_assert(node->camera->type == cgltf_camera_type_perspective, path,
              "Only perspective camera type is supported! Got type %d\n",
              node->camera->type);

  cgltf_camera_perspective cam = node->camera->data.perspective;
  scene->camera.projection_matrix.aspectRatio = cam.aspect_ratio;
  scene->camera.projection_matrix.yfov = cam.yfov;
  scene->camera.projection_matrix.zfar = cam.has_zfar ? cam.zfar : INFINITY;
  scene->camera.projection_matrix.znear = cam.znear;

  cgltf_float node_transform_matrix[16] = {0};
  cgltf_node_transform_world(node, scene->camera.view_matrix);
  ++scene->camera_count;
}
