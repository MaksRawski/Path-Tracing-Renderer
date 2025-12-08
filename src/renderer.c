#include "renderer.h"
#include "file_path.h"
#include "gui.h"
#include "gui/parameters.h"
#include "opengl/context.h"
#include "opengl/resolution.h"
#include "opengl/window_events.h"
#include "renderer/buffers_scene.h"
#include "renderer/inputs.h"
#include "renderer/parameters.h"
#include "renderer/uniforms.h"
#include "scene/camera.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

#define WINDOW_TITLE "Path Tracing Renderer"
#define VERTEX_SHADER_PATH "src/vertex.glsl"
#define FRAGMENT_SHADER_PATH "src/renderer.glsl"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define UNUSED (void)

Renderer Renderer_new(OpenGLResolution resolution) {
  Renderer self = {
      ._shaders = RendererShaders_new(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH),
      ._buffers = RendererBuffers_new(resolution.width, resolution.height),
      ._uniforms = RendererUniforms_new(resolution.width, resolution.height),
      ._last_resolution = resolution,
      ._fps_counter = 0,
      ._last_frame_time = 0,
      ._scene_parameters = ParametersScene_default()};

  return self;
}

void Renderer_load_scene(Renderer *self, const Scene *scene) {
  RendererBuffers_set_scene(&self->_buffers, scene);
  printf("Loaded triangles: %d\n", scene->triangles_count);
  printf("Created nodes: %d\n", scene->bvh.nodes_count);
}

void Renderer_load_gltf(Renderer *self, const char *gltf_path) {
  Scene scene = Scene_load_gltf(gltf_path);
  Renderer_load_scene(self, &scene);
  Scene_delete(&scene);
}

void display_fps(GLFWwindow *window, int *frame_counter,
                 double *last_frame_time);

// NOTE: this just forces the sample collection to restart
void Renderer_restart(Renderer *self) {
  RendererUniforms_reset(&self->_uniforms);
}

Camera Renderer__get_camera(const Renderer *self) {
  return RendererBuffersScene_get_camera(&self->_buffers.scene);
}

void Renderer__set_camera(Renderer *self, Camera cam) {
  RendererBuffersScene_set_camera(&self->_buffers.scene, cam);
}

GuiParameters Renderer_get_gui_parameters(const Renderer *self) {
  Camera c = Renderer__get_camera(self);
  RendererParameters rendering = self->_uniforms._params;
  return GuiParameters_new(&c, &rendering, &self->_scene_parameters);
}

void Renderer_apply_gui_parameters(Renderer *self, GuiParameters *params) {
  Renderer__set_camera(self, params->cam);
  if (FilePath_exists(&params->scene.gui_scene_path)) {
    params->scene.loaded_scene_path =
        FilePath_from_string(params->scene.gui_scene_path.file_path.str);
    Renderer_load_gltf(self, params->scene.loaded_scene_path.file_path.str);
    self->_scene_parameters = params->scene;
  }

  // NOTE: RendererParameters are set directly in uniforms_update call, with no
  // checks whatsoever as the renderer doesn't set them
}

void Renderer_update_state(Renderer *self, const WindowEventsData *events,
                           GuiParameters *gui_parameters) {
  bool should_reset = false;
  if (RendererShaders_update(&self->_shaders)) {
    should_reset = true;
  }

  // update focus
  bool lmb_pressed =
      WindowEventsData_is_mouse_button_pressed(events, GLFW_MOUSE_BUTTON_1);

  if (!Gui_is_focused() && lmb_pressed && !self->_focused) {
    self->_focused = true;
    // HACK: we shouldn't be accessing window like this!
    OpenGLContext_steal_mouse(events->_window);
  } else if (!lmb_pressed && self->_focused) {
    self->_focused = false;
    OpenGLContext_give_back_mouse(events->_window);
  }

  // update camera
  Camera camera = Renderer__get_camera(self);
  if (!Gui_is_focused()) {
    if (RendererInputs_move_camera(&camera, events)) {
      // ignore the value given through gui if it was changed directly
      gui_parameters->cam.pos = camera.pos;
      should_reset = true;
    }
  }
  if (self->_focused) {
    if (RendererInputs_rotate_camera(&camera, events)) {
      // ignore the value given through gui if it was changed directly
      gui_parameters->cam.dir = camera.dir;
      should_reset = true;
    }
  }
  Renderer__set_camera(self, camera);

  // if gui parameters are different from what the renderer holds
  // TODO: this should be done on per parameter basis instead
  GuiParameters current_params = Renderer_get_gui_parameters(self);
  if (!GuiParameters_eq(&current_params, gui_parameters)) {
    Renderer_apply_gui_parameters(self, gui_parameters);
    should_reset = true;
  }

  RendererUniforms_update(&self->_uniforms, events->window_size,
                          gui_parameters->rendering);

  // if window got resized
  if (!OpenGLResolution_eq(self->_last_resolution, events->window_size)) {
    self->_last_resolution = events->window_size;
    should_reset = true;
  }

  // update gui_parameters
  *gui_parameters = Renderer_get_gui_parameters(self);

  if (should_reset) {
    Renderer_restart(self);
  }
}

void Renderer_render_frame(Renderer *self, OpenGLContext *ctx) {
  // resize the backbuffer on the first frame
  if (self->_uniforms._iFrame == 0) {
    glBindTexture(GL_TEXTURE_2D, self->_buffers.back.fboTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, self->_last_resolution.width,
                 self->_last_resolution.height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  glClear(GL_COLOR_BUFFER_BIT);

  // setup the program and bind the vao associated with the quad
  // and the vbo holding the vertices of the quad
  glUseProgram(self->_shaders.program);
  glBindVertexArray(self->_buffers.internal.vao);

  // render the quad to the back buffer
  glBindTexture(GL_TEXTURE_2D, self->_buffers.back.fboTex);
  glBindFramebuffer(GL_FRAMEBUFFER, self->_buffers.back.fbo);
  glActiveTexture(GL_TEXTURE0);
  glUniform1i(glGetUniformLocation(self->_shaders.program, "BackBufferTexture"),
              0);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  RendererUniforms_update_in_program(&self->_uniforms, self->_shaders.program);

  // render the quad to the screen
  glBindFramebuffer(GL_READ_FRAMEBUFFER, self->_buffers.back.fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBlitFramebuffer(
      0, 0, self->_last_resolution.width, self->_last_resolution.height, //
      0, 0, self->_last_resolution.width, self->_last_resolution.height, //
      GL_COLOR_BUFFER_BIT, GL_LINEAR);

  display_fps(ctx->window, &self->_fps_counter, &self->_last_frame_time);
  ++self->_fps_counter;
  ++self->_uniforms._iFrame;
}

void Renderer_delete(Renderer *self) {
  RendererShaders_delete(&self->_shaders);
  RendererBuffers_delete(&self->_buffers);
  RendererUniforms_delete(&self->_uniforms);
  self = NULL;
}

void display_fps(GLFWwindow *window, int *fps_counter,
                 double *last_frame_time) {
  double current_time = glfwGetTime();
  double delta_time = current_time - *last_frame_time;
  char window_title[40];

  if (delta_time >= 2.0) {
    double fps = *fps_counter / delta_time;
    sprintf(window_title, "%s [%.2f FPS]", WINDOW_TITLE, fps);
    glfwSetWindowTitle(window, window_title);
#ifdef LOG_FPS
    printf("FPS: %.2f\n", fps);
#endif
    *fps_counter = 0;
    *last_frame_time = current_time;
  }
}
