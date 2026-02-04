#ifndef APP_STATE_DISPLAY_H_
#define APP_STATE_DISPLAY_H_

#include "app_state.h"
#include "gui.h"

void AppState_display(AppState *app_state, Renderer *renderer, GUIOverlay *gui,
                      Window *window);

#endif // APP_STATE_DISPLAY_H_
