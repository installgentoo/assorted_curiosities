#include "gl_state_control.h"

using namespace gl;

uint TextureControl::m_bound_unit = 0;
vector<TextureControl::Texunit> TextureControl::m_texture_units;

vector<uint> SamplerControl::m_texture_units;

uint GLState::s_fbo_w = 0, GLState::s_fbo_h = 0;
unordered_map<GLint, bool> GLState::m_states;

string gl::glCodeToError(GLenum code) {
  switch (code) {
  case GL_INVALID_ENUM:
    return "GL_INVALID_ENUM";
  case GL_INVALID_VALUE:
    return "GL_INVALID_VALUE";
  case GL_INVALID_OPERATION:
    return "GL_INVALID_OPERATION";
  case GL_STACK_OVERFLOW:
    return "GL_STACK_OVERFLOW";
  case GL_STACK_UNDERFLOW:
    return "GL_STACK_UNDERFLOW";
  case GL_OUT_OF_MEMORY:
    return "GL_OUT_OF_MEMORY";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  case GL_CONTEXT_LOST:
    return "GL_CONTEXT_LOST";
  }
  return "error";
}
