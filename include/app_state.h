#ifndef APP_STATE_H_
#define APP_STATE_H_

#include "action.h"
#include "app_state/app_state_save_image.h"
#include "input_handler.h"
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

typedef enum {
  RenderingState_NOT_RENDERING,
  RenderingState_RENDERING,
  RenderingState_FINISHED,
} RenderingState;

RenderingState AppState_get_rendering_state(const AppState *app_state);

void AppState_load_scene(AppState *app_state);
void AppState_build_bvh(AppState *app_state, Arena *tmp_arena);

void AppState_handle_inputs(AppState *app_state, InputHandler *input_handler,
                            const WindowEventsData *events);

void AppState_save_image(AppState *app_state, GLuint fbo,
                         WindowResolution resolution, Arena *tmp_arena);

#endif // APP_STATE_H_
