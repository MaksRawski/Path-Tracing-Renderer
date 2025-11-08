#include "scene/camera.h"

// NOTE: right now this is basically assuming that up is (0,1,0)
Camera Camera_new(vec3 pos, vec3 dir, vec3 up, float fov, float focal_length) {
  vec3 dir_xz_normalized = vec3_norm(vec3_new(dir.x, 0, dir.z));
  vec3 dir_normalized = vec3_new(dir_xz_normalized.x, dir.y, dir_xz_normalized.z);

  return (Camera){.pos = pos,
                  .dir = dir_normalized,
                  .up = up,
                  .fov = fov,
                  .focal_length = focal_length};
}

Camera Camera_default(void) {
  return Camera_new(DEFAULT_CAM_POS, DEFAULT_CAM_DIR, DEFAULT_CAM_UP,
                    DEFAULT_CAM_FOV, DEFAULT_CAM_FOCAL_LENGTH);
}
