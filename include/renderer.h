#ifndef RENDERER_H_
#define RENDERER_H_

#include "renderer/buffers.h"
#include "renderer/shaders.h"
#include "renderer/uniforms.h"

#include "opengl/context.h"

typedef struct {
  RendererShaders _shaders;
  RendererBuffers _buffers;
  RendererUniforms _uniforms;
  // the last known resolution to render for
  OpenGLResolution resolution;
  int _fps_counter;
  double _last_frame_time;
} Renderer;

// NOTE: the created window may not have the same resolution that was desired
// e.g. due to scaling
Renderer Renderer_new(OpenGLResolution real_resolution);
void Renderer_load_scene(Renderer *self, const Scene *scene,
                         GLFWUserData *user_data);
void Renderer_load_gltf(Renderer *self, const char *gltf_path,
                        GLFWUserData *user_data);
void Renderer_update_state(Renderer *self, GLFWUserData *user_data,
                           OpenGLResolution res);
void Renderer_render_frame(Renderer *self, OpenGLContext *ctx);
void Renderer_delete(Renderer *self);

void Renderer_handle_key_callback(GLFWUserData *user_data, int key,
                                  int scancode, int action, int mods);
void Renderer_handle_cursor_callback(GLFWUserData *user_data, double xPos,
                                     double yPos);

#endif // RENDERER_H_
