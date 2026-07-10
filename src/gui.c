#include "gui.h"
#include "asserts.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "gui/settings.h"

GUIOverlay GUIOverlay_new(Window *window) {
  GUIOverlay self = {._imgui_ctx = igCreateContext(NULL),
                     ._DEFAULT_STYLE = *igGetStyle(),
                     .ui_scale = 1.0,
                     .show_demo = false};

  // NOTE: assuming uniform scaling
  float scale;
  glfwGetWindowContentScale(window->glfw_window, &scale, NULL);
  GUIOverlay_scale(&self, scale);

  ImGuiIO *io = igGetIO_Nil();
  ImFontAtlas_AddFontFromFileTTF(io->Fonts, "fonts/Cousine/Cousine-Regular.ttf",
                                 12, NULL, NULL);

  ImGui_ImplOpenGL3_Init("#version 130");
  ImGui_ImplGlfw_InitForOpenGL(window->glfw_window, true);

  return self;
}

// updates params in place, using the provided value for display
void GUIOverlay_update_state(GUIOverlay *self, AppState *state) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  igNewFrame();
  GuiSettings_draw(self, state);
}

bool GUIOverlay_is_focused(void) { return igGetIO_Nil()->WantCaptureMouse; }

void GUIOverlay_render_frame(GUIOverlay *self) {
  UNUSED(self);
  igRender();
  ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}

void GUIOverlay_scale(GUIOverlay *self, float scale) {
  ImGuiStyle *style = igGetStyle();
  *style = self->_DEFAULT_STYLE;
  style->FontScaleDpi = scale;
  self->ui_scale = scale;
  ImGuiStyle_ScaleAllSizes(style, scale);
}

void GUIOverlay_delete(GUIOverlay *self) {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  igDestroyContext(self->_imgui_ctx);
}
