
#include "file_formats/gltf.h"
#include "mat4.h"
#include "structs.h"
#include "utils.h"
#include <stdlib.h>

#include <glad/gl.h>
//
#include <GLFW/glfw3.h>
//
#include "inputs.h"
#include "renderer.h"
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define WIDTH 1280
#define HEIGHT 720

const char help_str[] = "Usage: %s scene.gltf";

int main(int argc, char *argv[]) {
  (void)(argc);
  GLFWwindow *window = setup_opengl(WIDTH, HEIGHT, /* disable_vsync = */ false);

  GLuint shader_program;
  RFilesWatcher shader_watcher;
  init_shader_watcher(&shader_watcher, "src/vertex.glsl", "src/renderer.glsl");

  RBuffers rb;
  setup_renderer_buffers(&rb);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  RUniforms uniforms = runiforms_new(width, height);

  unsigned int frame_counter = 0;
  double last_frame_time = glfwGetTime();

  Scene scene = {0};

  printf("Loading scene %s...\n", argv[1]);
  load_gltf_scene(&scene, argv[1]);
  if (scene.camera_count == 1) {
    uniforms.cFov = scene.camera.projection_matrix.yfov;
    uniforms.cPos = mat4_mul_vec3(scene.camera.view_matrix, uniforms.cPos);
    uniforms.cLookat =
        mat4_mul_vec3(scene.camera.view_matrix, uniforms.cLookat);

    // TODO: adjust window aspect ratio based on projection_matrix
    printf("Pos: %s\n", vec3_str(uniforms.cPos).s);
    printf("Lookat: %s\n", vec3_str(uniforms.cLookat).s);
    printf("Fov: %+0.3f\n", uniforms.cFov);
  }

  GLFWUserData *userPtr = glfwGetWindowUserPointer(window);
  YawPitch yp = inputs_from_lookat(uniforms.cPos, uniforms.cLookat);
  userPtr->yaw = yp.yaw;
  userPtr->pitch = yp.pitch;
  printf("yaw: %0.3f, pitch: %0.3f\n", yp.yaw, yp.pitch);

  RBackBuffer bb;
  setup_back_buffer(&bb, width, height);

  BVHresult b_res = build_bvh(scene.triangles, scene.triangles_count);
  scene.bvh = b_res.bvh;

  // swap primitives according to swaps_lut
  for (int t = 0; t < scene.triangles_count; ++t) {
    int swap = b_res.swaps_lut[t];
    if (t < swap) {
      Primitive tmp = scene.primitives[t];
      scene.primitives[t] = scene.primitives[swap];
      scene.primitives[swap] = tmp;
    }
  }
  free(b_res.swaps_lut);

  /* printf("Materials: %d\n", scene.mats_count); */
  /* for (int m = 0; m < scene.mats_count; ++m) { */
  /*   printf("mat %d: %f %f %f\n", m, scene.mats[m].albedo[0], */
  /*          scene.mats[m].albedo[1], scene.mats[m].albedo[2]); */
  /* } */
  /* printf("\n"); */

  printf("Loaded triangles: %d\n", scene.triangles_count);
  /* for (int t = 0; t < scene.triangles_count; ++t) { */
  /*   printf("Triangle % 3d (mat: % 2d): ", t, scene.primitives[t].mat); */
  /*   printf("%+f %+f %+f -- ", scene.triangles[t].a.x, scene.triangles[t].a.y,
   */
  /*          scene.triangles[t].a.z); */
  /*   printf("%+f %+f %+f -- ", scene.triangles[t].b.x, scene.triangles[t].b.y,
   */
  /*          scene.triangles[t].b.z); */
  /*   printf("%+f %+f %+f\n", scene.triangles[t].c.x, scene.triangles[t].c.y,
   */
  /*          scene.triangles[t].c.z); */
  /* } */
  /* printf("\n"); */

  printf("Created nodes: %d\n", scene.bvh.nodes_count);
  /* for (int i = 0; i < scene.bvh.nodes_count; ++i) { */
  /*   BVHnode node = scene.bvh.nodes[i]; */
  /*   printf("Node %d: %+f %+f %+f --- %+f %+f %+f, ", i, node.bound_min.x, */
  /*          node.bound_min.y, node.bound_min.z, node.bound_max.x, */
  /*          node.bound_max.y, node.bound_max.z); */
  /*   if (node.count == 0) */
  /*     printf("(parent) %d\n", node.first); */
  /*   else */
  /*     printf("  (leaf) %d + %d\n", node.first, node.count); */
  /* } */
  RMeshBuffers rmb = rmb_build(&scene);
  RFrameStructs rfs = {
      .uniforms = &uniforms, .rb = &rb, .back_buffer = &bb, .rmb = &rmb};

  reload_shader(&shader_program, &shader_watcher);
  bool should_reset = false;
  while (!glfwWindowShouldClose(window)) {
    if (did_shader_change(&shader_watcher)) {
      reload_shader(&shader_program, &shader_watcher);
      rmb = rmb_build(&scene);
      should_reset = true;
    }

    if (update_inputs_uniforms(window, &uniforms)) {
      should_reset = true;
    }

    // recalculate window dimensions
    int new_width, new_height;
    glfwGetFramebufferSize(window, &new_width, &new_height);
    if (new_width != width || new_height != height) {
      width = new_width;
      height = new_height;
      uniforms.iResolution[0] = width;
      uniforms.iResolution[1] = height;

      // Resize the backbuffer texture
      glBindTexture(GL_TEXTURE_2D, bb.fboTex);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                   GL_UNSIGNED_BYTE, NULL);
      glBindTexture(GL_TEXTURE_2D, 0);
      glViewport(0, 0, width, height);

      should_reset = true;
    }

    if (should_reset) {
      frame_counter = 0;
      uniforms.iFrame = 0;
      should_reset = false;
    }

    display_fps(window, &frame_counter, &last_frame_time);
    frame_counter++;

    glClear(GL_COLOR_BUFFER_BIT);
    update_frame(shader_program, window, &rfs);

    glfwPollEvents();
    ++uniforms.iFrame;
  }

  free_gl_buffers(&rb, &bb, &rmb);
  delete_file_watcher(&shader_watcher);
  glDeleteProgram(shader_program);
  glfwTerminate();

  return 0;
}
