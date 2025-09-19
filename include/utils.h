#ifndef UTILS_H_
#define UTILS_H_
#include "renderer.h"

void print_triangles(Triangle *tri, vec3 *centroids, int count);

// FILES
char *read_file(const char *filename);
int watch_file(const char *path);
void init_shader_watcher(RFilesWatcher *shader_watcher,
                         const char *vertex_shader_filename,
                         const char *fragment_shader_filename);
void delete_file_watcher(RFilesWatcher *fw);

// OPENGL
void setup_gl_buffers(RMeshBuffers *mb, Scene *scene, GLuint shader_program);
void free_gl_buffers(RBuffers *rb, RBackBuffer *bb, RMeshBuffers *rmb);

#endif // UTILS_H_
