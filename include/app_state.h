#ifndef APP_STATE_H_
#define APP_STATE_H_

#include "app_state/app_state_save_image.h"
#include "gui/scene_paths.h"
#include "opengl/resolution.h"
#include "opengl/scaling.h"
#include "renderer.h"
#include "renderer/parameters.h"
#include "scene.h"
#include "scene/camera.h"
#include "stats.h"

// NOTE: *_changed attributes are added for each field that can be modified
// through gui
typedef struct {
  Camera cam;
  RendererParameters rendering_params;
  Stats stats;
  ScenePaths scene_paths;
  OpenGLResolution viewport_size;
  OpenGLScalingMode scaling_mode;
  AppStateSaveImageInfo save_image_info;
  Scene scene;
  bool cam_changed, rendering_params_changed, scene_paths_changed;
  bool gui_enabled, hot_reload_enabled, save_after_rendering,
      exit_after_rendering;
} AppState;

AppState AppState_new(Camera camera, RendererParameters rendering,
                      ScenePaths scene_pahts, OpenGLResolution res,
                      OpenGLScalingMode scale_mode, Scene scene,
                      AppStateSaveImageInfo save_image_info, bool gui_enabled,
                      bool hot_reload_enabled, bool save_after_rendering,
                      bool exit_after_rendering);

AppState AppState_default(void);

void AppState_update_scene(AppState *app_state, Renderer *renderer);
void AppState_update_camera(AppState *app_state, Renderer *renderer,
                            const WindowEventsData *events);

void AppState_hot_reload_shaders(AppState *app_state, Renderer *renderer);
void AppState_update_renderer_parameters(AppState *app_state,
                                         Renderer *renderer);

#endif // APP_STATE_H_
