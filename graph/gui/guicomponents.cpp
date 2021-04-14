#include "guicomponents.h"

#include <ft2build.h>
#include <math.h>

#include "base_classes/compute_quad.h"
#include "base_classes/oglmanagement.h"
#include "base_classes/shader.h"
#include "guielements.h"
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H

using namespace glgui;

SHADER(gui___text_vertex_shader)
"layout(location = 0)in vec4 Position;",
"out vec2 glTexCoord;",
"uniform vec2 offset;",
"",
"void main()",
"{",
"    gl_Position = vec4(Position.zw + offset, 0., 1.);",
"    glTexCoord = Position.xy;",
"}");

SHADER(gui___background_vertex_shader)
"layout(location = 0)in vec2 Position;",
"uniform vec2 offset;",
"",
"void main()",
"{",
"    gl_Position = vec4(Position.xy + offset, 0., 1.);",
"}");

SHADER(gui___text_pixel_shader)
"in vec2 glTexCoord;",
"layout(location = 0)out vec4 glFragColor;",
"uniform sampler2D src;",
"uniform vec4 color;",
"",
"void main()",
"{",
"    float c = texture(src, glTexCoord.xy).r;",
"    glFragColor = color * vec4(1., 1., 1., c);",
"}");

SHADER(gui___picture_pixel_shader)
"in vec2 glTexCoord;",
"layout(location = 0)out vec4 glFragColor;",
"uniform sampler2D src;",
"uniform vec4 color;",
"",
"void main()",
"{",
"    vec4 c = texture(src, glTexCoord.xy);",
"    glFragColor = c * color;",
"}");

SHADER(gui___background_pixel_shader)
"layout(location = 0)out vec4 glFragColor;",
"uniform vec4 color;",
"",
"void main()",
"{",
"    glFragColor = color;",
"}");

SHADER(gui___smooth_background_pixel_shader)
"in vec2 glTexCoord;",
"layout(location = 0)out vec4 glFragColor;",
"uniform sampler2D src, theme;",
"uniform vec4 color;",
"uniform float t;",
"",
"void main()",
"{",
"    vec2 coord = vec2(texture(src, glTexCoord.xy).r, t);",
"    vec4 c = texture(theme, coord);",
"    glFragColor = c * color;",
"}");

GUIFont::GUIFontObject::GUIFontObject(const char *name, uint size)
    : m_name(name), m_size(size) {
  FT_Library freetype;
  if (FT_Init_FreeType(&freetype) != 0) {
    CERROR("Error initializing freetype\n");
  }
  FT_Library_SetLcdFilter(freetype, FT_LCD_FILTER_LIGHT);
  FT_Face face;
  if (FT_New_Face(freetype, name, 0, &face) != 0) {
    CERROR("Error opening font file '" << name << "'\n");
  }
  if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0) {
    CERROR("Error setting font encoding from file '" << name << "'\n");
  };

  FT_Set_Pixel_Sizes(face, 0, size);

  const FT_GlyphSlot g = face->glyph;
  for (int i = 32; i < 128; ++i)
    if (0 == FT_Load_Char(face, cast<FT_ULong>(i),
                          FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT)) {
      if (g->bitmap.width && g->bitmap.rows) {
        m_width += g->bitmap.width;
        m_height = std::max(m_height, uint(g->bitmap.rows));
      }
    } else
      CINFO("No character '" << char(i) << "' in font file\n");

  uint w = 0;
  GLCHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

  m_font_tex.Use(*ApplicationManager::Get().samnear, 0);
  m_font_tex.Create(m_width, m_height, 1, GL_BYTE);

  for (int i = 32; i < 128; ++i)
    if (0 == FT_Load_Char(face, cast<FT_ULong>(i),
                          FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT)) {
      const int j = i - 32;
      m_font_map[j].advance = uint(round(double(g->advance.x) / 64));
      m_font_map[j].w = g->bitmap.width;
      m_font_map[j].h = g->bitmap.rows;
      m_font_map[j].l = g->bitmap_left;
      m_font_map[j].t = g->bitmap_top;
      m_font_map[j].start = w;
      m_font_map[j].newline = size;

      if (g->bitmap.width && g->bitmap.rows) {
        m_font_tex.Update(g->bitmap.buffer, GL_RED, GL_UNSIGNED_BYTE, w, 0,
                          g->bitmap.width, g->bitmap.rows);
        w += g->bitmap.width;
      }
    }

  GLCHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
  FT_Done_Face(face);
  FT_Done_FreeType(freetype);
}

GUIFont::GUIFont(const char *name, uint size) {
  m_obj = ApplicationManager::Get().gui_resource_manager().GetFont(name, size);
  CASSERT(m_obj, "GUIFont uninitialized");
  ++m_obj->m_instances;
}

GUIFont::~GUIFont() {
  if (m_obj->m_instances > 1)
    --m_obj->m_instances;
  else
    ApplicationManager::Get().gui_resource_manager().DeleteFont(m_obj);
}

GUIFont &GUIFont::operator=(const GUIFont &other) {
  if (other.m_obj != m_obj) {
    if (m_obj->m_instances > 1)
      --m_obj->m_instances;
    else
      ApplicationManager::Get().gui_resource_manager().DeleteFont(m_obj);

    m_obj = other.m_obj;
    ++m_obj->m_instances;
  }
  return *this;
}

GUIFont::GUIFontObject *GUIResourceManager::GetFont(const char *name,
                                                    uint size) {
  const string id_name = string(name) + std::to_string(size);
  const auto existing = m_fonts.find(id_name);
  if (existing != m_fonts.end())
    return existing->second;

  GUIFont::GUIFontObject *obj = new GUIFont::GUIFontObject(name, size);
  m_fonts[id_name] = obj;
  return obj;
}

void GUIResourceManager::DeleteFont(GUIFont::GUIFontObject *obj) {
  for (auto i = m_fonts.begin(), end = m_fonts.end(); i != end; ++i)
    if (i->second == obj) {
      delete obj;
      m_fonts.erase(i);
      return;
    }
}

GUIResourceManager::GUIResourceManager()
    : m_text_shader("gui___text_vertex_shader", "gui___text_pixel_shader"),
      m_img_shader("gui___text_vertex_shader", "gui___picture_pixel_shader"),
      m_bkg_shader("gui___background_vertex_shader",
                   "gui___background_pixel_shader"),
      m_rnd_bkg_shader("gui___text_vertex_shader",
                       "gui___smooth_background_pixel_shader"),
      m_rnd_bkgr_theme("custom_background_themes_atlas.png", 4) {
  m_text_shader.Uniformi("src", 0);
  m_img_shader.Uniformi("src", 0);
  m_rnd_bkg_shader.Uniformi("src", 0);
  m_rnd_bkg_shader.Uniformi("theme", 1);
}

void GUIBackground::SetBackground(vec2 size) {
  if (m_instances > 1)
    CINFO("Modifying shared object\n");
  m_size = size;

  vector<GLubyte> idx;
  idx.reserve(6);
  vector<GLfloat> coords;
  coords.reserve(8);

  MakeQuad(0.f, 0.f, size.x, size.y, coords, idx);

  m_vao.Bind();

  m_vbo_idx.AllocateBuffer(idx, GL_STATIC_DRAW);
  m_vbo_xyuv.AllocateBuffer(coords, GL_STATIC_DRAW);
  m_vbo_xyuv.AttribFormat(0, 2);
}

void GUIBackground::Draw(const vec2 &pos, const vec4 &color) const {
  m_vao.Bind();
  m_shader.Uniformf("color", color);
  m_shader.Uniformf("offset", pos);
  GLCHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0));
}

GUIComponentObject *GUIBackground::Clone() {
  return GUIComponentManager::Get().GetBackground(m_size, true);
}

GUIBackground::GUIBackground(vec2 size)
    : GUIComponentObject(
          ApplicationManager::Get().gui_resource_manager().m_bkg_shader) {
  SetBackground(size);
}

void GUICustomBackground::SetSmoothBackground(vec2 size, vec2 corner,
                                              uint theme) {
  if (m_instances > 1)
    CINFO("Modifying shared object\n");
  m_size = size;
  m_corner = corner;
  vec2 m = size - corner;
  auto &rec_manger = ApplicationManager::Get().gui_resource_manager();
  CASSERT(rec_manger.m_rnd_bkgr_theme.height() > theme,
          "No such background theme");
  m_theme = (double(theme) + 0.5) / rec_manger.m_rnd_bkgr_theme.height();
  float z = 0.5f / m_type->width();

  const GLubyte c_idx[54] = {
      0, 1, 4,  4,  1, 5,  5,  1, 2,  2,  5,  6,  6,  2,  3,  3,  6,  7,
      7, 6, 11, 11, 6, 10, 10, 6, 5,  5,  10, 9,  9,  5,  4,  4,  9,  8,
      8, 9, 12, 12, 9, 13, 13, 9, 10, 10, 13, 14, 14, 10, 11, 11, 14, 15};
  GLfloat c_coords[64] = {1, 1, 0,   0,        1, z, corner.x, 0,
                          1, z, m.x, 0,        1, 1, size.x,   0,
                          z, 1, 0,   corner.y, z, z, corner.x, corner.y,
                          z, z, m.x, corner.y, z, 1, size.x,   corner.y,
                          z, 1, 0,   m.y,      z, z, corner.x, m.y,
                          z, z, m.x, m.y,      z, 1, size.x,   m.y,
                          1, 1, 0,   size.y,   1, z, corner.x, size.y,
                          1, z, m.x, size.y,   1, 1, size.x,   size.y};

  vector<GLubyte> idx;
  idx.assign(c_idx, c_idx + 54);
  vector<GLfloat> coords;
  coords.assign(c_coords, c_coords + 64);

  m_vao.Bind();

  m_vbo_idx.AllocateBuffer(idx, GL_STATIC_DRAW);
  m_vbo_xyuv.AllocateBuffer(coords, GL_STATIC_DRAW);
  m_vbo_xyuv.AttribFormat(0, 4);
}

void GUICustomBackground::Draw(const vec2 &pos, const vec4 &color) const {
  m_vao.Bind();
  m_shader.Uniformf("color", color);
  m_shader.Uniformf("offset", pos);
  m_shader.Uniformf("t", m_theme);
  auto &rec_manger = ApplicationManager::Get().gui_resource_manager();
  m_type->Use(*ApplicationManager::Get().samlin, 0);
  rec_manger.m_rnd_bkgr_theme.Use(*ApplicationManager::Get().samlin, 1);
  GLCHECK(glDrawElements(GL_TRIANGLES, 54, GL_UNSIGNED_BYTE, 0));
}

GUIComponentObject *GUICustomBackground::Clone() {
  return GUIComponentManager::Get().GetCustomBackground(m_size, m_corner,
                                                        m_theme, true);
}

GUICustomBackground::GUICustomBackground(vec2 size, vec2 corner, uint theme)
    : GUIComponentObject(
          ApplicationManager::Get().gui_resource_manager().m_rnd_bkg_shader),
      m_type(ApplicationManager::Get()
                 .gui_resource_manager()
                 .default_custom_bkgr()) {
  SetSmoothBackground(size, corner, theme);
}

void GUIPicture::SetPicture(shared_ptr<Texture> &texture, vec2 size) {
  if (m_instances > 1)
    CINFO("Modifying shared object\n");
  m_picture = texture;

  vector<GLubyte> idx;
  idx.reserve(6);
  vector<GLfloat> coords;
  coords.reserve(16);

  MakeQuad(0.f, 0.f, 0.f, 0.f, 1.f, 1.f, size.x, size.y, coords, idx);

  m_vao.Bind();

  m_vbo_idx.AllocateBuffer(idx, GL_STATIC_DRAW);
  m_vbo_xyuv.AllocateBuffer(coords, GL_STATIC_DRAW);
  m_vbo_xyuv.AttribFormat(0, 4);
}

void GUIPicture::Draw(const vec2 &pos, const vec4 &color) const {
  m_vao.Bind();
  m_shader.Uniformf("color", color);
  m_shader.Uniformf("offset", pos);
  m_picture->Use(*ApplicationManager::Get().samlin, 0);
  GLCHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0));
}

GUIComponentObject *GUIPicture::Clone() {
  return GUIComponentManager::Get().GetPicture(m_picture, m_size, true);
}

GUIPicture::GUIPicture(shared_ptr<Texture> &texture, vec2 size)
    : GUIComponentObject(
          ApplicationManager::Get().gui_resource_manager().m_img_shader),
      m_picture(texture) {
  SetPicture(texture, size);
}

void GUIGLSurface::SetSurface(shared_ptr<Surface> &surface, vec2 size) {
  if (m_instances > 1)
    CINFO("Modifying shared object\n");
  m_surface = surface;

  vector<GLubyte> idx;
  idx.reserve(6);
  vector<GLfloat> coords;
  coords.reserve(16);

  MakeQuad(0.f, 0.f, 0.f, 0.f, 1.f, 1.f, size.x, size.y, coords, idx);

  m_vao.Bind();

  m_vbo_idx.AllocateBuffer(idx, GL_STATIC_DRAW);
  m_vbo_xyuv.AllocateBuffer(coords, GL_STATIC_DRAW);
  m_vbo_xyuv.AttribFormat(0, 4);
}

void GUIGLSurface::Draw(const vec2 &pos, const vec4 &color) const {
  m_vao.Bind();
  m_shader.Uniformf("color", color);
  m_shader.Uniformf("offset", pos);
  m_surface->Use(*ApplicationManager::Get().samnear, 0);
  GLCHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0));
}

GUIComponentObject *GUIGLSurface::Clone() {
  return GUIComponentManager::Get().GetSurface(m_surface, m_size, true);
}

GUIGLSurface::GUIGLSurface(shared_ptr<Surface> &surface, vec2 size)
    : GUIComponentObject(
          ApplicationManager::Get().gui_resource_manager().m_img_shader),
      m_surface(surface) {
  SetSurface(surface, size);
}

void GUIText::Write(const char *text) {
  CASSERT(m_instances < 2, "Modifying shared object");
  if (text)
    m_text = text;

  const uint text_length = m_text.size();

  vector<GLushort> idx;
  idx.reserve(16 * text_length);
  vector<GLfloat> coords;
  coords.reserve(24 * text_length);

  ushort counter = 0;

  auto &app_manager = ApplicationManager::Get();
  const uint width = m_font.width(), height = m_font.height(),
             window_width = app_manager.window_width(),
             window_height = app_manager.window_height();

  const float newline = (m_font.charinfo('a').newline * 2) / window_height;
  float x = 0., y = newline * std::count(m_text.begin(), m_text.end(), '\n');
  m_size = vec2(0., y + newline);
  if (text_length != 0)
    for (uint i = 0; i < text_length; ++i) {
      if ('\n' != m_text[i]) {
        const GUIFont::CharInfo &c = m_font.charinfo(uint(m_text[i]));
        if (c.h != 0) {
          const float u1 = c.start / width, u2 = u1 + double(c.w) / width,
                      v1 = double(c.h) / height, v2 = 0.f,

                      x1 = x + double(c.l * 2) / window_width,
                      x2 = x1 + double(c.w * 2) / window_width,
                      y2 = y + double(c.t * 2) / window_height,
                      y1 = y2 - double(c.h * 2) / window_height;

          MakeQuad(u1, v1, x1, y1, u2, v2, x2, y2, coords, idx, counter);
        }
        x += double(c.advance * 2) / window_width;
      } else {
        m_size.x = glm::max(m_size.x, x);
        x = 0;
        y -= newline;
      }
    }

  m_size.x = glm::max(m_size.x, x);

  m_num_verts = idx.size();

  m_vao.Bind();
  GLCHECK(glDisableVertexAttribArray(0));
  GLCHECK(glEnableVertexAttribArray(0));

  m_vbo_idx.AllocateBuffer(idx, GL_STATIC_DRAW);
  m_vbo_xyuv.AllocateBuffer(coords, GL_STATIC_DRAW);
  m_vbo_xyuv.AttribFormat(0, 4);
}

void GUIText::Draw(const vec2 &pos, const vec4 &color) const {
  m_vao.Bind();
  m_shader.Uniformf("color", color);
  m_shader.Uniformf("offset", pos);
  m_font.font_tex().Use(*ApplicationManager::Get().samnear, 0);
  GLCHECK(glDrawElements(GL_TRIANGLES, cast<GLsizei>(m_num_verts),
                         GL_UNSIGNED_SHORT, 0));
}

GUIComponentObject *GUIText::Clone() {
  return GUIComponentManager::Get().GetText(
      m_text.c_str(), m_font.name().c_str(), m_font.size(), true);
}

GUIText::GUIText(const char *text, const GUIFont &font)
    : GUIComponentObject(
          ApplicationManager::Get().gui_resource_manager().m_text_shader),
      m_font(font) {
  Write(text);
}
