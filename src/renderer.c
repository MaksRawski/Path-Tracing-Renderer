#include "renderer.h"
#include "opengl/gl_call.h"
#include "opengl/resolution.h"
#include "renderer/buffers/back.h"
#include "renderer/buffers/parameters_buffer.h"
#include "renderer/buffers_scene.h"
#include "renderer/parameters.h"
#include "scene/camera.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

#if defined(__linux__)
#define PATH_SEPARATOR "/"
#elif defined(_WIN32)
#define PATH_SEPARATOR "\\"
#endif

#define VERTEX_SHADER_PATH "shaders" PATH_SEPARATOR "vertex.glsl"
#define FRAGMENT_SHADER_PATH "shaders" PATH_SEPARATOR "renderer.glsl"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Renderer Renderer_new(void) {
  Renderer self = {
      ._shaders = RendererShaders_new(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH),
      ._buffers = RendererBuffers_new(),
      ._res = OpenGLResolution_new(1280, 720),
      ._focused = false};

  return self;
}

void Renderer_load_scene(Renderer *self, const Scene *scene) {
  RendererBuffers_set_scene(&self->_buffers, scene);
  printf("Loaded triangles: %d\n", scene->triangles_count);
  printf("Created nodes: %d\n", scene->bvh.nodes_count);
}

void Renderer_set_camera(Renderer *self, Camera cam) {
  RendererBuffersScene_set_camera(&self->_buffers.scene, cam);
}

void Renderer_set_params(Renderer *self, RendererParameters params) {
  self->_res = params.rendering_resolution;
  // update OpenGL's window coordinates
  GL_CALL(glViewport(0, 0, self->_res.width, self->_res.height));
  RendererParametersBuffer_set(&self->_buffers.parameters, &params);
}

void Renderer_clear_backbuffer(Renderer *self) {
  RendererBuffersBack_resize(&self->_buffers.back, self->_res);
}

void Renderer_set_focused(Renderer *self, bool focused) {
  self->_focused = focused;
}

bool Renderer_is_focused(const Renderer *self) { return self->_focused; }

// NOTE: steals or gives back mouse based on the WindowEventsData
void Renderer_update_focus(Renderer *renderer, const WindowEventsData *events,
                           Window *ctx, bool mouse_over_renderer) {
  bool lmb_pressed =
      WindowEventsData_is_mouse_button_pressed(events, GLFW_MOUSE_BUTTON_1);

  if (mouse_over_renderer && lmb_pressed && !Renderer_is_focused(renderer)) {
    Renderer_set_focused(renderer, true);
    Window_steal_mouse(ctx->glfw_window);
  } else if (!lmb_pressed && Renderer_is_focused(renderer)) {
    Renderer_set_focused(renderer, false);
    Window_give_back_mouse(ctx->glfw_window);
  }
}

GLuint Renderer_get_fbo(const Renderer *self) {
  return self->_buffers.back.fbo;
}

void Renderer_render_frame(const Renderer *self, unsigned int frame_number) {
  // setup the program and bind the vao associated with the quad
  // and the vbo holding the vertices of the quad
  GL_CALL(glUseProgram(self->_shaders.program));
  GL_CALL(
      glUniform1i(glGetUniformLocation(self->_shaders.program, "frame_number"),
                  frame_number));
  GL_CALL(glBindVertexArray(self->_buffers.internal.vao));

  GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, self->_buffers.back.fbo));
  GL_CALL(glBindTexture(GL_TEXTURE_2D, self->_buffers.back.fboTex));
  GL_CALL(glActiveTexture(GL_TEXTURE0));
  GL_CALL(glUniform1i(
      glGetUniformLocation(self->_shaders.program, "BackBufferTexture"), 0));
  GL_CALL(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
}

void Renderer_delete(Renderer *self) {
  RendererShaders_delete(&self->_shaders);
  RendererBuffers_delete(&self->_buffers);
  self = NULL;
}
