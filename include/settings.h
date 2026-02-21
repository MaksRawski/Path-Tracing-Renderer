#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "renderer/parameters.h"
#include "scene/bvh/strategies.h"
#include "scene/camera.h"
#include "small_string.h"
#include "window/scaling.h"

typedef struct {
  Camera cam;
  RendererParameters rendering_params;
  SmallString scene_path;
  BVHStrategy BVH_build_strat;
  SmallString saved_image_path;
  WindowScalingMode scaling_mode;
  bool gui_enabled, hot_reload_enabled, save_after_rendering,
      exit_after_rendering, movement_enabled;
} Settings;

inline static Settings Settings_default(void) {
  return (Settings){
      .cam = Camera_default(),
      .rendering_params = RendererParameters_default(),
      .scene_path = SmallString_new(""),
      .saved_image_path = SmallString_new(""),
      .gui_enabled = true,
      .hot_reload_enabled = true,
      .save_after_rendering = false,
      .exit_after_rendering = false,
      .movement_enabled = true,
      .BVH_build_strat = BVHStrategy_Midpoint,
  };
}

void Settings_set_Camera(Settings *settings, Camera cam);
void Settings_set_RendererParameters(Settings *settings, Camera cam);

#endif // SETTINGS_H_
