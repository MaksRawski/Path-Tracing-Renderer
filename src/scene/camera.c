#include "scene/camera.h"
#include "asserts.h"
#include "rad_deg.h"

// NOTE: right now this is basically assuming that up is (0,1,0)
Camera Camera_new(vec3 pos, vec3 dir, vec3 up, float fov, float focal_length) {
  vec3 dir_xz_normalized = vec3_norm(vec3_new(dir.x, 0, dir.z));
  vec3 dir_normalized =
      vec3_new(dir_xz_normalized.x, dir.y, dir_xz_normalized.z);

  return (Camera){.pos = pos,
                  .dir = dir_normalized,
                  .up = up,
                  .fov_rad = fov,
                  .focal_length = focal_length};
}

Camera Camera_default(void) {
  return Camera_new(DEFAULT_CAM_POS, DEFAULT_CAM_DIR, DEFAULT_CAM_UP,
                    DEFAULT_CAM_FOV, DEFAULT_CAM_FOCAL_LENGTH);
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
  cam->pos = vec3_add(cam->pos, vec3_mult(DEFAULT_CAM_UP, step_size * dir));
}

void Camera_move_forward(Camera *cam, float dir, float step_size) {
  vec3 move_dir = vec3_norm(vec3_mult(cam->dir, dir));
  cam->pos = vec3_add(cam->pos, vec3_mult(move_dir, step_size));
}

void Camera_move_left(Camera *cam, float dir, float step_size) {
  vec3 view_left = vec3_norm(vec3_cross(DEFAULT_CAM_UP, cam->dir));
  vec3 move_dir = vec3_mult(view_left, dir);
  cam->pos = vec3_add(cam->pos, vec3_mult(move_dir, step_size));
}
