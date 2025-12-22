#include "app_state.h"
#include "gui.h"
#include "opengl/scaling.h"
#include "renderer/inputs.h"
#include "opengl/gl_call.h"
#include "renderer.h"
#include "renderer/buffers_scene.h"
#include "renderer/shaders.h"


// this new is supposed to be an exhaustive constructor, i.e. should initalize
// every field
AppState AppState_new(Camera camera, RendererParameters rendering,
                      ScenePaths scene_pahts, OpenGLResolution res,
                      OpenGLScalingMode stretch_mode, Scene scene) {
  return (AppState){.cam = camera,
                    .rendering_params = rendering,
                    .scene_paths = scene_pahts,
                    .viewport_size = res,
                    .scene = scene,
                    .scaling_mode = stretch_mode,
                    .cam_changed = true,
                    .rendering_params_changed = true,
                    .scene_paths_changed = true};
}

AppState AppState_default(void) {
  return AppState_new(Camera_default(), RendererParameters_default(),
                      ScenePath_default(), OpenGLResolution_new(0, 0),
                      OpenGLScalingMode_FIT_CENTER, Scene_empty());
}

void AppState__restart_progressive_rendering(AppState *app_state, Renderer *renderer) {
  Renderer_clear_backbuffer(renderer);
  app_state->stats.frame_number = 0;
}

void AppState__set_camera(AppState *app_state, Renderer *renderer) {
  Renderer_set_camera(renderer, app_state->cam);
  AppState__restart_progressive_rendering(app_state, renderer);
}

// NOTE: handles scene_paths_changed signal and loads the scene if appropriate
void AppState_update_scene(AppState *app_state, Renderer *renderer) {
  FilePath *new_scene_path = &app_state->scene_paths.new_scene_path;
  FilePath *loaded_scene_path = &app_state->scene_paths.loaded_scene_path;

  if (app_state->scene_paths_changed && FilePath_exists(*new_scene_path)) {
    app_state->scene_paths_changed = false;
    app_state->scene = Scene_load_gltf(new_scene_path->file_path.str);
    app_state->cam = app_state->scene.camera;
    Renderer_load_scene(renderer, &app_state->scene);
    AppState__restart_progressive_rendering(app_state, renderer);
    FilePath_replace_copy(loaded_scene_path, new_scene_path->file_path.str);
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
    // HACK: forces setting of the uniform values in the new program as well as
    // restarts progressive rendering
    app_state->rendering_params_changed = true;
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
