#pragma once

#include <glm/vec4.hpp>

#include "guicomponents.h"
#include "guilogic.h"

namespace glgui {

struct GUIElementLogic;
struct GUIElementObject;
struct GUILayout;

template <class Derived, class T> struct GUIPositioningBase {
  void setPos(const vec2 &pos) {
    m_offset = pos;
    CorrectPos(true);
  }
  void movePos(const vec2 &pos) {
    m_offset += pos;
    CorrectPos(true);
  }
  void setAlign(uint align) {
    m_align = align;
    CorrectPos(true);
  }

  bool ChildResized(const vec2 &size) {
    bool resized = false;
    vec2 &this_size = static_cast<Derived *>(this)->size();
    if (size.x > this_size.x) {
      this_size.x = size.x;
      resized = true;
    }
    if (size.y > this_size.y) {
      this_size.y = size.y;
      resized = true;
    }

    if (resized) {
      static_cast<Derived *>(this)->Resized();
      return true;
    }
    return false;
  }

  void CorrectPos(bool selfinitiated = false) {
    if (parent) {
      m_pos = m_offset + parent->m_pos;

      uint al = m_align >> 2;
      al <<= 2;
      const vec2 &this_size = static_cast<Derived *>(this)->size(),
                 &size = parent->size();
      switch (al) {
      case GUI::CENTER:
        m_pos.y += (size.y - this_size.y) / 2;
        break;
      case GUI::TOP:
        m_pos.y += size.y - this_size.y;
        break;
      }

      switch (m_align - al) {
      case GUI::MIDDLE:
        m_pos.x += (size.x - this_size.x) / 2;
        break;
      case GUI::RIGHT:
        m_pos.x += size.x - this_size.x;
        break;
      }

      if (m_pos.x < parent->m_pos.x)
        m_pos.x = parent->m_pos.x;
      if (m_pos.y < parent->m_pos.y)
        m_pos.y = parent->m_pos.y;

      if (!(selfinitiated && parent->ChildResized(m_offset + this_size)))
        static_cast<Derived *>(this)->Resized();
    } else
      m_pos = m_offset;
  }

  vec2 m_pos = vec2(0);

protected:
  T *parent = nullptr;
  uint m_align = GUI::LEFT | GUI::BOTTOM;
  vec2 m_offset = vec2(0);

  GUIPositioningBase() = default;
  GUIPositioningBase(const GUIPositioningBase &other)
      : m_pos(other.m_pos), parent(other.parent), m_align(other.m_align),
        m_offset(other.m_offset) {}

private:
  GUIPositioningBase &operator=(const GUIPositioningBase &) = delete;
};

struct GUIComponentManager {
  static GUIComponentManager &Get();

  GUIComponentObject *GetText(const char *text, bool interactive);
  GUIComponentObject *GetText(const char *text, const char *font, uint size,
                              bool interactive);
  GUIComponentObject *GetBackground(vec2 size, bool interactive);
  GUIComponentObject *GetCustomBackground(vec2 size, vec2 corner, uint theme,
                                          bool interactive);
  GUIComponentObject *GetPicture(shared_ptr<Texture> &texture, const vec2 &size,
                                 bool interactive);
  GUIComponentObject *GetSurface(shared_ptr<Surface> &surface, const vec2 &size,
                                 bool interactive);
  void DeleteComponent(GUIComponentObject *obj);

private:
  vector<GUIText *> m_text_objects;
  vector<GUIBackground *> m_bkgr_objects;
  vector<GUICustomBackground *> m_rnd_bkgr_objects;
  vector<GUIPicture *> m_pic_objects;
  vector<GUIGLSurface *> m_surf_objects;

  GUIComponentManager() = default;
  ~GUIComponentManager() = default;
  GUIComponentManager(const GUIComponentManager &) = delete;
  GUIResourceManager &operator=(const GUIComponentManager &) = delete;
};

struct GUIComponent : GUIPositioningBase<GUIComponent, GUIElementObject> {
  friend struct GUIElementObject;

  void setColor(const vec4 &color) { m_color = color; }
  void setVisible(bool visible) { m_visible = visible; }
  void setShader(const Shader &shader);
  const vec2 &size() const { return m_obj->m_size; }
  GUIComponentObject *obj() const { return m_obj; }
  const string &text() const {
    GUIText *ptr = dynamic_cast<GUIText *>(m_obj);
    CASSERT(ptr, "Not a GUIText");
    return ptr->m_text;
  }

  ~GUIComponent();
  GUIComponent(const GUIComponent &other)
      : GUIPositioningBase<GUIComponent, GUIElementObject>(other),
        m_obj(other.m_obj), m_visible(other.m_visible), m_color(other.m_color) {
    if (m_obj->m_instances > 0)
      ++m_obj->m_instances;
    else
      m_obj = other.m_obj->Clone();
  }

  void Write(const char *text);
  void Write(const char *text, const GUIFont &font);
  void SetSmoothBackground(vec2 size, vec2 corner = vec2(0.02), uint theme = 0);
  void SetBackground(vec2 size);
  void SetPicture(shared_ptr<Texture> &texture, vec2 size = vec2(1));

  void Draw() const {
    if (m_visible)
      m_obj->Draw(m_pos, m_color);
  }
  void Resized() const {}

private:
  GUIComponentObject *m_obj;
  bool m_visible = true;
  vec4 m_color;

  void CloneIfStatic();

  GUIComponent(GUIComponentObject *obj, const vec4 &color);
  GUIComponent &operator=(const GUIComponent &) = delete;
};

struct GUIInplaceBase {
  friend struct GUILayout;

  void setVisible(bool visible = true) { m_visible = visible; }
  bool visible() { return m_visible; }

protected:
  bool m_visible = true;

  virtual const vec2 &pos() const = 0;
  virtual vec2 &size() = 0;
  virtual void Draw() const = 0;
  virtual void DefocusChild() = 0;
  virtual void CorrectBasePos() = 0;
  virtual void ReceiveMouseButton(int button, int mods, int action) = 0;
  virtual void ReceiveKey(int key, int action, int mods) = 0;
  virtual void ReceiveKey(uint character) = 0;
  virtual void ReceiveMouseMove(vec2 pos) = 0;
  virtual void ReceiveMouseWheel(double x, double y) = 0;

  virtual ~GUIInplaceBase() = default;
};

struct GUIElementObject : GUIPositioningBase<GUIElementObject, GUILayout>,
                          GUIInplaceBase {
  friend struct GUILayout;
  friend struct GUIElement;
  friend struct GUIElementObjectManager;

  vec2 &size() { return m_size; }

  GUIElementObject() = default;
  ~GUIElementObject() = default;
  GUIElementObject(const GUIElementObject &other);

  void AddText(const char *text, const vec4 &color = vec4(1),
               bool interactive = false);
  void AddText(const char *text, const char *font, uint size,
               const vec4 &color = vec4(1), bool interactive = false);
  void AddBackground(const vec2 &size = vec2(1), const vec4 &color = vec4(1),
                     bool interactive = false);
  void AddCustomBackground(const vec2 &size = vec2(1),
                           const vec2 &corner = vec2(0.02), uint theme = 0,
                           const vec4 &color = vec4(1),
                           bool interactive = false);
  void AddPicture(shared_ptr<Texture> &texture, const vec2 &size = vec2(1),
                  const vec4 &color = vec4(1), bool interactive = false);
  void AddSurface(shared_ptr<Surface> &surface, const vec2 &size = vec2(1),
                  const vec4 &color = vec4(1), bool interactive = false);
  void AddLogic(GUIElementLogic *logic);
  GUIComponent &Component(int num = -1);

  void Resized() {
    for (auto &i : m_components)
      i.CorrectPos();
  }

private:
  vec2 m_size;
  vector<GUIComponent> m_components;
  unique_ptr<GUIElementLogic> m_logic;

  void PushComponent(GUIComponent &&component);

  const vec2 &pos() const { return m_pos; }
  void ReceiveMouseButton(int button, int mods, int action) {
    if (m_logic)
      m_logic->ReceiveMouseButton(button, mods, action);
  }
  void ReceiveKey(int key, int action, int mods) {
    if (m_logic)
      m_logic->ReceiveKey(key, action, mods);
  }
  void ReceiveKey(uint character) {
    if (m_logic)
      m_logic->ReceiveKey(character);
  }
  void ReceiveMouseMove(vec2 pos) {
    if (m_logic)
      m_logic->ReceiveMouseMove(pos);
  }
  void ReceiveMouseWheel(double x, double y) {
    if (m_logic)
      m_logic->ReceiveMouseWheel(x, y);
  }
  void DefocusChild() {
    if (m_logic)
      m_logic->DefocusChild();
  }
  void Draw() const {
    for (auto &i : m_components)
      i.Draw();
  }
  void CorrectBasePos() { CorrectPos(); }
};

struct GUIElementManger {
  static GUIElementManger &Get();

  GUIElementObject *GetElement(const char *name);
  GUIElementObject &CloneElement(const char *name);
  void RegisterElement(const char *name, GUIElementObject *obj);
  void SaveTemplate(const char *name, GUIElementObject *obj);
  void CleanTemplates();

private:
  unordered_map<string, GUIElementObject *> m_elements, m_templates;

  GUIElementManger() = default;
  ~GUIElementManger() = default;
  GUIElementManger(const GUIElementManger &) = delete;
  GUIElementManger &operator=(const GUIElementManger &) = delete;
};

struct GUIElement {
  friend struct GUILayout;

  GUIElementObject *operator->() {
    CASSERT(m_obj, "GUIElement uninitialized");
    return m_obj;
  }

  GUIElement(const char *name, const char *parent, const vec2 &pos = vec2(0));
  GUIElement(const char *name, const vec2 &pos = vec2(0),
             uint align = GUI::BOTTOM | GUI::LEFT);
  ~GUIElement() {
    if (m_obj)
      delete m_obj;
  }

  void SaveAsTemplate(const char *name) {
    GUIElementManger::Get().SaveTemplate(name, m_obj);
  }

private:
  GUIElementObject *m_obj = nullptr;
};

struct GUILayout : GUIPositioningBase<GUILayout, GUILayout>,
                   public GUIInplaceBase {
  friend struct GUIBaseWindow;

  vec2 &size() { return m_size; }

  GUILayout(const vec2 &pos) { setPos(pos); }

  void AddElement(GUIElement &element, GUIElementLogic *logic = nullptr);
  void AddLayout(GUILayout *layout);

  void Resized() {
    for (auto i : m_elements)
      i->CorrectBasePos();
  }

private:
  vector<GUIInplaceBase *> m_elements;
  vec2 m_size = vec2(0);
  int m_focused_child = -1;

  void ReceiveMouseMove(vec2 pos);
  void DefocusChild();
  void ReceiveMouseButton(int button, int mods, int action);

  const vec2 &pos() const { return m_pos; }
  void CorrectBasePos() { CorrectPos(); }
  void Draw() const {
    for (auto i : m_elements)
      if (i->m_visible)
        i->Draw();
  }
  void ReceiveKey(int key, int action, int mods) {
    if (m_focused_child != -1)
      m_elements[cast<size_t>(m_focused_child)]->ReceiveKey(key, action, mods);
  }
  void ReceiveKey(uint character) {
    if (m_focused_child != -1)
      m_elements[cast<size_t>(m_focused_child)]->ReceiveKey(character);
  }
  void ReceiveMouseWheel(double x, double y) {
    if (m_focused_child != -1)
      m_elements[cast<size_t>(m_focused_child)]->ReceiveMouseWheel(x, y);
  }

  ~GUILayout() {
    for (auto i : m_elements)
      delete i;
  }

  GUILayout(const GUILayout &) = delete;
  GUILayout &operator=(const GUILayout &) = delete;
};

} // namespace glgui
