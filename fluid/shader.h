#ifndef SHADER_H_
#define SHADER_H_

#include "support.h"

class Shader {
public:
  GLuint prog;
  Shader(const char *filenamevs, const char *filenameps);
  ~Shader();
  void Log();
  void Use();

private:
  GLuint vert, pix;
};

#endif
