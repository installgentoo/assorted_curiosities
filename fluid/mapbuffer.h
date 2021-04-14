#ifndef MAPBUFFER_H_
#define MAPBUFFER_H_

#include "support.h"

class texmap {
public:
  GLuint pixbuff, text;
  texmap(GLsizei w, GLsizei h, int c);
  ~texmap();
  void Clear(float v);
};

class mapbuffer {
public:
  texmap *one, *two;
  mapbuffer(GLsizei w, GLsizei h, int c);
  ~mapbuffer();
  void Swap();
};

#endif
