#include "gui/settings.h"
#include "action.h"
#include "gui/file_browser.h"
#include "rad_deg.h"
#include "scene/bvh/strategies.h"
#include "scene/camera.h"
#include "small_string.h"
#include "stats.h"
#include "utils.h"
#include "window/scaling.h"
#include "yawpitch.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

static void tooltip(const char *desc) {
  igSameLine(0.0f, -1.0f);
  igTextDisabled("(?)");
  if (igBeginItemTooltip()) {
    igPushTextWrapPos(igGetFontSize() * 35.0f);
    igTextUnformatted(desc, NULL);
    igPopTextWrapPos();
    igEndTooltip();
  }
}

static void GuiSettings_Scene(AppState *state) {
  ImVec2 button_size = {.x = 0, .y = 0};
  if (igButton("Load scene", button_size)) {
    SmallString chosen_path = SmallString_new("");
    if (GuiFileBrowser_open(chosen_path.str, sizeof(SmallString))) {
      state->settings.scene_path = chosen_path;
      state->pending_actions |= Action_load_scene;
    }
  }

  if (igCombo_Str_arr("BVH type", (int *)&state->settings.BVH_build_strat,
                      BVHStrategy_str, BVHStrategy__COUNT, 5)) {
    state->pending_actions |= Action_build_bvh;
  }

  // if scene is loaded
  if (!SmallString_is_empty(&state->settings.scene_path)) {
    igText("Loaded scene: %s",
           FilePath_get_file_name(state->settings.scene_path.str));
    igText("Loaded Triangles: %d", state->scene.triangles_count);
    igText("Created BVH nodes: %d", state->scene.bvh_nodes_count);

    igText("Loading scene time: %s",
           Time_format(state->stats.scene_load.total_time).str);
    igText("BVH build time: %s",
           Time_format(state->stats.bvh_build.total_time).str);
  }
}

static void GUISettings_Camera(AppState *state) {
  igTextWrapped("To move the camera using keyboard (WASD keys) and mouse: "
                "Hold the left mouse button over the rendered scene "
                "(outside this Settings window).");
  // position
  float cam_pos[3] = {state->settings.cam.pos.x, state->settings.cam.pos.y,
                      state->settings.cam.pos.z};
  if (igInputFloat3("Position", cam_pos, "%6.3f", 0)) {
    state->pending_actions |= Action_update_ssbo_camera;
    state->settings.cam.pos.x = cam_pos[0];
    state->settings.cam.pos.y = cam_pos[1];
    state->settings.cam.pos.z = cam_pos[2];
  }

  // rotation
  YawPitch yp = YawPitch_from_dir(Vec3d_from_vec3(state->settings.cam.dir));
  bool dir_changed = false;

  float yaw_deg = rad_to_deg(yp.yaw_rad);
  if (igSliderFloat("Yaw", &yaw_deg, 0.0, 360.0, "%4.1f", 0)) {
    yp.yaw_rad = deg_to_rad(yaw_deg);
    dir_changed = true;
  }

  float pitch_deg = rad_to_deg(yp.pitch_rad);
  if (igSliderFloat("Pitch", &pitch_deg, CAMERA_PITCH_MIN_DEG,
                    CAMERA_PITCH_MAX_DEG, "%4.1f", 0)) {
    yp.pitch_rad = deg_to_rad(pitch_deg);
    dir_changed = true;
  }

  if (dir_changed) {
    state->settings.cam.dir = Vec3d_to_vec3(YawPitch_to_dir(yp));
    state->pending_actions |= Action_update_ssbo_camera;
  }

  // additional properties
  float fov_deg = Camera_get_fov_deg(&state->settings.cam);
  if (igSliderFloat("FOV", &fov_deg, CAMERA_FOV_MIN_DEG, CAMERA_FOV_MAX_DEG,
                    "%3.1f", 0)) {
    Camera_set_fov_deg(&state->settings.cam, fov_deg);
    state->pending_actions |= Action_update_ssbo_camera;
  }

  igSliderFloat("Movement speed", &state->settings.cam.step_size_per_second,
                CAMERA_MOVE_SPEED_PER_SECOND_MIN,
                CAMERA_MOVE_SPEED_PER_SECOND_MAX, "%.2f", 0);
  igSliderFloat("Sensitivity", &state->settings.cam.sensitivity,
                CAMERA_ROTATE_SENSITIVITY_MIN, CAMERA_ROTATE_SENSITIVITY_MAX,
                "%.4f", ImGuiSliderFlags_Logarithmic);
}

static void GuiSettings_Rendering(AppState *state) {
  bool rendering_param_changed = false;

  rendering_param_changed |= igColorEdit3(
      "Environment color", state->settings.rendering_params.env_color, 0);

  rendering_param_changed |=
      igInputInt("Max Bounce Count",
                 &state->settings.rendering_params.max_bounce_count, 1, 1, 0);
  tooltip("Maximum number of bounces a ray can do before terminating.");

  rendering_param_changed |=
      igInputInt("Samples Per Pixel",
                 &state->settings.rendering_params.samples_per_pixel, 1, 1, 0);
  tooltip("How many rays should be sent for each pixel in a single rendering "
          "pass.");

  rendering_param_changed |= igSliderFloat(
      "Diverge strength", &state->settings.rendering_params.diverge_strength,
      0.0, 1, "%6.5f", ImGuiSliderFlags_Logarithmic);
  tooltip("How much should the rays sent from the same pixel differ in "
          "a single sample. The higher the value the more blurry the image "
          "becomes. Serves as a simple form of antialiasing.");

  rendering_param_changed |=
      igInputInt("Frames to render (-1 for inf)",
                 &state->settings.rendering_params.frames_to_render, 1, 1, 0);
  tooltip("A control for progressive rendering. This number of frames will be "
          "rendered and immediately averaged, effectively causing more "
          "rays to be sent from each pixel over the span of many frames.");

  int res[2] = {state->settings.rendering_params.rendering_resolution.width,
                state->settings.rendering_params.rendering_resolution.height};

  rendering_param_changed |= igInputInt2("Resolution", res, 0);
  tooltip("The resolution for which the image should be rendered. NOTE: this "
          "is independent of window size! Look at the Misc section for Scaling "
          "settings.");
  state->settings.rendering_params.rendering_resolution.width = res[0];
  state->settings.rendering_params.rendering_resolution.height = res[1];

  igText("Rendering last frame took: %s",
         Time_format(state->stats.last_frame_rendering.total_time).str);

  RenderingState render_state = AppState_get_rendering_state(state);
  switch (render_state) {
  case RenderingState_NOT_RENDERING:
    break;
  case RenderingState_RENDERING:
    igText("Elapsed rendering time: %s",
           Time_format(StatsTimer_elapsed(&state->stats.rendering)).str);
    break;
  case RenderingState_FINISHED:
    igText("Total rendering time: %s",
           Time_format(state->stats.rendering.total_time).str);
    break;
  }
  igText("Rendered frames: %d", state->stats.frame_number);

  if (rendering_param_changed) {
    state->pending_actions |= Action_update_ssbo_renderer_parameters;
  }
}

static void GuiSettings_Misc(GUIOverlay *gui, AppState *state) {
  if (igSliderFloat("UI scale", &gui->ui_scale, 0.5, 2.5, "%3.2f", 0))
    GUIOverlay_scale(gui, gui->ui_scale);

  igCombo_Str_arr("Scaling", (int *)&state->settings.scaling_mode,
                  WindowScalingMode_str, WindowScalingMode__COUNT, 5);
  tooltip("Method to use for displaying the rendered image when the window "
          "size is different from the Rendering Resolution");

  igInputText("Saved image path", state->settings.saved_image_path.str,
              sizeof(state->settings.saved_image_path), 0, NULL, NULL);
  ImVec2 button_size = {.x = 0, .y = 0};
  if (igButton("Save image", button_size)) {
    state->pending_actions |= Action_save_image;
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
    GuiSettings_Scene(state);
  }
  if (igCollapsingHeader_TreeNodeFlags("Camera", 0)) {
    GUISettings_Camera(state);
  }
  if (igCollapsingHeader_TreeNodeFlags("Rendering", 0)) {
    GuiSettings_Rendering(state);
  }
  if (igCollapsingHeader_TreeNodeFlags("Misc",
                                       ImGuiTreeNodeFlags_DefaultOpen)) {
    GuiSettings_Misc(gui, state);
  }

  igEnd();
}
