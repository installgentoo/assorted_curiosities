#include "guielements.h"

#include <sstream>

#include "base_classes/oglmanagement.h"

using namespace glgui;

GUIComponentManager &GUIComponentManager::Get() {
  static GUIComponentManager s_manager;
  return s_manager;
}

GUIComponentObject *GUIComponentManager::GetText(const char *text,
                                                 bool interactive) {
  const GUIFont guifont =
      ApplicationManager::Get().gui_resource_manager().default_font();
  if (!interactive)
    for (auto i : m_text_objects)
      if (i->m_instances > 0 && 0 == i->m_text.compare(text) &&
          i->m_font == guifont)
        return i;

  GUIText *obj = new GUIText(text, guifont);
  m_text_objects.push_back(obj);
  if (interactive)
    obj->m_instances = -2;

  return obj;
}

GUIComponentObject *GUIComponentManager::GetText(const char *text,
                                                 const char *font, uint size,
                                                 bool interactive) {
  const GUIFont guifont(font, size);
  if (!interactive)
    for (auto i : m_text_objects)
      if (i->m_instances > 0 && 0 == i->m_text.compare(text) &&
          i->m_font == guifont)
        return i;

  GUIText *obj = new GUIText(text, guifont);
  m_text_objects.push_back(obj);
  if (interactive)
    obj->m_instances = -2;

  return obj;
}

GUIComponentObject *GUIComponentManager::GetBackground(vec2 size,
                                                       bool interactive) {
  if (!interactive)
    for (auto i : m_bkgr_objects)
      if (i->m_instances > 0 && size == i->m_size)
        return i;

  GUIBackground *obj = new GUIBackground(size);
  m_bkgr_objects.push_back(obj);
  if (interactive)
    obj->m_instances = -2;

  return obj;
}

GUIComponentObject *GUIComponentManager::GetCustomBackground(vec2 size,
                                                             vec2 corner,
                                                             uint theme,
                                                             bool interactive) {
  if (!interactive)
    for (auto i : m_rnd_bkgr_objects)
      if (i->m_instances > 0 && size == i->m_size && corner == i->m_corner)
        return i;

  GUICustomBackground *obj = new GUICustomBackground(size, corner, theme);
  m_rnd_bkgr_objects.push_back(obj);
  if (interactive)
    obj->m_instances = -2;

  return obj;
}

GUIComponentObject *
GUIComponentManager::GetPicture(shared_ptr<Texture> &texture, const vec2 &size,
                                bool interactive) {
  if (!interactive)
    for (auto i : m_pic_objects)
      if (i->m_instances > 0 && texture->tex() == i->m_picture->tex() &&
          size == i->m_size)
        return i;

  GUIPicture *obj = new GUIPicture(texture, size);
  m_pic_objects.push_back(obj);
  if (interactive)
    obj->m_instances = -2;

  return obj;
}

GUIComponentObject *
GUIComponentManager::GetSurface(shared_ptr<Surface> &surface, const vec2 &size,
                                bool interactive) {
  if (!interactive)
    for (auto i : m_surf_objects)
      if (i->m_instances > 0 && surface->tex() == i->m_surface->tex() &&
          size == i->m_size)
        return i;

  GUIGLSurface *obj = new GUIGLSurface(surface, size);
  m_surf_objects.push_back(obj);
  if (interactive)
    obj->m_instances = -2;

  return obj;
}

void GUIComponentManager::DeleteComponent(GUIComponentObject *obj) {
  for (auto i = m_text_objects.begin(), end = m_text_objects.end(); i != end;
       ++i)
    if (*i == obj) {
      delete *i;
      m_text_objects.erase(i);
      return;
    }
  for (auto i = m_bkgr_objects.begin(), end = m_bkgr_objects.end(); i != end;
       ++i)
    if (*i == obj) {
      delete *i;
      m_bkgr_objects.erase(i);
      return;
    }
  for (auto i = m_rnd_bkgr_objects.begin(), end = m_rnd_bkgr_objects.end();
       i != end; ++i)
    if (*i == obj) {
      delete *i;
      m_rnd_bkgr_objects.erase(i);
      return;
    }
  for (auto i = m_pic_objects.begin(), end = m_pic_objects.end(); i != end; ++i)
    if (*i == obj) {
      delete *i;
      m_pic_objects.erase(i);
      return;
    }
}

void GUIComponent::setShader(const Shader &shader) { m_obj->m_shader = shader; }

GUIComponent::~GUIComponent() {
  if (m_obj->m_instances > 1)
    --m_obj->m_instances;
  else
    GUIComponentManager::Get().DeleteComponent(m_obj);
}

void GUIComponent::Write(const char *text) {
  CloneIfStatic();

  GUIText *ptr = dynamic_cast<GUIText *>(m_obj);
  CASSERT(ptr, "Not a GUIText");
  ptr->Write(text);
}

void GUIComponent::Write(const char *text, const GUIFont &font) {
  CloneIfStatic();

  GUIText *ptr = dynamic_cast<GUIText *>(m_obj);
  CASSERT(ptr, "Not a GUIText");
  ptr->setFont(font);
  ptr->Write(text);
}

void GUIComponent::SetSmoothBackground(vec2 size, vec2 corner, uint theme) {
  CloneIfStatic();

  GUICustomBackground *ptr = dynamic_cast<GUICustomBackground *>(m_obj);
  CASSERT(ptr, "Not a GUICustomBackground");
  ptr->SetSmoothBackground(size, corner, theme);
}

void GUIComponent::SetBackground(vec2 size) {
  CloneIfStatic();

  GUIBackground *ptr = dynamic_cast<GUIBackground *>(m_obj);
  CASSERT(ptr, "Not a GUIBackground");
  ptr->SetBackground(size);
}

void GUIComponent::SetPicture(shared_ptr<Texture> &texture, vec2 size) {
  CloneIfStatic();

  GUIPicture *ptr = dynamic_cast<GUIPicture *>(m_obj);
  CASSERT(ptr, "Not a GUIPicture");
  ptr->SetPicture(texture, size);
}

void GUIComponent::CloneIfStatic() {
  if (m_obj->m_instances > 1) {
    --m_obj->m_instances;
    m_obj = m_obj->Clone();
  }
}

GUIComponent::GUIComponent(GUIComponentObject *obj, const vec4 &color)
    : m_obj(obj), m_color(color) {
  ++obj->m_instances;
}

GUIElementObject::GUIElementObject(const GUIElementObject &other)
    : GUIPositioningBase<GUIElementObject, GUILayout>(other),
      m_size(other.m_size), m_components(other.m_components) {
  if (other.m_logic)
    AddLogic(other.m_logic->Clone());
  for (auto &i : m_components)
    i.parent = this;
}

void GUIElementObject::AddText(const char *text, const vec4 &color,
                               bool interactive) {
  PushComponent(GUIComponent(
      GUIComponentManager::Get().GetText(text, interactive), color));
}

void GUIElementObject::AddText(const char *text, const char *font, uint size,
                               const vec4 &color, bool interactive) {
  PushComponent(GUIComponent(
      GUIComponentManager::Get().GetText(text, font, size, interactive),
      color));
}

void GUIElementObject::AddBackground(const vec2 &size, const vec4 &color,
                                     bool interactive) {
  PushComponent(GUIComponent(
      GUIComponentManager::Get().GetBackground(size, interactive), color));
}

void GUIElementObject::AddCustomBackground(const vec2 &size, const vec2 &corner,
                                           uint theme, const vec4 &color,
                                           bool interactive) {
  PushComponent(GUIComponent(GUIComponentManager::Get().GetCustomBackground(
                                 size, corner, theme, interactive),
                             color));
}

void GUIElementObject::AddPicture(shared_ptr<Texture> &texture,
                                  const vec2 &size, const vec4 &color,
                                  bool interactive) {
  PushComponent(GUIComponent(
      GUIComponentManager::Get().GetPicture(texture, size, interactive),
      color));
}

void GUIElementObject::AddSurface(shared_ptr<Surface> &surface,
                                  const vec2 &size, const vec4 &color,
                                  bool interactive) {
  PushComponent(GUIComponent(
      GUIComponentManager::Get().GetSurface(surface, size, interactive),
      color));
}

void GUIElementObject::AddLogic(GUIElementLogic *logic) {
  if (m_logic)
    CINFO("Overwriting current element logic\n");
  m_logic.reset(logic);
  m_logic->SetParent(this);
}

GUIComponent &GUIElementObject::Component(int num) {
  CASSERT(m_components.size() && num < int(m_components.size()),
          "No such component in element");
  if (num < 0)
    return m_components.back();

  return m_components[cast<size_t>(num)];
}

void GUIElementObject::PushComponent(GUIComponent &&component) {
  m_components.push_back(component);
  m_components.back().parent = this;
  if (!ChildResized(m_components.back().m_offset + m_components.back().size()))
    m_components.back().CorrectPos();
}

GUIElementManger &GUIElementManger::Get() {
  static GUIElementManger s_manager;
  return s_manager;
}

GUIElementObject *GUIElementManger::GetElement(const char *name) {
  CASSERT(m_elements.find(name) != m_elements.end(), "Element doesn't exist");
  return m_elements[name];
}

GUIElementObject &GUIElementManger::CloneElement(const char *name) {
  CASSERT(m_templates.end() != m_templates.find(name), "Element doesn't exist");
  return *m_templates[name];
}

void GUIElementManger::RegisterElement(const char *name,
                                       GUIElementObject *obj) {
  CASSERT(m_elements.find(name) == m_elements.end(),
          "Elements must have unique names");
  m_elements[name] = obj;
}

void GUIElementManger::SaveTemplate(const char *name, GUIElementObject *obj) {
  CASSERT(m_elements.end() == m_elements.find(name),
          "Templates expect unique names");
  m_templates[name] = new GUIElementObject(*obj);
}

void GUIElementManger::CleanTemplates() {
  for (auto i : m_templates)
    delete i.second;
  m_templates.clear();
}

GUIElement::GUIElement(const char *name, const char *parent, const vec2 &pos) {
  m_obj = new GUIElementObject(GUIElementManger::Get().CloneElement(parent));
  GUIElementManger::Get().RegisterElement(name, m_obj);

  m_obj->setPos(pos);
}

GUIElement::GUIElement(const char *name, const vec2 &pos, uint align) {
  m_obj = new GUIElementObject;
  GUIElementManger::Get().RegisterElement(name, m_obj);

  m_obj->setPos(pos);
  m_obj->setAlign(align);
}

void GUILayout::AddElement(GUIElement &element, GUIElementLogic *logic) {
  CASSERT(element.m_obj, "Element already added somewhere else");
  GUIElementObject *obj = element.m_obj;
  element.m_obj = nullptr;

  if (logic)
    obj->AddLogic(logic);
  obj->parent = this;
  m_elements.push_back(obj);
  if (!ChildResized(obj->m_offset + obj->m_size))
    obj->CorrectPos();
}

void GUILayout::AddLayout(GUILayout *layout) {
  m_elements.push_back(layout);
  layout->parent = this;
  if (!ChildResized(layout->m_offset + layout->m_size))
    layout->CorrectPos();
}

void GUILayout::ReceiveMouseMove(vec2 pos) {
  for (int i = m_elements.size() - 1; i >= 0; --i) {
    GUIInplaceBase *element = m_elements[cast<size_t>(i)];
    vec2 max = element->size(), min = pos - element->pos();
    if (element->m_visible && min.y > 0 && min.x > 0 && min.x < max.x &&
        min.y < max.y) {
      if (i != m_focused_child) {
        if (m_focused_child != -1)
          m_elements[cast<size_t>(m_focused_child)]->DefocusChild();
        m_focused_child = i;
      }
      element->ReceiveMouseMove(pos);
      return;
    }
  }

  if (m_focused_child != -1) {
    m_elements[cast<size_t>(m_focused_child)]->DefocusChild();
    m_focused_child = -1;
  }
}

void GUILayout::DefocusChild() {
  if (m_focused_child != -1) {
    m_elements[cast<size_t>(m_focused_child)]->DefocusChild();
    m_focused_child = -1;
  }
}

void GUILayout::ReceiveMouseButton(int button, int mods, int action) {
  if (m_focused_child != -1)
    m_elements[cast<size_t>(m_focused_child)]->ReceiveMouseButton(button, mods,
                                                                  action);
}
