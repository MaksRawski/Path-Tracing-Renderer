#include "renderer/inputs.h"
#include "opengl/window_events.h"
#include "yawpitch.h"
#include <math.h>

#include "vec3d.h"

#define STEP_SIZE_PER_FRAME 0.05
#define CURSOR_SENSITIVITY 0.001

#define UNUSED (void)

bool RendererInputs_update_camera(Camera *cam, const WindowEventsData *events) {
  bool changed = RendererInputs_move_camera(cam, events);
  changed |= RendererInputs_rotate_camera(cam, events);
  return changed;
}

bool RendererInputs_move_camera(Camera *cam, const WindowEventsData *events) {
  bool changed = false;

  bool w_pressed = WindowEventsData_is_key_pressed(events, GLFW_KEY_W);
  bool s_pressed = WindowEventsData_is_key_pressed(events, GLFW_KEY_S);
  bool a_pressed = WindowEventsData_is_key_pressed(events, GLFW_KEY_A);
  bool d_pressed = WindowEventsData_is_key_pressed(events, GLFW_KEY_D);
  bool r_pressed = WindowEventsData_is_key_pressed(events, GLFW_KEY_R);
  bool space_pressed = WindowEventsData_is_key_pressed(events, GLFW_KEY_SPACE);
  bool shift_pressed =
      WindowEventsData_is_key_pressed(events, GLFW_KEY_LEFT_SHIFT);

  if (r_pressed) {
    *cam = Camera_default();
    return true;
  }

  float moving_forward = 0.0;
  if (w_pressed && !s_pressed)
    moving_forward = 1.0;
  else if (s_pressed && !w_pressed)
    moving_forward = -1.0;

  if (moving_forward != 0.0) {
    Camera_move_forward(cam, moving_forward, STEP_SIZE_PER_FRAME);
    changed = true;
  }

  float moving_left = 0.0;
  if (a_pressed && !d_pressed)
    moving_left = 1.0;
  else if (d_pressed && !a_pressed)
    moving_left = -1.0;

  if (moving_left != 0) {
    Camera_move_left(cam, moving_left, STEP_SIZE_PER_FRAME);
    changed = true;
  }

  float moving_up = 0.0;
  if (space_pressed && !shift_pressed)
    moving_up = 1.0;
  else if (shift_pressed && !space_pressed)
    moving_up = -1.0;

  if (moving_up != 0) {
    Camera_move_up(cam, moving_up, STEP_SIZE_PER_FRAME);
    changed = true;
  }

  return changed;
}

bool RendererInputs_rotate_camera(Camera *cam, const WindowEventsData *events) {
  float dx = events->mouse_delta.x;
  float dy = events->mouse_delta.y;
  YawPitch yp = YawPitch_from_dir(Vec3d_from_vec3(cam->dir));

  yp.yaw_rad += dx * CURSOR_SENSITIVITY;
  yp.yaw_rad = fmod(yp.yaw_rad, 2 * M_PI);
  if (yp.yaw_rad < 0)
    yp.yaw_rad += 2 * M_PI;

  yp.pitch_rad -= dy * CURSOR_SENSITIVITY;
  yp.pitch_rad =
      yp.pitch_rad < CAMERA_PITCH_MIN_RAD ? CAMERA_PITCH_MIN_RAD : yp.pitch_rad;
  yp.pitch_rad =
      yp.pitch_rad > CAMERA_PITCH_MAX_RAD ? CAMERA_PITCH_MAX_RAD : yp.pitch_rad;

  vec3 new_dir = Vec3d_to_vec3(YawPitch_to_dir(yp));
  if (!vec3_eq(cam->dir, new_dir)) {
    cam->dir = new_dir;
    return true;
  }
  return false;
}
