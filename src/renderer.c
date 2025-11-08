#include "renderer.h"
#include "inputs.h"
#include "opengl/context.h"
#include "opengl/resolution.h"
#include "renderer/buffers_scene.h"
#include "renderer/inputs.h"
#include "renderer/uniforms.h"
#include "vec3d.h"
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
      .resolution = resolution,
      ._fps_counter = 0,
      ._last_frame_time = 0};

  return self;
}

void Renderer_load_scene(Renderer *self, const Scene *scene,
                         GLFWUserData *user_data) {
  RendererBuffers_set_scene(&self->_buffers, scene);
  user_data->renderer.yp =
      YawPitch_from_dir(Vec3d_from_vec3(scene->camera.dir));
  printf("Loaded triangles: %d\n", scene->triangles_count);
  printf("Created nodes: %d\n", scene->bvh.nodes_count);
}

void Renderer_load_gltf(Renderer *self, const char *gltf_path,
                        GLFWUserData *user_data) {
  Scene scene = Scene_load_gltf(gltf_path);
  Renderer_load_scene(self, &scene, user_data);
  Scene_delete(&scene);
}

void display_fps(GLFWwindow *window, int *frame_counter,
                 double *last_frame_time);

// NOTE: this doesn't reset the currently loaded scene,
// just forces the sample collection to restart
void Renderer_reset(Renderer *self, GLFWUserData *user_data) {
  UNUSED(user_data);
  RendererUniforms_reset(&self->_uniforms);
}

void Renderer_update_state(Renderer *self, GLFWUserData *user_data,
                           OpenGLResolution res) {
  bool should_reset = false;
  if (RendererShaders_update(&self->_shaders)) {
    should_reset = true;
  }
  should_reset |= !OpenGLResolution_eq(self->resolution, res);
  RendererUniforms_update(&self->_uniforms, res);

  // TODO: update_camera shouldn't be called here directly!
  bool new_camera =
      RendererInputs_update_camera(&self->_buffers.scene._camera, user_data);
  if (new_camera) {
    RendererBuffersScene_update_camera(&self->_buffers.scene,
                                       self->_buffers.scene._camera);
    should_reset = true;
  }

  if (user_data->renderer.resetPosition) {
    user_data->renderer.yp = YawPitch_new(0, 0);
    user_data->renderer.resetPosition = false;
    should_reset = true;
  }

  if (should_reset) {
    Renderer_reset(self, user_data);
  }
}

void Renderer_render_frame(Renderer *self, OpenGLContext *ctx) {
  // resize the backbuffer on the first frame
  if (self->_fps_counter == 0) {
    glBindTexture(GL_TEXTURE_2D, self->_buffers.back.fboTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, self->resolution.width,
                 self->resolution.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(self->_buffers.internal.vao);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

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

void Renderer_handle_key_callback(GLFWUserData *user_data, int key,
                                  int scancode, int action, int mods) {
  UNUSED(scancode);
  UNUSED(mods);
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_W) {
      user_data->renderer.movingForward = 1;
    } else if (key == GLFW_KEY_S) {
      user_data->renderer.movingForward = -1;
    }
    if (key == GLFW_KEY_A) {
      user_data->renderer.movingLeft = 1;
    } else if (key == GLFW_KEY_D) {
      user_data->renderer.movingLeft = -1;
    }
    if (key == GLFW_KEY_SPACE) {
      user_data->renderer.movingUp = 1;
    } else if (key == GLFW_KEY_C) {
      user_data->renderer.movingUp = -1;
    }
  } else if (action == GLFW_RELEASE) {
    if (key == GLFW_KEY_W || key == GLFW_KEY_S) {
      user_data->renderer.movingForward = 0;
    }
    if (key == GLFW_KEY_A || key == GLFW_KEY_D) {
      user_data->renderer.movingLeft = 0;
    }
    if (key == GLFW_KEY_SPACE || key == GLFW_KEY_C) {
      user_data->renderer.movingUp = 0;
    }
    if (key == GLFW_KEY_R) {
      user_data->renderer.resetPosition = true;
    }
  }
}

void Renderer_handle_cursor_callback(GLFWUserData *user_data, double xPos,
                                     double yPos) {
  float x = xPos - user_data->renderer.lastMouseX;
  float y = yPos - user_data->renderer.lastMouseY;

  user_data->renderer.yp.yaw += x * CURSOR_SENSITIVITY;
  for (int i = 0; i < 10 && user_data->renderer.yp.yaw >= 2 * M_PI; ++i)
    user_data->renderer.yp.yaw -= 2 * M_PI;

  for (int i = 0; i < 10 && user_data->renderer.yp.yaw <= 0; ++i)
    user_data->renderer.yp.yaw += 2 * M_PI;

  float pitch = user_data->renderer.yp.pitch - y * CURSOR_SENSITIVITY;
  if (pitch > -(M_PI / 2 - 0.05) && pitch < (M_PI / 2 - 0.05))
    user_data->renderer.yp.pitch = pitch;

  user_data->renderer.lastMouseX = xPos;
  user_data->renderer.lastMouseY = yPos;
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
