#pragma once

#include <glm/vec2.hpp>

#include "base_classes/shader.h"
#include "base_classes/texture.h"

namespace glgui {

struct GUI {
  enum {
    LEFT = 1u,
    MIDDLE = 2u,
    RIGHT = 3u,
    TOP = 4u,
    CENTER = 8u,
    BOTTOM = 12u
  };
};

struct GUIFont {
  friend struct GUIResourceManager;
  friend struct GUIComponentManager;
  friend struct GUIText;

  struct CharInfo {
    uint advance, w, h;
    int l, t;
    float start, newline;
  };

private:
  struct GUIFontObject {
    GUIFontObject(const char *name, uint size);

    string m_name;
    Texture m_font_tex;
    CharInfo m_font_map[96];
    uint m_instances = 0, m_size, m_width = 1, m_height = 0;
  };

public:
  uint width() const { return m_obj->m_width; }
  uint height() const { return m_obj->m_height; }
  string &name() const { return m_obj->m_name; }
  uint size() const { return m_obj->m_size; }
  const CharInfo &charinfo(char p) const {
    CASSERT(uint(p) >= 32u, "Ascii chars start at 32");
    return m_obj->m_font_map[uint(p) - 32u];
  }
  const Texture &font_tex() const { return m_obj->m_font_tex; }

  GUIFont(const char *name, uint size);
  ~GUIFont();
  GUIFont &operator=(const GUIFont &other);
  GUIFont(const GUIFont &other) : m_obj(other.m_obj) { ++m_obj->m_instances; }
  bool operator==(const GUIFont &other) { return m_obj == other.m_obj; }

private:
  GUIFontObject *m_obj;
};

struct GUIResourceManager {
  friend struct GUIBaseWindow;
  friend struct GUIFont;

  void setDefaultFont(const char *name, uint size) {
    m_default_font.reset(new GUIFont(name, size));
  }
  void setDefaultCustomBkgr(const char *name) {
    m_default_custom_bkgr_tex.reset(new Texture(name, 1));
  }
  const GUIFont &default_font() {
    CASSERT(m_default_font, "No default font set");
    return *m_default_font;
  }
  const shared_ptr<Texture> default_custom_bkgr() {
    CASSERT(m_default_custom_bkgr_tex, "No default background set");
    return m_default_custom_bkgr_tex;
  }

  const Shader m_text_shader, m_img_shader, m_bkg_shader, m_rnd_bkg_shader;
  const Texture m_rnd_bkgr_theme;

private:
  unordered_map<string, GUIFont::GUIFontObject *> m_fonts;
  shared_ptr<Texture> m_default_custom_bkgr_tex;
  unique_ptr<GUIFont> m_default_font;

  GUIFont::GUIFontObject *GetFont(const char *name, uint size);
  void DeleteFont(GUIFont::GUIFontObject *obj);

  GUIResourceManager();
  GUIResourceManager(const GUIResourceManager &);
  GUIResourceManager &operator=(const GUIResourceManager &);
};

struct GUIComponentObject {
  friend struct GUIComponentManager;
  friend struct GUIComponent;
  friend struct GUIElement;

  const vec2 &size() const { return m_size; }

  virtual void Draw(const vec2 &pos, const vec4 &color) const = 0;

protected:
  Shader m_shader;
  vec2 m_size = vec2(0.);
  GLvao m_vao;
  GLbuffer<GL_ELEMENT_ARRAY_BUFFER> m_vbo_idx;
  GLbuffer<GL_ARRAY_BUFFER> m_vbo_xyuv;
  int m_instances = 0;

  virtual GUIComponentObject *Clone() = 0;

  GUIComponentObject(Shader shd) : m_shader(shd) {}
  virtual ~GUIComponentObject() = default;
  GUIComponentObject(const GUIComponentObject &other) = delete;
  GUIComponentObject &operator=(const GUIComponentObject &) = delete;
};

struct GUIBackground : GUIComponentObject {
  friend struct GUIComponentManager;

  void SetBackground(vec2 size);
  void Draw(const vec2 &pos, const vec4 &color) const;

private:
  GUIComponentObject *Clone();

  GUIBackground(vec2 size);
  GUIBackground(const GUIBackground &) = delete;
  GUIBackground &operator=(const GUIBackground &) = delete;
};

struct GUICustomBackground : GUIComponentObject {
  friend struct GUIComponentManager;

  void SetSmoothBackground(vec2 size, vec2 corner, uint theme);
  void Draw(const vec2 &pos, const vec4 &color) const;

private:
  vec2 m_corner;
  shared_ptr<Texture> m_type;
  float m_theme;

  GUIComponentObject *Clone();

  GUICustomBackground(vec2 size, vec2 corner, uint theme);
  GUICustomBackground(const GUICustomBackground &) = delete;
  GUICustomBackground &operator=(const GUICustomBackground &) = delete;
};

struct GUIPicture : GUIComponentObject {
  friend struct GUIComponentManager;

  void SetPicture(shared_ptr<Texture> &texture, vec2 size);
  void Draw(const vec2 &pos, const vec4 &color) const;

private:
  shared_ptr<Texture> m_picture;

  GUIComponentObject *Clone();

  GUIPicture(shared_ptr<Texture> &texture, vec2 size);
  GUIPicture(const GUIPicture &) = delete;
  GUIPicture &operator=(const GUIPicture &) = delete;
};

struct GUIGLSurface : GUIComponentObject {
  friend struct GUIComponentManager;

  void SetSurface(shared_ptr<Surface> &surface, vec2 size);
  void Draw(const vec2 &pos, const vec4 &color) const;

private:
  shared_ptr<Surface> m_surface;

  GUIComponentObject *Clone();

  GUIGLSurface(shared_ptr<Surface> &surface, vec2 size);
  GUIGLSurface(const GUIGLSurface &) = delete;
  GUIGLSurface &operator=(const GUIGLSurface &) = delete;
};

struct GUIText : GUIComponentObject {
  friend struct GUIComponentManager;

  void setFont(const GUIFont &font) { m_font = font; }
  const GUIFont &font() const { return m_font; }

  void Write(const char *text = nullptr);
  void Draw(const vec2 &pos, const vec4 &color) const;

  string m_text;

private:
  uint m_num_verts = 0;
  GUIFont m_font;

  GUIComponentObject *Clone();

  GUIText(const char *text, const GUIFont &font);
  GUIText(const GUIText &) = delete;
  GUIText &operator=(const GUIText &) = delete;
};

} // namespace glgui
