#include "renderer/buffers_scene.h"
#include <stddef.h>

void generate_ssbo(GLuint *ssbo, const void *data, int size, int index) {
  glGenBuffers(1, ssbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, *ssbo);
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, *ssbo, 0, size);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

RendererBuffersScene RendererBuffersScene_new(const Scene *scene) {
  RendererBuffersScene self = {0};

  generate_ssbo(&self.triangles_ssbo, scene->triangles,
                scene->triangles_count * sizeof(Triangle), 1);
  generate_ssbo(&self.bvh_nodes_ssbo, scene->bvh.nodes,
                scene->bvh.nodes_count * sizeof(BVHnode), 2);
  generate_ssbo(&self.mats_ssbo, scene->mats,
                scene->mats_count * sizeof(Material), 3);
  // NOTE: this assumes that primitives are just a LUT for triangles
  generate_ssbo(&self.primitives_ssbo, scene->primitives,
                scene->triangles_count * sizeof(Primitive), 4);
  generate_ssbo(&self.camera_ssbo, &scene->camera, sizeof(Camera), 5);

  self.bvh_nodes_count = scene->bvh.nodes_count;
  self.triangle_count = scene->triangles_count;
  self.mats_count = scene->mats_count;
  self._camera = scene->camera;

  return self;
}

void RendererBuffersScene_update_camera(RendererBuffersScene *self, Camera cam) {
  glNamedBufferSubData(self->camera_ssbo, 0, sizeof(Camera), &cam);
}

void RendererBuffersScene_delete(RendererBuffersScene *self) {
  glDeleteBuffers(1, &self->bvh_nodes_ssbo);
  glDeleteBuffers(1, &self->triangles_ssbo);
  glDeleteBuffers(1, &self->mats_ssbo);
  glDeleteBuffers(1, &self->primitives_ssbo);
  glDeleteBuffers(1, &self->camera_ssbo);

  self = NULL;
}
