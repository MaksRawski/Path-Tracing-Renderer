#include "renderer/buffers_scene.h"
#include "opengl/generate_ssbo.h"
#include "opengl/gl_call.h"
#include "scene/mesh.h"
#include <stddef.h>

RendererBuffersScene RendererBuffersScene_new(const Scene *scene) {
  RendererBuffersScene self = {0};

  unsigned int meshes_count =
      scene->meshes_capacity == 0 ? 0 : scene->last_mesh_index + 1;
  unsigned int mats_count =
      scene->mats_capacity == 0 ? 0 : scene->last_mat_index + 1;
  generate_ssbo(&self.triangles_ssbo, scene->triangles,
                scene->triangles_count * sizeof(Triangle), 1);
  generate_ssbo(&self.bvh_nodes_ssbo, scene->bvh_nodes,
                scene->bvh_nodes_count * sizeof(BVHNode), 2);
  generate_ssbo(&self.mats_ssbo, scene->mats, mats_count * sizeof(Material), 3);
  generate_ssbo(&self.mesh_primitives_ssbo, scene->mesh_primitives,
                scene->mesh_primitives_count * sizeof(MeshPrimitive), 4);
  generate_ssbo(&self.meshes_ssbo, scene->meshes, meshes_count * sizeof(Mesh),
                5);
  generate_ssbo(&self.mesh_instances_ssbo, scene->mesh_instances,
                scene->mesh_instances_count * sizeof(MeshInstance), 6);
  generate_ssbo(&self.tlas_nodes_ssbo, scene->tlas_nodes,
                scene->tlas_nodes_count * sizeof(TLASNode), 7);
  generate_ssbo(&self.camera_ssbo, &scene->camera, sizeof(Camera), 8);

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
  GL_CALL(glDeleteBuffers(1, &self->triangles_ssbo));
  GL_CALL(glDeleteBuffers(1, &self->bvh_nodes_ssbo));
  GL_CALL(glDeleteBuffers(1, &self->mats_ssbo));
  GL_CALL(glDeleteBuffers(1, &self->mesh_primitives_ssbo));
  GL_CALL(glDeleteBuffers(1, &self->meshes_ssbo));
  GL_CALL(glDeleteBuffers(1, &self->mesh_instances_ssbo));
  GL_CALL(glDeleteBuffers(1, &self->tlas_nodes_ssbo));
  GL_CALL(glDeleteBuffers(1, &self->camera_ssbo));
}
