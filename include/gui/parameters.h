#ifndef GUI_PARAMETERS_H_
#define GUI_PARAMETERS_H_

#include "gui/parameters_scene.h"
#include "renderer/parameters.h"
#include "scene/camera.h"

typedef struct {
  Camera cam;
  RendererParameters rendering;
  ParametersScene scene;
} GuiParameters;

GuiParameters GuiParameters_new(const Camera *camera,
                                const RendererParameters *rendering,
                                const ParametersScene *scene);
void GuiParameters_apply(const GuiParameters *params, Camera *camera);
bool GuiParameters_eq(const GuiParameters *a, const GuiParameters *b);

#endif // GUI_PARAMETERS_H_
