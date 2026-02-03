#include "cli.h"
#include "renderer/parameters.h"
#include "stats.h"
#ifdef __linux__
#include <gtk/gtk.h>
#endif

#include "app_state.h"
#include "app_state_display.h"

#define DESIRED_WIDTH 1280
#define DESIRED_HEIGHT 720
#define WINDOW_TITLE "Path Tracing Renderer"

int main(int argc, char *argv[]) {
  AppState app_state = AppState_default();
  handle_args(argc, argv, &app_state);

#ifdef __linux__
  // Connects to X11 or Wayland, necessary for native file dialog creation.
  gtk_init(&argc, &argv);
#endif

  Window window = Window_new(WINDOW_TITLE, DESIRED_WIDTH, DESIRED_HEIGHT);
  app_state.viewport_size = Window_get_framebuffer_size(&window);

  GUIOverlay gui = GUIOverlay_new(&window);

  Renderer renderer = Renderer_new();

  while (!glfwWindowShouldClose(window.glfw_window)) {
    WindowEventsData events = Window_poll_events(&window);
    app_state.viewport_size = Window_get_framebuffer_size(&window);
    AppState_update_focus(&app_state, &events, &window);

    AppState_hot_reload_shaders(&app_state, &renderer);

    if (app_state.gui_enabled)
      GUIOverlay_update_state(&gui, &app_state);

    AppState_update_scene(&app_state, &renderer);
    AppState_update_camera(&app_state, &renderer, &events);
    AppState_update_renderer_parameters(&app_state, &renderer);

    AppState_display(&app_state, &renderer, &gui, &window);

    AppState_post_rendering(&app_state, &renderer);

    bool rendering_finished = (int)app_state.stats.frame_number ==
                              app_state.rendering_params.frames_to_render;
    if (app_state.exit_after_rendering && rendering_finished)
      break;
  }

  Renderer_delete(&renderer);
  GUIOverlay_delete(&gui);
  Window_delete(&window);

  return 0;
}
