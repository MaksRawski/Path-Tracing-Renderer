#include "file_path.h"
#include "gui/scene_paths.h"

ScenePaths ScenePath_default(void) {
  ScenePaths self = {0};
  self.new_scene_path = FilePath_empty();
  self.loaded_scene_path = FilePath_empty();
  self.scene_version = 0;
  return self;
}

void ScenePath_set_new_scene_path(ScenePaths *self, FilePath gui_scene_path) {
  self->new_scene_path = gui_scene_path;
  ++self->scene_version;
}

bool ScenePath_eq(ScenePaths a, ScenePaths b) {
  return a.scene_version == b.scene_version;
}
