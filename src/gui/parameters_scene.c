#include "gui/parameters_scene.h"
#include "file_path.h"

ParametersScene ParametersScene_default(void) {
  ParametersScene self = {0};
  self.gui_scene_path = FilePath_new(PARAMETERS_SCENE_DEFAULT_PATH_CAPACITY);
  self.loaded_scene_path = FilePath_new(PARAMETERS_SCENE_DEFAULT_PATH_CAPACITY);
  self.scene_version = 0;
  return self;
}

// TODO! is this correct?
bool ParametersScene_eq(ParametersScene a, ParametersScene b) {
  return a.scene_version == b.scene_version;
}
