#include "texture.h"

#include "policies/resource_policy.h"

using namespace gl;

Sampler::~Sampler() {
  if (m_obj->m_instances > 1)
    --m_obj->m_instances;
  else
    SamplerManager::Get().Delete(m_obj);
}

Sampler::SamplerObject *Sampler::Create(const char *name) {
  return SamplerManager::Get().Create(name);
}

SamplerManager &SamplerManager::Get() {
  static SamplerManager s_manager;
  return s_manager;
}

const GLsampler &SamplerManager::GetSampler(const char *name) const {
  const auto existing = m_samplers.find(name);
  if (existing == m_samplers.end())
    CERROR("Sampler '" << name << "' doesn't exist");
  return *(existing->second.get());
}

Sampler::SamplerObject *SamplerManager::Create(string name) {
  const auto existing = m_samplers.find(name);

  if (existing != m_samplers.end())
    return existing->second.get();

  auto *obj = new Sampler::SamplerObject;
  m_samplers.emplace(move(name), obj);
  return obj;
}

void SamplerManager::Delete(Sampler::SamplerObject *sampler) {
  for (auto i = m_samplers.begin(), end = m_samplers.end(); i != end; ++i)
    if (i->second.get() == sampler) {
      m_samplers.erase(i);
      return;
    }
}

ulint Texture::Params::Size(uint level) const {
  CASSERT(m_width && m_height, "Texture has to contain something");
  ulint chans = 4;
  switch (m_channels) {
  case GL_RED:
    chans = 1;
    break;
  case GL_RG:
    chans = 2;
    break;
  case GL_RGB:
    chans = 3;
    break;
  case GL_RGBA:
    chans = 4;
    break;
    CASSERT(0, "Invalid channels");
  }
  if (!level)
    return chans * m_width * m_height;

  const uint width = glm::max(1., std::floor(double(m_width) / pow(2, level))),
             height =
                 glm::max(1., std::floor(double(m_height) / pow(2, level)));

  return chans * width * height;
}

void Texture::Create(uint width, uint height, uint channels, GLenum PRECISION,
                     const void *data, GLenum DATAFORMAT, bool mipmap) {
  GLint fmt = GL_RGBA8;
  GLenum chans = GL_RGBA;
  switch (channels) {
  case 0:
    m_params.reset();
    return;
  case 1:
    chans = GL_RED;
    switch (PRECISION) {
    case GL_BYTE:
      fmt = GL_R8;
      break;
    case GL_HALF_FLOAT:
      fmt = GL_R16F;
      break;
    case GL_FLOAT:
      fmt = GL_R32F;
      break;
      CASSERT(0, "No such texture can be created");
    }
    break;
  case 2:
    chans = GL_RG;
    switch (PRECISION) {
    case GL_BYTE:
      fmt = GL_RG8;
      break;
    case GL_HALF_FLOAT:
      fmt = GL_RG16F;
      break;
    case GL_FLOAT:
      fmt = GL_RG32F;
      break;
      CASSERT(0, "No such texture can be created");
    }
    break;
  case 3:
    chans = GL_RGB;
    switch (PRECISION) {
    case GL_BYTE:
      fmt = GL_RGB8;
      break;
    case GL_HALF_FLOAT:
      fmt = GL_RGB16F;
      break;
    case GL_FLOAT:
      fmt = GL_RGB32F;
      break;
      CASSERT(0, "No such texture can be created");
    }
    break;
  case 4:
    chans = GL_RGBA;
    switch (PRECISION) {
    case GL_BYTE:
      fmt = GL_RGBA8;
      break;
    case GL_HALF_FLOAT:
      fmt = GL_RGBA16F;
      break;
    case GL_FLOAT:
      fmt = GL_RGBA32F;
      break;
      CASSERT(0, "No such texture can be created");
    }
    break;
    CASSERT(0, "Invalid precision");
  }

  if (!Compatible(width, height, fmt) || data) {
    Activate();
    GLCHECK(glTexImage2D(GL_TEXTURE_2D, 0, fmt, cast<GLint>(width),
                         cast<GLint>(height), 0, chans, DATAFORMAT, data));
  }

  if (mipmap) {
    Activate();
    GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_NEAREST_MIPMAP_NEAREST));
    GLCHECK(glGenerateMipmap(GL_TEXTURE_2D));
  }

  m_params.reset(new Params(width, height, fmt, chans));
}

void Texture::Download(unique_ptr<uchar[]> &data, uint level) const {
  CASSERT(m_params, "Texture uninitialized");
  data.reset(new uchar[m_params->Size(level)]);
  Activate();
  GLCHECK(glGetTexImage(GL_TEXTURE_2D, cast<GLint>(level), m_params->m_channels,
                        GL_UNSIGNED_BYTE, data.get()));
}

void Texture::Download(unique_ptr<float[]> &data, uint level) const {
  CASSERT(m_params, "Texture uninitialized");
  data.reset(new float[m_params->Size(level)]);
  Activate();
  GLCHECK(glGetTexImage(GL_TEXTURE_2D, cast<GLint>(level), m_params->m_channels,
                        GL_FLOAT, data.get()));
}

void Texture::Download(GLbuffer<GL_PIXEL_PACK_BUFFER> &buff, GLenum PRECISION,
                       uint level) const {
  CASSERT(m_params, "Texture uninitialized");
  buff.Bind();
  Activate();
  GLCHECK(glGetTexImage(GL_TEXTURE_2D, cast<GLint>(level), m_params->m_channels,
                        PRECISION, 0));
}

void Texture::Update(void *data, GLenum CHANNELS, GLenum PRECISION, uint x,
                     uint y, uint width, uint height) const {
  CASSERT((x + width <= this->width()) && (y + height <= this->height()),
          "Update out of texture bounds");
  if (!width)
    width = this->width();
  if (!height)
    height = this->height();
  Activate();
  GLCHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, cast<GLint>(x), cast<GLint>(y),
                          cast<GLint>(width), cast<GLint>(height), CHANNELS,
                          PRECISION, data));
}

void Texture::LoadFromFile(const char *filename, uint channels,
                           GLenum PRECISION, bool mipmap) {
  vector<char> fbuffer = ResourceLoader::LoadResource(filename);
  uint width, height;
  const vector<uchar> img =
      ImageDecoder::DecodeImage(fbuffer, channels, width, height);

  GLbuffer<GL_PIXEL_UNPACK_BUFFER> upload_buff;
  upload_buff.AllocateBuffer(0, img.size() * sizeof(uchar), GL_STREAM_DRAW);
  memcpy(upload_buff.MapBuffer(GL_WRITE_ONLY), img.data(),
         img.size() * sizeof(uchar));
  upload_buff.UnmapBuffer();
  Create(width, height, channels, PRECISION, 0, GL_UNSIGNED_BYTE, mipmap);
}

void Texture::Use(const char *sampler, uint n) const {
  m_tex.Bind(cast<int>(n));
  SamplerManager::Get().GetSampler(sampler).Bind(n);
}

void Surface::Create(uint width, uint height, uint channels, GLenum PRECISION,
                     bool mipmap) {
  CASSERT((channels > 0) && (channels < 5),
          "FBO can only contain from 1 to 4 channels");
  Texture::Create(width, height, channels, PRECISION, 0, GL_UNSIGNED_BYTE,
                  mipmap);

  m_fbo.Bind();
  GLCHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                 GL_TEXTURE_2D, m_tex.tex, 0));
  Clear();
}

void Surface::Clear(GLclampf v) {
  m_fbo.Bind();
  GLState::ClearColor(v);
  GLState::Clear(GL_COLOR_BUFFER_BIT);
  GLState::ClearColor(0.f);
}

void Surface::Bind() const {
  m_fbo.Bind();
  GLState::Viewport(this->width(), this->height());
}

void Framebuffer::Create(uint width, uint height, uint channels,
                         GLenum PRECISION, bool mipmap) {
  Surface::Create(width, height, channels, PRECISION, mipmap);

  m_depth.Bind();
  GLCHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                                cast<GLint>(width), cast<GLint>(height)));

  GLCHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                    GL_RENDERBUFFER, m_depth.obj));
  Clear();
}

void Slab::Create(uint width, uint height, uint channels, GLenum PRECISION) {
  if (!m_src)
    m_src.reset(new Surface);
  if (!m_tgt)
    m_tgt.reset(new Surface);
  m_src->Create(width, height, channels, PRECISION);
  m_tgt->Create(width, height, channels, PRECISION);
}
