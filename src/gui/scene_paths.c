#include "gui/scene_paths.h"

ScenePaths ScenePath_default(void) {
  ScenePaths self = {0};
  self.new_scene_path = SmallString_new("");
  self.loaded_scene_path = SmallString_new("");
  self.scene_version = 0;
  return self;
}
