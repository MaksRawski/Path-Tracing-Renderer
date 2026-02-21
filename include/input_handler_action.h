#ifndef INPUT_HANDLER_ACTION_H_
#define INPUT_HANDLER_ACTION_H_

#include "scene/camera.h"

typedef enum {
  InputHandlerActionType_Nothing,
  InputHandlerActionType_CameraMoved,
} InputHandlerActionType;

typedef struct {
  union {
    CameraTransformation CameraMoved;
  };
  InputHandlerActionType type;
} InputHandlerAction;

#endif // INPUT_HANDLER_ACTION_H_
