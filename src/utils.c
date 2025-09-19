#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// for hot reloading on linux
#include <fcntl.h>
#include <sys/inotify.h>

void print_triangles(Triangle *tri, vec3 *centroids, int count) {
  for (int t = 0; t < count; ++t) {
    printf("Triangle % 3d: ", t);
    printf("%+f %+f %+f -- ", tri[t].a.x, tri[t].a.y, tri[t].a.z);
    printf("%+f %+f %+f -- ", tri[t].b.x, tri[t].b.y, tri[t].b.z);
    printf("%+f %+f %+f ", tri[t].c.x, tri[t].c.y, tri[t].c.z);
    printf("(%+f %+f %+f)\n", centroids[t].x, centroids[t].y, centroids[t].z);
    /* printf("Normals     : "); */
    /* printf("%+f %+f %+f -- ", tri->na.x, tri->na.y, tri->na.z); */
    /* printf("%+f %+f %+f -- ", tri->nb.x, tri->nb.y, tri->nb.z); */
    /* printf("%+f %+f %+f\n", tri->nc.x, tri->nc.y, tri->nc.z); */
  }
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

void init_shader_watcher(RFilesWatcher *shader_watcher,
                         const char *vertex_shader_filename,
                         const char *fragment_shader_filename) {
  shader_watcher->num_of_files = 2;
  shader_watcher->file_names = malloc(2 * sizeof(char *));
  shader_watcher->file_names[0] = vertex_shader_filename;
  shader_watcher->file_names[1] = fragment_shader_filename;
  shader_watcher->watcher_fds = malloc(2 * sizeof(int));
  shader_watcher->watcher_fds[0] = watch_file(vertex_shader_filename);
  shader_watcher->watcher_fds[1] = watch_file(fragment_shader_filename);
}

void delete_file_watcher(RFilesWatcher *fw) {
  for (int i = 0; i < fw->num_of_files; ++i) {
    close(fw->watcher_fds[i]);
  }
  free(fw->file_names);
  free(fw->watcher_fds);
}

// === OPENGL ===

// expects the shader program to be loaded
// https://www.khronos.org/opengl/wiki/Buffer_Texture
void free_gl_buffers(RBuffers *rb, RBackBuffer *bb, RMeshBuffers *mb) {
  glDeleteVertexArrays(1, &rb->vao);
  glDeleteBuffers(1, &rb->vbo);
  glDeleteFramebuffers(1, &bb->fbo);
  glDeleteTextures(1, &bb->fboTex);
  glDeleteBuffers(1, &mb->bvh_nodes_ssbo);
  glDeleteBuffers(1, &mb->triangles_ssbo);
  glDeleteBuffers(1, &mb->mats_ssbo);
  /* glDeleteBuffers(1, &mb->tbo_meshes); */
  /* glDeleteTextures(1, &mb->tbo_tex_meshes); */
  /* glDeleteBuffers(1, &mb->tbo_materials); */
  /* glDeleteTextures(1, &mb->tbo_tex_materials); */
  /* free(mb->triangles); */
  /* free(mb->meshes_info); */
  /* free(mb->materials); */
}
