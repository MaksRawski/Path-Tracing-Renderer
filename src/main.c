#include "renderer.h"
#ifdef __linux__
#include <gtk/gtk.h>
#endif

#include "gui.h"
#include <GLFW/glfw3.h>

#define DESIRED_WIDTH 1280
#define DESIRED_HEIGHT 720
#define WINDOW_TITLE "Path Tracing Renderer"

int main(int argc, char *argv[]) {
#ifdef __linux__
  // Connect to X11 or Wayland, necessary for native file dialog creation
  gtk_init(&argc, &argv);
#endif

  OpenGLContext ctx =
      OpenGLContext_new(WINDOW_TITLE, DESIRED_WIDTH, DESIRED_HEIGHT);

  Gui gui = Gui_new(&ctx);

  Renderer renderer = Renderer_new(OpenGLContext_update_viewport_size(&ctx));
  if (argc == 2)
    Renderer_load_gltf(&renderer, argv[1]);

  GuiParameters gui_parameters = Renderer_get_gui_parameters(&renderer);

  while (!glfwWindowShouldClose(ctx.window)) {
    WindowEventsData events = OpenGLContext_poll_events(&ctx);

    Gui_update_params(&gui, &gui_parameters, &events);
    Renderer_update_state(&renderer, &events, &gui_parameters);

    Renderer_render_frame(&renderer, &ctx);
    Gui_render_frame(&gui);

    OpenGLContext_swap_buffers(&ctx);
  }

  Renderer_delete(&renderer);
  Gui_delete(&gui);
  OpenGLContext_delete(&ctx);

  return 0;
}
