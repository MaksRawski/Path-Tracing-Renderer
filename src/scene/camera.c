#include "scene/camera.h"
#include "asserts.h"
#include "rad_deg.h"
#include "vec3d.h"
#include "yawpitch.h"
#include <math.h>

// NOTE: right now this is basically assuming that up is (0,1,0)
Camera Camera_new(vec3 pos, vec3 dir, vec3 up, float fov, float focal_length,
                  float step_size_per_second, float sensitivity) {
  vec3 dir_xz_normalized = vec3_norm(vec3_new(dir.x, 0, dir.z));
  vec3 dir_normalized =
      vec3_new(dir_xz_normalized.x, dir.y, dir_xz_normalized.z);

  return (Camera){.pos = pos,
                  .dir = dir_normalized,
                  .up = up,
                  .fov_rad = fov,
                  .focal_length = focal_length,
                  .step_size_per_second = step_size_per_second,
                  .sensitivity = sensitivity};
}

Camera Camera_default(void) {
  return Camera_new(DEFAULT_CAM_POS, DEFAULT_CAM_DIR, DEFAULT_CAM_UP,
                    DEFAULT_CAM_FOV, DEFAULT_CAM_FOCAL_LENGTH,
                    DEFAULT_CAM_MOVE_SPEED_PER_SECOND,
                    DEFAULT_CAM_ROTATE_SENSITIVITY);
}

float Camera_get_fov_deg(const Camera *self) {
  return rad_to_deg(self->fov_rad);
}

void Camera_set_fov_deg(Camera *self, float deg) {
  ASSERTQ_CONDF(deg > CAMERA_FOV_MIN_DEG && deg < CAMERA_FOV_MAX_DEG, deg);
  self->fov_rad = deg_to_rad(deg);
}

bool Camera_eq(Camera a, Camera b) {
  return vec3_eq(a.pos, b.pos) && vec3_eq(a.dir, b.dir) &&
         vec3_eq(a.up, b.up) && a.fov_rad == b.fov_rad &&
         a.focal_length == b.focal_length;
}

void Camera_move_up(Camera *cam, float dir, float step_size) {
  if (dir == 0)
    return;
  cam->pos = vec3_add(cam->pos, vec3_mult(DEFAULT_CAM_UP, step_size * dir));
}

void Camera_move_forward(Camera *cam, float dir, float step_size) {
  if (dir == 0)
    return;
  vec3 move_dir = vec3_norm(vec3_mult(cam->dir, dir));
  cam->pos = vec3_add(cam->pos, vec3_mult(move_dir, step_size));
}

void Camera_move_left(Camera *cam, float dir, float step_size) {
  if (dir == 0)
    return;
  vec3 view_left = vec3_norm(vec3_cross(DEFAULT_CAM_UP, cam->dir));
  vec3 move_dir = vec3_mult(view_left, dir);
  cam->pos = vec3_add(cam->pos, vec3_mult(move_dir, step_size));
}

void Camera_move(Camera *cam, CameraTranslation translation, double step_size) {
  Camera_move_forward(cam, translation.forward, step_size);
  Camera_move_left(cam, translation.left, step_size);
  Camera_move_up(cam, translation.up, step_size);
}

double Camera__fix_yaw(double yaw) {
  yaw = fmod(yaw, 2 * M_PI);
  if (yaw < 0)
    yaw += 2 * M_PI;
  return yaw;
}

double Camera__fix_pitch(double pitch) {
  pitch = pitch < CAMERA_PITCH_MIN_RAD ? CAMERA_PITCH_MIN_RAD : pitch;
  pitch = pitch > CAMERA_PITCH_MAX_RAD ? CAMERA_PITCH_MAX_RAD : pitch;
  return pitch;
}

void Camera__fix_yaw_pitch(YawPitch *yp) {
  yp->yaw_rad = Camera__fix_yaw(yp->yaw_rad);
  yp->pitch_rad = Camera__fix_pitch(yp->pitch_rad);
}

void Camera_rotate(Camera *cam, YawPitch rotation) {
  YawPitch yp = YawPitch_from_dir(Vec3d_from_vec3(cam->dir));
  yp.yaw_rad += rotation.yaw_rad;
  yp.pitch_rad += rotation.pitch_rad;
  Camera__fix_yaw_pitch(&yp);
  cam->dir = Vec3d_to_vec3(YawPitch_to_dir(yp));
}
