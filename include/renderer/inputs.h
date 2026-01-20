#ifndef RENDERER_INPUTS_H_
#define RENDERER_INPUTS_H_

#include "opengl/window_events.h"
#include "scene/camera.h"
/* #include "yawpitch.h" */

bool Inputs_update_camera(Camera *cam, const WindowEventsData *events,
                          double dt, bool allow_rotate);

CameraTranslation Inputs_move(const WindowEventsData *events);
YawPitch Inputs_rotate(const WindowEventsData *events);

#endif // RENDERER_INPUTS_H_
