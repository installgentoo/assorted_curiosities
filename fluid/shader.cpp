#include "shader.h"

using namespace std;

void PrintShaderLog(const GLuint &obj) {
  int infologLength = 0, maxLength;
  if (glIsShader(obj))
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &maxLength);
  else
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &maxLength);
  char infoLog[maxLength];
  if (glIsShader(obj))
    glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
  else
    glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);
  if (infologLength > 0)
    printf("%s\n", infoLog);
}

Shader::Shader(const char *filenamevs, const char *filenameps) {
  string pixshader, vershader;
  try {
    loadProgSource(filenamevs, &vershader);
  } catch (std::ifstream::failure e) {
    cerr << "Error opening file " << filenamevs << "\n";
    exit(-1);
  };
  try {
    loadProgSource(filenameps, &pixshader);
  } catch (std::ifstream::failure e) {
    cerr << "Error opening file " << filenameps << "\n";
    exit(-1);
  };
  prog = glCreateProgram();
  vert = glCreateShader(GL_VERTEX_SHADER_ARB);
  pix = glCreateShader(GL_FRAGMENT_SHADER_ARB);
  const char *vw = vershader.c_str(), *pw = pixshader.c_str();
  glShaderSource(vert, 1, &vw, NULL);
  glShaderSource(pix, 1, &pw, NULL);
  glCompileShader(vert);
  glCompileShader(pix);
  glAttachShader(prog, vert);
  glAttachShader(prog, pix);
  glLinkProgram(prog);
}

Shader::~Shader() {
  glDeleteShader(pix);
  glDeleteShader(vert);
  glDeleteProgram(prog);
}

void Shader::Log() { PrintShaderLog(prog); }

void Shader::Use() { glUseProgram(prog); }
