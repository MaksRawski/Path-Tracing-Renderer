#include "app_state.h"
#include "file_formats/gltf.h"
#include "gui.h"
#include "opengl/gl_call.h"
#include "renderer/inputs.h"
#include "stats.h"
#include "stb_image_write.h"
#include "utils.h"
#include <stdio.h>

AppState AppState_default(void) {
  return (AppState){
      .cam = Camera_default(),
      .rendering_params = RendererParameters_default(),
      .scene_paths = ScenePath_default(),
      .viewport_size = OpenGLResolution_new(0, 0),
      .scene = Scene_default(),
      .scaling_mode = OpenGLScalingMode_FIT_CENTER,
      .save_image_info = AppStateSaveImageInfo_default(),
      .gui_enabled = true,
      .hot_reload_enabled = true,
      .save_after_rendering = false,
      .exit_after_rendering = false,
      .movement_enabled = true,
      .BVH_build_strat = BVHStrategy_Naive,
      .stats = Stats_default(),
      //
      .cam_changed = true,
      .rendering_params_changed = true,
      .scene_paths_changed = true,
      .BVH_build_strat_changed = true,
      ._renderer_focused = false,
  };
}

static void AppState__restart_progressive_rendering(AppState *app_state,
                                                    Renderer *renderer) {
  Renderer_clear_backbuffer(renderer);
  Stats_reset_rendering(&app_state->stats);
}

static void AppState__set_camera(AppState *app_state, Renderer *renderer) {
  Renderer_set_camera(renderer, app_state->cam);
  AppState__restart_progressive_rendering(app_state, renderer);
}

// NOTE: handles scene_paths_changed and BVH_strat_changed signals
void AppState_update_scene(AppState *app_state, Renderer *renderer,
                           Arena *arena) {
  SmallString *new_scene_path = &app_state->scene_paths.new_scene_path;
  SmallString *loaded_scene_path = &app_state->scene_paths.loaded_scene_path;
  bool scene_changed = false;
  bool bvh_built = false;

  if (app_state->scene_paths_changed && FilePath_exists(new_scene_path->str)) {
    app_state->scene_paths_changed = false;
    StatsTimer_start(&app_state->stats.scene_load);
    load_gltf_scene(&app_state->scene, new_scene_path->str);
    StatsTimer_stop(&app_state->stats.scene_load);
    app_state->cam = app_state->scene.camera;
    *loaded_scene_path = SmallString_new(new_scene_path->str);
    scene_changed = true;
  }

  if (scene_changed || app_state->BVH_build_strat_changed) {
    app_state->BVH_build_strat_changed = false;
    StatsTimer_start(&app_state->stats.bvh_build);
    Scene_build_blas(&app_state->scene, app_state->BVH_build_strat, arena);
    StatsTimer_stop(&app_state->stats.bvh_build);
    char bvh_build_time[16] = {0};
    Stats_string_time(app_state->stats.bvh_build.total_time, bvh_build_time,
                      sizeof(bvh_build_time));
    printf("Building BVH using the strategy '%s' took: %s\n",
           BVHStrategy_str[app_state->BVH_build_strat], bvh_build_time);
    bvh_built = true;
  }

  if (scene_changed) {
    StatsTimer_start(&app_state->stats.tlas_build);
    Scene_build_tlas(&app_state->scene, arena);
    StatsTimer_stop(&app_state->stats.tlas_build);
    char tlas_build_time[16] = {0};
    Stats_string_time(app_state->stats.tlas_build.total_time, tlas_build_time,
                      sizeof(tlas_build_time));
    printf("Building TLAS took: %s\n", tlas_build_time);
  }

  if (scene_changed || bvh_built) {
    Renderer_load_scene(renderer, &app_state->scene);
    AppState__restart_progressive_rendering(app_state, renderer);
    *loaded_scene_path = SmallString_new(new_scene_path->str);
  }
}

// NOTE: handles cam_changed signal or updates camera from keyboard + mouse
// inputs
void AppState_update_camera(AppState *app_state, Renderer *renderer,
                            const WindowEventsData *events) {
  if (!app_state->movement_enabled || Scene_is_empty(&app_state->scene) ||
      !app_state->_renderer_focused)
    return;

  // NOTE: GUI changes have a higher priority than keyboard + mouse
  if (app_state->cam_changed) {
    app_state->cam_changed = false;
    AppState__set_camera(app_state, renderer);
  } else if (Inputs_update_camera(
                 &app_state->cam, events,
                 app_state->stats.last_frame_rendering.total_time)) {
    AppState__set_camera(app_state, renderer);
  }
}

// steals or gives back mouse based on WindowEventsData
// NOTE: updates value of _renderer_focused
void AppState_update_focus(AppState *app_state, const WindowEventsData *events,
                           Window *ctx) {
  bool lmb_pressed =
      WindowEventsData_is_mouse_button_pressed(events, GLFW_MOUSE_BUTTON_1);
  bool mouse_over_renderer = !GUIOverlay_is_focused();

  if (mouse_over_renderer && lmb_pressed && !app_state->_renderer_focused) {
    app_state->_renderer_focused = true;
    Window_steal_mouse(ctx->glfw_window);
  } else if (!lmb_pressed && app_state->_renderer_focused) {
    app_state->_renderer_focused = false;
    Window_give_back_mouse(ctx->glfw_window);
  }
}

void AppState_hot_reload_shaders(AppState *app_state, Renderer *renderer) {
  if (app_state->hot_reload_enabled &&
      RendererShaders_update(&renderer->_shaders)) {
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

void AppState_post_rendering(AppState *app_state, Renderer *renderer,
                             Arena *arena) {
  bool rendering_finished = (int)app_state->stats.frame_number ==
                            app_state->rendering_params.frames_to_render;
  bool should_save_image =
      (app_state->save_after_rendering && rendering_finished) ||
      app_state->save_image_info.to_save;

  bool display_rendering_time = false;
  if (should_save_image) {

    // NOTE: this will also stop the rendering timer as requesting pixels
    // from the GPU forces synchronization, which allows for _accurate_
    // reading of rendering time (albeit with the additional time of
    // transferring pixels included). According to the OpenGL documentation
    // (https://wikis.khronos.org/opengl/Synchronization#Implicit_synchronization):
    // "attempt to read from a framebuffer to CPU memory (not to a buffer
    // object) will halt until all rendering commands affecting that
    // framebuffer have completed."
    AppState_save_image(app_state, Renderer_get_fbo(renderer),
                        app_state->rendering_params.rendering_resolution,
                        arena);
    display_rendering_time = true;
  }

  if (rendering_finished && app_state->stats.rendering.total_time == 0) {
    // NOTE: this reading will be slightly inaccurate as it's stopped right
    // after *queueing* all operations to the GPU and there is no guarantee
    // whatsoever whether all the frames have been rendered.
    StatsTimer_stop(&app_state->stats.rendering);
    display_rendering_time = true;
  }

  if (display_rendering_time) {
    char rendering_time_str[16];
    Stats_string_time(app_state->stats.rendering.total_time, rendering_time_str,
                      sizeof(rendering_time_str));
    printf("Rendered %d frames in %s.\n",
           app_state->rendering_params.frames_to_render, rendering_time_str);
  }
}

const int BYTES_PER_PIXEL = 3;
void AppState_save_image(AppState *app_state, GLuint fbo,
                         OpenGLResolution resolution, Arena *arena) {
  app_state->save_image_info.to_save = false;
  GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo));
  GL_CALL(glReadBuffer(GL_COLOR_ATTACHMENT0));
  GL_CALL(glPixelStorei(GL_PACK_ALIGNMENT, 1));

  unsigned int alloced_bytes =
      resolution.width * resolution.height * BYTES_PER_PIXEL;
  void *pixels = Arena_alloc(arena, alloced_bytes);

  GL_CALL(glReadPixels(0, 0, resolution.width, resolution.height, GL_RGB,
                       GL_UNSIGNED_BYTE, pixels));

  StatsTimer_stop(&app_state->stats.rendering);

  const int stride = resolution.width * 3;
  stbi_flip_vertically_on_write(true);
  if (stbi_write_png(app_state->save_image_info.path, resolution.width,
                     resolution.height, 3, pixels, stride)) {
    printf("Sucessfully saved image to %s\n", app_state->save_image_info.path);
  }
  GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));

  Image_add_metadata(app_state->save_image_info.path,
                     &app_state->rendering_params);
  arena->offset -= alloced_bytes;
}
