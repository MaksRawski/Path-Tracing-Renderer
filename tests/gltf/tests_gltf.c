#include "tests_gltf.h"
#include "asserts.h"
#include "file_formats/gltf.h"
#include "rad_deg.h"
#include "tests_macros.h"
#include <float.h>

bool test_load_gltf_scene__cube_camera(void) {
  Scene scene = {0};
  load_gltf_scene(&scene, "tests/gltf/cube-camera.glb");

  ASSERT_EQ(scene.triangles_count, 12);
  ASSERT_EQ(scene.mats_count, 1); // just the default material
  ASSERT_EQ(scene.mesh_primitives_count, 1);
  ASSERT_EQ(scene.meshes_count, 1);
  ASSERT_EQ(scene.mesh_instances_count, 1);

  ASSERT_VEC3_EQ(scene.camera.pos, vec3_new(-3.8, 0, 0));
  ASSERT_VEC3_EQ(scene.camera.dir, vec3_new(1, 0, 0));
  ASSERT_EQF(scene.camera.fov_rad, deg_to_rad(39.6), FLT_EPSILON);

  ASSERT_COND(0 < scene.bvh_nodes_count &&
                  scene.bvh_nodes_count < 0 * 2 * scene.triangles_count,
              scene.bvh_nodes_count);
  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_min, vec3_new(-1, -1, -1));
  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_max, vec3_new(1, 1, 1));

  return true;
}

bool test_load_gltf_scene__suzanne(void) {
  Scene scene = {0};
  load_gltf_scene(&scene, "tests/gltf/suzanne.glb");

  ASSERT_EQ(scene.triangles_count, 968);
  ASSERT_EQ(scene.mats_count, 1); // just the default material
  ASSERT_EQ(scene.mesh_primitives_count, 1);
  ASSERT_EQ(scene.meshes_count, 1);
  ASSERT_EQ(scene.mesh_instances_count, 1);

  Camera default_cam = Camera_default();
  ASSERT_VEC3_EQ(scene.camera.pos, default_cam.pos);
  ASSERT_VEC3_EQ(scene.camera.dir, default_cam.dir);
  ASSERT_VEC3_EQ(scene.camera.up, default_cam.up);
  ASSERT_EQF(scene.camera.fov_rad, default_cam.fov_rad, FLT_EPSILON);
  ASSERT_EQF(scene.camera.focal_length, default_cam.focal_length, FLT_EPSILON);

  ASSERT_EQ(scene.mesh_instances[0].mesh_index, 0);
  ASSERT_EQ(scene.meshes[0].mesh_primitive_first, 0);
  ASSERT_EQ(scene.mesh_primitives[0].BVH_index, 0);

  ASSERT_COND(scene.bvh_nodes_count > 0, scene.bvh_nodes_count);

  for (unsigned int n = 0; n < scene.bvh_nodes_count; ++n) {
    // non-leaf nodes should point to other (defined later) nodes
    if (scene.bvh_nodes[n].count == 0)
      ASSERT_COND(scene.bvh_nodes[n].first > n, n);

    float node_bound_volume = vec3_mag(
        vec3_sub(scene.bvh_nodes[n].bound_max, scene.bvh_nodes[n].bound_min));
    ASSERT_COND(node_bound_volume > 0.0, node_bound_volume);
  }

  return true;
}

bool all_gltf_tests(void) {
  bool ok = true;
  TEST_RUN(test_load_gltf_scene__cube_camera, &ok);
  return ok;
}
