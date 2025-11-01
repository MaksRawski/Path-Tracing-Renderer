#ifndef BUFFERS_H_
#define BUFFERS_H_

#include "scene.h"
#include <glad/gl.h>

typedef struct {
  GLuint fbo, fboTex;
} RendererBuffersBack;

typedef struct {
  int triangle_count, bvh_nodes_count, mats_count;
  GLuint triangles_ssbo, bvh_nodes_ssbo, mats_ssbo, primitives_ssbo;
} RendererBuffersMesh;

typedef struct {
  GLuint vao;
  GLuint vbo;
} RendererBuffersInternal;

typedef struct {
  RendererBuffersBack back;
  RendererBuffersMesh mesh;
  RendererBuffersInternal internal;
} RendererBuffers;

RendererBuffers RendererBuffers_new(unsigned int width, unsigned int height,
                                    const Scene *scene);
void RendererBuffers_rebuild(RendererBuffers *self, const Scene *scene);
void RendererBuffers_delete(RendererBuffers *self);

#endif // BUFFERS_H_
