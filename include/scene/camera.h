#ifndef CAMERA_H_
#define CAMERA_H_

#include "vec3.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEFAULT_CAM_POS vec3_new(0, 0, 0)
#define DEFAULT_CAM_DIR vec3_new(0, 0, -1)
#define DEFAULT_CAM_UP vec3_new(0, 1, 0)
#define DEFAULT_CAM_FOV M_PI / 4.0
#define DEFAULT_CAM_FOCAL_LENGTH 20.0

typedef struct {
  vec3 pos;
  vec3 dir;
  vec3 up;
  float fov;
  float focal_length;
} Camera;

Camera Camera_new(vec3 pos, vec3 dir, vec3 up, float fov, float focal_length);
Camera Camera_default(void);

#endif // CAMERA_H_
