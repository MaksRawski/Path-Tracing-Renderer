#include "gui/settings.h"
#include "file_path.h"
#include "gui/file_browser.h"
#include "gui/parameters.h"
#include "opengl/window_events.h"
#include "rad_deg.h"
#include "scene/camera.h"
#include "vec3d.h"
#include "yawpitch.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

void _Scene_settings(GuiParameters *params) {
  ImVec2 button_size = {.x = 0, .y = 0};
  if (igButton("Load scene", button_size)) {
    char *chosen_path = GuiFileBrowser_open_linux();
    if (chosen_path != NULL) {
      params->scene.gui_scene_path = FilePath_of_string(chosen_path);
      ++params->scene.scene_version;
    }
  }

  if (params->scene.loaded_scene_path.file_path.str[0] != '\0') {
    igText("Loaded scene: %s",
           FilePath_get_filename(&params->scene.loaded_scene_path));
  }

  // TODO: build bvh checkbox (default on)
}

void _Camera_settings(GuiParameters *params) {
  // position
  float cam_pos[3] = {params->cam.pos.x, params->cam.pos.y, params->cam.pos.z};
  igInputFloat3("Position", cam_pos, "%6.3f", 0);
  params->cam.pos.x = cam_pos[0];
  params->cam.pos.y = cam_pos[1];
  params->cam.pos.z = cam_pos[2];

  // rotation
  YawPitch yp = YawPitch_from_dir(Vec3d_from_vec3(params->cam.dir));
  float yaw_deg = rad_to_deg(yp.yaw_rad);
  float pitch_deg = rad_to_deg(yp.pitch_rad);
  igSliderFloat("Yaw", &yaw_deg, 0.0, 360.0, "%4.1f", 0);
  igSliderFloat("Pitch", &pitch_deg, CAMERA_PITCH_MIN_DEG, CAMERA_PITCH_MAX_DEG,
                "%4.1f", 0);

  yp.yaw_rad = deg_to_rad(yaw_deg);
  yp.pitch_rad = deg_to_rad(pitch_deg);
  params->cam.dir = Vec3d_to_vec3(YawPitch_to_dir(yp));

  // additional properties
  float fov_deg = Camera_get_fov_deg(&params->cam);
  igSliderFloat("FOV", &fov_deg, CAMERA_FOV_MIN_DEG, CAMERA_FOV_MAX_DEG,
                "%3.1f", 0);
  Camera_set_fov_deg(&params->cam, fov_deg);

  igSliderFloat("Focal length", &params->cam.focal_length,
                CAMERA_FOCAL_LENGTH_MIN, CAMERA_FOCAL_LENGTH_MAX, "%3.1f", 0);
}

void _Rendering_settings(GuiParameters *params) {
  igInputInt("Max Bounce Count", &params->rendering.max_bounce_count, 1, 1, 0);
  igInputInt("Samples Per Pixel", &params->rendering.samples_per_pixel, 1, 1,
             0);
  igSliderFloat("Diverge strength", &params->rendering.diverge_strength, 0.0, 1,
                "%6.5f", ImGuiSliderFlags_Logarithmic);
}

void _Misc_settings(Gui *gui, WindowEventsData *events) {
  if (igSliderFloat("UI scale", &gui->ui_scale, 0.5, 2.5, "%3.2f", 0))
    Gui_scale(gui, gui->ui_scale);

  int res[2] = {events->window_size.width, events->window_size.height};
  // HACK: accessing _window, which we shouldn't!
  if (igInputInt2("Window size", res, 0)) {
    glfwSetWindowSize(events->_window, res[0], res[1]);
  }

#ifndef NDEBUG
  igCheckbox("Show ImGui demo", &gui->show_demo);
#endif

  if (gui->show_demo)
    igShowDemoWindow(NULL);
}

void GuiSettings_draw(Gui *gui, GuiParameters *params,
                      WindowEventsData *events) {
  igBegin("Settings", NULL, 0);

  if (igCollapsingHeader_TreeNodeFlags("Scene",
                                       ImGuiTreeNodeFlags_DefaultOpen)) {
    _Scene_settings(params);
  }
  if (igCollapsingHeader_TreeNodeFlags("Camera", 0)) {
    _Camera_settings(params);
  }
  if (igCollapsingHeader_TreeNodeFlags("Rendering", 0)) {
    _Rendering_settings(params);
  }
  if (igCollapsingHeader_TreeNodeFlags("Misc", 0)) {
    _Misc_settings(gui, events);
  }

  igEnd();
}
