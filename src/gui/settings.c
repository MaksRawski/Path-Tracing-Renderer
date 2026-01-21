#include "gui/settings.h"
#include "gui/file_browser.h"
#include "opengl/scaling.h"
#include "rad_deg.h"
#include "scene/bvh/strategies.h"
#include "scene/camera.h"
#include "small_string.h"
#include "stats.h"
#include "utils.h"
#include "yawpitch.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

void tooltip(const char *desc) {
  igSameLine(0.0f, -1.0f);
  igTextDisabled("(?)");
  if (igBeginItemTooltip()) {
    igPushTextWrapPos(igGetFontSize() * 35.0f);
    igTextUnformatted(desc, NULL);
    igPopTextWrapPos();
    igEndTooltip();
  }
}

bool _Scene_settings(AppState *state) {
  bool changed = false;
  ImVec2 button_size = {.x = 0, .y = 0};
  if (igButton("Load scene", button_size)) {
    SmallString chosen_path = SmallString_new("");
    if (GuiFileBrowser_open(chosen_path.str, sizeof(SmallString))) {
      state->scene_paths.new_scene_path = chosen_path;
      changed = true;
    }
  }

  if (igCombo_Str_arr("BVH type", (int *)&state->BVH_build_strat,
                      BVHStrategy_str, BVHStrategy__COUNT, 5)) {
    state->BVH_build_strat_changed = true;
  }

  if (!SmallString_is_empty(&state->scene_paths.loaded_scene_path)) {
    igText("Loaded scene: %s",
           FilePath_get_file_name(state->scene_paths.loaded_scene_path.str));
    igText("Loaded Triangles: %d", state->scene.triangles_count);
    igText("Created BVH nodes: %d", state->scene.bvh.nodes_count);

    char load_scene_time[16] = {0};
    char bvh_build_time[16] = {0};
    Stats_string_time(state->stats.scene_load.total_time, load_scene_time,
                      sizeof(load_scene_time));
    Stats_string_time(state->stats.bvh_build.total_time, bvh_build_time,
                      sizeof(bvh_build_time));
    igText("Loading scene time: %s", load_scene_time);
    igText("BVH build time: %s", bvh_build_time);
  }
  return changed;
}

bool _Camera_settings(AppState *params) {
  bool changed = false;

  // position
  float cam_pos[3] = {params->cam.pos.x, params->cam.pos.y, params->cam.pos.z};
  changed |= igInputFloat3("Position", cam_pos, "%6.3f", 0);
  params->cam.pos.x = cam_pos[0];
  params->cam.pos.y = cam_pos[1];
  params->cam.pos.z = cam_pos[2];

  // rotation
  YawPitch yp = YawPitch_from_dir(Vec3d_from_vec3(params->cam.dir));
  float yaw_deg = rad_to_deg(yp.yaw_rad);
  float pitch_deg = rad_to_deg(yp.pitch_rad);
  changed |= igSliderFloat("Yaw", &yaw_deg, 0.0, 360.0, "%4.1f", 0);
  changed |= igSliderFloat("Pitch", &pitch_deg, CAMERA_PITCH_MIN_DEG,
                           CAMERA_PITCH_MAX_DEG, "%4.1f", 0);

  yp.yaw_rad = deg_to_rad(yaw_deg);
  yp.pitch_rad = deg_to_rad(pitch_deg);
  params->cam.dir = Vec3d_to_vec3(YawPitch_to_dir(yp));

  // additional properties
  float fov_deg = Camera_get_fov_deg(&params->cam);
  changed |= igSliderFloat("FOV", &fov_deg, CAMERA_FOV_MIN_DEG,
                           CAMERA_FOV_MAX_DEG, "%3.1f", 0);
  Camera_set_fov_deg(&params->cam, fov_deg);

  changed |= igSliderFloat("Focal length", &params->cam.focal_length,
                           CAMERA_FOCAL_LENGTH_MIN, CAMERA_FOCAL_LENGTH_MAX,
                           "%3.1f", 0);
  changed |= igSliderFloat("Movement speed (in units per second)",
                           &params->cam.step_size_per_second,
                           CAMERA_MOVE_SPEED_PER_SECOND_MIN,
                           CAMERA_MOVE_SPEED_PER_SECOND_MAX, "%.2f", 0);
  changed |= igSliderFloat(
      "Sensitivity", &params->cam.sensitivity, CAMERA_ROTATE_SENSITIVITY_MIN,
      CAMERA_ROTATE_SENSITIVITY_MAX, "%.4f", ImGuiSliderFlags_Logarithmic);
  return changed;
}

bool _Rendering_settings(AppState *state) {
  bool changed = false;

  igSetNextItemWidth(0.5 * igGetWindowWidth());
  changed |= igColorPicker3(
      "Environment color", state->rendering_params.env_color,
      ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);

  changed |= igInputInt("Max Bounce Count",
                        &state->rendering_params.max_bounce_count, 1, 1, 0);
  tooltip("Maximum number of bounces a ray can do before terminating.");

  changed |= igInputInt("Samples Per Pixel",
                        &state->rendering_params.samples_per_pixel, 1, 1, 0);
  tooltip("How many rays should be sent for each pixel in a single rendering "
          "pass.");

  changed |= igSliderFloat("Diverge strength",
                           &state->rendering_params.diverge_strength, 0.0, 1,
                           "%6.5f", ImGuiSliderFlags_Logarithmic);
  tooltip("How much should the rays sent from the same pixel differ in "
          "a single sample. The higher the value the more blurry the image "
          "becomes. Serves as a simple form of antialiasing.");

  changed |= igInputInt("Passes (-1 for inf)",
                        &state->rendering_params.frames_to_render, 1, 1, 0);
  tooltip("A control for progressive rendering. Chosen count of frames will be "
          "rendered and immediately averaged, which effectively causes more "
          "rays to be sent from each pixel over the span of many frames.");

  // TODO: ideally we would do this without copying
  int res[2] = {state->rendering_params.rendering_resolution.width,
                state->rendering_params.rendering_resolution.height};

  changed |= igInputInt2("Resolution", res, 0);
  tooltip("The resolution for which the image should be rendered. NOTE: this "
          "is independent of window size! Look at the Misc section for Scaling "
          "settings.");
  state->rendering_params.rendering_resolution.width = res[0];
  state->rendering_params.rendering_resolution.height = res[1];

  char last_frame_time_str[16] = {0};
  double last_frame_time = state->stats.last_frame_rendering.total_time;
  Stats_string_time(last_frame_time, last_frame_time_str,
                    sizeof(last_frame_time_str));

  igText("Rendering last frame took: %s", last_frame_time_str);
  char rendering_time_str[16] = {0};
  if (state->stats.rendering.total_time == 0) {
    double rendering_time = StatsTimer_elapsed(&state->stats.rendering);
    Stats_string_time(rendering_time, rendering_time_str,
                      sizeof(rendering_time_str));
    igText("Elapsed rendering time: %s", rendering_time_str);
  } else {
    double rendering_time = state->stats.rendering.total_time;
    Stats_string_time(rendering_time, rendering_time_str,
                      sizeof(rendering_time_str));
    igText("Total rendering time: %s", rendering_time_str);
  }
  igText("Rendered frames: %d", state->stats.frame_number);

  return changed;
}

void _Misc_settings(GUIOverlay *gui, AppState *state) {
  if (igSliderFloat("UI scale", &gui->ui_scale, 0.5, 2.5, "%3.2f", 0))
    GUIOverlay_scale(gui, gui->ui_scale);

  igCombo_Str_arr("Scaling", (int *)&state->scaling_mode, OpenGLScalingMode_str,
                  OpenGLScalingMode__COUNT, 5);
  tooltip("Strategy to use for displaying the rendered image when the window "
          "size is different from the "
          "Rendering Resolution");

  igInputText("Saved image path", state->save_image_info.path,
              sizeof(state->save_image_info.path), 0, NULL, NULL);
  ImVec2 button_size = {.x = 0, .y = 0};
  if (igButton("Save image", button_size)) {
    state->save_image_info.to_save = true;
  }

#ifndef NDEBUG
  igCheckbox("Show ImGui demo", &gui->show_demo);
#endif

  if (gui->show_demo)
    igShowDemoWindow(NULL);
}

void GuiSettings_draw(GUIOverlay *gui, AppState *state) {
  igBegin("Settings", NULL, 0);

  if (igCollapsingHeader_TreeNodeFlags("Scene",
                                       ImGuiTreeNodeFlags_DefaultOpen)) {
    state->scene_paths_changed |= _Scene_settings(state);
  }
  if (igCollapsingHeader_TreeNodeFlags("Camera", 0)) {
    state->cam_changed |= _Camera_settings(state);
  }
  if (igCollapsingHeader_TreeNodeFlags("Rendering", 0)) {
    state->rendering_params_changed |= _Rendering_settings(state);
  }
  if (igCollapsingHeader_TreeNodeFlags("Misc",
                                       ImGuiTreeNodeFlags_DefaultOpen)) {
    _Misc_settings(gui, state);
  }

  igEnd();
}
