#ifndef APP_STATE_H_
#define APP_STATE_H_

#include "app_state/app_state_save_image.h"
#include "renderer.h"
#include "scene.h"
#include "settings.h"
#include "stats.h"
#include "window.h"
#include "window/resolution.h"
#include "window/scaling.h"
#include "window/window_events.h"

typedef struct {
  Stats stats;
  WindowResolution viewport_size;
  WindowScalingMode scaling_mode;
  AppStateSaveImageInfo save_image_info;
  Scene scene;
  Settings settings;
  bool _renderer_focused;
} AppState;

AppState AppState_default(void);

void AppState_update_scene(AppState *app_state, Renderer *renderer, Arena *tmp_arena);
void AppState_update_camera(AppState *app_state, Renderer *renderer,
                            const WindowEventsData *events);
void AppState_update_focus(AppState *app_state, const WindowEventsData *events,
                           Window *window);

void AppState_hot_reload_shaders(AppState *app_state, Renderer *renderer);
void AppState_update_renderer_parameters(AppState *app_state,
                                         Renderer *renderer);

void AppState_post_rendering(AppState *app_state, Renderer *renderer);
void AppState_save_image(AppState *save_image_info, GLuint fbo,
                         WindowResolution resolution);

#endif // APP_STATE_H_
