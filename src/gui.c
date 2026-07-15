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
  io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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

  const ImGuiID dockspace_id = igGetID_Str("dockspace");
  const ImGuiViewport *viewport = igGetMainViewport();
  if (igDockBuilderGetNode(dockspace_id) == NULL) {
    igDockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    igDockBuilderSetNodeSize(dockspace_id, viewport->Size);
    const ImGuiID dock_main = dockspace_id;
    const ImGuiID dock_settings = 0;
    // TODO: this one shouldn't be movable?
    igDockBuilderDockWindow("Rendering", dock_main);
    igDockBuilderDockWindow("Settings", dock_settings);
    igDockBuilderFinish(dockspace_id);
  }
  igDockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode, ImGuiWindowClass_ImGuiWindowClass());
  
  GuiSettings_draw(self, state);
}

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
