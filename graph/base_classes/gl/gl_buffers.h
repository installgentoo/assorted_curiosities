#pragma once
#include "gl_state_control.h"

namespace gl {

template <GLenum m_type> struct GLtex {
  GLtex() {
    GLCHECK(glGenTextures(1, &tex));
    CASSERT(tex, "OpenGL variable not initilized");
  }
  ~GLtex() {
    if (tex) {
      TextureControl::Delete(tex);
      GLCHECK(glDeleteTextures(1, &tex));
    }
  }
  GLtex(GLtex &&other) : tex(other.tex) { other.tex = 0; }
  GLtex &operator=(GLtex &&other) {
    if (tex) {
      TextureControl::Delete(tex);
      GLCHECK(glDeleteTextures(1, &tex));
    }
    tex = other.tex;
    other.tex = 0;
    return *this;
  }

  bool operator==(const GLtex &other) const {
    CASSERT(tex && other.tex, "Comparing uninitialized objects");
    return tex == other.tex;
  }

  void Bind(int unit) const {
    if (unit < 0)
      TextureControl::Bind(m_type, tex, TextureControl::m_bound_unit);
    else
      TextureControl::Bind(m_type, tex, cast<uint>(unit));
  }

  uint tex = 0;

  CDISABLE_COPY(GLtex)
};

struct GLsampler {
  GLsampler() {
    GLCHECK(glGenSamplers(1, &obj));
    CASSERT(obj, "OpenGL variable not initilized");
  }
  ~GLsampler() {
    if (obj) {
      SamplerControl::Delete(obj);
      GLCHECK(glDeleteSamplers(1, &obj));
    }
  }
  GLsampler(GLsampler &&other) : obj(other.obj) { other.obj = 0; }

  bool operator==(const GLsampler &other) const {
    CASSERT(obj && other.obj, "Comparing uninitialized objects");
    return obj == other.obj;
  }

  void Bind(uint unit) const { SamplerControl::Bind(obj, unit); }
  void Parameteri(GLint param, int value) const {
    GLCHECK(glSamplerParameteri(obj, cast<GLenum>(param), value));
  }
  void Parameterf(GLint param, float value) const {
    GLCHECK(glSamplerParameterf(obj, cast<GLenum>(param), value));
  }
  void Parameteri(GLint param, const int *value) const {
    GLCHECK(glSamplerParameteriv(obj, cast<GLenum>(param), value));
  }
  void Parameterf(GLint param, const float *value) const {
    GLCHECK(glSamplerParameterfv(obj, cast<GLenum>(param), value));
  }

  uint obj = 0;

  CDISABLE_COPY(GLsampler)
};

template <class m_policy> struct GLobject {
  GLobject() { StateControl<m_policy>::Create(&obj); }
  ~GLobject() { StateControl<m_policy>::Delete(&obj); }
  GLobject(GLobject &&other) : obj(other.obj) { other.obj = 0; }

  bool operator==(const GLobject &other) const {
    CASSERT(obj && other.obj, "Comparing uninitialized objects");
    return obj == other.obj;
  }

  void Bind() const { StateControl<m_policy>::Bind(obj); }

  uint obj = 0;

  CDISABLE_COPY(GLobject)
};

typedef GLobject<FboPolicy> GLfbo;
typedef GLobject<RenderbufferPolicy> GLrenderbuff;

struct GLvao : GLobject<VaoPolicy> {
  void Draw(const vector<uchar> &idx_buff, GLenum mode = GL_TRIANGLES) const {
    Bind();
    GLCHECK(glDrawElements(mode, idx_buff.size(), GL_UNSIGNED_BYTE, 0));
  }
  void Draw(const vector<ushort> &idx_buff, GLenum mode = GL_TRIANGLES) const {
    Bind();
    GLCHECK(glDrawElements(mode, idx_buff.size(), GL_UNSIGNED_SHORT, 0));
  }
  void Draw(const vector<uint> &idx_buff, GLenum mode = GL_TRIANGLES) const {
    Bind();
    GLCHECK(glDrawElements(mode, idx_buff.size(), GL_UNSIGNED_INT, 0));
  }
};

template <GLenum m_type> struct GLbuffer : GLobject<BuffPolicy<m_type>> {
  template <class T>
  void AllocateBuffer(const vector<T> &data, GLenum USAGE) const {
    AllocateBuffer(data.data(), sizeof(data[0]) * data.size(), USAGE);
  }

  void AllocateBuffer(const void *data, size_t size, GLenum USAGE) const {
    GLobject<BuffPolicy<m_type>>::Bind();
    GLCHECK(glBufferData(m_type, cast<GLsizeiptr>(size), data, USAGE));
  }

  template <class T>
  void UpdateBuffer(const vector<T> &data, size_t offset) const {
    GLobject<BuffPolicy<m_type>>::Bind();
    GLCHECK(glBufferSubData(m_type, sizeof(data[0]) * offset,
                            sizeof(data[0]) * data.size(), data.data()));
  }

  void UpdateBuffer(const void *data, size_t size, size_t offset) const {
    GLobject<BuffPolicy<m_type>>::Bind();
    GLCHECK(glBufferSubData(m_type, offset, size, data));
  }

  void *MapBuffer(GLenum ACCESS) const {
    GLobject<BuffPolicy<m_type>>::Bind();
    void *buff_ptr = GLCHECK(glMapBuffer(m_type, ACCESS));
    CASSERT(buff_ptr, "Passed nullptr as target memory");
    return buff_ptr;
  }

  void UnmapBuffer() const {
    GLobject<BuffPolicy<m_type>>::Bind();
    GLboolean valid = GLCHECK(glUnmapBuffer(m_type));
    CASSERT(valid == GL_TRUE, "Buffer memory was corrupted by OS");
  }

  void AttribFormat(uint idx, int size, GLenum TYPE = GL_FLOAT,
                    GLboolean NORMALIZED = GL_FALSE, int stride = 0,
                    const void *first = 0) const {
    CASSERT((size > 0) && (size < 5),
            "Attribute can only contain 1 to 4 values");
    GLobject<BuffPolicy<m_type>>::Bind();
    GLCHECK(glEnableVertexAttribArray(idx));
    GLCHECK(glVertexAttribPointer(idx, size, TYPE, NORMALIZED, stride, first));
  }
};

template <class T, uint D> struct GLstorage {
  void insert(size_t offset, const vector<T> &vec) {
    buff.insert(buff.begin() + cast<ptrdiff_t>(offset * D), vec.begin(),
                vec.end());
    needs_flush = true;
  }
  void insert(size_t offset, size_t size, array<T, D> &&arr) {
    buff.insert(buff.begin() + cast<ptrdiff_t>(offset * D), size * D, 0);
    incopy(offset, size, move(arr));
  }
  void insert(size_t offset, size_t size, T val) {
    buff.insert(buff.begin() + cast<ptrdiff_t>(offset * D), size * D, val);
  }
  void incopy(size_t offset, const vector<T> &vec) {
    CASSERT(offset + vec.size() <= buff.size(), "Out of bounds");
    std::copy(vec.begin(), vec.end(),
              buff.begin() + cast<ptrdiff_t>(offset * D));
    needs_flush = true;
  }
  void incopy(size_t offset, size_t size, array<T, D> &&arr) {
    CASSERT(size * D + offset <= buff.size(), "Out of bounds");
    for (size_t i = offset * D, end = i + size * D; i < end; i += D)
      std::copy(arr.begin(), arr.end(), buff.begin() + cast<ptrdiff_t>(i));
    needs_flush = true;
  }
  void erase(size_t at, size_t offset) {
    const auto f = buff.begin() + cast<ptrdiff_t>(at * D);
    buff.erase(f, f + cast<ptrdiff_t>(offset * D));
    needs_flush = true;
  }
  bool check() {
    if (needs_flush) {
      needs_flush = false;
      return true;
    }
    return false;
  }

  bool needs_flush = false;
  vector<T> buff;
};

template <GLenum m_type> struct GLQuery : GLobject<QueryPolicy<m_type>> {
  void Begin() {
    CASSERT(!m_started, "GLquery already started");
    m_started = true;
    GLCHECK(glBeginQuery(GL_TIME_ELAPSED, GLobject<QueryPolicy<m_type>>::obj));
  }
  GLint64 End() {
    CASSERT(m_started, "GLquery wasn't started");
    GLCHECK(glEndQuery(GL_TIME_ELAPSED));
    GLint64 result = 0;
    GLCHECK(glGetQueryObjecti64v(GLobject<QueryPolicy<m_type>>::obj,
                                 GL_QUERY_RESULT, &result));
    m_started = false;
    return result;
  }

private:
  static bool m_started;

  void Bind() const {}
};
template <GLenum m_type> bool GLQuery<m_type>::m_started = false;

} // namespace gl
