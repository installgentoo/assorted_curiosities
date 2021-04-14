#pragma once
#include <GL/gl3w.h>

#include "base_classes/policies/logging_policy.h"

namespace gl {

string glCodeToError(GLenum code);

struct TextureControl {
  struct Texunit {
    void Set(GLenum _type, uint _tex) {
      type = _type;
      tex = _tex;
    }
    uint type = 0, tex = 0;
  };

  static void Bind(GLenum type, uint tex, uint n) {
    if (m_texture_units.size() <= n) {
      int max_tex_units = 0;
      GLCHECK(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_tex_units));
      if (!(n < cast<uint>(max_tex_units)))
        CERROR("Not enough texture units avaliable, " << n << " units in use");

      m_texture_units.resize(n + 1);
    }

    if (n != m_bound_unit) {
      GLCHECK(glActiveTexture(GL_TEXTURE0 + n));
      m_bound_unit = n;
    }

    if ((m_texture_units[m_bound_unit].tex != tex) ||
        (m_texture_units[m_bound_unit].type != type)) {
      GLCHECK(glBindTexture(type, tex));
      m_texture_units[m_bound_unit].Set(type, tex);
    }
  }
  static void Delete(uint tex) {
    for (size_t i = 0; i < m_texture_units.size(); ++i)
      if (m_texture_units[i].tex == tex)
        m_texture_units[i].Set(0, 0);
  }

  static uint m_bound_unit;
  static vector<Texunit> m_texture_units;
};

struct SamplerControl {
  static void Bind(uint obj, uint n) {
    if (m_texture_units.size() <= n) {
      int max_tex_units = 0;
      GLCHECK(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_tex_units));
      if (!(n < cast<uint>(max_tex_units)))
        CERROR("Not enough texture units avaliable, " << n << " units in use");

      m_texture_units.resize(n + 1, 0);
    }

    if (m_texture_units[n] != obj) {
      GLCHECK(glBindSampler(n, obj));
      m_texture_units[n] = obj;
    }
  }
  static void Delete(uint obj) {
    for (size_t i = 0; i < m_texture_units.size(); ++i)
      if (m_texture_units[i] == obj)
        m_texture_units[i] = 0;
  }

  static vector<uint> m_texture_units;
};

struct ShaderProgramPolicy {
  static void bind(uint obj) { glUseProgram(obj); }
  static void gen(uint *obj) { *obj = glCreateProgram(); }
  static void del(uint *obj) { glDeleteProgram(*obj); }
};
struct FboPolicy {
  static void bind(uint obj) { glBindFramebuffer(GL_FRAMEBUFFER, obj); }
  static void gen(uint *obj) { glGenFramebuffers(1, obj); }
  static void del(uint *obj) { glDeleteFramebuffers(1, obj); }
};
struct RenderbufferPolicy {
  static void bind(uint obj) { glBindRenderbuffer(GL_RENDERBUFFER, obj); }
  static void gen(uint *obj) { glGenRenderbuffers(1, obj); }
  static void del(uint *obj) { glDeleteRenderbuffers(1, obj); }
};
struct VaoPolicy {
  static void bind(uint obj) { glBindVertexArray(obj); }
  static void gen(uint *obj) { glGenVertexArrays(1, obj); }
  static void del(uint *obj) { glDeleteVertexArrays(1, obj); }
};
template <GLenum m_type> struct BuffPolicy {
  static void bind(uint obj) { glBindBuffer(m_type, obj); }
  static void gen(uint *obj) { glGenBuffers(1, obj); }
  static void del(uint *obj) { glDeleteBuffers(1, obj); }
};
template <GLenum m_type> struct QueryPolicy {
  static void bind(uint) {}
  static void gen(uint *obj) { glGenQueries(1, obj); }
  static void del(uint *obj) { glDeleteQueries(1, obj); }
};

template <class m_policy> struct StateControl {
  static void Bind(uint obj) {
    if (m_bound_target != obj) {
      GLCHECK(m_policy::bind(obj));
      m_bound_target = obj;
    }
  }
  static void Create(uint *obj) {
    GLCHECK(m_policy::gen(obj));
    CASSERT(*obj, "OpenGL variable not initilized");
  }
  static void Delete(uint *obj) {
    if (*obj) {
      if (m_bound_target == *obj)
        m_bound_target = 0;
      GLCHECK(m_policy::del(obj));
    }
  }

  static uint m_bound_target;
};
template <class m_policy> uint StateControl<m_policy>::m_bound_target = 0;

struct GLState {
  static void BindOutputFramebuffer() {
    if (StateControl<FboPolicy>::m_bound_target != 0)
      StateControl<FboPolicy>::Bind(0);
  }
  static void ClearColor(float r, float g, float b, float a = 1.f) {
    GLCHECK(glClearColor(r, g, b, a));
  }
  static void ClearColor(float c, float a = 1.f) { ClearColor(c, c, c, a); }
  static void Clear(uint mask) { GLCHECK(glClear(mask)); }

  static void Viewport(uint w, uint h, uint x = 0, uint y = 0) {
    if ((s_fbo_w != w) || (s_fbo_h != h)) {
      s_fbo_w = w;
      s_fbo_h = h;
      GLCHECK(glViewport(cast<GLsizei>(x), cast<GLsizei>(y), cast<GLsizei>(w),
                         cast<GLsizei>(h)));
    }
  }

  template <class... Args> static void Enable(GLint arg1, Args... args) {
    auto found = m_states.find(arg1);
    if (found != m_states.end()) {
      if (found->second != true) {
        GLCHECK(glEnable(cast<GLenum>(arg1)));
        found->second = true;
      }
    } else {
      GLCHECK(glEnable(cast<GLenum>(arg1)));
      m_states.emplace(arg1, true);
    }

    Enable(args...);
  }
  template <class... Args> static void Disable(GLint arg1, Args... args) {
    auto found = m_states.find(arg1);
    if (found != m_states.end()) {
      if (found->second != false) {
        GLCHECK(glDisable(cast<GLenum>(arg1)));
        found->second = false;
      }
    } else {
      GLCHECK(glDisable(cast<GLenum>(arg1)));
      m_states.emplace(arg1, false);
    }

    Disable(args...);
  }

private:
  static uint s_fbo_w, s_fbo_h;
  static unordered_map<GLint, bool> m_states;

  static void Enable() {}
  static void Disable() {}
};

} // namespace gl
