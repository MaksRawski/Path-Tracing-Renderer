#ifndef PARAMETERS_SCENE_H_
#define PARAMETERS_SCENE_H_

#include "small_string.h"
#include <stdbool.h>

#define FILEPATH_DEFAULT_CAPACITY 256

typedef struct {
  SmallString new_scene_path;
  SmallString loaded_scene_path;
  // each time a new scene is loaded this gets incremented
  unsigned int scene_version;
} ScenePaths;

ScenePaths ScenePath_default(void);

#endif // PARAMETERS_SCENE_H_
