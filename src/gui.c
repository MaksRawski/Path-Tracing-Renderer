#include "gui.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "gui/settings.h"

#define UNUSED (void)

// NOTE: needs context to set rendering user data? I HOPE NOT
Gui Gui_new(OpenGLContext *ctx) {

  Gui self = {._imgui_ctx = igCreateContext(NULL),
              ._DEFAULT_STYLE = *igGetStyle(),
              .ui_scale = 1.0,
              .show_demo = false};

  // NOTE: assuming uniform scaling
  float scale;
  glfwGetWindowContentScale(ctx->window, &scale, NULL);
  Gui_scale(&self, scale);

  ImGuiIO *io = igGetIO_Nil();
  ImFontAtlas_AddFontFromFileTTF(
      io->Fonts, "lib/cimgui/imgui/misc/fonts/Cousine-Regular.ttf", 12, NULL,
      NULL);

  ImGui_ImplOpenGL3_Init("#version 130");
  ImGui_ImplGlfw_InitForOpenGL(ctx->window, true);

  return self;
}

// updates params in place, using the provided value for display
void Gui_update_params(Gui *self, GuiParameters *params,
                       WindowEventsData *events) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  igNewFrame();
  GuiSettings_draw(self, params, events);
}

bool Gui_is_focused(void) { return igGetIO_Nil()->WantCaptureMouse; }

void Gui_render_frame(Gui *self) {
  UNUSED(self);
  igRender();
  ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}

void Gui_delete(Gui *self) {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  igDestroyContext(self->_imgui_ctx);
  self = NULL;
}

void Gui_scale(Gui *self, float scale) {
  ImGuiStyle *style = igGetStyle();
  *style = self->_DEFAULT_STYLE;
  style->FontScaleDpi = scale;
  self->ui_scale = scale;
  ImGuiStyle_ScaleAllSizes(style, scale);
}
