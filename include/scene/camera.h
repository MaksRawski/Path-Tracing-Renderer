#ifndef CAMERA_H_
#define CAMERA_H_

#include "vec3.h"
#include "yawpitch.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEFAULT_CAM_POS vec3_new(0, 0, 0)
#define DEFAULT_CAM_DIR vec3_new(0, 0, -1)
#define DEFAULT_CAM_UP vec3_new(0, 1, 0)
#define DEFAULT_CAM_FOV (M_PI / 4.0)
#define DEFAULT_CAM_FOCAL_LENGTH 20.0

#define CAMERA_FOCAL_LENGTH_MIN 0.1
#define CAMERA_FOCAL_LENGTH_MAX 50

#define CAMERA_FOV_MIN_RAD 0.001
#define CAMERA_FOV_MAX_RAD (M_PI - CAMERA_FOV_MIN_RAD)

#define CAMERA_PITCH_MAX_RAD (M_PI / 2 - 0.05)
#define CAMERA_PITCH_MIN_RAD -CAMERA_PITCH_MAX_RAD

#define CAMERA_PITCH_MIN_DEG (CAMERA_PITCH_MIN_RAD / M_PI * 180.0)
#define CAMERA_PITCH_MAX_DEG (CAMERA_PITCH_MAX_RAD / M_PI * 180.0)

#define CAMERA_FOV_MIN_DEG (CAMERA_FOV_MIN_RAD / M_PI * 180.0)
#define CAMERA_FOV_MAX_DEG (CAMERA_FOV_MAX_RAD / M_PI * 180.0)

typedef struct {
  vec3 pos;
  vec3 dir;
  vec3 up;
  float fov_rad;
  float focal_length;
} Camera;

typedef enum {
  CameraMovementDirection_OPPOSITE = -1,
  CameraMovementDirection_NONE = 0,
  CameraMovementDirection_TOWARDS = 1,
} CameraMovementDirection;

// translation but relative to the camera's direction
typedef struct {
  CameraMovementDirection forward;
  CameraMovementDirection left;
  CameraMovementDirection up;
} CameraTranslation;

Camera Camera_new(vec3 pos, vec3 dir, vec3 up, float fov, float focal_length);

float Camera_get_fov_deg(const Camera *self);
void Camera_set_fov_deg(Camera *self, float deg);

void Camera_move_up(Camera *cam, float dir, float step_size);
void Camera_move_forward(Camera *cam, float dir, float step_size);
void Camera_move_left(Camera *cam, float dir, float step_size);

void Camera_move(Camera *cam, CameraTranslation translation, double step_size);
void Camera_rotate(Camera *cam, YawPitch rotation);

Camera Camera_default(void);

#endif // CAMERA_H_
