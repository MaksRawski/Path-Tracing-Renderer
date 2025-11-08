#include "renderer/inputs.h"
#include "vec3d.h"
#include <stdio.h>

#define STEP_SIZE_PER_FRAME 0.05

void moveUp(Camera *cam, float dir);
void moveForward(Camera *cam, float dir);
void moveLeft(Camera *cam, float dir);

bool RendererInputs_update_camera(Camera *cam, const GLFWUserData *user_data) {
  bool changed = false;
  if (user_data->renderer.movingForward != 0) {
    moveForward(cam, (float)user_data->renderer.movingForward);
    changed = true;
  }
  if (user_data->renderer.movingLeft != 0) {
    moveLeft(cam, (float)user_data->renderer.movingLeft);
    changed = true;
  }
  if (user_data->renderer.movingUp != 0) {
    moveUp(cam, (float)user_data->renderer.movingUp);
    changed = true;
  }
  vec3 new_dir = Vec3d_to_vec3(YawPitch_to_dir(user_data->renderer.yp));
  if (!vec3_eq(cam->dir, new_dir)) {
    printf("old dir: %s, new_dir: %s\n", vec3_str(cam->dir).s, vec3_str(new_dir).s);
    cam->dir = new_dir;
    changed = true;
  }

  return changed;
}

void moveUp(Camera *cam, float dir) {
  cam->pos =
      vec3_add(cam->pos, vec3_mult(DEFAULT_CAM_UP, STEP_SIZE_PER_FRAME * dir));
}

void moveForward(Camera *cam, float dir) {
  vec3 move_dir = vec3_norm(vec3_mult(cam->dir, dir));
  cam->pos = vec3_add(cam->pos, vec3_mult(move_dir, STEP_SIZE_PER_FRAME));
}

void moveLeft(Camera *cam, float dir) {
  vec3 view_left = vec3_norm(vec3_cross(DEFAULT_CAM_UP, cam->dir));
  vec3 move_dir = vec3_mult(view_left, dir);
  cam->pos = vec3_add(cam->pos, vec3_mult(move_dir, STEP_SIZE_PER_FRAME));
}
