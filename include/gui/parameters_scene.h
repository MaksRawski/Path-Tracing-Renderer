#ifndef PARAMETERS_SCENE_H_
#define PARAMETERS_SCENE_H_

#include "file_path.h"
#include <stdbool.h>

#define PARAMETERS_SCENE_DEFAULT_PATH_CAPACITY 256

typedef struct {
  FilePath gui_scene_path;
  FilePath loaded_scene_path;
  // each time a new scene is loaded this gets incremented
  unsigned int scene_version;
} ParametersScene;

ParametersScene ParametersScene_default(void);

// conditionally sets loaded_scene_path if the maybe_scene_path is a file and
// exists
void ParametersScene_set_scene_path(ParametersScene *self);
bool ParametersScene_eq(ParametersScene a, ParametersScene b);

#endif // PARAMETERS_SCENE_H_
