#include "app_state_display.h"
#include "app_state.h"
#include "app_state/app_state_save_image.h"
#include "opengl/gl_call.h"
#include "window.h"

// NOTE: renders everything that has to be rendered to the screen
void AppState_render_and_display_frame(AppState *app_state, Renderer *renderer,
                                       GUIOverlay *gui, Window *window) {
  RenderingState render_state = AppState_get_rendering_state(app_state);
  GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

  if (render_state == RenderingState_RENDERING) {
    if (app_state->stats.frame_number == 0) {
      StatsTimer_start(&app_state->stats.rendering);
    }
    StatsTimer_start(&app_state->stats.last_frame_rendering);
    Renderer_render_frame(renderer, app_state->stats.frame_number++);
  }
  if (render_state == RenderingState_RENDERING ||
      render_state == RenderingState_FINISHED) {
    Window_display_framebuffer(
        Renderer_get_fbo(renderer),
        app_state->settings.rendering_params.rendering_resolution,
        Window_get_framebuffer_size(window), app_state->settings.scaling_mode);
  }

  GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  if (app_state->settings.gui_enabled)
    GUIOverlay_render_frame(gui);

  Window_swap_buffers(window);
  // NOTE: only at this point can we expect the frame to actually be rendered
  StatsTimer_stop(&app_state->stats.last_frame_rendering);
}
