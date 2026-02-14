#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "gui/scene_paths.h"
#include "renderer/parameters.h"
#include "scene/bvh/strategies.h"
#include "scene/camera.h"

typedef struct {
  Camera cam;
  RendererParameters rendering_params;
  ScenePaths scene_paths;
  BVHStrategy BVH_build_strat;
  bool gui_enabled, hot_reload_enabled, save_after_rendering,
      exit_after_rendering, movement_enabled;
  bool cam_changed, rendering_params_changed, scene_paths_changed,
      BVH_build_strat_changed;
} Settings;

inline static Settings Settings_default(void) {
  return (Settings){
      .cam = Camera_default(),
      .rendering_params = RendererParameters_default(),
      .scene_paths = ScenePath_default(),
      .gui_enabled = true,
      .hot_reload_enabled = true,
      .save_after_rendering = false,
      .exit_after_rendering = false,
      .movement_enabled = true,
      .BVH_build_strat = BVHStrategy_Midpoint,
      .cam_changed = true,
      .rendering_params_changed = true,
      .scene_paths_changed = true,
      .BVH_build_strat_changed = true,
  };
}

#endif // SETTINGS_H_
