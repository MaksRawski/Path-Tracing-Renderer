#ifndef PARAMETERS_SCENE_H_
#define PARAMETERS_SCENE_H_

#include "file_path.h"
#include <stdbool.h>

#define FILEPATH_DEFAULT_CAPACITY 256

typedef struct {
  FilePath new_scene_path;
  FilePath loaded_scene_path;
  // each time a new scene is loaded this gets incremented
  unsigned int scene_version;
} ScenePaths;

ScenePaths ScenePath_default(void);
void ScenePath_set_new_scene_path(ScenePaths *self, FilePath gui_scene_path);

bool ScenePath_eq(ScenePaths a, ScenePaths b);

#endif // PARAMETERS_SCENE_H_
