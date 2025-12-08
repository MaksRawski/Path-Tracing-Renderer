#ifndef GUI_H_
#define GUI_H_

#include "gui/parameters.h"
#include "opengl/context.h"
#include "opengl/window_events.h"

#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

typedef struct {
  ImGuiContext *_imgui_ctx;
  const ImGuiStyle _DEFAULT_STYLE;
  float ui_scale;
  bool show_demo;
} Gui;

Gui Gui_new(OpenGLContext *ctx);
bool Gui_is_focused(void);
void Gui_update_params(Gui *self, GuiParameters *params, WindowEventsData *events);
void Gui_render_frame(Gui *self);
void Gui_delete(Gui *self);

void Gui_scale(Gui *self, float scale);
#endif // GUI_H_
