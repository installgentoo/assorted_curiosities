#include "mapbuffer.h"

texmap::texmap(GLsizei w, GLsizei h, int c) {
  glGenFramebuffers(1, &pixbuff);
  glBindFramebuffer(GL_FRAMEBUFFER, pixbuff);
  glGenTextures(1, &text);

  glBindTexture(GL_TEXTURE_2D, text);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  switch (c) {
  case 1:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, w, h, 0, GL_RED, GL_FLOAT, 0);
    break;
  case 2:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, w, h, 0, GL_RG, GL_FLOAT, 0);
    break;
  }

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         text, 0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void texmap::Clear(float v) {
  glBindFramebuffer(GL_FRAMEBUFFER, pixbuff);
  glClearColor(v, v, v, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

texmap::~texmap() {
  glDeleteFramebuffers(1, &pixbuff);
  glDeleteTextures(1, &text);
}

mapbuffer::mapbuffer(GLsizei w, GLsizei h, int c) {
  one = new texmap(w, h, c);
  two = new texmap(w, h, c);
}

mapbuffer::~mapbuffer() {
  delete (one);
  delete (two);
}

void mapbuffer::Swap() {
  texmap *temp;
  temp = one;
  one = two;
  two = temp;
}
