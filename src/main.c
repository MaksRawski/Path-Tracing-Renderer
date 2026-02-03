#include "cli.h"
#include "renderer/parameters.h"
#include "stats.h"
#ifdef __linux__
#include <gtk/gtk.h>
#endif

#include "app_state.h"
#include "app_state/app_state_save_image.h"
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

  Window window =
      Window_new(WINDOW_TITLE, DESIRED_WIDTH, DESIRED_HEIGHT);
  app_state.viewport_size = Window_get_framebuffer_size(&window);

  GUIOverlay gui = GUIOverlay_new(&window);

  Renderer renderer = Renderer_new();

  while (!glfwWindowShouldClose(window.glfw_window)) {
    if (app_state.hot_reload_enabled)
      AppState_hot_reload_shaders(&app_state, &renderer);

    WindowEventsData events = Window_poll_events(&window);
    app_state.viewport_size = Window_get_framebuffer_size(&window);
    Renderer_update_focus(&renderer, &events, &window, !GUIOverlay_is_focused());

    if (app_state.gui_enabled)
      GUIOverlay_update_state(&gui, &app_state);

    // NOTE: loads a new scene if necessary
    AppState_update_scene(&app_state, &renderer);

    // NOTE: should be done after scene updating, so that camera isn't somehow
    // maintained from previous scene
    if (app_state.movement_enabled)
      AppState_update_camera(&app_state, &renderer, &events);

    // NOTE: this includes the renderer resolution
    AppState_update_renderer_parameters(&app_state, &renderer);

    AppState_display(&app_state, &renderer, &gui, &window);

    bool rendering_finished = (int)app_state.stats.frame_number ==
                              app_state.rendering_params.frames_to_render;

    if (app_state.save_after_rendering && rendering_finished)
      app_state.save_image_info.to_save = true;

    bool display_rendering_time = false;
    if (app_state.save_image_info.to_save) {
      // NOTE: this will also stop the rendering timer as requesting pixels
      // from the GPU forces synchronization, which allows for _accurate_
      // reading of rendering time (albeit with the additional time of
      // transferring pixels included). According to the OpenGL documentation
      // (https://wikis.khronos.org/opengl/Synchronization#Implicit_synchronization):
      // "attempt to read from a framebuffer to CPU memory (not to a buffer
      // object) will halt until all rendering commands affecting that
      // framebuffer have completed."
      AppState_save_image(&app_state, Renderer_get_fbo(&renderer),
                          app_state.rendering_params.rendering_resolution);
      display_rendering_time = true;
    } else if (rendering_finished &&
               app_state.stats.rendering.total_time == 0) {
      // NOTE: this reading will be slightly inaccurate as it's stopped right
      // after *queueing* all operations to the GPU and there is no guarantee
      // whatsoever whether all the frames have been rendered.
      StatsTimer_stop(&app_state.stats.rendering);
      display_rendering_time = true;
    }

    if (display_rendering_time) {
      char rendering_time_str[16];
      Stats_string_time(app_state.stats.rendering.total_time,
                        rendering_time_str, sizeof(rendering_time_str));
      printf("Rendered %d frames in %s.\n",
             app_state.rendering_params.frames_to_render, rendering_time_str);
    }

    if (rendering_finished && app_state.exit_after_rendering)
      break;
  }

  Renderer_delete(&renderer);
  GUIOverlay_delete(&gui);
  Window_delete(&window);

  return 0;
}
