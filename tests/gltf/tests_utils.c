#include "tests_utils.h"
#include "arena.h"
#include "asserts.h"
#include "cgltf.h"
#include "file_formats/gltf_utils.h"
#include "scene.h"
#include "tests_macros.h"
#include <stdlib.h>

bool test_traverse_nodes__count_mesh_instances(void) {
  Arena arena = Arena_new(1024 * 1024);

  cgltf_data *data = Arena_alloc(&arena, sizeof(cgltf_data));

#define L1_NODES 5
#define L2_NODES 4
#define L3_NODES 3

  data->nodes = Arena_alloc(&arena, L1_NODES * sizeof(cgltf_node));
  data->nodes_count = L1_NODES;
  for (size_t n = 0; n < L1_NODES; ++n) {
    cgltf_node *l1_node = data->nodes + n;
    l1_node->mesh = Arena_alloc(&arena, sizeof(cgltf_mesh));

    l1_node->children = Arena_alloc(&arena, L2_NODES * sizeof(cgltf_node *));
    l1_node->children_count = L2_NODES;

    for (size_t c2 = 0; c2 < L2_NODES; ++c2) {
      cgltf_node *l2_node = Arena_alloc(&arena, sizeof(cgltf_node));
      l1_node->children[c2] = l2_node;
      l2_node->mesh = Arena_alloc(&arena, sizeof(cgltf_mesh));

      l2_node->children = Arena_alloc(&arena, L3_NODES * sizeof(cgltf_node *));
      l2_node->children_count = L3_NODES;

      for (size_t c3 = 0; c3 < L3_NODES; ++c3) {
        cgltf_node *l3_node = Arena_alloc(&arena, sizeof(cgltf_node));
        l2_node->children[c3] = l3_node;
        l3_node->mesh = Arena_alloc(&arena, sizeof(cgltf_mesh));
      }
    }
  }

#define TOTAL_L2_NODES (L1_NODES * L2_NODES)
#define TOTAL_L3_NODES (TOTAL_L2_NODES * L3_NODES)
#define TOTAL_NODES (L1_NODES + TOTAL_L2_NODES + TOTAL_L3_NODES)

  Scene scene = {0};
  traverse_nodes("", data, &scene, count_mesh_instances);
  ASSERT_EQI(scene.mesh_instances_count, TOTAL_NODES);
  Arena_delete(&arena);

  return true;
}

bool test_next_power_of_2(void) {
  ASSERT_EQI(next_power_of_2(1), 1);
  ASSERT_EQI(next_power_of_2(2), 2);
  ASSERT_EQI(next_power_of_2(3), 4);
  ASSERT_EQI(next_power_of_2(4), 4);
  ASSERT_EQI(next_power_of_2(5), 8);
  ASSERT_EQI(next_power_of_2(10), 16);
  ASSERT_EQI(next_power_of_2(100), 128);
  ASSERT_EQI(next_power_of_2(500), 512);
  ASSERT_EQI(next_power_of_2(1000), 1024);
  ASSERT_EQI(next_power_of_2(10000), 16384);
  ASSERT_EQI(next_power_of_2(50000), 65536);
  ASSERT_EQI(next_power_of_2(100 * 1000), 131072);
  ASSERT_EQI(next_power_of_2(1000 * 1000), 1048576);
  ASSERT_EQI(next_power_of_2(1000 * 1000 * 1000), 1073741824);
  ASSERT_EQI(next_power_of_2(2 * 1000 * 1000 * 1000), 2147483648);

  return true;
}

bool all_gltf_utils_tests(void) {
  bool ok = true;
  TEST_RUN(test_traverse_nodes__count_mesh_instances, &ok);
  TEST_RUN(test_next_power_of_2, &ok);

  return ok;
}
