#ifndef SUPPORT_H_
#define SUPPORT_H_

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>

void loadProgSource(const char *filename, std::string *data);

void glSetup(const int &w, const int &h);

class Quad {
public:
  GLuint vbo, vao;
  Quad();
  ~Quad();
  void Draw();
};

#endif
