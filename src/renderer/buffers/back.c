#include "renderer/buffers/back.h"
#include "opengl/gl_call.h"
#include "opengl/resolution.h"
#include <stddef.h>

RendererBuffersBack RendererBuffersBack_new(void) {
  RendererBuffersBack self = {0};

  GL_CALL(glGenFramebuffers(1, &self.fbo));
  GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, self.fbo));

  // create the texture for the framebuffer
  GL_CALL(glGenTextures(1, &self.fboTex));
  GL_CALL(glBindTexture(GL_TEXTURE_2D, self.fboTex));
  // NOTE: resolution has to be set with the resize function, which is assumed
  // to happen before drawing to this framebuffer
  GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 0, GL_RGB,
                       GL_UNSIGNED_BYTE, NULL));
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

  // attach the texture to our framebuffer
  GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                 GL_TEXTURE_2D, self.fboTex, 0));
  GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  return self;
}

void RendererBuffersBack_resize(RendererBuffersBack *self,
                                OpenGLResolution res) {
  GL_CALL(glBindTexture(GL_TEXTURE_2D, self->fboTex));
  GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, res.width, res.height, 0,
                       GL_RGB, GL_UNSIGNED_BYTE, NULL));
  GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}

void RendererBuffersBack_delete(RendererBuffersBack *self) {
  GL_CALL(glDeleteFramebuffers(1, &self->fbo));
  GL_CALL(glDeleteTextures(1, &self->fboTex));
}
