#include "app_state.h"
#include "app_state/app_state_save_image.h"
#include "opengl/scaling.h"
#include "renderer.h"
#include "renderer/inputs.h"
#include "renderer/shaders.h"
#include "utils.h"

// this new is supposed to be an exhaustive constructor, i.e. should initalize
// every field
AppState AppState_new(Camera camera, RendererParameters rendering,
                      ScenePaths scene_pahts, OpenGLResolution res,
                      OpenGLScalingMode scale_mode, Scene scene,
                      AppStateSaveImageInfo save_image_info, bool gui_enabled,
                      bool hot_reload_enabled, bool save_after_rendering,
                      bool exit_after_rendering, bool movement_enabled) {
  return (AppState){.cam = camera,
                    .rendering_params = rendering,
                    .scene_paths = scene_pahts,
                    .viewport_size = res,
                    .scene = scene,
                    .scaling_mode = scale_mode,
                    .save_image_info = save_image_info,
                    .gui_enabled = gui_enabled,
                    .hot_reload_enabled = hot_reload_enabled,
                    .save_after_rendering = save_after_rendering,
                    .exit_after_rendering = exit_after_rendering,
                    .movement_enabled = movement_enabled,
                    .cam_changed = true,
                    .rendering_params_changed = true,
                    .scene_paths_changed = true};
}

AppState AppState_default(void) {
  return AppState_new(
      Camera_default(), RendererParameters_default(), ScenePath_default(),
      OpenGLResolution_new(0, 0), OpenGLScalingMode_FIT_CENTER, Scene_empty(),
      AppStateSaveImageInfo_default(), true, true, false, false, true);
}

void AppState__restart_progressive_rendering(AppState *app_state,
                                             Renderer *renderer) {
  Renderer_clear_backbuffer(renderer);
  Stats_reset(&app_state->stats);
}

void AppState__set_camera(AppState *app_state, Renderer *renderer) {
  Renderer_set_camera(renderer, app_state->cam);
  AppState__restart_progressive_rendering(app_state, renderer);
}

// NOTE: handles scene_paths_changed signal and loads the scene if appropriate
void AppState_update_scene(AppState *app_state, Renderer *renderer) {
  SmallString *new_scene_path = &app_state->scene_paths.new_scene_path;
  SmallString *loaded_scene_path = &app_state->scene_paths.loaded_scene_path;

  if (app_state->scene_paths_changed && FilePath_exists(new_scene_path->str)) {
    app_state->scene_paths_changed = false;
    app_state->scene = Scene_load_gltf(new_scene_path->str);
    app_state->cam = app_state->scene.camera;
    Renderer_load_scene(renderer, &app_state->scene);
    AppState__restart_progressive_rendering(app_state, renderer);
    *loaded_scene_path = SmallString_new(new_scene_path->str);
  }
}

// NOTE: handles cam_changed signal or updates camera from keyboard + mouse
// inputs
void AppState_update_camera(AppState *app_state, Renderer *renderer,
                            const WindowEventsData *events) {
  if (Scene_is_empty(&app_state->scene))
    return;
  // NOTE: GUI changes have a higher priority than keyboard + mouse
  if (app_state->cam_changed) {
    app_state->cam_changed = false;
    AppState__set_camera(app_state, renderer);
  } else if (Inputs_update_camera(&app_state->cam, events,
                                  /* allow camera rotation if */
                                  Renderer_is_focused(renderer))) {
    AppState__set_camera(app_state, renderer);
  }
}

void AppState_hot_reload_shaders(AppState *app_state, Renderer *renderer) {
  if (RendererShaders_update(&renderer->_shaders)) {
    AppState__restart_progressive_rendering(app_state, renderer);
  }
}

void AppState_update_renderer_parameters(AppState *app_state,
                                         Renderer *renderer) {
  if (app_state->rendering_params_changed) {
    app_state->rendering_params_changed = false;
    Renderer_set_params(renderer, app_state->rendering_params);
    AppState__restart_progressive_rendering(app_state, renderer);
  }
}
