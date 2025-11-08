#ifndef RENDERER_INPUTS_H_
#define RENDERER_INPUTS_H_

#include "opengl/context.h"
#include "scene/camera.h"

bool RendererInputs_update_camera(Camera *cam, const GLFWUserData *user_data);

#endif // RENDERER_INPUTS_H_
