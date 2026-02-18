#ifndef RENDERER_BUFFERS_SCENE_H_
#define RENDERER_BUFFERS_SCENE_H_

#include "glad/gl.h" // GLuint
#include "scene.h"

typedef struct {
  GLuint triangles_ssbo, bvh_nodes_ssbo, mats_ssbo, triangles_data_ssbo,
      camera_ssbo;
} RendererBuffersScene;

RendererBuffersScene RendererBuffersScene_new(const Scene *scene);
void RendererBuffersScene_set_camera(RendererBuffersScene *self, Camera cam);

void RendererBuffersScene_delete(RendererBuffersScene *self);

#endif // RENDERER_BUFFERS_SCENE_H_
