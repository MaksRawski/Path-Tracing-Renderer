#include "cli.h"
#include "renderer/parameters.h"
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

  OpenGLContext ctx =
      OpenGLContext_new(WINDOW_TITLE, DESIRED_WIDTH, DESIRED_HEIGHT);
  app_state.viewport_size = OpenGLContext_update_viewport_size(&ctx);

  GUIOverlay gui = GUIOverlay_new(&ctx);

  Renderer renderer = Renderer_new();

  // TODO: what if shader fails? could we skip all the other stuff then?
  // It's not a fatal error but an error nonetheless
  while (!glfwWindowShouldClose(ctx.window)) {
    if (app_state.hot_reload_enabled)
      AppState_hot_reload_shaders(&app_state, &renderer);

    WindowEventsData events = OpenGLContext_poll_events(&ctx);
    app_state.viewport_size = OpenGLContext_update_viewport_size(&ctx);
    Renderer_update_focus(&renderer, &events, &ctx, !GUIOverlay_is_focused());

    // TODO: if window is not focused skip iteration

    if (app_state.gui_enabled)
      GUIOverlay_update_state(&gui, &app_state);

    // NOTE: loads a new scene if necessary
    AppState_update_scene(&app_state, &renderer);

    // NOTE: should be done after scene updating, so that camera isn't somehow
    // maintained from previous scene
    AppState_update_camera(&app_state, &renderer, &events);

    // NOTE: this includes the renderer resolution
    AppState_update_renderer_parameters(&app_state, &renderer);

    AppState_display(&app_state, &renderer, &gui, &ctx);

    bool rendering_finished = (int)app_state.stats.frame_number ==
                              app_state.rendering_params.frames_to_render;

    if (app_state.save_after_rendering && rendering_finished)
      app_state.save_image_info.to_save = true;

    if (app_state.save_image_info.to_save) {
      AppState_save_image(&app_state, Renderer_get_fbo(&renderer),
                          app_state.rendering_params.rendering_resolution);
    }

    if (app_state.exit_after_rendering && rendering_finished)
      break;
  }

  Renderer_delete(&renderer);
  GUIOverlay_delete(&gui);
  OpenGLContext_delete(&ctx);

  return 0;
}
