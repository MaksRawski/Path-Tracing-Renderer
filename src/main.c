#include "renderer.h"

#include "action.h"
#include "app_state.h"
#include "app_state_display.h"
#include "cli.h"
#include "input_handler.h"
#include "stats.h"
#include <stdint.h>

#ifdef __linux__
#include <gtk/gtk.h>
#endif

#define DESIRED_WIDTH 1280
#define DESIRED_HEIGHT 720
#define WINDOW_TITLE "Path Tracing Renderer"

int main(int argc, char *argv[]) {
  // pre-allocate 16MB of memory for any operations that may need it
  Arena tmp_arena = Arena_new(16 * 1024 * 1024);
  AppState app_state = AppState_default();
  handle_args(argc, argv, &app_state);

#ifdef __linux__
  // NOTE: Connects to X11 or Wayland. Necessary for native file dialog
  // creation.
  gtk_init(&argc, &argv);
#endif

  Window window = Window_new(WINDOW_TITLE, DESIRED_WIDTH, DESIRED_HEIGHT);
  GUIOverlay gui = GUIOverlay_new(&window);
  Renderer renderer = Renderer_new();
  InputHandler input_handler = InputHandler_new(&window);

  Renderer_set_params(&renderer, app_state.settings.rendering_params);

  while (!glfwWindowShouldClose(window.glfw_window)) {
    // NOTE: must poll every frame for the OS to know that this application is
    // working
    WindowEventsData events = Window_poll_events(&window);

    // === Settings ===
    if (app_state.settings.hot_reload_enabled) {
      if (RendererShaders_update(&renderer._shaders))
        app_state.pending_actions |= Action_restart_rendering;
    }

    if (app_state.settings.movement_enabled)
      AppState_handle_inputs(&app_state, &input_handler, &events);

    if (app_state.settings.gui_enabled)
      GUIOverlay_update_state(&gui, &app_state);

    // === pre-render Actions ===
    if (Action_load_scene & app_state.pending_actions)
      AppState_load_scene(&app_state);

    if (Action_build_bvh & app_state.pending_actions)
      AppState_build_bvh(&app_state, &tmp_arena);

    if (Action_update_ssbo_scene & app_state.pending_actions) {
      Renderer_load_scene(&renderer, &app_state.scene);
      app_state.pending_actions |= Action_restart_rendering;
    }

    if (Action_update_ssbo_camera & app_state.pending_actions) {
      Renderer_set_camera(&renderer, app_state.settings.cam);
      app_state.pending_actions |= Action_restart_rendering;
    }

    if (Action_update_ssbo_renderer_parameters & app_state.pending_actions) {
      Renderer_set_params(&renderer, app_state.settings.rendering_params);
      app_state.pending_actions |= Action_restart_rendering;
    }

    if (Action_restart_rendering & app_state.pending_actions) {
      Renderer_clear_backbuffer(&renderer);
      Stats_reset_rendering(&app_state.stats);
    }

    // === Rendering ===
    AppState_render_and_display_frame(&app_state, &renderer, &gui, &window);

    // === post-render Actions ===
    // if just finished rendering
    if (AppState_get_rendering_state(&app_state) == RenderingState_FINISHED &&
        app_state.stats.rendering.total_time == 0) {
      StatsTimer_stop(&app_state.stats.rendering);
      printf("Rendered %d frames in %s.\n",
             app_state.settings.rendering_params.frames_to_render,
             Stats_display(app_state.stats.rendering.total_time).str);

      if (app_state.settings.save_after_rendering)
        app_state.pending_actions |= Action_save_image;

      if (app_state.settings.exit_after_rendering)
        app_state.pending_actions |= Action_exit;
    }

    if (Action_save_image & app_state.pending_actions) {
      AppState_save_image(
          &app_state, Renderer_get_fbo(&renderer),
          app_state.settings.rendering_params.rendering_resolution, &tmp_arena);
    }

    if (Action_exit & app_state.pending_actions) {
      break;
    }

    // reset pending_actions for the next frame
    app_state.pending_actions = 0;
  }

  Renderer_delete(&renderer);
  GUIOverlay_delete(&gui);
  Window_delete(&window);

  return 0;
}
