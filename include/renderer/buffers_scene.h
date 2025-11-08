#ifndef RENDERER_BUFFERS_SCENE_H_
#define RENDERER_BUFFERS_SCENE_H_

#include "scene.h"
#include <glad/gl.h> // GLuint

typedef struct {
  int triangle_count, bvh_nodes_count, mats_count;
  GLuint triangles_ssbo, bvh_nodes_ssbo, mats_ssbo;
  GLuint primitives_ssbo, camera_ssbo;
  Camera _camera;
} RendererBuffersScene;

RendererBuffersScene RendererBuffersScene_new(const Scene *scene);
void RendererBuffersScene_update_camera(RendererBuffersScene *self, Camera cam);
void RendererBuffersScene_delete(RendererBuffersScene *self);

#endif // RENDERER_BUFFERS_SCENE_H_
