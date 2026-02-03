#include "app_state_display.h"
#include "app_state/app_state_save_image.h"
#include "opengl/gl_call.h"

// NOTE: sets the rendering_params received from GUI in Renderer
// NOTE: renders everything that has to be rendered to the screen
void AppState_display(AppState *app_state, Renderer *renderer, GUIOverlay *gui,
                      Window *window) {
  GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

  if (!Scene_is_empty(&app_state->scene)) {
    int frames_to_render = app_state->rendering_params.frames_to_render;
    unsigned int frame_number = app_state->stats.frame_number;
    GLuint renderer_fbo = Renderer_get_fbo(renderer);

    bool infinite_progressive_rendering = frames_to_render < 0;
    bool should_render_new_frame =
        infinite_progressive_rendering ||
        frame_number < (unsigned int)frames_to_render;

    if (frames_to_render == 0) {
      Renderer_clear_backbuffer(renderer);
    }

    if (should_render_new_frame) {
      StatsTimer_start(&app_state->stats.last_frame_rendering);
      Renderer_render_frame(renderer, app_state->stats.frame_number++);
    }

    Window_display_framebuffer(
        renderer_fbo, app_state->rendering_params.rendering_resolution,
        app_state->viewport_size, app_state->scaling_mode);
  }

  if (app_state->gui_enabled) {
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GUIOverlay_render_frame(gui);
  }

  Window_swap_buffers(window);

  // NOTE: getting to this point doesn't necessarily mean that the frame (or
  // even the one before this one) has finished rendering, as we're simply
  // queuing commands for rendering. Those times only seem somewhat
  // reasonable when at least 3 frames have been queued (on my machine).
  // According to OpenGL documentation: "Swapping the back and front buffers
  // on the Default Framebuffer may cause some form of synchronization
  // (though the actual moment of synchronization event may be delayed until
  // later GL commands), if there are still commands affecting the default
  // framebuffer that have not yet completed. Swapping buffers only
  // technically needs to sync to the last command that affects the default
  // framebuffer, but it may perform a full glFinish."
  StatsTimer_stop(&app_state->stats.last_frame_rendering);
}
