#include <glad/gl.h>
//
#include <GLFW/glfw3.h>
//
#include "renderer.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
  GLFWwindow *window = setup_opengl(/* disable_vsync =*/false);

  GLuint shader_program;
  int shader_watcher_fd;
  RendererBuffers rb;
  setup_renderer(&shader_program, &shader_watcher_fd, &rb);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  BackBuffer bb;
  setup_back_buffer(&bb, width, height);

  Uniforms uniforms = {.backBufferTexture = bb.fbo_texture,
                       .camFov = PI / 2.0,
                       .camLookat = {0.0, 1.0, 0.0},
                       .camPos = {-5.0, 1.0, 0.0},
                       .camUp = {0.0, 1.0, 0.0},
                       .iResolution = {width, height},
                       .iFrame = 0};

  // BUG: for some reason i get a segfault if i remove this line
  // even though i don't reference it
  char window_title[40];
  unsigned int frame_counter = 0;
  double last_frame_time = glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    bool did_reload =
        reload_shader(shader_watcher_fd, &shader_program, "renderer.glsl");
    if (did_reload) {
      frame_counter = 0;
      uniforms.iFrame = 0;
      display_fps(window, frame_counter, &last_frame_time);
    }

    // recalculate window dimensions
    int new_width, new_height;
    glfwGetFramebufferSize(window, &new_width, &new_height);
    if (new_width != width || new_height != height) {
      width = new_width;
      height = new_height;

      // Resize the backbuffer texture
      glBindTexture(GL_TEXTURE_2D, bb.fbo_texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                   GL_UNSIGNED_BYTE, NULL);
      glBindTexture(GL_TEXTURE_2D, 0);

      glViewport(0, 0, width, height);
      uniforms.iFrame = 0;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    display_fps(window, frame_counter, &last_frame_time);

    update_frame(shader_program, window, &uniforms, &rb, &bb);
    ++frame_counter;

    glfwPollEvents();
    ++uniforms.iFrame;
  }

  free_gl_buffers(&rb, &bb);
  glDeleteProgram(shader_program);
  glfwTerminate();
  close(shader_watcher_fd);

  return 0;
}
