#include "renderer/buffers/back.h"
#include <stddef.h>

RendererBuffersBack RendererBuffersBack_new(unsigned int width,
                                            unsigned int height) {
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
