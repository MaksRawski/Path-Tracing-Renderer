#include "utils.h"
#include "obj_parser.h"
#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// for hot reloading on linux
#include <fcntl.h>
#include <sys/inotify.h>

// === MODELS BUFFER ===

ModelsBuffer mb_init() {
  ModelsBuffer mb = {0};

  mb.materials_capacity = 8;
  mb.materials = calloc(mb.materials_capacity, sizeof(Material));

  mb.meshes_capacity = 8;
  mb.meshes_info = calloc(mb.meshes_capacity, sizeof(MeshInfo));

  mb.triangles_capacity =
      1024 * 1024; // 1 MB, should be more than enough for start
  mb.triangles = malloc(mb.triangles_capacity * sizeof(Triangle));

  return mb;
}

// reallocs if number of meshes is at more than half of the capacity
void mb_realloc_mi(ModelsBuffer *mb) {
  if (2 * mb->num_of_meshes > mb->meshes_capacity) {
    mb->meshes_capacity *= 2;
    void *result =
        realloc(mb->meshes_info, mb->meshes_capacity * sizeof(MeshInfo));
    if (result == NULL) {
      fprintf(stderr,
              "Failed to reallocate %lu bytes of memory for meshesInfo!\n",
              mb->num_of_meshes * sizeof(MeshInfo));
      free(mb->meshes_info);
      exit(EXIT_FAILURE);
    }
    mb->meshes_info = result;
  }
}
// reallocates mb->triangles if adding num_of_triangles overflows capacity
void mb_realloc_triangles(ModelsBuffer *mb, int num_of_triangles) {
  int new_size = mb->num_of_triangles + num_of_triangles;

  if (mb->triangles_capacity < new_size) {
    do
      mb->triangles_capacity *= 2;
    while (mb->triangles_capacity < new_size);

    void *result =
        realloc(mb->triangles, mb->triangles_capacity * sizeof(Triangle));

    if (result == NULL) {
      free(mb->triangles);
      fprintf(stderr,
              "Failed to reallocate %lu bytes of memory for triangles!\n",
              mb->triangles_capacity);
      exit(EXIT_FAILURE);
    } else {
      mb->triangles = result;
    }
  }
}

// reallocs if number of materials is at more than half of the capacity
void mb_realloc_materials(ModelsBuffer *mb) {
  if (mb->num_of_materials * 2 > mb->materials_capacity) {
    int new_min_capacity = mb->num_of_materials * 2;
    do
      mb->materials_capacity *= 2;
    while (mb->materials_capacity < new_min_capacity);

    void *result =
        realloc(mb->materials, mb->materials_capacity * sizeof(Material));

    if (result == NULL) {
      free(mb->materials);
      fprintf(stderr,
              "Failed to reallocate %lu bytes of memory for materials!\n",
              mb->materials_capacity);
      exit(EXIT_FAILURE);
    } else {
      mb->materials = result;
    }
  }
}

// returns index to the first allocated triangle
int mb_add_triangles(ModelsBuffer *mb, Triangle *triangles,
                     int num_of_triangles) {
  mb_realloc_triangles(mb, num_of_triangles);
  memcpy(mb->triangles + mb->num_of_triangles, triangles, num_of_triangles * sizeof(Triangle));

  int index = mb->num_of_triangles;
  mb->num_of_triangles += num_of_triangles;
  return index;
}

// TODO: choosable mesh material?
void mb_add_mesh(ModelsBuffer *mb, ObjInfo info, int first_triangle_index) {
  // realloc meshes info if necessary
  mb_realloc_mi(mb);

  // set the mesh info for the new model
  MeshInfo mi = {.firstTriangleIndex = first_triangle_index,
                 .numTriangles = info.f,
                 // default material will be stored at index 0
                 .materialIndex = 0,
                 .boundsMin = {info.bounds_min.l[0], info.bounds_min.l[1],
                               info.bounds_min.l[2]},
                 .boundsMax = {info.bounds_max.l[0], info.bounds_max.l[1],
                               info.bounds_max.l[2]}};

  mb->meshes_info[mb->num_of_meshes++] = mi;
}

// expects mb to be initialized with `models_buffer_init`
void set_material_slot(ModelsBuffer *mb, int index, const Material *mat) {
  if (index > mb->num_of_materials) {
    printf("Tried to set a material at slot %d even though slot %d is free! "
           "Doing nothing...\n",
           index, mb->num_of_materials);
    return;
  }

  if (index == mb->num_of_materials)
    mb->num_of_materials = index + 1;
  mb->materials[index] = *mat;
}

void set_model_material(ModelsBuffer *mb, int model_id, int mat_index) {
  if (model_id >= mb->num_of_meshes) {
    fprintf(stderr, "No loaded model has ID %d!\n", model_id);
    exit(EXIT_FAILURE);
  }
  if (mat_index >= mb->num_of_materials) {
    fprintf(stderr, "No material has ID %d!\n", model_id);
    exit(EXIT_FAILURE);
  }

  mb->meshes_info[model_id].materialIndex = mat_index;
}

// === FILES ===

char *read_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Error: Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = (char *)malloc(length + 1);
  if (!buffer) {
    fprintf(stderr, "Error: Could not allocate memory for file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fread(buffer, 1, length, file);
  buffer[length] = '\0';

  fclose(file);
  return buffer;
}

int watch_file(const char *path) {
  int fd = inotify_init();
  if (fd < 0) {
    perror("inotify_init");
    exit(EXIT_FAILURE);
  }

  int wd = inotify_add_watch(fd, path, IN_MODIFY);
  if (wd == -1) {
    fprintf(stderr, "Cannot watch '%s'\n", path);
    exit(EXIT_FAILURE);
  }

  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  return fd;
}

void delete_file_watcher(FilesWatcher *fw) {
  for (int i = 0; i < fw->num_of_files; ++i) {
    close(fw->watcher_fds[i]);
  }
  free(fw->file_names);
  free(fw->watcher_fds);
}

// === OPENGL ===

// expects the shader program to be loaded
// https://www.khronos.org/opengl/wiki/Buffer_Texture
void create_gl_texture(GLuint *tbo, GLuint *tbo_tex, unsigned long size,
                       void *data, GLenum tex_format, GLenum tex_index) {
  glActiveTexture(tex_index);

  // write texture data to tbo
  glGenBuffers(1, tbo);
  glBindBuffer(GL_ARRAY_BUFFER, *tbo);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

  glGenTextures(1, tbo_tex);
  glBindTexture(GL_TEXTURE_BUFFER, *tbo_tex);
  // depending on the chosen format each "texel" (pixel of a texture)
  // will contain a value in this format so e.g. for GL_RGB32F each texel
  // will store 3 floats AKA vec3
  glTexBuffer(GL_TEXTURE_BUFFER, tex_format, *tbo);

  // unbind the texture buffer
  glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

// NOTE: should be called after all the models have been loaded
void setup_gl_buffers(ModelsBuffer *mb, GLuint shader_program) {
  // create texture buffers for triangles and meshesInfo
  create_gl_texture(&mb->tbo_triangles, &mb->tbo_tex_triangles,
                    mb->num_of_triangles * sizeof(Triangle), mb->triangles,
                    GL_RGB32F, GL_TEXTURE1);
  glUniform1i(glGetUniformLocation(shader_program, "trianglesBuffer"), 1);

  create_gl_texture(&mb->tbo_meshes, &mb->tbo_tex_meshes,
                    mb->num_of_meshes * sizeof(MeshInfo), mb->meshes_info,
                    GL_RGB32F, GL_TEXTURE2);

  glUniform1i(glGetUniformLocation(shader_program, "meshesInfoBuffer"), 2);

  create_gl_texture(&mb->tbo_materials, &mb->tbo_tex_materials,
                    mb->num_of_materials * sizeof(Material), mb->materials,
                    GL_RGBA32F, GL_TEXTURE3);
  glUniform1i(glGetUniformLocation(shader_program, "materialsBuffer"), 3);

  glUniform1i(glGetUniformLocation(shader_program, "numOfMeshes"),
              mb->num_of_meshes);

  // unbind the texture buffer
  glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void free_gl_buffers(RendererBuffers *rb, BackBuffer *bb, ModelsBuffer *mb) {
  glDeleteVertexArrays(1, &rb->vao);
  glDeleteBuffers(1, &rb->vbo);
  glDeleteFramebuffers(1, &bb->fbo);
  glDeleteTextures(1, &bb->fboTex);
  glDeleteBuffers(1, &mb->tbo_triangles);
  glDeleteTextures(1, &mb->tbo_tex_triangles);
  glDeleteBuffers(1, &mb->tbo_meshes);
  glDeleteTextures(1, &mb->tbo_tex_meshes);
  glDeleteBuffers(1, &mb->tbo_materials);
  glDeleteTextures(1, &mb->tbo_tex_materials);
  free(mb->triangles);
  free(mb->meshes_info);
  free(mb->materials);
}
