#include "app_state.h"
#include "action.h"
#include "input_handler.h"
#include "opengl/gl_call.h"
#include "scene.h"
#include "scene/file_formats/gltf.h"
#include "stats.h"
#include "stb_image_write.h"
#include "utils.h"
#include <stdio.h>

void AppState_restart_progressive_rendering(AppState *app_state,
                                            Renderer *renderer) {
  Renderer_clear_backbuffer(renderer);
  Stats_reset_rendering(&app_state->stats);
}

void AppState_update_ssbo_camera(AppState *app_state, Renderer *renderer) {
  Renderer_set_camera(renderer, app_state->settings.cam);
  app_state->pending_actions |= Action_restart_rendering;
}

void AppState_load_scene(AppState *app_state) {
  StatsTimer_start(&app_state->stats.scene_load);
  load_gltf_scene(&app_state->scene, app_state->settings.scene_path.str);
  StatsTimer_stop(&app_state->stats.scene_load);
  app_state->pending_actions |= Action_update_ssbo_scene;
  app_state->pending_actions |= Action_build_bvh;

  // set the camera to the one defined in the scene
  app_state->settings.cam = app_state->scene.camera;
  app_state->pending_actions |= Action_update_ssbo_camera;
}

void AppState_build_bvh(AppState *app_state, Arena *tmp_arena) {
  StatsTimer_start(&app_state->stats.bvh_build);
  Scene_build_bvh(&app_state->scene, app_state->settings.BVH_build_strat,
                  tmp_arena);
  StatsTimer_stop(&app_state->stats.bvh_build);

  char bvh_build_time[16] = {0};
  Stats_string_time(app_state->stats.bvh_build.total_time, bvh_build_time,
                    sizeof(bvh_build_time));
  printf("Building BVH using the strategy '%s' took: %s\n",
         BVHStrategy_str[app_state->settings.BVH_build_strat], bvh_build_time);
}

void AppState_handle_inputs(AppState *app_state, InputHandler *input_handler,
                            const WindowEventsData *events) {
  InputHandlerAction input_handler_action =
      InputHandler_update(input_handler, events);
  switch (input_handler_action.type) {
  case InputHandlerActionType_CameraMoved:
    Camera_transform(&app_state->settings.cam, input_handler_action.CameraMoved,
                     app_state->settings.cam.step_size_per_second);
    break;
  case InputHandlerActionType_Nothing:
    break;
  }
}

const int BYTES_PER_PIXEL = 3;
void AppState_save_image(AppState *app_state, GLuint fbo,
                         WindowResolution resolution) {
  GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo));
  GL_CALL(glReadBuffer(GL_COLOR_ATTACHMENT0));
  GL_CALL(glPixelStorei(GL_PACK_ALIGNMENT, 1));

  void *pixels = malloc(resolution.width * resolution.height * BYTES_PER_PIXEL);

  GL_CALL(glReadPixels(0, 0, resolution.width, resolution.height, GL_RGB,
                       GL_UNSIGNED_BYTE, pixels));

  const int stride = resolution.width * 3;
  stbi_flip_vertically_on_write(true);
  if (stbi_write_png(app_state->settings.saved_image_path.str, resolution.width,
                     resolution.height, 3, pixels, stride)) {
    printf("Sucessfully saved image to %s\n",
           app_state->settings.saved_image_path.str);
  }
  GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));

  Image_add_metadata(app_state->settings.saved_image_path.str,
                     &app_state->settings.rendering_params);
  free(pixels);
}
