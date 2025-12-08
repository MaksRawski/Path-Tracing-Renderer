#ifndef RENDERER_H_
#define RENDERER_H_

#include "renderer/buffers.h"
#include "renderer/shaders.h"
#include "renderer/uniforms.h"

#include "gui/parameters.h"
#include "opengl/context.h"
#include "opengl/window_events.h"

typedef struct {
  RendererShaders _shaders;
  RendererBuffers _buffers;
  RendererUniforms _uniforms;
  // the last known resolution to render for
  OpenGLResolution _last_resolution;
  int _fps_counter;
  double _last_frame_time;
  bool _focused;
  ParametersScene _scene_parameters;
} Renderer;

// NOTE: the created window may not have the same resolution that was desired
// e.g. due to scaling
Renderer Renderer_new(OpenGLResolution real_resolution);

void Renderer_load_scene(Renderer *self, const Scene *scene);
void Renderer_load_gltf(Renderer *self, const char *gltf_path);
void Renderer_update_state(Renderer *self, const WindowEventsData *events,
                           GuiParameters *gui_parameters);
void Renderer_render_frame(Renderer *self, OpenGLContext *ctx);

GuiParameters Renderer_get_gui_parameters(const Renderer *self);

void Renderer_delete(Renderer *self);

#endif // RENDERER_H_
