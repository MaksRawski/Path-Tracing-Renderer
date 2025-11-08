#ifndef RENDERER_USER_DATA_H_
#define RENDERER_USER_DATA_H_
#include "yawpitch.h"
#include <stdbool.h>

// data that Renderer needs to store in GLFWUserData
typedef struct {
  // 1 means true, -1 the opposite direction and 0 no
  int movingForward, movingLeft, movingUp;
  YawPitch yp;
  float lastMouseX, lastMouseY;
  bool resetPosition;
} RendererUserData;



#endif // RENDERER_USER_DATA_H_
