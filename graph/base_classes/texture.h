#pragma once
#include "gl/gl_buffers.h"

namespace gl {

struct Sampler {
  struct SamplerObject : GLsampler {
    uint m_instances = 0;
  };

  const GLsampler &obj() const {
    CASSERT(m_obj, "Sampler uninitialized");
    return *static_cast<GLsampler *>(m_obj);
  }

  template <class... Pack>
  Sampler(const char *name, const Pack... packs) : m_obj(Create(name)) {
    ++m_obj->m_instances;

    if (m_obj->m_instances < 2)
      SetParameter(packs...);
    else if (sizeof...(packs) > 0)
      CINFO("Sampler '" << name
                        << "' already exists with different parameters");
  }
  ~Sampler();
  Sampler(const Sampler &other) : m_obj(other.m_obj) { ++m_obj->m_instances; }

  void Bind(uint unit) const { obj().Bind(unit); }

private:
  SamplerObject *m_obj;

  template <class... Pack>
  void SetParameter(GLint param, GLint value, const Pack... packs) const {
    m_obj->Parameteri(param, value);
    SetParameter(packs...);
  }
  template <class... Pack>
  void SetParameter(GLint param, GLfloat value, const Pack... packs) const {
    m_obj->Parameterf(param, value);
    SetParameter(packs...);
  }
  void SetParameter() const {}
  SamplerObject *Create(const char *name);
};

struct SamplerManager : CUNIQUE {
  friend struct Sampler;

  static SamplerManager &Get();

  const GLsampler &GetSampler(const char *name) const;

private:
  unordered_map<string, unique_ptr<Sampler::SamplerObject>> m_samplers;

  Sampler::SamplerObject *Create(string name);
  void Delete(Sampler::SamplerObject *sampler);
};

struct Texture {
  struct Params {
    Params(uint width, uint height, GLint fmt, GLenum channels)
        : m_width(width), m_height(height), m_channels(channels), m_fmt(fmt) {}
    ulint Size(uint level = 0) const;

    const uint m_width, m_height, m_channels;
    const GLint m_fmt;
  };

  const GLtex<GL_TEXTURE_2D> &tex() const { return m_tex; }
  ulint size(uint level = 0) const {
    CASSERT(m_params, "Texture uninitialized");
    return m_params->Size(level);
  }
  uint width() const {
    CASSERT(m_params, "Texture uninitialized");
    return m_params->m_width;
  }
  uint height() const {
    CASSERT(m_params, "Texture uninitialized");
    return m_params->m_height;
  }

  Texture() = default;
  Texture(uint width, uint height, uint channels, GLenum PRECISION = GL_BYTE,
          const void *data = 0, GLenum DATAFORMAT = GL_UNSIGNED_BYTE,
          bool mipmap = false) {
    Create(width, height, channels, PRECISION, data, DATAFORMAT, mipmap);
  }
  Texture(const char *filename, uint channels = 3, GLenum PRECISION = GL_BYTE,
          bool mipmap = false) {
    LoadFromFile(filename, channels, PRECISION, mipmap);
  }
  virtual ~Texture() = default;
  Texture(Texture &&other)
      : m_tex(move(other.m_tex)), m_params(move(other.m_params)) {}
  Texture &operator=(Texture &&other) {
    m_tex = move(other.m_tex);
    m_params = move(other.m_params);
    return *this;
  }

  void Create(uint width, uint height, uint channels = 0,
              GLenum PRECISION = GL_BYTE, const void *data = 0,
              GLenum DATAFORMAT = GL_UNSIGNED_BYTE, bool mipmap = false);
  void Download(unique_ptr<uchar[]> &data, uint level = 0) const;
  void Download(unique_ptr<float[]> &data, uint level = 0) const;
  void Download(GLbuffer<GL_PIXEL_PACK_BUFFER> &buff,
                GLenum PRECISION = GL_UNSIGNED_BYTE, uint level = 0) const;
  void Update(void *data, GLenum CHANNELS = GL_RGBA,
              GLenum PRECISION = GL_UNSIGNED_BYTE, uint x = 0, uint y = 0,
              uint width = 0, uint height = 0) const;
  void LoadFromFile(const char *filename, uint channels = 3,
                    GLenum PRECISION = GL_BYTE, bool mipmap = false);

  void Use(const char *sampler, uint n) const;
  void Use(const Sampler &sampler, uint n) const {
    m_tex.Bind(cast<int>(n));
    sampler.Bind(n);
  }
  void Activate() const { m_tex.Bind(-1); }

protected:
  GLtex<GL_TEXTURE_2D> m_tex;
  unique_ptr<Params> m_params;

  bool Compatible(uint width, uint height, GLint fmt) {
    return m_params && (width == m_params->m_width) &&
           (height == m_params->m_height) && (fmt == m_params->m_fmt);
  }

  CDISABLE_COPY(Texture)
};

struct Surface : Texture {
  const GLfbo &fbo() const { return m_fbo; }

  Surface() = default;
  Surface(uint width, uint height, uint channels, GLenum PRECISION = GL_BYTE,
          bool mipmap = false) {
    Create(width, height, channels, PRECISION, mipmap);
  }

  void Create(uint width, uint height, uint channels,
              GLenum PRECISION = GL_BYTE, bool mipmap = false);
  void Clear(GLclampf v);
  void Bind() const;
  Texture PullOutTexture() { return move(*static_cast<Texture *>(this)); }
  void Clear() { GLState::Clear(GL_COLOR_BUFFER_BIT); }

protected:
  GLfbo m_fbo;

  CDISABLE_COPY(Surface)
};

struct Framebuffer : Surface {
  Framebuffer() = default;
  Framebuffer(uint width, uint height, uint channels,
              GLenum PRECISION = GL_BYTE, bool mipmap = false) {
    Create(width, height, channels, PRECISION, mipmap);
  }

  void Create(uint width, uint height, uint channels,
              GLenum PRECISION = GL_BYTE, bool mipmap = false);
  void Clear() { GLState::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

private:
  GLrenderbuff m_depth;

  CDISABLE_COPY(Framebuffer)
};

struct Slab {
  Surface &src() const {
    CASSERT(m_src, "Slab uninitialized");
    return *m_src.get();
  }
  Surface &tgt() const {
    CASSERT(m_tgt, "Slab uninitialized");
    return *m_tgt.get();
  }

  Slab() = default;
  Slab(uint width, uint height, uint channels, GLenum PRECISION = GL_BYTE) {
    Create(width, height, channels, PRECISION);
  }

  void Create(uint width, uint height, uint channels,
              GLenum PRECISION = GL_BYTE);

  void Swap() { m_src.swap(m_tgt); }
  void Use(const Sampler &sampler, uint n) const {
    tgt().Bind();
    src().Use(sampler, n);
  }

private:
  unique_ptr<Surface> m_src, m_tgt;

  CDISABLE_COPY(Slab)
};

} // namespace gl
