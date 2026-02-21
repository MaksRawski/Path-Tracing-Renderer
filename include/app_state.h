#ifndef APP_STATE_H_
#define APP_STATE_H_

#include "action.h"
#include "app_state/app_state_save_image.h"
#include "input_handler.h"
#include "renderer.h"
#include "scene.h"
#include "settings.h"
#include "stats.h"
#include "window/resolution.h"
#include "window/window_events.h"

typedef struct {
  Scene scene;
  Stats stats;
  Settings settings;
  Action pending_actions;
} AppState;

inline static AppState AppState_default(void) {
  return (AppState){
      .settings = Settings_default(),
      .scene = Scene_default(),
      .stats = Stats_default(),
      .pending_actions = 0,
  };
}

inline static bool AppState_is_rendering_finished(AppState *app_state) {
  return app_state->settings.rendering_params.frames_to_render >= 0 &&
         app_state->stats.frame_number ==
             (uint32_t)app_state->settings.rendering_params.frames_to_render;
}

void AppState_restart_progressive_rendering(AppState *app_state,
                                            Renderer *renderer);
void AppState_load_scene(AppState *app_state);
void AppState_build_bvh(AppState *app_state, Arena *tmp_arena);

void AppState_handle_inputs(AppState *app_state, InputHandler *input_handler,
                            const WindowEventsData *events);

void AppState_save_image(AppState *save_image_info, GLuint fbo,
                         WindowResolution resolution);

#endif // APP_STATE_H_
