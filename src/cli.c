#include "cli.h"
#include "action.h"
#include "app_state.h"
#include "asserts.h"
#include "scene/bvh/strategies.h"
#include "utils.h"
#include "vec3.h"
#include "window/resolution.h"
#include "window/scaling.h"
#include "yawpitch.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rad_deg.h"

// NOTE: Argument is what is provided on the command line
// NOTE: Option is what's supported by the application

typedef struct {
  // NOTE: these two are compile time known so they're pointers to static strings
  const char *short_name;
  const char *long_name;
  // NOTE: should be enough as the longest formatted value is vec3 which has len 29
  char default_value[32];
  // NOTE: simply run valgrind on --help to see if this is enough :)
  char description[128];
} HelpLine;

typedef struct {
  int longest_short_name;
  int longest_long_name;
  int longest_default_value;
} ColStats;

void HelpLine_print(HelpLine self, ColStats stats) {
  printf(" %-*s", stats.longest_short_name, self.short_name ? self.short_name : "");
  // TODO: this name is stupid
  int pre_desc_len = printf(" %s",   self.long_name ? self.long_name : "");
  if (self.default_value[0] != 0)
    pre_desc_len += printf(" [%s]", self.default_value);
  // fill with spaces to align columns
  printf("%*s", stats.longest_long_name + stats.longest_default_value - pre_desc_len, "");
  printf(" %s\n", self.description);
}

ColStats ColStats_get(HelpLine *help_lines, size_t help_lines_count) {
  ColStats stats = {0};

  for (size_t i = 0; i < help_lines_count; ++i) {
    const int short_name_len    = help_lines[i].short_name  == NULL ? 0 : strlen(help_lines[i].short_name);
    const int long_name_len     = help_lines[i].short_name  == NULL ? 0 : strlen(help_lines[i].long_name);
    const int default_value_len = help_lines[i].short_name  == NULL ? 0 : strlen(help_lines[i].default_value);

    if (short_name_len > stats.longest_short_name)       stats.longest_short_name = short_name_len;
    if (long_name_len > stats.longest_long_name)         stats.longest_long_name = long_name_len;
    if (default_value_len > stats.longest_default_value) stats.longest_default_value = default_value_len;
  }

  return stats;
}

// Option interface
typedef HelpLine(GetHelpLineFn)(const AppState *);
typedef void(SetOptionFn)(AppState *, int argc, const char **argv, int *iargv);
// Helpers
typedef void(GetDescFn)(char *buf); 
typedef void(GetValueStrFn)(char *buf, const AppState *app_state);

// === Value parsing helpers === 
static bool parse_vec3(const char *str, vec3 *v) {
  float x, y, z;
  if (sscanf(str, "%g,%g,%g", &x, &y, &z) == 3) {
    *v = vec3_new(x, y, z);
    return true;
  }
  return false;
}
// the longest possible string has length 29
static void format_vec3(char *buf, vec3 v) {
  sprintf(buf, "%g,%g,%g", v.x, v.y, v.z);
}
static bool parse_float(const char *str, float *val) {
  if (sscanf(str, "%g", val) == 1)
    return true;
  else if (errno != 0) {
    perror("sscanf");
  }
  return false;
}
// the longest possible string has length 9
static void format_float(char *buf, float v) {
  sprintf(buf, "%g", v);
}
static bool parse_int(const char *str, int *n) {
  if (sscanf(str, "%d", n) == 1)
    return true;
  else if (errno != 0) {
    perror("sscanf");
  }
  return false;
}
// the longest possible string has length 11
static void format_int(char *buf, int v) {
  sprintf(buf, "%d", v);
}
static bool parse_WindowResolution(const char *str, WindowResolution *res) {
  char *x_pos = strchr(str, 'x');
  if (!x_pos)
    return false;

  if (sscanf(str, "%dx%d", &res->width, &res->height) != 2) {
    if (errno != 0)
      perror("sscanf");
    return false;
  }

  if (res->width <= 0 || res->height <= 0)
    return false;
  return true;
}
// the longest possible string has length 21
static void format_WindowResolution(char *buf, WindowResolution res) {
  sprintf(buf, "%dx%d", res.width, res.height);
}
static bool parse_YawPitch(const char *str, YawPitch *yp) {
  double yaw_deg, pitch_deg;
  if (sscanf(str, "%lf,%lf", &yaw_deg, &pitch_deg) == 2) {
    *yp = YawPitch_new(deg_to_rad(yaw_deg), deg_to_rad(pitch_deg));
    return true;
  }
  return false;
}
// the longest possible string has length 21
static void format_YawPitch(char *buf, YawPitch yp) {
  sprintf(buf, "%lg,%lg", rad_to_deg(yp.yaw_rad), rad_to_deg(yp.pitch_rad));
}

static const char *get_value_for_option(int argc, const char **argv, int *iargv) {
  if (*iargv + 1 >= argc)
    ERROR_FMT("Option %s requires a value", argv[*iargv]);

  return argv[++(*iargv)];
}

#define get_value__template(_type)                                             \
  static _type get_value_##_type(int argc, const char **argv, int *iargv) {    \
    _type out;                                                                 \
    const char *arg = argv[*iargv];                                            \
    const char *val = get_value_for_option(argc, argv, iargv);                 \
    if (!parse_##_type(val, &out))                                             \
      ERROR_FMT("Invalid value '%s' provided for %s", val, arg);               \
    return out;                                                                \
  }

// NOTE: these generated get_value_<type> functions,
// take the value provided for the current argument and parse it into <type>,
// exiting on error
get_value__template(WindowResolution)
get_value__template(YawPitch)
get_value__template(vec3)
get_value__template(float)
get_value__template(int)

// == OPTIONS (DECLARATIONS) ==
// === SCENE ===
#define scene_bvh_type_short NULL
#define scene_bvh_type_long "--bvh"
GetHelpLineFn scene_bvh_type_help_line;
SetOptionFn scene_bvh_type_set;
GetDescFn scene_bvh_type_desc_fn;
GetValueStrFn scene_bvh_type_value_str;

// === CAMERA ===
#define camera_position_short NULL
#define camera_position_long "--pos"
#define camera_position_desc "Initial camera position"
GetHelpLineFn camera_position_help_line;
SetOptionFn camera_position_set;
GetValueStrFn camera_position_value_str;

#define camera_rotation_short NULL
#define camera_rotation_long "--rot"
#define camera_rotation_desc "Initial camera rotation YAW_DEG,PITCH_DEG"
GetHelpLineFn camera_rotation_help_line;
SetOptionFn camera_rotation_set;
GetValueStrFn camera_rotation_value_str;

#define camera_fov_short NULL
#define camera_fov_long "--fov"
#define camera_fov_desc "Camera Field of View in degrees"
GetHelpLineFn camera_fov_help_line;
SetOptionFn camera_fov_set;
GetValueStrFn camera_fov_value_str;

#define camera_movement_speed_short NULL
#define camera_movement_speed_long "--movement-speed"
#define camera_movement_speed_desc "Camera movement speed per second"
GetHelpLineFn camera_movement_speed_help_line;
SetOptionFn camera_movement_speed_set;
GetValueStrFn camera_movement_speed_value_str;

#define camera_sensitivity_short NULL
#define camera_sensitivity_long "--sensitivity"
#define camera_sensitivity_desc "Camera rotation sensitivity"
GetHelpLineFn camera_sensitivity_help_line;
SetOptionFn camera_sensitivity_set;
GetValueStrFn camera_sensitivity_value_str;

// === RENDERING ===
#define rendering_env_color_short NULL
#define rendering_env_color_long "--env-color"
#define rendering_env_color_desc "Environment color R,G,B"
GetHelpLineFn rendering_env_color_help_line;
SetOptionFn rendering_env_color_set;
GetValueStrFn rendering_env_color_value_str;

#define rendering_max_bounce_count_short NULL
#define rendering_max_bounce_count_long "--max-bounce-count"
#define rendering_max_bounce_count_desc "Maximum ray bounce count"
GetHelpLineFn rendering_max_bounce_count_help_line;
SetOptionFn rendering_max_bounce_count_set;
GetValueStrFn rendering_max_bounce_count_value_str;

#define rendering_samples_per_pixel_short "-spp"
#define rendering_samples_per_pixel_long "--samples-per-pixel"
#define rendering_samples_per_pixel_desc "Samples per pixel"
GetHelpLineFn rendering_samples_per_pixel_help_line;
SetOptionFn rendering_samples_per_pixel_set;
GetValueStrFn rendering_samples_per_pixel_value_str;

#define rendering_diverge_strength_short NULL
#define rendering_diverge_strength_long "--diverge-strength"
#define rendering_diverge_strength_desc "Diverge strength"
GetHelpLineFn rendering_diverge_strength_help_line;
SetOptionFn rendering_diverge_strength_set;
GetValueStrFn rendering_diverge_strength_value_str;

#define rendering_frames_to_render_short "-F"
#define rendering_frames_to_render_long "--frames-to-render"
#define rendering_frames_to_render_desc "Frames to render before stopping"
GetHelpLineFn rendering_frames_to_render_help_line;
SetOptionFn rendering_frames_to_render_set;
GetValueStrFn rendering_frames_to_render_value_str;

#define rendering_resolution_short "-R"
#define rendering_resolution_long "--resolution"
#define rendering_resolution_desc "Rendering resolution"
GetHelpLineFn rendering_resolution_help_line;
SetOptionFn rendering_resolution_set;
GetValueStrFn rendering_resolution_value_str;

// === MISC ===
#define misc_scaling_short NULL
#define misc_scaling_long "--scaling"
GetDescFn misc_scaling_desc_fn;
GetHelpLineFn misc_scaling_help_line;
SetOptionFn misc_scaling_set;
GetValueStrFn misc_scaling_value_str;

#define misc_no_movement_short "-NM"
#define misc_no_movement_long "--no-movement"
#define misc_no_movement_desc "Disable camera movement"
GetHelpLineFn misc_no_movement_help_line;
SetOptionFn misc_no_movement_set;
GetValueStrFn misc_no_movement_value_str;

#define misc_no_hot_reload_short "-NH"
#define misc_no_hot_reload_long "--no-hot-reload"
#define misc_no_hot_reload_desc "Disable hot-reloading of shaders"
GetHelpLineFn misc_no_hot_reload_help_line;
SetOptionFn misc_no_hot_reload_set;
GetValueStrFn misc_no_hot_reload_value_str;

#define misc_no_gui_short "-NG"
#define misc_no_gui_long "--no-gui"
#define misc_no_gui_desc "Run without GUI overlay"
GetHelpLineFn misc_no_gui_help_line;
SetOptionFn misc_no_gui_set;
GetValueStrFn misc_no_gui_value_str;

#define misc_save_on_frame_short NULL
#define misc_save_on_frame_long "--save-after-rendering"
#define misc_save_on_frame_desc "Automatically save rendered image to PNG"
GetHelpLineFn misc_save_on_frame_help_line;
SetOptionFn misc_save_on_frame_set;
GetValueStrFn misc_save_on_frame_value_str;

#define misc_just_render_short "-J"
#define misc_just_render_long "--just-render"
#define misc_just_render_desc "Alias for --no-gui --no-hot-reload --no-movement --save-after-rendering --exit-after-rendering"
GetHelpLineFn misc_just_render_help_line;
SetOptionFn misc_just_render_set;
GetValueStrFn misc_just_render_value_str;

#define misc_output_path_short "-o"
#define misc_output_path_long "--out"
#define misc_output_path_desc "Output image path"
GetHelpLineFn misc_output_path_help_line;
SetOptionFn misc_output_path_set;
GetValueStrFn misc_output_path_value_str;

#define misc_exit_after_rendering_short "-X"
#define misc_exit_after_rendering_long "--exit-after-rendering"
#define misc_exit_after_rendering_desc "Exit application after rendering is finished"
GetHelpLineFn misc_exit_after_rendering_help_line;
SetOptionFn misc_exit_after_rendering_set;
GetValueStrFn misc_exit_after_rendering_value_str;

#define help_short "-h"
#define help_long "--help"
#define help_desc "Display this help message and exit"
GetHelpLineFn help_help_line;
SetOptionFn help_set;
GetValueStrFn help_value_str;

// NOTE: maybe a bit wasteful for each option to contain a prefix but makes it much easier for a human to comprehend what's going on
// removing it wouldn't even allow for other prefixes on different platform as they would look like /very-long-option which ig is awkward
const char *options_short[] = {scene_bvh_type_short, camera_position_short, camera_rotation_short, camera_fov_short, camera_movement_speed_short, camera_sensitivity_short, rendering_env_color_short, rendering_max_bounce_count_short, rendering_samples_per_pixel_short, rendering_diverge_strength_short, rendering_frames_to_render_short, rendering_resolution_short, misc_scaling_short, misc_no_movement_short, misc_no_hot_reload_short, misc_no_gui_short, misc_save_on_frame_short, misc_just_render_short, misc_output_path_short, misc_exit_after_rendering_short, help_short};
const char *options_long[] = {scene_bvh_type_long, camera_position_long, camera_rotation_long, camera_fov_long, camera_movement_speed_long, camera_sensitivity_long, rendering_env_color_long, rendering_max_bounce_count_long, rendering_samples_per_pixel_long, rendering_diverge_strength_long, rendering_frames_to_render_long, rendering_resolution_long, misc_scaling_long, misc_no_movement_long, misc_no_hot_reload_long, misc_no_gui_long, misc_save_on_frame_long, misc_just_render_long, misc_output_path_long, misc_exit_after_rendering_long, help_long};
GetHelpLineFn *options_help_line[] = {scene_bvh_type_help_line, camera_position_help_line, camera_rotation_help_line, camera_fov_help_line, camera_movement_speed_help_line, camera_sensitivity_help_line, rendering_env_color_help_line, rendering_max_bounce_count_help_line, rendering_samples_per_pixel_help_line, rendering_diverge_strength_help_line, rendering_frames_to_render_help_line, rendering_resolution_help_line, misc_scaling_help_line, misc_no_movement_help_line, misc_no_hot_reload_help_line, misc_no_gui_help_line, misc_save_on_frame_help_line, misc_just_render_help_line, misc_output_path_help_line, misc_exit_after_rendering_help_line, help_help_line};
SetOptionFn *options_set[] = {scene_bvh_type_set, camera_position_set, camera_rotation_set, camera_fov_set, camera_movement_speed_set, camera_sensitivity_set, rendering_env_color_set, rendering_max_bounce_count_set, rendering_samples_per_pixel_set, rendering_diverge_strength_set, rendering_frames_to_render_set, rendering_resolution_set, misc_scaling_set, misc_no_movement_set, misc_no_hot_reload_set, misc_no_gui_set, misc_save_on_frame_set, misc_just_render_set, misc_output_path_set, misc_exit_after_rendering_set, help_set};

#define count(_arr) (sizeof(_arr) / sizeof(*_arr))
#define options_count count(options_short)

static_assert(count(options_long) == options_count, "Different number of options_long from options_short!");
static_assert(count(options_help_line) == options_count, "Different number of options_help_line from options_short!");
static_assert(count(options_set) == options_count, "Different number of options_set from options_short!");


// == OPTIONS (IMPLEMENTATIONS) ==
// === SCENE ===
void scene_bvh_type_desc_fn(char *buf) {
  const char desc[] = "BVH strategy to use, choose from: ";
  memcpy(buf, desc, sizeof(desc));
  StringArray_join(buf + sizeof(desc) - 1, BVHStrategy_str, BVHStrategy__COUNT, ", ");
}
void scene_bvh_type_value_str(char *buf, const AppState *app_state) {
  strcpy(buf, BVHStrategy_str[app_state->settings.BVH_build_strat]);
}
HelpLine scene_bvh_type_help_line(const AppState *app_state) {
  HelpLine help_line = {.short_name = scene_bvh_type_short, .long_name = scene_bvh_type_long};
  scene_bvh_type_value_str(help_line.default_value, app_state);
  scene_bvh_type_desc_fn(help_line.description);
  return help_line;
}
void scene_bvh_type_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  const char *arg = argv[*iargv];
  const char *val = get_value_for_option(argc, argv, iargv);

  const int bvh_type_res = StringArray_find_closest_match(val, strlen(val), BVHStrategy_str, BVHStrategy__COUNT);
  if (bvh_type_res == StringArray_find_closest_match_none)
    ERROR_FMT("Invalid value '%s' for option %s", val, arg);
  else if (bvh_type_res == StringArray_find_closest_match_ambiguous)
    ERROR_FMT("Ambiguous value '%s' for option %s", val, arg);

  app_state->settings.BVH_build_strat = bvh_type_res;
}

// TODO: SIMPLIFY some of the implementations

// NOTE: formatting has to be done on per setting basis and not delegated to sth like vec3_str to ensure that the user can understand
// the accepted format for input
// so in other words they should be kept in sync with the parsing utils

// === CAMERA ===
void camera_position_value_str(char *buf, const AppState *app_state) {
  format_vec3(buf, app_state->settings.cam.pos);
}
HelpLine camera_position_help_line(const AppState *app_state) {
  HelpLine help_line = {.short_name = camera_position_short, .long_name = camera_position_long};
  strncpy(help_line.description, camera_position_desc, sizeof(help_line.description));
  camera_position_value_str(help_line.default_value, app_state);
  return help_line;
}
void camera_position_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  app_state->settings.cam.pos = get_value_vec3(argc, argv, iargv);
}

void camera_rotation_value_str(char *buf, const AppState *app_state){
   format_YawPitch(buf, YawPitch_from_dir(Vec3d_from_vec3(app_state->settings.cam.dir)));
}
HelpLine camera_rotation_help_line(const AppState *app_state) {
  HelpLine help_line = {.short_name = camera_rotation_short, .long_name = camera_rotation_long};
  strncpy(help_line.description, camera_rotation_desc, sizeof(help_line.description));
  camera_rotation_value_str(help_line.default_value, app_state);
  return help_line;
}
void camera_rotation_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  app_state->settings.cam.dir = Vec3d_to_vec3(YawPitch_to_dir(get_value_YawPitch(argc, argv, iargv)));
}

void camera_fov_value_str(char *buf, const AppState *app_state){
  format_float(buf, rad_to_deg(app_state->settings.cam.fov_rad));
}
HelpLine camera_fov_help_line(const AppState *app_state) {
  HelpLine help_line = {.short_name = camera_fov_short, .long_name = camera_fov_long};
  strncpy(help_line.description, camera_fov_desc, sizeof(help_line.description));
  camera_fov_value_str(help_line.default_value, app_state);
  return help_line;
}
void camera_fov_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  app_state->settings.cam.fov_rad = deg_to_rad(get_value_float(argc, argv, iargv));
}

void camera_movement_speed_value_str(char *buf, const AppState *app_state){
  format_float(buf, app_state->settings.cam.step_size_per_second);
}
HelpLine camera_movement_speed_help_line(const AppState *app_state) {
  HelpLine help_line = {.short_name = camera_movement_speed_short, .long_name = camera_movement_speed_long};
  strncpy(help_line.description, camera_movement_speed_desc, sizeof(help_line.description));
  camera_movement_speed_value_str(help_line.default_value, app_state);
  return help_line;
}
void camera_movement_speed_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  app_state->settings.cam.step_size_per_second = get_value_float(argc, argv, iargv);
}

void camera_sensitivity_value_str(char *buf, const AppState *app_state){
  format_float(buf, app_state->settings.cam.sensitivity);
}
HelpLine camera_sensitivity_help_line(const AppState *app_state) {
  HelpLine help_line = {.short_name = camera_sensitivity_short, .long_name = camera_sensitivity_long};
  strncpy(help_line.description, camera_sensitivity_desc, sizeof(help_line.description));
  camera_sensitivity_value_str(help_line.default_value, app_state);
  return help_line;
}
void camera_sensitivity_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  app_state->settings.cam.sensitivity = get_value_float(argc, argv, iargv);
}

// === RENDERING ===
void rendering_env_color_value_str(char *buf, const AppState *app_state){
  format_vec3(buf, vec3_from_float3(app_state->settings.rendering_params.env_color));
}
HelpLine rendering_env_color_help_line(const AppState *app_state) {
  HelpLine help_line = {.short_name = rendering_env_color_short, .long_name = rendering_env_color_long};
  strncpy(help_line.description, rendering_env_color_desc, sizeof(help_line.description));
  rendering_env_color_value_str(help_line.default_value, app_state);
  return help_line;
}
void rendering_env_color_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  vec3 color = get_value_vec3(argc, argv, iargv);
  app_state->settings.rendering_params.env_color[0] = color.x;
  app_state->settings.rendering_params.env_color[1] = color.y;
  app_state->settings.rendering_params.env_color[2] = color.z;
  app_state->pending_actions |= Action_update_ssbo_renderer_parameters;
}

void rendering_max_bounce_count_value_str(char *buf, const AppState *app_state){
  format_int(buf, app_state->settings.rendering_params.max_bounce_count);
}
HelpLine rendering_max_bounce_count_help_line(const AppState *app_state) {
  HelpLine help_line = {.short_name = rendering_max_bounce_count_short, .long_name = rendering_max_bounce_count_long};
  strncpy(help_line.description, rendering_max_bounce_count_desc, sizeof(help_line.description));
  rendering_max_bounce_count_value_str(help_line.default_value, app_state);
  return help_line;
}
void rendering_max_bounce_count_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  app_state->settings.rendering_params.max_bounce_count = get_value_int(argc, argv, iargv);
  app_state->pending_actions |= Action_update_ssbo_renderer_parameters;
}

void rendering_samples_per_pixel_value_str(char *buf, const AppState *app_state){
  format_int(buf, app_state->settings.rendering_params.samples_per_pixel);
}
HelpLine rendering_samples_per_pixel_help_line(const AppState *app_state) {
  HelpLine help_line = {.short_name = rendering_samples_per_pixel_short, .long_name = rendering_samples_per_pixel_long};
  strncpy(help_line.description, rendering_samples_per_pixel_desc, sizeof(help_line.description));
  rendering_samples_per_pixel_value_str(help_line.default_value, app_state);
  return help_line;
}
void rendering_samples_per_pixel_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  app_state->settings.rendering_params.samples_per_pixel = get_value_int(argc, argv, iargv);
  app_state->pending_actions |= Action_update_ssbo_renderer_parameters;
}

void rendering_diverge_strength_value_str(char *buf, const AppState *app_state){
  // NOTE: this one has to format explicitly
  // and it has to be .6g as smallest in magnitude value we expect here is 0.000001
  sprintf(buf, "%.6g", app_state->settings.rendering_params.diverge_strength);
}
HelpLine rendering_diverge_strength_help_line(const AppState *app_state) {
  HelpLine help_line = {.short_name = rendering_diverge_strength_short, .long_name = rendering_diverge_strength_long};
  strncpy(help_line.description, rendering_diverge_strength_desc, sizeof(help_line.description));
  rendering_diverge_strength_value_str(help_line.default_value, app_state);
  return help_line;
}
void rendering_diverge_strength_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  app_state->settings.rendering_params.diverge_strength = get_value_float(argc, argv, iargv);
  app_state->pending_actions |= Action_update_ssbo_renderer_parameters;
}

void rendering_frames_to_render_value_str(char *buf, const AppState *app_state){
  format_int(buf, app_state->settings.rendering_params.frames_to_render);
}
HelpLine rendering_frames_to_render_help_line(const AppState *app_state) {
  HelpLine help_line = {.short_name = rendering_frames_to_render_short, .long_name = rendering_frames_to_render_long};
  strncpy(help_line.description, rendering_frames_to_render_desc, sizeof(help_line.description));
  rendering_frames_to_render_value_str(help_line.default_value, app_state);
  return help_line;
}
void rendering_frames_to_render_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  app_state->settings.rendering_params.frames_to_render = get_value_int(argc, argv, iargv);
  app_state->pending_actions |= Action_update_ssbo_renderer_parameters;
}

void rendering_resolution_value_str(char *buf, const AppState *app_state){
  format_WindowResolution(buf, app_state->settings.rendering_params.rendering_resolution);
}
HelpLine rendering_resolution_help_line(const AppState *app_state) {
  HelpLine help_line = {.short_name = rendering_resolution_short, .long_name = rendering_resolution_long};
  strncpy(help_line.description, rendering_resolution_desc, sizeof(help_line.description));
  rendering_resolution_value_str(help_line.default_value, app_state);
  return help_line;
}
void rendering_resolution_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  app_state->settings.rendering_params.rendering_resolution = get_value_WindowResolution(argc, argv, iargv);
  app_state->pending_actions |= Action_update_ssbo_renderer_parameters;
}

// === MISC ===
// TODO: do we want a gui scale setting to be available from the CLI too?
void misc_scaling_desc_fn(char *buf) {
  const char desc[] =  "How the rendered image should be scaled inside the window, choose from: ";
  memcpy(buf, desc, sizeof(desc));
  StringArray_join(buf + sizeof(desc) - 1, WindowScalingMode_str, WindowScalingMode__COUNT, ", ");
}
HelpLine misc_scaling_help_line(const AppState *app_state) {
  HelpLine help_line = {.short_name = misc_scaling_short, .long_name = misc_scaling_long};
  misc_scaling_desc_fn(help_line.description);
  strncpy(help_line.default_value, WindowScalingMode_str[app_state->settings.scaling_mode], sizeof(help_line.default_value));
  return help_line;
}
void misc_scaling_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  const char *arg = argv[*iargv];
  const char *val = get_value_for_option(argc, argv, iargv);
  const int scaling_res = StringArray_find_closest_match(val, strlen(val), WindowScalingMode_str, WindowScalingMode__COUNT);
  if (scaling_res == StringArray_find_closest_match_none)
    ERROR_FMT("Invalid value '%s' for option %s", val, arg);
  else if (scaling_res == StringArray_find_closest_match_ambiguous)
    ERROR_FMT("Ambiguous value '%s' for option %s", val, arg);
  app_state->settings.scaling_mode = scaling_res;
}

HelpLine misc_no_movement_help_line(const AppState *app_state) {
  UNUSED(app_state);
  HelpLine help_line = {.short_name = misc_no_movement_short, .long_name = misc_no_movement_long};
  strncpy(help_line.default_value, "", sizeof(help_line.default_value));
  strncpy(help_line.description, misc_no_movement_desc, sizeof(help_line.description));
  return help_line;
}
void misc_no_movement_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  UNUSED(argc, argv, iargv);
  app_state->settings.movement_enabled = false;
}

HelpLine misc_no_hot_reload_help_line(const AppState *app_state) {
  UNUSED(app_state);
  HelpLine help_line = {.short_name = misc_no_hot_reload_short, .long_name = misc_no_hot_reload_long};
  strncpy(help_line.default_value, "", sizeof(help_line.default_value));
  strncpy(help_line.description, misc_no_hot_reload_desc, sizeof(help_line.description));
  return help_line;
}
void misc_no_hot_reload_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  UNUSED(argc, argv, iargv);
  app_state->settings.hot_reload_enabled = false;
}

HelpLine misc_no_gui_help_line(const AppState *app_state) {
  UNUSED(app_state);
  HelpLine help_line = {.short_name = misc_no_gui_short, .long_name = misc_no_gui_long};
  strncpy(help_line.default_value, "", sizeof(help_line.default_value));
  strncpy(help_line.description, misc_no_gui_desc, sizeof(help_line.description));
  return help_line;
}
void misc_no_gui_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  UNUSED(argc, argv, iargv);
  app_state->settings.gui_enabled = false;
}

HelpLine misc_save_on_frame_help_line(const AppState *app_state) {
  UNUSED(app_state);
  HelpLine help_line = {.short_name = misc_save_on_frame_short, .long_name = misc_save_on_frame_long};
  strncpy(help_line.default_value, "", sizeof(help_line.default_value));
  strncpy(help_line.description, misc_save_on_frame_desc, sizeof(help_line.description));
  return help_line;
}
void misc_save_on_frame_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  UNUSED(argc, argv, iargv);
  app_state->settings.save_after_rendering = true;
}

HelpLine misc_just_render_help_line(const AppState *app_state) {
  UNUSED(app_state);
  HelpLine help_line = {.short_name = misc_just_render_short, .long_name = misc_just_render_long};
  strncpy(help_line.default_value, "", sizeof(help_line.default_value));
  strncpy(help_line.description, misc_just_render_desc, sizeof(help_line.description));
  return help_line;
}
void misc_just_render_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  UNUSED(argc, argv, iargv);
  app_state->settings.gui_enabled = false;
  app_state->settings.hot_reload_enabled = false;
  app_state->settings.movement_enabled = false;
  app_state->settings.save_after_rendering = true;
  app_state->settings.exit_after_rendering = true;
}

HelpLine misc_output_path_help_line(const AppState *app_state) {
  HelpLine help_line = {.short_name = misc_output_path_short, .long_name = misc_output_path_long};
  strncpy(help_line.default_value, app_state->settings.saved_image_path.str, sizeof(help_line.default_value));
  strncpy(help_line.description, misc_output_path_desc, sizeof(help_line.description));
  return help_line;
}
  
void misc_output_path_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  const char *val = get_value_for_option(argc, argv, iargv);
  strncpy(app_state->settings.saved_image_path.str, val, sizeof(app_state->settings.saved_image_path.str));
}

HelpLine misc_exit_after_rendering_help_line(const AppState *app_state) {
  UNUSED(app_state);
  HelpLine help_line = {.short_name = misc_exit_after_rendering_short, .long_name = misc_exit_after_rendering_long};
  strncpy(help_line.default_value, "", sizeof(help_line.default_value));
  strncpy(help_line.description, misc_exit_after_rendering_desc, sizeof(help_line.description));
  return help_line;
}
void misc_exit_after_rendering_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  UNUSED(argc, argv, iargv);
  app_state->settings.exit_after_rendering = true;
}

HelpLine help_help_line(const AppState *app_state) {
  UNUSED(app_state);
  HelpLine help_line = {.short_name = help_short, .long_name = help_long};
  strncpy(help_line.default_value, "", sizeof(help_line.default_value));
  strncpy(help_line.description, help_desc, sizeof(help_line.description));
  return help_line;
}

static void display_help(const char *program_name, const AppState *app_state) {
  printf("Usage: %s [GLTF_SCENE] [OPTIONS]\n", program_name);
  printf("Example:\n");
  printf("%s dragon.glb -J --frames-to-render 100 --out dragon.png\n\n",
         program_name);

  printf("Options:\n");
  HelpLine help_lines[options_count];
  for (size_t i = 0; i < options_count; ++i) {
    help_lines[i] = options_help_line[i](app_state);
  }

  ColStats cols_stats = ColStats_get(help_lines, options_count);
  for (size_t i = 0; i < options_count; ++i) {
    HelpLine_print(help_lines[i], cols_stats);
  }
}

void help_set(AppState *app_state, int argc, const char **argv, int *iargv) {
  UNUSED(argc, iargv);
  display_help(argv[0], app_state);
  exit(EXIT_SUCCESS);
}

// TODO: iterate through all options and find_closest_match, then run its SetOption
static void parse_arg(int argc, const char **argv, int *i, AppState *app_state) {
  const char *arg = argv[*i];
  const int arg_len = strlen(arg);
  if (arg_len < 2) ERROR_FMT("Invalid argument: %s", arg); 

  const bool is_op = arg[0] == '-';
  const bool is_long_op = is_op && arg[1] == '-';

  if (is_op) {
    int op_i = -1;
    if (is_long_op)
      op_i = StringArray_find_closest_match(arg, arg_len, options_long, options_count); 
    else
      op_i = StringArray_find_closest_match(arg, arg_len, options_short, options_count); 

    if (op_i == StringArray_find_closest_match_none)
      ERROR_FMT("No such option '%s'", arg);
    else if (op_i == StringArray_find_closest_match_ambiguous)
      ERROR_FMT("Ambiguous option '%s'", arg);

    SetOptionFn *option_set_value_fn = options_set[op_i];
    option_set_value_fn(app_state, argc, argv, i);
  } else {
    // NOTE: if it's not an option then it's value of a positional

    // if positional was already provided
    if (app_state->pending_actions & Action_load_scene)
      ERROR("Scene path was specified twice");

    app_state->settings.scene_path = SmallString_new(arg);
    app_state->pending_actions |= Action_load_scene;
  }
}

// NOTE: assuming app_state contains its default state
void handle_args(int argc, const char **argv, AppState *app_state) {
  if (argc <= 1)
    return;

  for (int i = 1; i < argc; ++i) {
    parse_arg(argc, argv, &i, app_state);
  }

  if (!app_state->settings.gui_enabled &&
      SmallString_is_empty(&app_state->settings.scene_path)) {
    ERROR("GUI was disabled, yet no scene was specified.\n");
  }

  if (!app_state->settings.gui_enabled &&
      app_state->settings.save_after_rendering &&
      app_state->settings.rendering_params.frames_to_render < 0) {
    fprintf(stderr,
            YELLOW("WARNING:") " GUI was disabled, and infinite number of frames were "
            "asked for, if you wanted to render an image consider setting "
            "--frames-to-render to a positive value");
  }
}
