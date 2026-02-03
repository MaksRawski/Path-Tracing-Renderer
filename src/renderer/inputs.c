#include "renderer/inputs.h"
#include "scene/camera.h"
#include <float.h>

bool Inputs_update_camera(Camera *cam, const WindowEventsData *events,
                          double dt) {
  bool changed = false;

  vec3 old_cam_pos = cam->pos;
  Camera_move(cam, Inputs_move(events), cam->step_size_per_second * dt);
  changed |= !vec3_eq(old_cam_pos, cam->pos, FLT_EPSILON);

  vec3 old_cam_dir = cam->dir;
  YawPitch yp = Inputs_rotate(events);
  yp.yaw_rad *= cam->sensitivity;
  yp.pitch_rad *= cam->sensitivity;
  Camera_rotate(cam, yp);
  changed |= !vec3_eq(old_cam_dir, cam->dir, FLT_EPSILON);

  return changed;
}

CameraTranslation Inputs_move(const WindowEventsData *events) {
  CameraTranslation translation = {0};

  bool w_pressed = WindowEventsData_is_key_pressed(events, GLFW_KEY_W);
  bool s_pressed = WindowEventsData_is_key_pressed(events, GLFW_KEY_S);
  bool a_pressed = WindowEventsData_is_key_pressed(events, GLFW_KEY_A);
  bool d_pressed = WindowEventsData_is_key_pressed(events, GLFW_KEY_D);
  bool space_pressed = WindowEventsData_is_key_pressed(events, GLFW_KEY_SPACE);
  bool shift_pressed =
      WindowEventsData_is_key_pressed(events, GLFW_KEY_LEFT_SHIFT);

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

YawPitch Inputs_rotate(const WindowEventsData *events) {
  float dx = events->mouse_delta.x / events->window_size.width;
  float dy = events->mouse_delta.y / events->window_size.height;

  // NOTE: assuming mouse_delta.y grows downwards, yet pitch grows upwards
  return YawPitch_new(dx, -dy);
}
