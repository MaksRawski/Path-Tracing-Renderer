#ifndef RENDERER_INPUTS_H_
#define RENDERER_INPUTS_H_

#include "opengl/window_events.h"
#include "scene/camera.h"
#include <GLFW/glfw3.h>

bool RendererInputs_update_camera(Camera *cam, const WindowEventsData *events);
bool RendererInputs_move_camera(Camera *cam, const WindowEventsData *events);
bool RendererInputs_rotate_camera(Camera *cam, const WindowEventsData *events);

#endif // RENDERER_INPUTS_H_
