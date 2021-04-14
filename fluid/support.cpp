#include "support.h"

using namespace std;

void loadProgSource(const char *filename, string *data) {
  ifstream file(filename, ios::in);
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  data->assign(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));
  file.close();
}

void glSetup(const int &w, const int &h) {
  glfwSwapInterval(1);
  glViewport(0, 0, w, h);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float ratio = w / (float)h;
  glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
}

Quad::Quad() {
  GLshort coords[] = {-1, -1, 1, -1, -1, 1, 1, 1};
  GLsizeiptr size = sizeof(coords), space = 2 * sizeof(coords[0]);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, size, coords, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, space, 0);
}

Quad::~Quad() {
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
}

void Quad::Draw() { glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); }
