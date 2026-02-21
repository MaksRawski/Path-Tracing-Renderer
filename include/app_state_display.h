#ifndef APP_STATE_DISPLAY_H_
#define APP_STATE_DISPLAY_H_

#include "app_state.h"
#include "gui.h"
#include "renderer.h"

void AppState_render_and_display_frame(AppState *app_state, Renderer *renderer,
                                       GUIOverlay *gui, Window *window);

#endif // APP_STATE_DISPLAY_H_
