#include "shader.h"

#include "policies/resource_policy.h"

using namespace gl;

static const char *s_shader_strings[] = {"vertex", "fragment", "geometry"};

static string PrintShaderLog(uint obj) {
  int infologLength = 0, maxLength;
  if (glIsShader(obj)) {
    GLCHECK(glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &maxLength));
  } else {
    GLCHECK(glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &maxLength));
  }
  vector<char> infoLog(cast<uint>(maxLength));
  if (glIsShader(obj)) {
    GLCHECK(glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog.data()));
  } else {
    GLCHECK(
        glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog.data()));
  }

  if (infologLength > 0)
    return string(infoLog.data());
  else
    return string("");
}

Shader::ShaderObject::ShaderObject(uint vert, uint pix) {
  GLCHECK(glAttachShader(obj, vert));
  GLCHECK(glAttachShader(obj, pix));

  GLCHECK(glLinkProgram(obj));

  GLCHECK(glDetachShader(obj, vert));
  GLCHECK(glDetachShader(obj, pix));
}

Shader::ShaderObject::ShaderObject(uint vert, uint pix, uint geom) {
  GLCHECK(glAttachShader(obj, vert));
  GLCHECK(glAttachShader(obj, pix));
  GLCHECK(glAttachShader(obj, geom));

  GLCHECK(glLinkProgram(obj));

  GLCHECK(glDetachShader(obj, vert));
  GLCHECK(glDetachShader(obj, pix));
  GLCHECK(glDetachShader(obj, geom));
}

void Shader::ShaderObject::Log() const { PrintShaderLog(obj); }

int Shader::ShaderObject::GetUniform(const char *name) const {
  const auto found = m_uniforms.find(name);

  if (found != m_uniforms.end())
    return found->second;

  const int addr = GLCHECK(glGetUniformLocation(obj, name));
  if (-1 == addr)
    CINFO("No uniform named '" << name << "' in shader '"
                               << ShaderManager::Get().GetNameFromProgIdx(obj)
                               << "', or uniform was optimized out");
  m_uniforms.emplace(name, addr);
  return addr;
}

Shader::Shader(const char *vertex, const char *pixel, const char *geometry)
    : m_obj(ShaderManager::Get().GetShader(vertex, pixel, geometry)) {
  CASSERT(m_obj, "Returned no shader");
  if (!m_obj->m_instances)
    m_obj->Log();
  ++m_obj->m_instances;
}

Shader::~Shader() {
  if (m_obj->m_instances > 1)
    --m_obj->m_instances;
  else
    ShaderManager::Get().Delete(m_obj);
}

ShaderManager &ShaderManager::Get() {
  static ShaderManager s_manager;
  return s_manager;
}

void ShaderManager::LoadAllShadersFromFile(const char *str) {
  string data;
  {
    vector<char> res = ResourceLoader::LoadResource(str);
    data.assign(res.data(), res.size());
  }

  string header;
  size_t start = 0;
  if (data.find("//--GLOBAL:") != string::npos) {
    start = data.find("//--", 4);
    header.assign(data, 11, start - 11);
  }

  for (size_t c_line = start; start != string::npos;
       start = data.find("//--", start)) {
    for (; (c_line < start) && (c_line != string::npos);
         c_line = data.find("\n", c_line) + 1)
      header += "\n";

    if ((data.find("//--VER", start) == start) ||
        (data.find("//--PIX", start) == start) ||
        (data.find("//--GEO", start) == start)) {
      start += 7;
      while (data.find(" ", start) == start)
        ++start;
      size_t stop = data.find("\n", start);
      size_t len = stop - start;
      const string name = data.substr(start, len);
      start = stop;

      stop = data.find("//--", start);
      if (string::npos == stop)
        stop = data.size();
      len = stop - start;
      const string text = header + data.substr(start, len);
      start = stop;
      LoadShaderSource(move(name), move(text));
    } else
      start += 4;
  }
}

void ShaderManager::LoadShaderSource(string name, string source) {
  CASSERT(m_shader_sources.find(name) == m_shader_sources.end(),
          "Shader source '" << name << "' already exists");
  m_shader_sources.emplace(move(name), move(source));
}

void ShaderManager::ClearCache() {
  for (const auto &i : m_shader_objects) {
    GLCHECK(glDeleteShader(i.second));
  }
  m_shader_sources.clear();
  m_shader_objects.clear();
}

unordered_map<string, uint>::iterator
ShaderManager::LoadShaderObject(string name, const char *source,
                                ShaderType type) {
  uint id;
  switch (type) {
  case VERTEX:
    id = GLCHECK(glCreateShader(GL_VERTEX_SHADER));
    break;
  case FRAGMENT:
    id = GLCHECK(glCreateShader(GL_FRAGMENT_SHADER));
    break;
  case GEOMETRY:
    id = GLCHECK(glCreateShader(GL_GEOMETRY_SHADER));
    break;
  }
  GLCHECK(glShaderSource(id, 1, &source, nullptr));
  GLCHECK(glCompileShader(id));
  int status;
  GLCHECK(glGetShaderiv(id, GL_COMPILE_STATUS, &status));
  if (status != true || !id) {
    CERROR("Error compiling " << s_shader_strings[type] << " shader '" << name
                              << "'\n"
                              << PrintShaderLog(id));
    GLCHECK(glDeleteShader(id));
  }

  return m_shader_objects.emplace(move(name), id).first;
}

Shader::ShaderObject *ShaderManager::GetShader(const char *vertex,
                                               const char *pixel,
                                               const char *geometry) {
  const bool has_geometry = geometry != nullptr;
  const string vert(move(vertex)), pix(move(pixel));
  string geom;
  if (has_geometry)
    geom = move(geometry);

  const string name =
      "v:" + vert + "|p:" + pix + (has_geometry ? "|g:" + geom : "");
  const auto existing = m_shaders.find(name);

  if (existing != m_shaders.end())
    return existing->second.get();

  auto exists_vert = m_shader_objects.find(vert);
  if (m_shader_objects.end() == exists_vert) {
    const auto found = m_shader_sources.find(vert);
    if (found != m_shader_sources.end())
      exists_vert = LoadShaderObject(move(vert), found->second.c_str(), VERTEX);
    else
      CERROR("No vertex shader '" << vert << "' is loaded");
  }

  auto exists_pix = m_shader_objects.find(pix);
  if (m_shader_objects.end() == exists_pix) {
    const auto found = m_shader_sources.find(pix);
    if (found != m_shader_sources.end())
      exists_pix = LoadShaderObject(move(pix), found->second.c_str(), FRAGMENT);
    else
      CERROR("No pixel shader '" << pix << "' is loaded");
  }

  Shader::ShaderObject *obj = nullptr;
  if (!has_geometry)
    obj = new Shader::ShaderObject(exists_vert->second, exists_pix->second);
  else {
    auto exists_geom = m_shader_objects.find(geom);
    if (m_shader_objects.end() == exists_geom) {
      const auto found = m_shader_sources.find(geom);
      if (found != m_shader_sources.end())
        exists_geom =
            LoadShaderObject(move(geom), found->second.c_str(), GEOMETRY);
      else
        CERROR("No pixel shader '" << geom << "' is loaded");
    }
    obj = new Shader::ShaderObject(exists_vert->second, exists_pix->second,
                                   exists_geom->second);
  }

  m_shaders.emplace(move(name), obj);
  return obj;
}

void ShaderManager::Delete(Shader::ShaderObject *shader) {
  for (auto i = m_shaders.begin(), end = m_shaders.end(); i != end; ++i)
    if (i->second.get() == shader) {
      m_shaders.erase(i);
      return;
    }
}

string ShaderManager::GetNameFromProgIdx(uint prog_idx) {
  for (const auto &i : m_shaders)
    if (i.second->obj == prog_idx)
      return i.first;

  return "name not found";
}
