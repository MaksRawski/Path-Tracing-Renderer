#include "gui.h"

#include "input_handler.h"
#include "input_handler_action.h"
#include "scene/camera.h"
#include "window.h"
#include "window/window_events.h"
#include <float.h>

static CameraTranslation InputHandler_move_camera(const Window *window);
static YawPitch InputHandler_rotate_camera(const WindowEventsData *events);

InputHandlerAction InputHandler_update(InputHandler *self,
                                       const WindowEventsData *events) {
  InputHandler_update_focus(self);
  if (self->renderer_focused) {
    CameraTranslation translation = InputHandler_move_camera(self->window);
    YawPitch rotation = InputHandler_rotate_camera(events);
    return (InputHandlerAction){
        .type = InputHandlerActionType_CameraMoved,
        .CameraMoved = (CameraTransformation){.translation = translation,
                                              .rotation = rotation}};
  }
  return (InputHandlerAction){.type = InputHandlerActionType_Nothing};
}

void InputHandler_update_focus(InputHandler *self) {
  bool lmb_pressed =
      Window_is_mouse_button_pressed(self->window, GLFW_MOUSE_BUTTON_1);
  bool mouse_over_renderer = !GUIOverlay_is_focused();
  if (mouse_over_renderer && lmb_pressed && !self->renderer_focused) {
    self->renderer_focused = true;
    Window_steal_mouse(self->window->glfw_window);
  } else if (!lmb_pressed && self->renderer_focused) {
    self->renderer_focused = false;
    Window_give_back_mouse(self->window->glfw_window);
  }
}

static CameraTranslation InputHandler_move_camera(const Window *window) {
  CameraTranslation translation = {0};

  bool w_pressed = Window_is_key_pressed(window, GLFW_KEY_W);
  bool s_pressed = Window_is_key_pressed(window, GLFW_KEY_S);
  bool a_pressed = Window_is_key_pressed(window, GLFW_KEY_A);
  bool d_pressed = Window_is_key_pressed(window, GLFW_KEY_D);
  bool space_pressed = Window_is_key_pressed(window, GLFW_KEY_SPACE);
  bool shift_pressed = Window_is_key_pressed(window, GLFW_KEY_LEFT_SHIFT);

  if (w_pressed && !s_pressed)
    translation.forward = 1.0;
  else if (s_pressed && !w_pressed)
    translation.forward = -1.0;

  if (a_pressed && !d_pressed)
    translation.left = 1.0;
  else if (d_pressed && !a_pressed)
    translation.left = -1.0;

  if (space_pressed && !shift_pressed)
    translation.up = 1.0;
  else if (shift_pressed && !space_pressed)
    translation.up = -1.0;

  return translation;
}

static YawPitch InputHandler_rotate_camera(const WindowEventsData *events) {
  float dx = events->mouse_delta.x / events->window_size.width;
  float dy = events->mouse_delta.y / events->window_size.height;

  // NOTE: assuming mouse_delta.y grows downwards, yet pitch grows upwards
  return YawPitch_new(dx, -dy);
}
