#include "tests_gltf.h"
#include "asserts.h"
#include "file_formats/gltf.h"
#include "rad_deg.h"
#include "tests_macros.h"
#include <float.h>

bool test_load_gltf_scene__cube_camera(void) {
  Scene scene = {0};
  load_gltf_scene(&scene, "tests/gltf/scenes/cube-camera.glb");

  ASSERT_EQ(scene.triangles_count, 12);
  ASSERT_EQ(scene.last_mat_index, 0); 
  ASSERT_EQ(scene.mesh_primitives_count, 1);
  ASSERT_EQ(scene.last_mesh_index, 0);
  ASSERT_EQ(scene.mesh_instances_count, 1);

  ASSERT_VEC3_EQ(scene.camera.pos, vec3_new(-7, 0, 0), FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.camera.dir, vec3_new(1, 0, 0), FLT_EPSILON);
  ASSERT_EQF(scene.camera.fov_rad, deg_to_rad(20), FLT_EPSILON);

  ASSERT_RANGE_EX(scene.bvh.nodes_count, 0, 2 * scene.triangles_count);
  ASSERT_VEC3_EQ(scene.bvh.nodes[0].bound_min, vec3_new(-1, -1, -1),
                 FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_max, vec3_new(1, 1, 1), FLT_EPSILON);

  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_min, scene.tlas_nodes[0].aabbMin,
                 FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_max, scene.tlas_nodes[0].aabbMax,
                 FLT_EPSILON);

  return true;
}

bool test_load_gltf_scene__suzanne(void) {
  Scene scene = {0};
  load_gltf_scene(&scene, "tests/gltf/scenes/suzanne.glb");

  ASSERT_EQ(scene.triangles_count, 968);
  ASSERT_EQ(scene.last_mat_index, 0);
  ASSERT_EQ(scene.mesh_primitives_count, 1);
  ASSERT_EQ(scene.last_mesh_index, 0);
  ASSERT_EQ(scene.mesh_instances_count, 1);

  Camera default_cam = Camera_default();
  ASSERT_VEC3_EQ(scene.camera.pos, default_cam.pos, FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.camera.dir, default_cam.dir, FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.camera.up, default_cam.up, FLT_EPSILON);
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
  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_min, scene.tlas_nodes[0].aabbMin,
                 FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_max, scene.tlas_nodes[0].aabbMax,
                 FLT_EPSILON);

  return true;
}

bool test_load_gltf_scene__rotated_cube(void) {
  Scene scene = {0};
  load_gltf_scene(&scene, "tests/gltf/scenes/rotated-cube.glb");

  ASSERT_EQ(scene.triangles_count, 12);
  ASSERT_EQ(scene.last_mat_index, 0); 
  ASSERT_EQ(scene.mesh_primitives_count, 1);
  ASSERT_EQ(scene.last_mesh_index, 0);
  ASSERT_EQ(scene.mesh_instances_count, 1);

  ASSERT_RANGE_EX(scene.bvh_nodes_count, 0, 2 * scene.triangles_count);

  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_min, vec3_new(-1, -1, -1),
                 FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_max, vec3_new(1, 1, 1), FLT_EPSILON);

  ASSERT_VEC3_EQ(scene.tlas_nodes[0].aabbMin,
                 vec3_new(0, -sqrt(2.0), -sqrt(2.0)), FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.tlas_nodes[0].aabbMax, vec3_new(2, sqrt(2.0), sqrt(2.0)),
                 FLT_EPSILON);

  return true;
}

bool test_load_gltf_scene__transformed_cube(void) {
  Scene scene = {0};
  load_gltf_scene(&scene, "tests/gltf/scenes/transformed-cube.glb");

  ASSERT_EQ(scene.triangles_count, 12);
  ASSERT_EQ(scene.last_mat_index, 0);
  ASSERT_EQ(scene.mesh_primitives_count, 1);
  ASSERT_EQ(scene.last_mesh_index, 0);
  ASSERT_EQ(scene.mesh_instances_count, 1);

  ASSERT_RANGE_EX(scene.bvh_nodes_count, 0, 2 * scene.triangles_count);

  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_min, vec3_new(-1, -1, -1),
                 FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_max, vec3_new(1, 1, 1), FLT_EPSILON);

  ASSERT_VEC3_EQ(scene.tlas_nodes[0].aabbMin, vec3_new(2.9, 1.4, -11.6), 0.1);
  ASSERT_VEC3_EQ(scene.tlas_nodes[0].aabbMax, vec3_new(7.1, 6.6, -6.4), 0.1);

  return true;
}

bool test_load_gltf_scene__two_cubes__copies(void) {
  Scene scene = {0};
  load_gltf_scene(&scene, "tests/gltf/scenes/two-cubes.glb");

  ASSERT_EQ(scene.triangles_count, 24);
  ASSERT_EQ(scene.last_mat_index, 0); 
  ASSERT_EQ(scene.mesh_primitives_count, 2);
  ASSERT_EQ(scene.last_mesh_index, 1);
  ASSERT_EQ(scene.mesh_instances_count, 2);

  ASSERT_RANGE_EX(scene.bvh_nodes_count, 0, 2 * scene.triangles_count);

  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_min, vec3_new(-1, -1, -1),
                 FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_max, vec3_new(1, 1, 1), FLT_EPSILON);

  ASSERT_VEC3_EQ(scene.tlas_nodes[0].aabbMin, vec3_new(-2, 0, -2), FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.tlas_nodes[0].aabbMax, vec3_new(2, 4, 2), FLT_EPSILON);

  return true;
}

bool test_load_gltf_scene__two_cubes__instancing(void) {
  Scene scene = {0};
  load_gltf_scene(&scene, "tests/gltf/scenes/two-cubes-instancing.glb");

  ASSERT_EQ(scene.triangles_count, 12);
  ASSERT_EQ(scene.last_mat_index, 0); 
  ASSERT_EQ(scene.mesh_primitives_count, 1);
  ASSERT_EQ(scene.last_mesh_index, 0);
  ASSERT_EQ(scene.mesh_instances_count, 2);

  ASSERT_RANGE_EX(scene.bvh_nodes_count, 0, 2 * scene.triangles_count);

  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_min, vec3_new(-1, -1, -1),
                 FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_max, vec3_new(1, 1, 1), FLT_EPSILON);

  ASSERT_VEC3_EQ(scene.tlas_nodes[0].aabbMin, vec3_new(-2, 0, -2), FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.tlas_nodes[0].aabbMax, vec3_new(2, 4, 2), FLT_EPSILON);

  return true;
}

bool test_load_gltf_scene__cube_from_planes(void) {
  Scene scene = {0};
  load_gltf_scene(&scene, "tests/gltf/scenes/cube-from-planes.glb");

  ASSERT_EQ(scene.triangles_count, 2);
  ASSERT_EQ(scene.last_mat_index, 0); 
  ASSERT_EQ(scene.mesh_primitives_count, 1);
  ASSERT_EQ(scene.last_mesh_index, 0);
  ASSERT_EQ(scene.mesh_instances_count, 6);

  ASSERT_RANGE_EX(scene.bvh_nodes_count, 0, 2 * scene.triangles_count);

  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_min, vec3_new(-1, 0, -1),
                 FLT_EPSILON);
  ASSERT_VEC3_EQ(scene.bvh_nodes[0].bound_max, vec3_new(1, 0, 1), FLT_EPSILON);

  ASSERT_VEC3_EQ(scene.tlas_nodes[0].aabbMin, vec3_new(-1, 0, -1), 0.1);
  ASSERT_VEC3_EQ(scene.tlas_nodes[0].aabbMax, vec3_new(1, 2, 1), FLT_EPSILON);

  return true;
}

bool all_gltf_tests(void) {
  bool ok = true;
  TEST_RUN(test_load_gltf_scene__cube_camera, &ok);
  TEST_RUN(test_load_gltf_scene__suzanne, &ok);
  TEST_RUN(test_load_gltf_scene__rotated_cube, &ok);
  TEST_RUN(test_load_gltf_scene__transformed_cube, &ok);
  TEST_RUN(test_load_gltf_scene__two_cubes__copies, &ok);
  TEST_RUN(test_load_gltf_scene__two_cubes__instancing, &ok);
  TEST_RUN(test_load_gltf_scene__cube_from_planes, &ok);
  return ok;
}
