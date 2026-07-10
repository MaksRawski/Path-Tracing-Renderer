#ifndef GUI_H_
#define GUI_H_

#include "app_state.h"
#include "window.h"

#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

typedef struct {
  ImGuiContext *_imgui_ctx;
  const ImGuiStyle _DEFAULT_STYLE;
  float ui_scale;
  bool show_demo;
} GUIOverlay;

GUIOverlay GUIOverlay_new(Window *window);
bool GUIOverlay_is_focused(void);
void GUIOverlay_update_state(GUIOverlay *self, AppState *params);
void GUIOverlay_render_frame(GUIOverlay *self);
void GUIOverlay_scale(GUIOverlay *self, float scale);
void GUIOverlay_delete(GUIOverlay *self);

#endif // GUI_H_
