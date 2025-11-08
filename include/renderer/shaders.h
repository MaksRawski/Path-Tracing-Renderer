#ifndef SHADERS_H_
#define SHADERS_H_

#include "glad/gl.h"
#include "utils/file_watcher.h"

typedef struct {
  GLuint program;
  FileWatcher vertex_shader;
  FileWatcher fragment_shader;
} RendererShaders;

RendererShaders RendererShaders_new(const char *vertex_shader_path,
                                    const char *fragment_shader_path);
bool RendererShaders_update(RendererShaders *self);
void RendererShaders_force_update(RendererShaders *self);
void RendererShaders_delete(RendererShaders *self);

#endif // SHADERS_H_
