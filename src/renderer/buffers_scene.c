#include "renderer/buffers_scene.h"
#include "opengl/generate_ssbo.h"
#include "opengl/gl_call.h"
#include <stddef.h>

RendererBuffersScene RendererBuffersScene_new(const Scene *scene) {
  RendererBuffersScene self = {0};

  generate_ssbo(&self.triangles_ssbo, scene->triangles,
                scene->triangles_count * sizeof(Triangle), 1);
  generate_ssbo(&self.bvh_nodes_ssbo, scene->bvh_nodes,
                scene->bvh_nodes_count * sizeof(BVHNode), 2);
  generate_ssbo(&self.mats_ssbo, scene->mats,
                scene->mats_count * sizeof(Material), 3);
  generate_ssbo(&self.camera_ssbo, &scene->camera, sizeof(Camera), 5);

  return self;
}

Camera RendererBuffersScene_get_camera(const RendererBuffersScene *self) {
  return self->_camera;
}

void RendererBuffersScene_set_camera(RendererBuffersScene *self,
                                     Camera camera) {
  self->_camera = camera;
  GL_CALL(glNamedBufferSubData(self->camera_ssbo, 0, sizeof(Camera), &camera));
}

void RendererBuffersScene_delete(RendererBuffersScene *self) {
  GL_CALL(glDeleteBuffers(1, &self->bvh_nodes_ssbo));
  GL_CALL(glDeleteBuffers(1, &self->triangles_ssbo));
  GL_CALL(glDeleteBuffers(1, &self->mats_ssbo));
  GL_CALL(glDeleteBuffers(1, &self->primitives_ssbo));
  GL_CALL(glDeleteBuffers(1, &self->camera_ssbo));

  self = NULL;
}
