#ifndef RENDERER_INPUTS_H_
#define RENDERER_INPUTS_H_

#include "input_handler_action.h"
#include "window.h"

typedef struct {
  Window *window;
  bool renderer_focused;
} InputHandler;

inline static InputHandler InputHandler_new(Window *window) {
  return (InputHandler){.window = window, .renderer_focused = false};
}

InputHandlerAction InputHandler_update(InputHandler *self,
                                       const WindowEventsData *events);
void InputHandler_update_focus(InputHandler *self);

/* bool InputHandler_update_camera(Camera *cam, const Window *window, */
/*                                 const WindowEventsData *events, double dt);
 */

/* CameraTranslation InputHandler_move_camera(const Window *window); */
/* YawPitch InputHandler_rotate_camera(const WindowEventsData *events); */

#endif // RENDERER_INPUTS_H_
