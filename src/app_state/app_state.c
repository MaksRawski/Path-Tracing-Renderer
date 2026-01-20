#include "app_state.h"
#include "renderer/inputs.h"
#include "stats.h"
#include "utils.h"

AppState AppState_default(void) {
  return (AppState){
      .cam = Camera_default(),
      .rendering_params = RendererParameters_default(),
      .scene_paths = ScenePath_default(),
      .viewport_size = OpenGLResolution_new(0, 0),
      .scene = Scene_empty(),
      .scaling_mode = OpenGLScalingMode_FIT_CENTER,
      .save_image_info = AppStateSaveImageInfo_default(),
      .gui_enabled = true,
      .hot_reload_enabled = true,
      .save_after_rendering = false,
      .exit_after_rendering = false,
      .movement_enabled = true,
      .BVH_build_strat = FindBestSplitFn_Variants_Naive,
      //
      .cam_changed = true,
      .rendering_params_changed = true,
      .scene_paths_changed = true,
      .BVH_build_strat_changed = true,
  };
}

void AppState__restart_progressive_rendering(AppState *app_state,
                                             Renderer *renderer) {
  Renderer_clear_backbuffer(renderer);
  Stats_reset_rendering(&app_state->stats);
}

void AppState__set_camera(AppState *app_state, Renderer *renderer) {
  Renderer_set_camera(renderer, app_state->cam);
  AppState__restart_progressive_rendering(app_state, renderer);
}

// NOTE: handles scene_paths_changed and BVH_strat_changed signals
void AppState_update_scene(AppState *app_state, Renderer *renderer) {
  SmallString *new_scene_path = &app_state->scene_paths.new_scene_path;
  SmallString *loaded_scene_path = &app_state->scene_paths.loaded_scene_path;
  bool scene_changed = false;

  if (app_state->scene_paths_changed && FilePath_exists(new_scene_path->str)) {
    app_state->scene_paths_changed = false;
    StatsTimer_start(&app_state->stats.scene_load);
    app_state->scene = Scene_load_gltf(new_scene_path->str);
    StatsTimer_stop(&app_state->stats.scene_load);
    app_state->cam = app_state->scene.camera;
    *loaded_scene_path = SmallString_new(new_scene_path->str);
    scene_changed = true;
  }

  if (app_state->BVH_build_strat_changed) {
    app_state->BVH_build_strat_changed = false;
    StatsTimer_start(&app_state->stats.bvh_build);
    Scene_build_bvh(&app_state->scene, app_state->BVH_build_strat);
    StatsTimer_stop(&app_state->stats.bvh_build);
    scene_changed = true;
  }

  if (scene_changed) {
    Renderer_load_scene(renderer, &app_state->scene);
    AppState__restart_progressive_rendering(app_state, renderer);
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
