#include "cli.h"
#include "app_state.h"
#include "asserts.h"
#include "opengl/resolution.h"
#include "scene/bvh/strategies.h"
#include "utils.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Options {
  Options_BVH_TYPE,
  Options_SAMPLES_PER_PIXEL,
  Options_MAX_BOUNCE_COUNT,
  Options_FRAMES_TO_RENDER,
  Options_RESOLUTION,
  Options_OUTPUT_PATH,
  Options_FLAG_NO_GUI,
  Options_FLAG_NO_HOT_RELOAD,
  Options_FLAG_NO_MOVEMENT,
  Options_FLAG_EXIT_AFTER_RENDERING,
  Options_FLAG_SAVE_AFTER_RENDERING,
  Options_FLAG_JUST_RENDER,
  Options_FLAG_HELP,
  Options__COUNT,
};

const int FIRST_OTHER_OPTION = Options_OUTPUT_PATH;

typedef struct {
  char description[256];
  char long_name[128];
  char default_value[64];
  char short_name[8];
} Option;

#define RETURN_OPTION_DV_FMT(_short_name, _long_name, _desc,                   \
                             _default_value_fmt, ...)                          \
  do {                                                                         \
    Option res = {.short_name = _short_name,                                   \
                  .long_name = _long_name,                                     \
                  .description = _desc};                                       \
                                                                               \
    snprintf(res.default_value, sizeof(res.default_value), _default_value_fmt, \
             __VA_ARGS__);                                                     \
    return res;                                                                \
  } while (0);

#define RETURN_OPTION(_short_name, _long_name, _desc)                          \
  return (Option){.short_name = _short_name,                                   \
                  .long_name = _long_name,                                     \
                  .description = _desc,                                        \
                  .default_value = ""};

Option Option_new(enum Options option, const AppState *default_app_state) {
  switch (option) {
  case Options_SAMPLES_PER_PIXEL:
    RETURN_OPTION_DV_FMT("-spp", "--samples-per-pixel", "", "%d",
                         default_app_state->rendering_params.samples_per_pixel);
  case Options_MAX_BOUNCE_COUNT:
    RETURN_OPTION_DV_FMT("", "--max-bounce-count", "", "%d",
                         default_app_state->rendering_params.max_bounce_count);
  case Options_FRAMES_TO_RENDER:
    RETURN_OPTION_DV_FMT("-F", "--frames-to-render", "", "%d",
                         default_app_state->rendering_params.frames_to_render);
  case Options_RESOLUTION: {
    OpenGLResolution resolution =
        default_app_state->rendering_params.rendering_resolution;
    RETURN_OPTION_DV_FMT("-R", "--resolution", "", "%dx%d", resolution.width,
                         resolution.height);
  }
  case Options_OUTPUT_PATH:
    RETURN_OPTION_DV_FMT("-o", "--out", "", "%s",
                         default_app_state->save_image_info.path);
  case Options_BVH_TYPE: {
    Option res = {.short_name = "", .long_name = "--bvh"};
    strcpy(res.description, "BVH strategy to use, choose from: ");
    StringArray_join(res.description, sizeof(res.description), BVHStrategy_str,
                     BVHStrategy__COUNT, ", ");
    strncpy(res.default_value,
            BVHStrategy_str[default_app_state->BVH_build_strat],
            sizeof(res.default_value));
    return res;
  }

  case Options_FLAG_EXIT_AFTER_RENDERING:
    RETURN_OPTION("-X", "--exit-after-rendering", "");
  case Options_FLAG_SAVE_AFTER_RENDERING:
    RETURN_OPTION("-S", "--save-after-rendering",
                  "Automatically save rendered image to PNG.");
  case Options_FLAG_NO_GUI:
    RETURN_OPTION("-NG", "--no-gui", "Run without GUI overlay.");
  case Options_FLAG_NO_HOT_RELOAD:
    RETURN_OPTION("-NH", "--no-hot-reload",
                  "Disable hot-reloading of shaders.");
  case Options_FLAG_NO_MOVEMENT:
    RETURN_OPTION("-NM", "--no-movement", "Disable camera movement.");
  case Options_FLAG_HELP:
    RETURN_OPTION("-h", "--help", "Display this help");
  case Options_FLAG_JUST_RENDER:
    RETURN_OPTION("-J", "--just-render",
                  "Alias for --no-gui --no-hot-reload --no-movement "
                  "--save-after-rendering "
                  "--exit-after-rendering.");
  case Options__COUNT:
    UNREACHABLE();
  }
}

static void display_help(const char *program_name,
                         const Option all_options[Options__COUNT]) {
  printf("Usage: %s [GLTF_SCENE] [OPTIONS]\n", program_name);
  printf("Example:\n");
  printf("%s dragon.glb -J --frames-to-render 100 --out dragon.png\n",
         program_name);
  printf("\n");

  printf("Rendering Options:\n");
  for (int i = 0; i < FIRST_OTHER_OPTION; ++i) {
    Option op = all_options[i];
    printf("  %s %s [%s]  %s\n", op.short_name, op.long_name, op.default_value,
           op.description);
  }

  printf("\nOther Options:\n");
  for (int i = FIRST_OTHER_OPTION; i < Options__COUNT; ++i) {
    Option op = all_options[i];
    // if a flag
    if (op.default_value[0] == 0) {
      printf("  %s %s  %s\n", op.short_name, op.long_name, op.description);
    } else {
      printf("  %s %s [%s] %s\n", op.short_name, op.long_name, op.default_value,
             op.description);
    }
  }
  exit(0);
}

// NOTE: returns last option if arg wasn't found
enum Options find_option(const char *arg,
                         const Option all_options[Options__COUNT]) {
  for (int i = 0; i < Options__COUNT; ++i) {
    if (((all_options[i].short_name[0] != 0) &&
         strcmp(arg, all_options[i].short_name) == 0) ||
        strcmp(arg, all_options[i].long_name) == 0) {
      return i;
    }
  }
  return Options__COUNT;
}

bool parse_number(const char *str, int *n) {
  if (sscanf(str, "%d", n) == 1)
    return true;
  else if (errno != 0) {
    perror("sscanf");
  }
  return false;
}

void parse_number_exit(const char *str, int *n, const char *arg) {
  if (!parse_number(str, n)) {
    fprintf(stderr,
            "Invalid value '%s' provided for %s, expected an integer.\n", str,
            arg);
    exit(1);
  }
}

bool parse_resolution(const char *str, unsigned int *width,
                      unsigned int *height) {
  char *x_pos = strchr(str, 'x');
  if (!x_pos)
    return false;

  if (sscanf(str, "%dx%d", width, height) != 2) {
    if (errno != 0)
      perror("sscanf");
    return false;
  }

  if (*width <= 0 || *height <= 0)
    return false;
  return true;
}

static char *get_value_for_option(int *i, int argc, char *argv[],
                                  const char *flag) {
  if (*i + 1 >= argc) {
    fprintf(stderr, "Error: Option %s requires a value.\n", flag);
    exit(1);
  }
  return argv[++(*i)];
}

static int find_closest_match(const char *s, unsigned int s_len,
                              const char *list[], unsigned int list_length) {
  int match = -1;
  for (unsigned int i = 0; i < list_length; ++i) {
    bool equal = true;
    for (unsigned int c = 0; c < s_len; ++c)
      equal &= (tolower(s[c]) == tolower(list[i][c]));

    if (equal) {
      if (match != -1)
        TODO("Ambiguous argument");
      match = i;
    }
  }
  return match;
}

static void parse_arg(int argc, char **argv, int *i,
                      const Option all_options[Options__COUNT],
                      AppState *app_state) {
  char *arg = argv[*i];
  enum Options option = find_option(arg, all_options);
  if (option == Options__COUNT) {
    if (arg[0] == '-') {
      fprintf(stderr,
              "Invalid option: %s\nTry '%s --help' for more information.\n",
              argv[*i], argv[0]);
      exit(1);
    } else {
      app_state->scene_paths.new_scene_path = SmallString_new(arg);
      return;
    }
  }

  switch (option) {
  case Options_FLAG_HELP:
    display_help(argv[0], all_options);
    break;

  case Options_SAMPLES_PER_PIXEL:
    parse_number_exit(get_value_for_option(i, argc, argv, arg),
                      &app_state->rendering_params.samples_per_pixel, arg);
    break;

  case Options_MAX_BOUNCE_COUNT:
    parse_number_exit(get_value_for_option(i, argc, argv, arg),
                      &app_state->rendering_params.max_bounce_count, arg);
    break;

  case Options_FRAMES_TO_RENDER:
    parse_number_exit(get_value_for_option(i, argc, argv, arg),
                      &app_state->rendering_params.frames_to_render, arg);
    break;

  case Options_RESOLUTION: {
    char *res_value = get_value_for_option(i, argc, argv, arg);
    if (!parse_resolution(
            res_value, &app_state->rendering_params.rendering_resolution.width,
            &app_state->rendering_params.rendering_resolution.height)) {
      fprintf(stderr, "Error: Invalid resolution format '%s'.\n", res_value);
      exit(1);
    }
    break;
  }

  case Options_OUTPUT_PATH: {
    char *path = get_value_for_option(i, argc, argv, arg);
    snprintf(app_state->save_image_info.path,
             sizeof(app_state->save_image_info.path), "%s", path);
    break;
  }
  case Options_BVH_TYPE: {
    char *arg_val_str = get_value_for_option(i, argc, argv, arg);
    int arg_val_len = strlen(arg_val_str);

    int match = find_closest_match(arg_val_str, arg_val_len, BVHStrategy_str,
                                   BVHStrategy__COUNT);
    if (match == -1)
      ERROR_FMT("BVH strategy '%s' is not available", arg_val_str);
    app_state->BVH_build_strat = match;
    break;
  }

  case Options_FLAG_NO_GUI:
    app_state->gui_enabled = false;
    break;

  case Options_FLAG_NO_HOT_RELOAD:
    app_state->hot_reload_enabled = false;
    break;

  case Options_FLAG_NO_MOVEMENT:
    app_state->movement_enabled = false;
    break;

  case Options_FLAG_EXIT_AFTER_RENDERING:
    app_state->exit_after_rendering = true;
    break;

  case Options_FLAG_SAVE_AFTER_RENDERING:
    app_state->save_after_rendering = true;
    break;

  case Options_FLAG_JUST_RENDER:
    app_state->gui_enabled = false;
    app_state->hot_reload_enabled = false;
    app_state->movement_enabled = false;
    app_state->save_after_rendering = true;
    app_state->exit_after_rendering = true;
    break;

  case Options__COUNT:
    UNREACHABLE();
  }
}

// NOTE: assuming app_state contains its default state
void handle_args(int argc, char *argv[], AppState *app_state) {
  // no arguments to handle
  if (argc < 2)
    return;

  Option all_options[Options__COUNT] = {0};
  for (int i = 0; i < Options__COUNT; ++i) {
    all_options[i] = Option_new(i, app_state);
  }

  for (int i = 1; i < argc; ++i) {
    parse_arg(argc, argv, &i, all_options, app_state);
  }

  if (!app_state->gui_enabled &&
      SmallString_is_empty(&app_state->scene_paths.new_scene_path)) {
    fprintf(stderr, "GUI was disabled, yet no scene was specified. Exiting!\n");
    exit(1);
  }

  if (!app_state->gui_enabled && app_state->save_after_rendering &&
      app_state->rendering_params.frames_to_render < 0) {
    fprintf(stderr,
            "WARNING: GUI was disabled, and infinite number of frames were "
            "asked for, if you wanted to render an image consider setting "
            "--frames-to-render to a positive value.");
  }
}
