#include "gui/parameters.h"
#include "gui/parameters_scene.h"
#include "renderer/parameters.h"
#include "scene/camera.h"

// this new is supposed to be an exhaustive constructor, i.e. should initalize
// every field
GuiParameters GuiParameters_new(const Camera *camera,
                                const RendererParameters *rendering,
                                const ParametersScene *scene) {
  GuiParameters self = {0};
  self.cam = *camera;
  self.rendering = *rendering;
  self.scene = *scene;

  return self;
}

bool GuiParameters_eq(const GuiParameters *a, const GuiParameters *b) {
  return Camera_eq(a->cam, b->cam) &&
         RendererParameters_eq(a->rendering, b->rendering) &&
         ParametersScene_eq(a->scene, b->scene);
}
