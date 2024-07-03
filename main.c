
#define DEBUG

#include <glad/gl.h>
//
#include <GLFW/glfw3.h>
//
#include "obj_parser.h"
#include "renderer.h"
//
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
  GLFWwindow *window = setup_opengl(/* disable_vsync = */ false);

  GLuint shader_program;
  FilesWatcher shader_watcher;
  RendererBuffers rb;
  setup_renderer("vertex.glsl", "renderer.glsl", &shader_program,
                 &shader_watcher, &rb);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  BackBuffer bb;
  setup_back_buffer(shader_program, &bb, width, height);

  Uniforms uniforms = {.camFov = PI / 2.0,
                       .camLookat = {0.0, 1.0, 0.0},
                       .camPos = {-5.0, 1.0, 0.0},
                       .camUp = {0.0, 1.0, 0.0},
                       .iResolution = {width, height},
                       .iFrame = 0};

  unsigned int frame_counter = 0;
  double last_frame_time = glfwGetTime();

  ModelsBuffer mb = {0};
  int suzanne_id = load_obj_model("suzanne.obj", shader_program, &mb, NULL);

  Material gold_mat = {
      .albedo = {1.0, 0.84, 0.0},
      .emissionColor = {0, 0, 0},
      .emissionStrength = 0,
      .specularComponent = 0.4,
  };
  set_material_slot(&mb, 0, &gold_mat);
  set_model_material(&mb, suzanne_id, 0);

  while (!glfwWindowShouldClose(window)) {
    bool did_reload = reload_shader(&shader_program, &shader_watcher);
    if (did_reload) {
      frame_counter = 0;
      uniforms.iFrame = 0;
      display_fps(window, &frame_counter, &last_frame_time);
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
      uniforms.iFrame = 0;
    }
    display_fps(window, &frame_counter, &last_frame_time);
    frame_counter++;

    glClear(GL_COLOR_BUFFER_BIT);
    update_frame(shader_program, window, &uniforms, &rb, &bb, &mb);

    glfwPollEvents();
    ++uniforms.iFrame;
  }

  free_gl_buffers(&rb, &bb, &mb);
  delete_file_watcher(&shader_watcher);
  glDeleteProgram(shader_program);
  glfwTerminate();

  return 0;
}
