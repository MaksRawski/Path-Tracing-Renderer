#ifndef ACTION_H_
#define ACTION_H_

typedef enum {
  Action_restart_rendering = (1 << 0),
  Action_load_scene = (1 << 1),
  Action_build_bvh = (1 << 2),
  Action_update_ssbo_camera = (1 << 3),
  Action_update_ssbo_scene = (1 << 4),
  Action_update_ssbo_renderer_parameters = (1 << 5),
  Action_save_image = (1 << 6),
  Action_exit = (1 << 7),
} Action;

#endif // ACTION_H_
