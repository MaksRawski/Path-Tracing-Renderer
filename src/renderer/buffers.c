#include "renderer/buffers.h"
#include <stddef.h>

void generate_ssbo(GLuint *ssbo, const void *data, int size, int index) {
  glGenBuffers(1, ssbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, *ssbo);
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_READ);
  /* glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, *ssbo); */
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, *ssbo, 0, size);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// === RendererBuffersBack ===
RendererBuffersBack RendererBuffersBack_new(unsigned int width, unsigned int height) {
  RendererBuffersBack self = {0};

  glGenFramebuffers(1, &self.fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, self.fbo);

  // create the texture for the framebuffer
  glGenTextures(1, &self.fboTex);
  glBindTexture(GL_TEXTURE_2D, self.fboTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  // attach the texture to our framebuffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         self.fboTex, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return self;
}

void RendererBuffersBack_delete(RendererBuffersBack *self) {
  glDeleteFramebuffers(1, &self->fbo);
  glDeleteTextures(1, &self->fboTex);

  self = NULL;
}

// === RendererBuffersMesh ===
// NOTE: requires that scene outlives this struct
RendererBuffersMesh RendererBuffersMesh_new(const Scene *scene) {
  RendererBuffersMesh self = {0};

  generate_ssbo(&self.triangles_ssbo, scene->triangles,
                scene->triangles_count * sizeof(Triangle), 1);
  generate_ssbo(&self.bvh_nodes_ssbo, scene->bvh.nodes,
                scene->bvh.nodes_count * sizeof(BVHnode), 2);
  generate_ssbo(&self.mats_ssbo, scene->mats,
                scene->mats_count * sizeof(Material), 3);
  // NOTE: this assumes that primitives are just a LUT for triangles
  generate_ssbo(&self.primitives_ssbo, scene->primitives,
                scene->triangles_count * sizeof(Primitive), 4);

  self.bvh_nodes_count = scene->bvh.nodes_count;
  self.triangle_count = scene->triangles_count;
  self.mats_count = scene->mats_count;

  return self;
}

void RendererBuffersMesh_delete(RendererBuffersMesh *self) {
  glDeleteBuffers(1, &self->bvh_nodes_ssbo);
  glDeleteBuffers(1, &self->triangles_ssbo);
  glDeleteBuffers(1, &self->mats_ssbo);

  self = NULL;
}

// === RendererBuffersInternal ===
RendererBuffersInternal RendererBuffersInternal_new(void) {
  RendererBuffersInternal self = {0};

  // define vertices for a full-screen quad
  float vertices[] = {
      -1.0f, 1.0f,  // top left
      1.0f,  1.0f,  // top right
      1.0f,  -1.0f, // bottom right
      -1.0f, -1.0f  // bottom left
  };

  // generate vertex array object
  glGenVertexArrays(1, &self.vao);
  // generate vertex buffer object
  glGenBuffers(1, &self.vbo);

  // bind both buffers (order is important!)
  glBindVertexArray(self.vao);
  glBindBuffer(GL_ARRAY_BUFFER, self.vbo);

  // upload vertex data to VBO
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), vertices, GL_STATIC_DRAW);

  // index of the attribute = 0 (will be used in the vertex shader)
  // how many values per vertex = 2,
  // type of each element = GL_FLOAT,
  // do we have coordinates as integers?,
  // stride, space between every vertex
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  // unbind the vbo
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // unbind the vao
  glBindVertexArray(0);

  return self;
}

void RendererBuffersInternal_delete(RendererBuffersInternal *self) {
  glDeleteVertexArrays(1, &self->vao);
  glDeleteBuffers(1, &self->vbo);

  self = NULL;
}

// NOTE: scene must outlive this struct
RendererBuffers RendererBuffers_new(unsigned int width, unsigned int height, const Scene *scene) {
  RendererBuffers self = {0};

  self.back = RendererBuffersBack_new(width, height);
  self.internal = RendererBuffersInternal_new();
  self.mesh = RendererBuffersMesh_new(scene);

  return self;
}

void RendererBuffers_rebuild(RendererBuffers *self, const Scene *scene) {
  self->mesh = RendererBuffersMesh_new(scene);
}

void RendererBuffers_delete(RendererBuffers *self) {
  RendererBuffersBack_delete(&self->back);
  RendererBuffersInternal_delete(&self->internal);
  RendererBuffersMesh_delete(&self->mesh);

  self = NULL;
}
