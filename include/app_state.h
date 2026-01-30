#ifndef APP_STATE_H_
#define APP_STATE_H_

#include "app_state/app_state_save_image.h"
#include "gui/scene_paths.h"
#include "opengl/resolution.h"
#include "opengl/scaling.h"
#include "renderer.h"
#include "renderer/parameters.h"
#include "scene.h"
#include "scene/bvh/strategies.h"
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
  BVHStrategy BVH_build_strat;
  bool cam_changed, rendering_params_changed, scene_paths_changed,
      BVH_build_strat_changed;
  bool gui_enabled, hot_reload_enabled, save_after_rendering,
      exit_after_rendering, movement_enabled;
} AppState;

AppState AppState_default(void);

void AppState_update_scene(AppState *app_state, Renderer *renderer);
void AppState_update_camera(AppState *app_state, Renderer *renderer,
                            const WindowEventsData *events);

void AppState_hot_reload_shaders(AppState *app_state, Renderer *renderer);
void AppState_update_renderer_parameters(AppState *app_state,
                                         Renderer *renderer);

void AppState_save_image(AppState *save_image_info, GLuint fbo,
                         OpenGLResolution resolution);

#endif // APP_STATE_H_
