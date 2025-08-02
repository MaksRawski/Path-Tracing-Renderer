
#define DEBUG

#include <glad/gl.h>
//
#include <GLFW/glfw3.h>
//
#include "inputs.h"
#include "obj_parser.h"
#include "renderer.h"
//
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char help_str[] =
    "Usage: %s MODEL POSITION MATERIAL\n"
    "Renders MODEL at a POSITION using one of the builtin MATERIALs.\n\n"
    "MODEL can be either a path to triangulated obj file or a special keyword\n"
    "SPHERER, where R should be replaced with the radius of the sphere e.g. "
    "SPHERE1.5\n"
    "POSITION should be specified as x,y,z e.g. 0,1.5,-2.1\n"
    "MATERIAL should be a name of one of the builtin materials: WHITE, MIRROR, "
    "RED, GREEN, BLUE, GOLD, BLACK.\n\n"
    "Example: %s SPHERE1.75 2,0,2 MIRROR, SPHERE1.75 -2,0,2 MIRROR\n"
    "Note: camera will always be at 0,0,0 and will look at a point 0,0,2\n";

void parse_cli(int argc, char *argv[], char *model_path[], vec3 *offset,
               int *mat_index) {
  // 0 -> MODEL
  // 1 -> POSITION
  // 2 -> MATERIAL
  // 3 -> DONE
  int arg_type = 0;
  offset = NULL;
  *mat_index = 0;
  *model_path = NULL;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printf(help_str, argv[0], argv[0]);
      exit(EXIT_SUCCESS);
    }

    switch (arg_type++) {
    case 0:
      *model_path = malloc((strlen(argv[i]) + 1) * sizeof(char));
      strcpy(*model_path, argv[i]);
      break;
    case 1:
      offset = malloc(sizeof(vec3));
      sscanf(argv[i], "%f,%f,%f", &offset->l[0], &offset->l[1], &offset->l[2]);
      break;
    case 2:
      if (strcmp(argv[i], "WHITE"))
        *mat_index = 0;
      else if (strcmp(argv[i], "MIRROR"))
        *mat_index = 1;
      else if (strcmp(argv[i], "RED"))
        *mat_index = 2;
      else if (strcmp(argv[i], "GREEN"))
        *mat_index = 3;
      else if (strcmp(argv[i], "BLUE"))
        *mat_index = 4;
      else if (strcmp(argv[i], "GOLD"))
        *mat_index = 5;
      else if (strcmp(argv[i], "BLACK"))
        *mat_index = 6;
      else {
        fprintf(stderr,
                "Unknown material: %s\nCan only be one of: WHITE, MIRROR, RED, "
                "GREEN, BLUE, GOLD, BLACK.",
                argv[i]);
        exit(EXIT_FAILURE);
      }
      printf("Using %s material.\n", argv[i]);
      break;
    default:
      break;
    }
  }
}

void load_model(char model_path[], vec3 offset, int mat_index, ModelsBuffer *mb,
                int shader_program) {
  if (strncmp(model_path, "SPHERE", 6) == 7) {
    // TODO: handle SPHERER
    fprintf(stderr, "SPHERE model type not yet implemented!\n");
    exit(EXIT_FAILURE);
  } else {
    int model_id = load_obj_model(model_path, shader_program, mb, &offset);
    set_model_material(mb, model_id, mat_index);
  }
}

int main(int argc, char *argv[]) {
  char *model_path = NULL;
  vec3 offset;
  int mat_index;
  parse_cli(argc, argv, &model_path, &offset, &mat_index);

  GLFWwindow *window = setup_opengl(/* disable_vsync = */ false);

  GLuint shader_program;
  FilesWatcher shader_watcher;
  RendererBuffers rb;
  // TODO: replace with just renderer.glsl
  setup_renderer("vertex.glsl", "renderer.glsl", &shader_program,
                 &shader_watcher, &rb);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  BackBuffer bb;
  setup_back_buffer(shader_program, &bb, width, height);

  Uniforms uniforms = {.camFov = PI / 2.0,
                       .camLookat = {1.0, 1.0, 0.0},
                       .camPos = {0.0, 1.0, 0.0},
                       .camUp = {0.0, 1.0, 0.0},
                       .iResolution = {width, height},
                       .iFrame = 0};

  unsigned int frame_counter = 0;
  double last_frame_time = glfwGetTime();

  ModelsBuffer mb = {0};

  set_material_slot(&mb, 0, &white_mat);
  set_material_slot(&mb, 1, &mirror_mat);
  set_material_slot(&mb, 2, &red_mat);
  set_material_slot(&mb, 3, &green_mat);
  set_material_slot(&mb, 4, &blue_mat);
  set_material_slot(&mb, 5, &gold_mat);
  set_material_slot(&mb, 6, &black_mat);
  printf("mat0 = %f %f %f\n", mb.materials[0].albedo[0],
         mb.materials[0].albedo[1], mb.materials[0].albedo[2]);
  printf("mat1 = %f %f %f\n", mb.materials[1].albedo[0],
         mb.materials[1].albedo[1], mb.materials[1].albedo[2]);
  printf("mat2 = %f %f %f\n", mb.materials[2].albedo[0],
         mb.materials[2].albedo[1], mb.materials[2].albedo[2]);
  printf("mat3 = %f %f %f\n", mb.materials[3].albedo[0],
         mb.materials[3].albedo[1], mb.materials[3].albedo[2]);
  printf("mat4 = %f %f %f\n", mb.materials[4].albedo[0],
         mb.materials[4].albedo[1], mb.materials[4].albedo[2]);
  printf("mat5 = %f %f %f\n", mb.materials[5].albedo[0],
         mb.materials[5].albedo[1], mb.materials[5].albedo[2]);
  printf("mat6 = %f %f %f\n", mb.materials[6].albedo[0],
         mb.materials[6].albedo[1], mb.materials[6].albedo[2]);

  if (model_path != NULL) {
    printf("Loading model %s\n", model_path);
    load_model(model_path, offset, mat_index, &mb, shader_program);
    free(model_path);
  } else {
    printf("Loading default scene...\n");
  }

  while (!glfwWindowShouldClose(window)) {
    bool did_reload = reload_shader(&shader_program, &shader_watcher);
    if (update_uniforms(window, &uniforms)) {
      did_reload = true;
    }
    if (did_reload) {
      frame_counter = 0;
      uniforms.iFrame = 0;
      display_fps(window, &frame_counter, &last_frame_time);
    }

    // recalculate window dimensions
    int new_width, new_height;
    glfwGetFramebufferSize(window, &new_width, &new_height);
    if (new_width != width || new_height != height) {
      width = new_width;
      height = new_height;
      uniforms.iResolution[0] = width;
      uniforms.iResolution[1] = height;

      // Resize the backbuffer texture
      glBindTexture(GL_TEXTURE_2D, bb.fboTex);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                   GL_UNSIGNED_BYTE, NULL);
      glBindTexture(GL_TEXTURE_2D, 0);

      glViewport(0, 0, width, height);
      uniforms.iFrame = 0;
    }
    display_fps(window, &frame_counter, &last_frame_time);
    frame_counter++;

    glClear(GL_COLOR_BUFFER_BIT);
    update_frame(shader_program, window, &uniforms, &rb, &bb, &mb);

    glfwPollEvents();
    ++uniforms.iFrame;
  }

  free_gl_buffers(&rb, &bb, &mb);
  delete_file_watcher(&shader_watcher);
  glDeleteProgram(shader_program);
  glfwTerminate();

  return 0;
}
