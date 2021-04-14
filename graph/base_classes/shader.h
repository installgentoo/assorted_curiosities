#pragma once
#include <glm/gtc/type_ptr.hpp>

#include "gl/gl_buffers.h"

namespace gl {

struct Shader {
  struct ShaderObject : GLobject<ShaderProgramPolicy> {
    ShaderObject(uint vert, uint pix);
    ShaderObject(uint vert, uint pix, uint geom);

    void Log() const;
    int GetUniform(const char *name) const;

    uint m_instances = 0;
    mutable unordered_map<string, int> m_uniforms;
  };

  Shader(const char *vertex, const char *pixel, const char *geometry = nullptr);
  ~Shader();
  Shader(ShaderObject *other) : m_obj(other) { ++m_obj->m_instances; }
  Shader(const Shader &other) : m_obj(other.m_obj) { ++m_obj->m_instances; }

  void Use() const { m_obj->Bind(); }

  void Uniformi(const char *name, int value) const {
    Use();
    GLCHECK(glUniform1i(m_obj->GetUniform(name), value));
  }

  void Uniformf(const char *name, double value) const {
    Use();
    GLCHECK(glUniform1f(m_obj->GetUniform(name), float(value)));
  }

  void Uniformf(const char *name, const vec2 &value) const {
    Use();
    GLCHECK(glUniform2f(m_obj->GetUniform(name), value.x, value.y));
  }

  void Uniformf(const char *name, const vec3 &value) const {
    Use();
    GLCHECK(glUniform3f(m_obj->GetUniform(name), value.x, value.y, value.z));
  }

  void Uniformf(const char *name, const vec4 &value) const {
    Use();
    GLCHECK(glUniform4f(m_obj->GetUniform(name), value.r, value.g, value.b,
                        value.a));
  }

  void Uniformf2x2(const char *name, const mat2 &matrix,
                   GLboolean transpose = GL_FALSE) const {
    Use();
    GLCHECK(glUniformMatrix2fv(m_obj->GetUniform(name), 1, transpose,
                               glm::value_ptr(matrix)));
  }

  void Uniformf3x3(const char *name, const mat3 &matrix,
                   GLboolean transpose = GL_FALSE) const {
    Use();
    GLCHECK(glUniformMatrix3fv(m_obj->GetUniform(name), 1, transpose,
                               glm::value_ptr(matrix)));
  }

  void Uniformf4x4(const char *name, const mat4 &matrix,
                   GLboolean transpose = GL_FALSE) const {
    Use();
    GLCHECK(glUniformMatrix4fv(m_obj->GetUniform(name), 1, transpose,
                               glm::value_ptr(matrix)));
  }

  void Uniformf4x3(const char *name, const mat4x3 &matrix,
                   GLboolean transpose = GL_FALSE) const {
    Use();
    GLCHECK(glUniformMatrix4x3fv(m_obj->GetUniform(name), 1, transpose,
                                 glm::value_ptr(matrix)));
  }

private:
  ShaderObject *m_obj;
};

struct ShaderManager : CUNIQUE {
  friend struct Shader;
  enum ShaderType { VERTEX = 0, FRAGMENT = 1, GEOMETRY = 2 };

  static ShaderManager &Get();

  void LoadAllShadersFromFile(const char *str);
  void LoadShaderSource(string name, string source);
  void ClearCache();

private:
  unordered_map<string, uint> m_shader_objects;
  unordered_map<string, unique_ptr<Shader::ShaderObject>> m_shaders;
  unordered_map<string, string> m_shader_sources;

  unordered_map<string, uint>::iterator
  LoadShaderObject(string name, const char *source, ShaderType type);
  Shader::ShaderObject *GetShader(const char *vertex, const char *pixel,
                                  const char *geometry);
  void Delete(Shader::ShaderObject *shader);
  string GetNameFromProgIdx(uint prog_idx);
};

struct ShaderText {
  template <class... Strs> ShaderText(string &&name, Strs &&...strings) {
    AddLine(move(strings)...);
    ShaderManager::Get().LoadShaderSource(move(name), move(m_text));
  }

private:
  string m_text;

  template <class... Strs> void AddLine(string &&arg1, const Strs &&...args) {
    m_text += arg1 + " \n";
    AddLine(move(args)...);
  }
  void AddLine() const {}
};

template <class T>
string GLSLUnwindedLoop(string command, T start, T end, T step = 1) {
  string text;
  CASSERT(step > 0, "Can't unwind into zero steps");
  for (T i = start; i <= end; i += step) {
    for (size_t found; (found = command.find("$$iter")) != command.npos;)
      command.replace(found, found + 6, to_string(i));

    text += command + "; ";
  }
  return text;
}

#define SHADER(name) const ShaderText shadertext___##name(#name,"#version 330 core",

} // namespace gl
