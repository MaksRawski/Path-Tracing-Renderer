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
  mb.materials = calloc(8, sizeof(Material));
  mb.meshesInfo = calloc(8, sizeof(MeshInfo));
  return mb;
}

void mb_realloc_mi(ModelsBuffer *mb) {
  // realloc if the initial 8 elements have been filled and
  // every time num_of_meshes is a power of two
  if (mb->num_of_meshes >= 8 &&
      (mb->num_of_meshes & (mb->num_of_meshes - 1)) == 0) {
    void *result =
        realloc(mb->meshesInfo, 2 * mb->num_of_meshes * sizeof(MeshInfo));
    if (result == NULL) {
      fprintf(stderr,
              "Failed to reallocate %lu bytes of memory for meshesInfo!\n",
              2 * mb->num_of_meshes * sizeof(MeshInfo));
      free(mb->meshesInfo);
      exit(EXIT_FAILURE);
    }
    mb->meshesInfo = result;
  }
}

// returns index to the first allocated triangle
int mb_add_triangles(ModelsBuffer *mb, Triangle *triangles,
                     int num_of_triangles) {
  int triangles_already_loaded =
      mb->num_of_meshes == 0
          ? 0
          : mb->meshesInfo[mb->num_of_meshes - 1].firstTriangleIndex +
                mb->meshesInfo[mb->num_of_meshes - 1].numTriangles;

  // realloc to fit new triangles
  void *result =
      realloc(mb->triangles,
              (triangles_already_loaded + num_of_triangles) * sizeof(Triangle));
  if (result == NULL) {
    free(mb->triangles);
    fprintf(stderr, "Failed to reallocate %lu bytes of memory for triangles!\n",
            (triangles_already_loaded + num_of_triangles) * sizeof(Triangle));
    exit(EXIT_FAILURE);
  } else
    mb->triangles = result;

  memcpy(mb->triangles + triangles_already_loaded, triangles,
         num_of_triangles * sizeof(Triangle));

  /* // delete previous buffers and textures as we will set them later */
  /* // with new data */
  /* glDeleteBuffers(1, &mb->tbo_triangles); */
  /* glDeleteTextures(1, &mb->tbo_tex_triangles); */
  /* glDeleteBuffers(1, &mb->tbo_meshes); */
  /* glDeleteTextures(1, &mb->tbo_tex_meshes); */

  return triangles_already_loaded;
}

// TODO: choosable mesh material?
void mb_add_mesh(ModelsBuffer *mb, ObjInfo info, int first_triangle_index) {
  // realloc meshes info if necessary
  mb_realloc_mi(mb);

  // set the mesh info for the new model
  MeshInfo mi = {.firstTriangleIndex = first_triangle_index,
                 .numTriangles = info.f,
                 .materialIndex =
                     0, // default material will be stored at index 0
                 .boundsMin = {info.bounds_min.l[0], info.bounds_min.l[1],
                               info.bounds_min.l[2]},
                 .boundsMax = {info.bounds_max.l[0], info.bounds_max.l[1],
                               info.bounds_max.l[2]}};

  mb->meshesInfo[mb->num_of_meshes++] = mi;
}

// expects mb to be initialized with `models_buffer_init`
void set_material_slot(ModelsBuffer *mb, int index, const Material *mat) {
  if (mb->num_of_materials == 0) {
    mb->materials = malloc(4 * sizeof(Material));
    mb->materials_capacity = 4;
  }

  // if we try to set a material that has an index bigger than 4
  // that are allocated initially (or are allocated overall),
  // we should reallocate but only when the num_of_materials
  // is a power of 2 and then we increase the size by multiplying by 2
  else if (index >= (int)mb->materials_capacity) {
    while ((int)mb->materials_capacity < index + 1) {
      mb->materials_capacity *= 2;
    }
    void *result =
        realloc(mb->materials, mb->materials_capacity * sizeof(Material));
    if (result == NULL) {
      fprintf(stderr,
              "Failed to realocate %zu bytes of memory for materials!\n",
              2 * index * sizeof(Material));
      exit(EXIT_FAILURE);
    } else
      mb->materials = result;
  }
  if (index >= (int)mb->num_of_materials)
    mb->num_of_materials = index + 1;
  mb->materials[index] = *mat;

  // TODO: this should be done outside of this function
  /* glDeleteBuffers(1, &mb->tbo_materials); */
  /* glDeleteTextures(1, &mb->tbo_tex_materials); */

}

void set_model_material(ModelsBuffer *mb, int model_id, int mat_index) {
  if ((unsigned int)model_id >= mb->num_of_meshes) {
    fprintf(stderr, "No loaded model has ID %d!\n", model_id);
    exit(EXIT_FAILURE);
  }
  if ((unsigned int)mat_index >= mb->num_of_materials) {
    fprintf(stderr, "No material has ID %d!\n", model_id);
    exit(EXIT_FAILURE);
  }

  mb->meshesInfo[model_id].materialIndex = mat_index;

  // TODO: this should be done outside of this function
  /* glDeleteBuffers(1, &mb->tbo_meshes); */
  /* glDeleteTextures(1, &mb->tbo_tex_meshes); */

  /* create_gl_buffer(&mb->tbo_meshes, &mb->tbo_tex_meshes, */
  /*                  mb->num_of_meshes * sizeof(MeshInfo), mb->meshesInfo, */
  /*                  GL_RGB32F, GL_TEXTURE2); */
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
                   mb->num_of_meshes * sizeof(MeshInfo), mb->meshesInfo,
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
  free(mb->meshesInfo);
  free(mb->materials);
}
