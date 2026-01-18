#include "app_state_display.h"
#include "app_state/app_state_save_image.h"
#include "opengl/gl_call.h"

// NOTE: sets the rendering_params received from GUI in Renderer
// NOTE: renders everything that has to be rendered to the screen
void AppState_display(AppState *app_state, Renderer *renderer, GUIOverlay *gui,
                      OpenGLContext *ctx) {
  int frames_to_render = app_state->rendering_params.frames_to_render;
  unsigned int frame_number = app_state->stats.frame_number;

  GLuint renderer_fbo = Renderer_get_fbo(renderer);

  bool infinite_progressive_rendering = frames_to_render < 0;
  bool should_render_new_frame = frame_number < (unsigned int)frames_to_render;

  GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

  if (frames_to_render == 0) {
    Renderer_clear_backbuffer(renderer);
  }

  if (infinite_progressive_rendering || should_render_new_frame) {
    Renderer_render_frame(renderer, app_state->stats.frame_number++);
    // NOTE: getting to this point doesn't mean that the frame (or even the one
    // before this one) has finished rendering, as we're simply queuing commands
    // for rendering. Those times seem to only have meaning when rendering at
    // least 3 frames (on my machine).
    double render_end_time = glfwGetTime();
    double render_start_time = app_state->stats.last_frame_end_time;
    app_state->stats.last_frame_end_time = render_end_time;
    app_state->stats.last_frame_time = render_end_time - render_start_time;
  }

  OpenGLContext_display_framebuffer(
      renderer_fbo, app_state->rendering_params.rendering_resolution,
      app_state->viewport_size, app_state->scaling_mode);

  GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  if (app_state->gui_enabled)
    GUIOverlay_render_frame(gui);

  OpenGLContext_swap_buffers(ctx);
}
